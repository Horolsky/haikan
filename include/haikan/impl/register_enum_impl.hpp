/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <boost/describe.hpp>
#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <tuple>


#include "haikan/impl/register_type.hpp"


namespace haikan {
namespace impl {

template <class T, class=void>
struct enum_stringify;

template <class T>
struct enum_stringify<T, mp_if<boost::describe::has_describe_enumerators<T>, void>>
{
    using Descr = boost::describe::describe_enumerators<T>;

    static boost::optional<T> from_string(std::string const& nominal)
    {
        boost::optional<T> value_found{};
        boost::mp11::mp_for_each<Descr>([&](auto descr) {
            if (!value_found && (nominal == descr.name)) {
                value_found = descr.value;
            }
        });
        return value_found;
    }

    static boost::optional<std::string> to_string(T const value)
    {
        boost::optional<std::string> nominal_found{};
        boost::mp11::mp_for_each<Descr>([&](auto descr) {
            if (!nominal_found && (value == descr.value)) {
                nominal_found = descr.name;
            }
        });
        return nominal_found;
    }
};


// userdata wrapper
template <class T>
class user_data_enum
{
    static_assert(std::is_enum<T>::value, "T is not enum");

  public:
    using value_type = T;
    using underlying_type = std::underlying_type_t<T>;

    using sol_constructors = sol::constructors<
        sol::types<>,
        sol::types<user_data_enum const&>,
        sol::types<user_data_enum &&>,
        sol::types<value_type const>,
        sol::types<underlying_type const>,
        sol::types<std::string const&>
    >;

    user_data_enum() = default;
    user_data_enum(user_data_enum const&) = default;
    user_data_enum(user_data_enum &&) = default;
    user_data_enum& operator=(user_data_enum const&) = default;
    user_data_enum& operator=(user_data_enum &&) = default;

    // T conversion
    user_data_enum(value_type const value) : value_{value}
    {
    }

    user_data_enum& operator=(T const value)
    {
        value_ = value;
        return *this;
    }

    value_type value() const
    {
        return value_;
    }

    explicit operator value_type() const
    {
        return value_;
    }

    // integer conversion
    user_data_enum(underlying_type const value) : value_{static_cast<value_type>(value)}
    {
    }

    user_data_enum& operator=(underlying_type const value)
    {
        value_ = static_cast<value_type>(value);
        return *this;
    }

    explicit operator underlying_type() const
    {
        return static_cast<underlying_type>(value_);
    }

    // string conversion
    user_data_enum(std::string const& nominal)
        : value_{enum_stringify<T>::from_string(nominal).value()}
    {
    }

    user_data_enum& operator=(std::string const& nominal)
    {
        value_ = enum_stringify<T>::from_string(nominal).value();
        return *this;
    }

    std::string to_string() const
    {
        return enum_stringify<T>::to_string(value_).value_or(std::to_string(static_cast<underlying_type>(value_)));
    }

    explicit operator std::string() const
    {
        return to_string();
    }

    bool operator==(user_data_enum const& o)
    {
        return value_ == o.value_;
    }

  private:
    value_type value_;
};






template <class T, class Seen>
struct register_type_impl<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, user_data_enum<T>>>, boost::describe::has_describe_enumerators<T>>, void>>
{
    using U = user_data_enum<T>;

    static void operator()(sol::state_view L)
    {
        using ctors = typename U::sol_constructors;

        L.new_simple_usertype<U>(sol::usertype_traits<T>::name(),
            sol::meta_function::construct, ctors(),
            sol::call_constructor, ctors(),
            "str", &U::to_string,
            "num", &U::value,
            sol::meta_function::to_string, &U::to_string
        );
    }
};


} // namespace impl
} // namespace haikan
