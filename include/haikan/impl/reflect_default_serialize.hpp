/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include <boost/describe.hpp>
#include <boost/utility/string_view.hpp>

#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/type_tag.hpp"

namespace haikan {
namespace impl {


inline int hexdump_value(char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    }
    if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    }
    return -1;
}

template <class T>
struct default_reflect_serialize
{
    static sol::table get_or_create_table(sol::state_view L, sol::table parent, char const* name)
    {
        sol::object obj = parent[name];
        if (obj.get_type() == sol::type::table)
        {
            return obj;
        }
        sol::table tbl = L.create_table();
        parent[name] = tbl;
        return tbl;
    }


    static bool may_have_registered_utype(sol::state_view L, std::string const& name)
    {
        sol::object usertype_obj = L[name];
        return usertype_obj.get_type() != sol::type::nil && usertype_obj.get_type() != sol::type::none;
    }

    static sol::optional<sol::table> metadata_from_object(sol::object obj)
    {
        if (obj.get_type() != sol::type::userdata)
        {
            return sol::optional<sol::table>{};
        }
        try
        {
            sol::userdata userdata = obj;
            sol::table metatable = userdata[sol::metatable_key];
            sol::optional<sol::table> meta = metatable["__haikan"];
            if (meta)
            {
                return meta;
            }

            sol::optional<sol::table> index = metatable["__index"];
            if (index)
            {
                sol::optional<sol::table> index_meta = index.value()["__haikan"];
                if (index_meta)
                {
                    return index_meta;
                }
            }
        }
        catch (sol::error const&)
        {
        }

        try
        {
            sol::userdata userdata = obj;
            sol::optional<sol::table> meta = userdata["__haikan"];
            return meta;
        }
        catch (sol::error const&)
        {
            return sol::optional<sol::table>{};
        }
    }

    static sol::optional<sol::table> metadata(sol::state_view L, T const* obj = nullptr)
    {
        sol::object root_obj = L["haikan"];
        if (root_obj.get_type() != sol::type::table)
        {
            return sol::optional<sol::table>{};
        }

        sol::table root = root_obj;
        sol::object utypes_obj = root["utypes"];
        if (utypes_obj.get_type() == sol::type::table)
        {
            sol::table utypes = utypes_obj;
            sol::object registered_meta = utypes[typeid(T).hash_code()];
            if (registered_meta.get_type() == sol::type::nil || registered_meta.get_type() == sol::type::none)
            {
                registered_meta = utypes[std::string(sol::usertype_traits<T>::name())];
            }
            if (registered_meta.get_type() == sol::type::table)
            {
                return registered_meta.as<sol::table>();
            }
        }

        if (!may_have_registered_utype(L, std::string(sol::usertype_traits<T>::name())))
        {
            return sol::optional<sol::table>{};
        }

        if (obj != nullptr)
        {
            sol::optional<sol::table> meta = metadata_from_object(sol::make_object(L.lua_state(), *obj));
            if (meta)
            {
                return meta;
            }
        }

        auto init = reflect<T>::init();
        if (init)
        {
            return metadata_from_object(sol::make_object(L.lua_state(), init.value()));
        }

        return sol::optional<sol::table>{};
    }

    template <class U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value, sol::object>::type
    serialize_fallback(U const& obj, sol::state_view L)
    {
        static char const hex[] = "0123456789abcdef";

        auto const* bytes = reinterpret_cast<unsigned char const*>(&obj);
        std::string out;
        out.reserve(sizeof(U) * 2);
        for (std::size_t i = 0; i < sizeof(U); ++i)
        {
            out.push_back(hex[bytes[i] >> 4]);
            out.push_back(hex[bytes[i] & 0x0F]);
        }
        return sol::make_object(L.lua_state(), out);
    }

    template <class U = T>
    static typename std::enable_if<!std::is_trivially_copyable<U>::value, sol::object>::type
    serialize_fallback(U const& obj, sol::state_view L)
    {
        return sol::make_object(L.lua_state(), std::cref(obj));
    }

    template <class U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value && std::is_default_constructible<U>::value, boost::optional<U>>::type
    deserialize_fallback(sol::object obj)
    {
        if (obj.is<U>())
        {
            return obj.as<U>();
        }
        if (!obj.is<std::string>())
        {
            return boost::none;
        }

        std::string const input = obj.as<std::string>();
        if (input.size() != sizeof(U) * 2)
        {
            return boost::none;
        }

        U value{};
        auto* bytes = reinterpret_cast<unsigned char*>(&value);
        for (std::size_t i = 0; i < sizeof(U); ++i)
        {
            int const high = hexdump_value(input[2 * i]);
            int const low = hexdump_value(input[2 * i + 1]);
            if (high < 0 || low < 0)
            {
                return boost::none;
            }
            bytes[i] = static_cast<unsigned char>((high << 4) | low);
        }
        return value;
    }

    template <class U = T>
    static typename std::enable_if<!(std::is_trivially_copyable<U>::value && std::is_default_constructible<U>::value), boost::optional<U>>::type
    deserialize_fallback(sol::object obj)
    {
        if (obj.is<U>())
        {
            return obj.as<U>();
        }
        return boost::none;
    }

    static sol::object serialize(T const& obj, sol::state_view L)
    {
        sol::optional<sol::table> meta = metadata(L, &obj);
        if (meta)
        {
            sol::optional<sol::function> serialize_fn = meta.value()["serialize"];
            if (serialize_fn)
            {
                sol::object value_in = sol::make_object(L.lua_state(), std::ref(const_cast<T&>(obj)));
                sol::protected_function_result result = serialize_fn.value()(value_in);
                sol::object value = result.get<sol::object>();
                return sol::make_object(L.lua_state(), value);
            }
        }
        return serialize_fallback(obj, L);
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        sol::state_view L(obj.lua_state());
        sol::optional<sol::table> meta = metadata(L);
        if (meta)
        {
            sol::optional<sol::function> deserialize_fn = meta.value()["deserialize"];
            if (deserialize_fn)
            {
                sol::protected_function_result result = deserialize_fn.value()(obj);
                sol::object value = result.get<sol::object>();
                value = sol::make_object(L.lua_state(), value);
                if (value.is<T>())
                {
                    return value.as<T>();
                }
            }
        }
        return deserialize_fallback(obj);
    }
};

} // namespace impl
} // namespace haikan
