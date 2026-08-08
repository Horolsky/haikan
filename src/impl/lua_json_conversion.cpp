/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include "include/haikan/impl/lua_json_conversion.hpp"
#include "include/haikan/logger.hpp"

#include <algorithm>
#include <cstdint>
#include <ios>
#include <limits>
#include <vector>


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

boost::optional<boost::json::value> lua_to_json_impl(
    lua_State* L, int index, std::vector<void const*>& path)
{
    LuaStackRestore const restore{L};
    index = lua_absindex(L, index);

    switch (lua_type(L, index))
    {
    case LUA_TNIL:
        return boost::json::value{nullptr};

    case LUA_TBOOLEAN:
        return boost::json::value{lua_toboolean(L, index) != 0};

    case LUA_TNUMBER:
        if (lua_isinteger(L, index))
        {
            return boost::json::value{static_cast<std::int64_t>(lua_tointeger(L, index))};
        }
        return boost::json::value{static_cast<double>(lua_tonumber(L, index))};

    case LUA_TSTRING:
        {
            std::size_t size = 0;
            char const* const data = lua_tolstring(L, index, &size);
            return boost::json::value{boost::json::string_view{data, size}};
        }

    case LUA_TTABLE:
        {
            void const* const identity = lua_topointer(L, index);
            if (std::find(path.begin(), path.end(), identity) != path.end())
            {
                return boost::none;
            }
            path.push_back(identity);
            PathPop const pop_path{path};

            std::size_t const array_size = lua_rawlen(L, index);
            bool is_array = true;
            bool kind_known = false;
            std::size_t item_count = 0;
            bool dense_array = true;
            std::vector<std::pair<lua_Integer, boost::json::value>> array_items;
            boost::json::object object;

            lua_pushnil(L);
            while (lua_next(L, index) != 0)
            {
                int const key_type = lua_type(L, -2);
                bool const integer_key = key_type == LUA_TNUMBER && lua_isinteger(L, -2);
                bool const string_key = key_type == LUA_TSTRING;

                if (!kind_known)
                {
                    if (!integer_key && !string_key)
                    {
                        return boost::none;
                    }
                    is_array = integer_key;
                    kind_known = true;
                    if (is_array)
                    {
                        array_items.reserve(array_size);
                    }
                    else
                    {
                        object.reserve(array_size);
                    }
                }
                else if (integer_key != is_array)
                {
                    return boost::none;
                }

                auto converted = lua_to_json_impl(L, -1, path);
                if (!converted)
                {
                    return boost::none;
                }

                if (is_array)
                {
                    lua_Integer const lua_index = lua_tointeger(L, -2);
                    if (lua_index < 1 || static_cast<std::uint64_t>(lua_index) > array_size)
                    {
                        dense_array = false;
                    }
                    array_items.emplace_back(lua_index, std::move(*converted));
                }
                else
                {
                    std::size_t key_size = 0;
                    char const* const key = lua_tolstring(L, -2, &key_size);
                    object.emplace(boost::json::string_view{key, key_size}, std::move(*converted));
                }

                ++item_count;
                lua_pop(L, 1);
            }

            if (!kind_known)
            {
                return boost::json::value{boost::json::array{}};
            }
            if (!is_array)
            {
                return boost::json::value{std::move(object)};
            }
            if (dense_array && item_count == array_size)
            {
                boost::json::array array(array_size, nullptr);
                for (auto& item : array_items)
                {
                    array[static_cast<std::size_t>(item.first - 1)] = std::move(item.second);
                }
                return boost::json::value{std::move(array)};
            }

            boost::json::array pairs;
            pairs.reserve(item_count);
            for (auto& item : array_items)
            {
                boost::json::array pair;
                pair.reserve(2);
                pair.emplace_back(static_cast<std::int64_t>(item.first));
                pair.emplace_back(std::move(item.second));
                pairs.emplace_back(std::move(pair));
            }
            return boost::json::value{std::move(pairs)};
        }

    default:
        return boost::none;
    }
}

bool push_json(lua_State* L, boost::json::value const& value)
{
    switch (value.kind())
    {
    case boost::json::kind::null:
        lua_pushnil(L);
        return true;

    case boost::json::kind::bool_:
        lua_pushboolean(L, value.as_bool());
        return true;

    case boost::json::kind::int64:
        lua_pushinteger(L, static_cast<lua_Integer>(value.as_int64()));
        return true;

    case boost::json::kind::uint64:
        {
            std::uint64_t const number = value.as_uint64();
            if (number <= static_cast<std::uint64_t>(std::numeric_limits<lua_Integer>::max()))
            {
                lua_pushinteger(L, static_cast<lua_Integer>(number));
            }
            else
            {
                lua_pushnumber(L, static_cast<lua_Number>(number));
            }
            return true;
        }

    case boost::json::kind::double_:
        lua_pushnumber(L, static_cast<lua_Number>(value.as_double()));
        return true;

    case boost::json::kind::string:
        {
            boost::json::string const& string = value.as_string();
            lua_pushlstring(L, string.data(), string.size());
            return true;
        }

    case boost::json::kind::array:
        {
            boost::json::array const& array = value.as_array();
            if (array.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            {
                return false;
            }
            lua_createtable(L, static_cast<int>(array.size()), 0);
            int const table = lua_absindex(L, -1);
            lua_Integer index = 1;
            for (auto const& item : array)
            {
                if (!push_json(L, item))
                {
                    return false;
                }
                lua_rawseti(L, table, index++);
            }
            return true;
        }

    case boost::json::kind::object:
        {
            boost::json::object const& object = value.as_object();
            if (object.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            {
                return false;
            }
            lua_createtable(L, 0, static_cast<int>(object.size()));
            int const table = lua_absindex(L, -1);
            for (auto const& item : object)
            {
                lua_pushlstring(L, item.key().data(), item.key().size());
                if (!push_json(L, item.value()))
                {
                    return false;
                }
                lua_rawset(L, table);
            }
            return true;
        }
    }

    return false;
}

enum class StreamFormat
{
    lua,
    json,
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

void write_lua_string(std::ostream& os, boost::json::string_view string)
{
    static constexpr char hex[] = "0123456789ABCDEF";
    os.put('"');
    char const* segment = string.data();
    char const* const end = segment + string.size();
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

void value_to_stream(
    std::ostream& os, boost::json::value const& value, int indent,
    StreamFormat format, bool const pretty_print)
{
    if (value.is_array())
    {
        boost::json::array const& array = value.as_array();
        if (array.empty())
        {
            os << (format == StreamFormat::json ? "[]" : "{}");
            return;
        }

        os.put(format == StreamFormat::json ? '[' : '{');
        for (std::size_t i = 0; i < array.size(); ++i)
        {
            if (i != 0)
            {
                os.put(',');
            }
            if (pretty_print)
            {
                os.put('\n');
                write_indent(os, indent + 1);
            }
            value_to_stream(os, array[i], indent + 1, format, pretty_print);
        }
        if (pretty_print)
        {
            os.put('\n');
            write_indent(os, indent);
        }
        os.put(format == StreamFormat::json ? ']' : '}');
        return;
    }

    if (value.is_object())
    {
        boost::json::object const& object = value.as_object();
        if (object.empty())
        {
            os << "{}";
            return;
        }

        os.put('{');
        std::size_t i = 0;
        for (auto const& member : object)
        {
            if (i++ != 0)
            {
                os.put(',');
            }
            if (pretty_print)
            {
                os.put('\n');
                write_indent(os, indent + 1);
            }
            if (format == StreamFormat::lua)
            {
                os.put('[');
                write_lua_string(os, member.key());
                os << (pretty_print ? "] = " : "]=");
            }
            else
            {
                os << boost::json::serialize(member.key())
                   << (pretty_print ? ": " : ":");
            }
            value_to_stream(os, member.value(), indent + 1, format, pretty_print);
        }
        if (pretty_print)
        {
            os.put('\n');
            write_indent(os, indent);
        }
        os.put('}');
        return;
    }

    if (format == StreamFormat::lua && value.is_null())
    {
        os << "nil";
    }
    else if (format == StreamFormat::lua && value.is_string())
    {
        write_lua_string(os, value.as_string());
    }
    else
    {
        os << boost::json::serialize(value);
    }
}

bool lua_value_to_stream(
    std::ostream& os, lua_State* L, int index, int indent,
    bool const pretty_print, std::vector<void const*>& path)
{
    LuaStackRestore const restore{L};
    index = lua_absindex(L, index);

    if (lua_type(L, index) != LUA_TTABLE)
    {
        auto const converted = lua_to_json_impl(L, index, path);
        if (!converted)
        {
            return false;
        }
        value_to_stream(os, *converted, indent, StreamFormat::lua, pretty_print);
        return true;
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
    bool kind_known = false;
    bool is_array = true;
    bool dense_array = true;

    lua_pushnil(L);
    while (lua_next(L, index) != 0)
    {
        int const key_type = lua_type(L, -2);
        bool const integer_key = key_type == LUA_TNUMBER && lua_isinteger(L, -2);
        bool const string_key = key_type == LUA_TSTRING;
        if (!integer_key && !string_key)
        {
            return false;
        }
        if (!kind_known)
        {
            is_array = integer_key;
            kind_known = true;
        }
        else if (integer_key != is_array)
        {
            return false;
        }
        if (integer_key)
        {
            lua_Integer const lua_index = lua_tointeger(L, -2);
            if (lua_index < 1 || static_cast<std::uint64_t>(lua_index) > array_size)
            {
                dense_array = false;
            }
        }
        ++item_count;
        lua_pop(L, 1);
    }

    if (!kind_known)
    {
        os << "{}";
        return true;
    }
    dense_array = is_array && dense_array && item_count == array_size;

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
            if (is_array)
            {
                os << lua_tointeger(L, -2);
            }
            else
            {
                std::size_t key_size = 0;
                char const* const key = lua_tolstring(L, -2, &key_size);
                write_lua_string(os, boost::json::string_view{key, key_size});
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

std::ostream& lua_to_formatted_stream(
    std::ostream& os, sol::object const value, int const indent,
    StreamFormat format, bool const pretty_print)
{
    auto const converted = lua_to_json(value);
    if (!converted)
    {
        os.setstate(std::ios_base::failbit);
        return os;
    }

    if (!pretty_print && format == StreamFormat::json)
    {
        os << boost::json::serialize(*converted);
        return os;
    }

    int const initial_indent = pretty_print ? (std::max)(indent, 0) : 0;
    if (pretty_print)
    {
        write_indent(os, initial_indent);
    }
    value_to_stream(os, *converted, initial_indent, format, pretty_print);
    return os;
}

} // namespace


boost::optional<boost::json::value> lua_to_json(sol::object const value)
{
    try
    {
        lua_State* const L = value.lua_state();
        if (L == nullptr)
        {
            HAIKAN_LOG_CERR(ERROR) << "null state";
            return boost::none;
        }

        LuaStackRestore const restore{L};
        value.push();
        std::vector<void const*> path;
        return lua_to_json_impl(L, -1, path);
    }
    catch (...)
    {
        return boost::none;
    }
}

boost::optional<sol::object> json_to_lua(sol::state_view L, boost::json::value const& value)
{
    try
    {
        lua_State* const state = L.lua_state();
        if (state == nullptr)
        {
            return boost::none;
        }

        LuaStackRestore const restore{state};
        if (!push_json(state, value))
        {
            return boost::none;
        }
        return sol::stack::get<sol::object>(state, -1);
    }
    catch (...)
    {
        return boost::none;
    }
}

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

std::ostream& lua_to_json_stream(
    std::ostream& os, sol::object const value, int const indent, bool const pretty_print)
{
    return lua_to_formatted_stream(os, value, indent, StreamFormat::json, pretty_print);
}


} // namespace impl
} // namespace haikan
