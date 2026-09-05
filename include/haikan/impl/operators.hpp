/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <boost/type_traits.hpp>


#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/error.hpp"
#include "haikan/impl/expression_parameter.hpp"
#include "haikan/impl/operator_base.hpp"
#include "haikan/impl/set_operators.hpp"
#include "haikan/impl/type_info.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/impl/pp.hpp"




namespace haikan {


namespace impl {
template <class T>
using has_boolean = std::is_convertible<T, bool>;
}


namespace op
{

struct negate : impl::operator_base<negate, boost::has_negate>
{
using operator_base::evaluate;

template <class T>
static sol::object evaluate(std::true_type, lua_State* L, T&& x)
{
    return sol::make_object(L, -x);
}

};


struct complement : impl::operator_base<complement, boost::has_complement>
{
using operator_base::evaluate;

template <class T>
static sol::object evaluate(std::true_type, lua_State* L, T&& x)
{
    return sol::make_object(L, ~x);
}
};


struct logical_not : impl::operator_base<logical_not, boost::has_logical_not>
{
using operator_base::evaluate;

template <class T>
static sol::object evaluate(std::true_type, lua_State* L, T&& x)
{
    return sol::make_object(L, !x);
}
};


struct plus : impl::operator_base<plus, boost::has_plus>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x + y);
}
};


struct minus : impl::operator_base<minus, boost::has_minus>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x - y);
}
};


struct equal_to : impl::operator_base<equal_to, boost::has_equal_to>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x == y);
}
};

struct less : impl::operator_base<less, boost::has_less>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x < y);
}
};


struct greater : impl::operator_base<greater, boost::has_greater>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x > y);
}
};


struct less_equal : impl::operator_base<less_equal, boost::has_less_equal>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x <= y);
}
};

struct greater_equal : impl::operator_base<greater_equal, boost::has_greater_equal>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x >= y);
}
};


struct multiplie : impl::operator_base<multiplie, boost::has_multiplies>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x * y);
}
};


struct divide : impl::operator_base<divide, boost::has_divides>
{
using operator_base::evaluate;

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

struct modulo : impl::operator_base<modulo, boost::has_modulus>
{
using operator_base::evaluate;

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


struct bit_and : impl::operator_base<bit_and, boost::has_bit_and>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x & y);
}
};


struct bit_or : impl::operator_base<bit_or, boost::has_bit_or>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x | y);
}
};


struct bit_xor : impl::operator_base<bit_xor, boost::has_bit_xor>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x ^ y);
}
};


struct left_shift : impl::operator_base<left_shift, boost::has_left_shift>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x << y);
}
};


struct right_shift : impl::operator_base<right_shift, boost::has_right_shift>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x >> y);
}
};


struct logical_and : impl::operator_base<logical_and, boost::has_logical_and>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x && y);
}
};


struct logical_or : impl::operator_base<logical_or, boost::has_logical_or>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, x || y);
}
};


struct boolean : impl::operator_base<boolean, impl::has_boolean>
{
using operator_base::evaluate;

template <class T1>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x)
{
    return sol::make_object(L, static_cast<bool>(x));
}
};


} // namespace op
} // namespace haikan
