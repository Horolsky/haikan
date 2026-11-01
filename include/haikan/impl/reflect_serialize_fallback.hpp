/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <functional>
#include <string>
#include <type_traits>

#include <boost/optional.hpp>

#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/user_data_enum.hpp"


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
struct is_narrow_string : std::false_type {};

template <class Traits, class Allocator>
struct is_narrow_string<std::basic_string<char, Traits, Allocator>> : std::true_type {};


template <class T, class = void>
struct reflect_serialize_fallback
{
    template <class U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value, sol::object>::type
    serialize(U const& obj, sol::state_view L)
    {
        static char const hex[] = "0123456789abcdef";

        unsigned char bytes[sizeof(U)];
        std::memcpy(bytes, &obj, sizeof(U));
        std::string out(sizeof(U) * 2, '\0');
        for (std::size_t i = 0; i < sizeof(U); ++i)
        {
            out[2 * i] = hex[bytes[i] >> 4];
            out[2 * i + 1] = hex[bytes[i] & 0x0F];
        }
        return sol::make_object(L.lua_state(), out);
    }

    template <class U = T>
    static typename std::enable_if<!std::is_trivially_copyable<U>::value, sol::object>::type
    serialize(U const& obj, sol::state_view L)
    {
        return sol::make_object(L.lua_state(), std::cref(obj));
    }

    template <class U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value && std::is_default_constructible<U>::value, boost::optional<U>>::type
    deserialize(sol::object obj)
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

        unsigned char bytes[sizeof(U)];
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
        U value{};
        std::memcpy(&value, bytes, sizeof(U));
        return value;
    }

    template <class U = T>
    static typename std::enable_if<!(std::is_trivially_copyable<U>::value && std::is_default_constructible<U>::value), boost::optional<U>>::type
    deserialize(sol::object obj)
    {
        if (obj.is<U>())
        {
            return obj.as<U>();
        }
        return boost::none;
    }

};


template <class T>
struct reflect_serialize_fallback<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
    static sol::object serialize(T const obj, sol::state_view L)
    {
        return sol::make_object(L.lua_state(), obj);
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        if (!obj.is<T>())
        {
            return boost::none;
        }
        return obj.as<T>();
    }
};


template <class T>
struct reflect_serialize_fallback<T, typename std::enable_if<std::is_enum<T>::value>::type>
{
    using underlying_type = typename std::underlying_type<T>::type;

    static sol::object serialize(T const obj, sol::state_view L)
    {
        return sol::make_object(L.lua_state(), static_cast<underlying_type>(obj));
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        if (obj.is<T>())
        {
            return obj.as<T>();
        }
        if (obj.is<underlying_type>())
        {
            return static_cast<T>(obj.as<underlying_type>());
        }
        if (obj.is<user_data_enum<T>>())
        {
            return obj.as<user_data_enum<T>>().value();
        }
        return boost::none;
    }
};


template <class T>
struct reflect_serialize_fallback<T, typename std::enable_if<is_narrow_string<T>::value>::type>
{
    static sol::object serialize(T const& obj, sol::state_view L)
    {
        return sol::make_object(L.lua_state(), obj);
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        if (!obj.is<T>())
        {
            return boost::none;
        }
        return obj.as<T>();
    }
};

} // namespace impl
} // namespace haikan
