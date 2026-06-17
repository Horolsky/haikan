/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <cstdint>
#include <cstdlib>
#include <stdexcept>

#include "haikan/impl/json_ptr_lua.hpp"


#define HAIKAN_MAX_JPTR_TOKEN_LEN 255


namespace {


bool json_ptr_is_array_index(boost::string_view token, std::int64_t& idx)
{
    if (token.empty())
    {
        return false;
    }

    std::uint64_t value = 0;
    for (char const c : token)
    {
        if ((c < '0') || (c > '9'))
        {
            return false;
        }
        value = (value * 10u) + static_cast<unsigned>(c - '0');
        if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max() - 1))
        {
            return false;
        }
    }

    idx = static_cast<std::int64_t>(value) + 1;
    return true;
}


char const* json_ptr_unescape(boost::string_view token, char (&out)[(HAIKAN_MAX_JPTR_TOKEN_LEN + 1)])
{
    char* dst = out;
    char* const end = out + HAIKAN_MAX_JPTR_TOKEN_LEN;

    for (std::size_t i = 0; i < token.size(); ++i)
    {
        char c = token[i];
        if (c == '~')
        {
            if (++i == token.size())
            {
                throw std::invalid_argument("Invalid JSON Pointer escape");
            }

            switch (token[i])
            {
            case '0': c = '~'; break;
            case '1': c = '/'; break;
            default: throw std::invalid_argument("Invalid JSON Pointer escape");
            }
        }

        if (dst == end)
        {
            throw std::length_error("JSON Pointer token is too long");
        }
        *dst++ = c;
    }

    *dst = '\0';
    return out;
}

boost::string_view json_ptr_next_token(boost::string_view& ptr)
{
    BOOST_ASSERT(!ptr.empty());
    BOOST_ASSERT(ptr.front() == '/');

    ptr.remove_prefix(1);
    std::size_t const slash = ptr.find('/');
    if (slash == boost::string_view::npos)
    {
        boost::string_view const token = ptr;
        ptr = {};
        return token;
    }

    boost::string_view const token = ptr.substr(0, slash);
    ptr.remove_prefix(slash);
    return token;
}

sol::object json_ptr_get(sol::object obj, boost::string_view ptr)
{
    if (ptr.empty())
    {
        return obj;
    }
    if (ptr.front() != '/')
    {
        throw std::invalid_argument("Invalid JSON Pointer");
    }

    while (!ptr.empty())
    {
        if (obj.get_type() != sol::type::table)
        {
            throw std::out_of_range("JSON Pointer does not reference an object");
        }

        boost::string_view const token = json_ptr_next_token(ptr);
        sol::table const table = obj.as<sol::table>();

        std::int64_t idx = 0;
        if (json_ptr_is_array_index(token, idx))
        {
            obj = table[idx];
        }
        else
        {
            char key[(HAIKAN_MAX_JPTR_TOKEN_LEN + 1)];
            obj = table[json_ptr_unescape(token, key)];
        }

        if (obj == sol::nil)
        {
            throw std::out_of_range("JSON Pointer does not exist");
        }
    }

    return obj;
}

bool json_ptr_set(sol::object& obj, boost::string_view ptr, sol::object value, std::error_code& ec)
{
    ec.clear();

    if (ptr.empty())
    {
        obj = value;
        return true;
    }
    if (ptr.front() != '/')
    {
        ec = std::make_error_code(std::errc::invalid_argument);
        return false;
    }

    sol::state_view L(obj.lua_state());
    sol::object parent = obj;
    char key[(HAIKAN_MAX_JPTR_TOKEN_LEN + 1)];

    while (true)
    {
        if (parent.get_type() != sol::type::table)
        {
            ec = std::make_error_code(std::errc::not_a_directory);
            return false;
        }

        boost::string_view const token = json_ptr_next_token(ptr);
        sol::table table = parent.as<sol::table>();

        std::int64_t idx = 0;
        bool const array_index = json_ptr_is_array_index(token, idx);
        if (!array_index)
        {
            json_ptr_unescape(token, key);
        }

        if (ptr.empty())
        {
            if (array_index)
            {
                table[idx] = value;
            }
            else
            {
                table[key] = value;
            }
            return true;
        }

        sol::object child = array_index ? sol::object(table[idx]) : sol::object(table[key]);
        if (child == sol::nil)
        {
            child = L.create_table();
            if (array_index)
            {
                table[idx] = child;
            }
            else
            {
                table[key] = child;
            }
        }
        parent = child;
    }
}

} // namespace

namespace haikan {
namespace impl {


sol::object
at_pointer(
    sol::object obj,
    boost::string_view ptr)
{
    return json_ptr_get(obj, ptr);
}


sol::object
set_at_pointer(
    sol::object& obj,
    boost::string_view ptr,
    sol::object value)
{
    std::error_code ec;
    if (!json_ptr_set(obj, ptr, value, ec))
    {
        throw std::system_error(ec);
    }
    return value;
}
}
}
