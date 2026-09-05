/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <type_traits>
#include <boost/type_traits.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/error.hpp"
#include "haikan/impl/type_info.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/impl/pp.hpp"



namespace haikan {

namespace impl
{

template <class Impl, template <class...> class Trait>
struct operator_base
{

    template <class... T>
    static sol::object operator()(lua_State* L, T&&... x) noexcept {
        try
        {
            constexpr bool is_valid = Trait<std::remove_cv_t<std::remove_reference_t<T>>...>::value;
            using enable = std::integral_constant<bool, is_valid>;
            return Impl::evaluate(enable{}, L, std::forward<T>(x)...);
        }
        catch (std::exception const& e)
        {
            return sol::make_object(L, error(e.what(), impl::type_name<Impl>()));
        }
    }

    template <class... T>
    static sol::object evaluate(std::false_type, lua_State* L, T&&...)
    {
        return sol::make_object(L, error("operator not implemented", impl::type_name<Impl>()));
    }

    template <class T>
    static sol::object cast_and_evaluate(sol::object x)
    {
        if (HAIKAN_UNLIKELY(!x.is<T>()))
        {
            return sol::make_object(x.lua_state(), error("invalid operand", impl::type_name<Impl>()));
        }
        return operator()(x.lua_state(), x.as<T>());
    }

    template <class T1, class T2>
    static sol::object cast_and_evaluate(sol::object x, sol::object y)
    {
        if (HAIKAN_UNLIKELY(!x.is<T1>() || !y.is<T2>()))
        {
            return sol::make_object(x.lua_state(), error("invalid operands", impl::type_name<Impl>()));
        }
        return operator()(x.lua_state(), x.as<T1>(), y.as<T2>());
    }

};

} // namespace impl
} // namespace haikan
