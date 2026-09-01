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
#include "haikan/impl/expression_parameter.hpp"
#include "haikan/impl/type_info.hpp"
#include "haikan/impl/pp.hpp"



namespace haikan {

namespace op
{

template <class Impl, template <class...> class Trait>
struct base
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



struct negate : base<negate, boost::has_negate>
{
using base::evaluate;

template <class T>
static sol::object evaluate(std::true_type, lua_State* L, T&& x)
{
    return sol::make_object(L, -x);
}

};


struct complement : base<complement, boost::has_complement>
{
using base::evaluate;

template <class T>
static sol::object evaluate(std::true_type, lua_State* L, T&& x)
{
    return sol::make_object(L, ~x);
}
};


struct logical_not : base<logical_not, boost::has_logical_not>
{
using base::evaluate;

template <class T>
static sol::object evaluate(std::true_type, lua_State* L, T&& x)
{
    return sol::make_object(L, !x);
}
};


struct plus : base<plus, boost::has_plus>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x + y);
}
};


struct minus : base<minus, boost::has_minus>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x - y);
}
};


struct equal_to : base<equal_to, boost::has_equal_to>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x == y);
}
};

struct less : base<less, boost::has_less>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x < y);
}
};


struct greater : base<greater, boost::has_greater>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x > y);
}
};


struct less_equal : base<less_equal, boost::has_less_equal>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x <= y);
}
};

struct greater_equal : base<greater_equal, boost::has_greater_equal>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x >= y);
}
};


struct multiplie : base<multiplie, boost::has_multiplies>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x * y);
}
};


struct divide : base<divide, boost::has_divides>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate_zero(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    if (HAIKAN_UNLIKELY(y == 0))
    {
        return sol::make_object(L, error("zero division", impl::type_name<divide>()));
    }
    return sol::make_object(L, x / y);
}

template <class T1, class T2>
static sol::object evaluate_zero(std::false_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x / y);
}

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    constexpr bool x_is_arithmetic = std::is_arithmetic<std::remove_cv_t<std::remove_reference_t<T1>>>::value;
    constexpr bool y_is_arithmetic = std::is_arithmetic<std::remove_cv_t<std::remove_reference_t<T2>>>::value;
    using maybe_zero_div = std::integral_constant<bool, x_is_arithmetic && y_is_arithmetic>;
    return evaluate_zero(maybe_zero_div{}, L, std::forward<T1>(x), std::forward<T2>(y));
}
};

struct modulo : base<modulo, boost::has_modulus>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate_zero(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    if (HAIKAN_UNLIKELY(y == 0))
    {
        return sol::make_object(L, error("zero division", impl::type_name<modulo>()));
    }
    return sol::make_object(L, x % y);
}

template <class T1, class T2>
static sol::object evaluate_zero(std::false_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x % y);
}

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    constexpr bool x_is_arithmetic = std::is_arithmetic<std::remove_cv_t<std::remove_reference_t<T1>>>::value;
    constexpr bool y_is_arithmetic = std::is_arithmetic<std::remove_cv_t<std::remove_reference_t<T2>>>::value;
    using maybe_zero_div = std::integral_constant<bool, x_is_arithmetic && y_is_arithmetic>;
    return evaluate_zero(maybe_zero_div{}, L, std::forward<T1>(x), std::forward<T2>(y));
}
};


struct bit_and : base<bit_and, boost::has_bit_and>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x & y);
}
};


struct bit_or : base<bit_or, boost::has_bit_or>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x | y);
}
};


struct bit_xor : base<bit_xor, boost::has_bit_xor>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x ^ y);
}
};


struct left_shift : base<left_shift, boost::has_left_shift>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x << y);
}
};


struct right_shift : base<right_shift, boost::has_right_shift>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x >> y);
}
};


struct logical_and : base<logical_and, boost::has_logical_and>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x && y);
}
};


struct logical_or : base<logical_or, boost::has_logical_or>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x || y);
}
};


template <class T>
using has_boolean = std::is_convertible<T, bool>;

struct boolean : base<boolean, has_boolean>
{
using base::evaluate;

template <class T1>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x)
{
    return sol::make_object(L, static_cast<bool>(x));
}
};


} // namespace impl
} // namespace haikan
