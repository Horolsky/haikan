/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <boost/describe.hpp>
#include <boost/format.hpp>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <tuple>
#include "haikan/impl/reflect_default_init.hpp"
#include "haikan/impl/reflect_default_next_fn.hpp"
#include "haikan/impl/type_info.hpp"

#include "haikan/impl/reflect.hpp"


#include "haikan/impl/register_type.hpp"
#include "haikan/impl/register_struct_impl.hpp"
#include "haikan/impl/register_enum_impl.hpp"
#include "haikan/impl/register_misc_impl.hpp"


namespace haikan {
namespace impl {


namespace detail {
template <typename T> using sol_make_object_call_t = decltype(sol::make_object(std::declval<lua_State*>(), std::declval<T>()));
}

template <typename T> using is_sol_make_object_valid = boost::mp11::mp_valid<detail::sol_make_object_call_t, T>;


template <class T>
struct default_reflect<T, mp_if<is_sol_make_object_valid<T>, void>> : default_reflect_init<T>
{

    static sol::object solify(T const& value, sol::state_view L)
    {
        register_type<T>()(L);
        return sol::make_object(L, value);
    }

    static sol::object solify(T&& value, sol::state_view L)
    {
        register_type<T>()(L);
        return sol::make_object(L, std::move(value));
    }

    static boost::optional<T> desolify(sol::object const& obj)
    {
        register_type<T>()(obj.lua_state());
        if (obj.is<T>())
        {
            return boost::optional<T>{obj.as<T>()};
        }
        return boost::none;
    }
};

} // namespace impl
} // namespace haikan
