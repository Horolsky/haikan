/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include "haikan/impl/lua_stream.hpp"

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <ostream>
#include <vector>

// IWYU pragma: no_include <__ostream/basic_ostream.h>

extern "C"
{
#include <lua.h>
}

namespace haikan {
namespace impl {

namespace {

struct LuaStackRestore
{
    lua_State* L;
    int top;

    explicit LuaStackRestore(lua_State* state) noexcept
        : L{state}, top{lua_gettop(state)}
    {
    }

    ~LuaStackRestore()
    {
        lua_settop(L, top);
    }
};

struct PathPop
{
    std::vector<void const*>& path;

    ~PathPop()
    {
        path.pop_back();
    }
};


void write_indent(std::ostream& os, int indent)
{
    static constexpr char spaces[] =
        "                                                                ";
    std::size_t count = static_cast<std::size_t>(indent) * 4;
    while (count != 0)
    {
        std::size_t const chunk = (std::min)(count, sizeof(spaces) - 1);
        os.write(spaces, static_cast<std::streamsize>(chunk));
        count -= chunk;
    }
}

void write_lua_string(std::ostream& os, char const* data, std::size_t size)
{
    static constexpr char hex[] = "0123456789ABCDEF";
    os.put('"');
    char const* segment = data;
    char const* const end = data + size;
    for (char const* current = segment; current != end; ++current)
    {
        unsigned char const c = static_cast<unsigned char>(*current);
        char escape = 0;
        switch (c)
        {
        case '\a': escape = 'a'; break;
        case '\b': escape = 'b'; break;
        case '\f': escape = 'f'; break;
        case '\n': escape = 'n'; break;
        case '\r': escape = 'r'; break;
        case '\t': escape = 't'; break;
        case '\v': escape = 'v'; break;
        case '"': escape = '"'; break;
        case '\\': escape = '\\'; break;
        default: break;
        }

        if (escape != 0 || c < 0x20 || c == 0x7f)
        {
            os.write(segment, current - segment);
            if (escape != 0)
            {
                os.put('\\');
                os.put(escape);
            }
            else
            {
                char const escaped[] = {
                    '\\', 'x', hex[c >> 4], hex[c & 0x0f],
                };
                os.write(escaped, sizeof(escaped));
            }
            segment = current + 1;
        }
    }
    os.write(segment, end - segment);
    os.put('"');
}

bool lua_value_to_stream(std::ostream& os, lua_State* L, int index, int indent,bool const pretty_print, std::vector<void const*>& path)
{
    LuaStackRestore const restore{L};
    index = lua_absindex(L, index);

    switch (lua_type(L, index))
    {
    case LUA_TNIL:
        os << "nil";
        return true;

    case LUA_TBOOLEAN:
        os << (lua_toboolean(L, index) != 0 ? "true" : "false");
        return true;

    case LUA_TNUMBER:
        {
            lua_pushvalue(L, index);
            std::size_t size = 0;
            char const* const number = lua_tolstring(L, -1, &size);
            os.write(number, static_cast<std::streamsize>(size));
            return true;
        }

    case LUA_TSTRING:
        {
            std::size_t size = 0;
            char const* const string = lua_tolstring(L, index, &size);
            write_lua_string(os, string, size);
            return true;
        }

    case LUA_TTABLE:
        break;

    default:
        return false;
    }

    void const* const identity = lua_topointer(L, index);
    if (std::find(path.begin(), path.end(), identity) != path.end())
    {
        return false;
    }
    path.push_back(identity);
    PathPop const pop_path{path};

    std::size_t const array_size = lua_rawlen(L, index);
    std::size_t item_count = 0;
    bool dense_array = true;

    lua_pushnil(L);
    while (lua_next(L, index) != 0)
    {
        if (lua_type(L, -2) == LUA_TNUMBER && lua_isinteger(L, -2))
        {
            lua_Integer const lua_index = lua_tointeger(L, -2);
            if (lua_index < 1 || static_cast<std::uint64_t>(lua_index) > array_size)
            {
                dense_array = false;
            }
        }
        else
        {
            dense_array = false;
        }
        ++item_count;
        lua_pop(L, 1);
    }

    if (item_count == 0)
    {
        os << "{}";
        return true;
    }
    dense_array = dense_array && item_count == array_size;

    os.put('{');
    std::size_t emitted = 0;
    auto const write_separator = [&]
    {
        if (emitted++ != 0)
        {
            os.put(',');
        }
        if (pretty_print)
        {
            os.put('\n');
            write_indent(os, indent + 1);
        }
    };

    if (dense_array)
    {
        for (std::size_t i = 1; i <= array_size; ++i)
        {
            write_separator();
            lua_rawgeti(L, index, static_cast<lua_Integer>(i));
            if (!lua_value_to_stream(os, L, -1, indent + 1, pretty_print, path))
            {
                return false;
            }
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pushnil(L);
        while (lua_next(L, index) != 0)
        {
            write_separator();
            os.put('[');
            if (!lua_value_to_stream(os, L, -2, indent + 1, false, path))
            {
                return false;
            }
            os << (pretty_print ? "] = " : "]=");
            if (!lua_value_to_stream(os, L, -1, indent + 1, pretty_print, path))
            {
                return false;
            }
            lua_pop(L, 1);
        }
    }

    if (pretty_print)
    {
        os.put('\n');
        write_indent(os, indent);
    }
    os.put('}');
    return true;
}

} // namespace


std::ostream& lua_to_stream(
    std::ostream& os, sol::object const value, int const indent, bool const pretty_print)
{
    try
    {
        lua_State* const L = value.lua_state();
        if (L == nullptr)
        {
            os.setstate(std::ios_base::failbit);
            return os;
        }

        LuaStackRestore const restore{L};
        value.push();
        int const initial_indent = pretty_print ? (std::max)(indent, 0) : 0;
        if (pretty_print)
        {
            write_indent(os, initial_indent);
        }
        std::vector<void const*> path;
        if (!lua_value_to_stream(os, L, -1, initial_indent, pretty_print, path))
        {
            os.setstate(std::ios_base::failbit);
        }
    }
    catch (...)
    {
        os.setstate(std::ios_base::failbit);
    }
    return os;
}



} // namespace impl
} // namespace haikan
