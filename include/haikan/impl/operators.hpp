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
#include "haikan/impl/type_info.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/impl/pp.hpp"



namespace haikan {


namespace impl {
template <class T>
using has_boolean = std::is_convertible<T, bool>;

template <class Collection, class Item>
using has_contains = std::integral_constant<bool,
    has_find<Collection, Item>::value || has_linear_find<Collection, Item>::value>;

template <class Item, class Collection>
using has_is_in = has_contains<Collection, Item>;

template <class Subset, class Superset, class = void>
struct has_is_subset : std::false_type {};

template <class Subset, class Superset>
struct has_is_subset<Subset, Superset, void_t<
    decltype(std::begin(std::declval<Subset const&>())),
    decltype(std::end(std::declval<Subset const&>()))>>
    : has_contains<Superset, std::remove_cv_t<std::remove_reference_t<
          decltype(*std::begin(std::declval<Subset const&>()))>>> {};

template <class Subset, class Superset>
using has_is_proper_subset = std::integral_constant<bool,
    has_is_subset<Subset, Superset>::value && has_is_subset<Superset, Subset>::value>;

template <class Lhs, class Rhs>
using has_set_equal = std::integral_constant<bool,
    has_is_subset<Lhs, Rhs>::value && has_is_subset<Rhs, Lhs>::value>;

template <class Superset, class Subset>
using has_is_superset = has_is_subset<Subset, Superset>;

template <class Superset, class Subset>
using has_is_proper_superset = has_is_proper_subset<Subset, Superset>;

template <class Collection, class Item>
static bool collection_contains(std::true_type, Collection const& collection, Item const& item)
{
    return collection.find(item) != std::end(collection);
}

template <class Collection, class Item>
static bool collection_contains(std::false_type, Collection const& collection, Item const& item)
{
    return std::find(std::begin(collection), std::end(collection), item) != std::end(collection);
}

template <class Collection, class Item>
static bool collection_contains(Collection const& collection, Item const& item)
{
    return collection_contains(has_find<Collection, Item>{}, collection, item);
}

template <class Subset, class Superset>
static bool subset_of(Subset const& subset, Superset const& superset)
{
    for (auto const& item : subset)
    {
        if (HAIKAN_UNLIKELY(!collection_contains(superset, item)))
        {
            return false;
        }
    }
    return true;
}

template <class Subset, class Superset>
static bool proper_subset_of(Subset const& subset, Superset const& superset)
{
    if (!subset_of(subset, superset))
    {
        return false;
    }
    for (auto const& item : superset)
    {
        if (!collection_contains(subset, item))
        {
            return true;
        }
    }
    return false;
}

template <class Lhs, class Rhs>
static bool set_equal(Lhs const& lhs, Rhs const& rhs)
{
    return subset_of(lhs, rhs) && subset_of(rhs, lhs);
}
}

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


struct boolean : base<boolean, impl::has_boolean>
{
using base::evaluate;

template <class T1>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x)
{
    return sol::make_object(L, static_cast<bool>(x));
}
};




struct contains : base<contains, impl::has_contains>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& collection, T2&& item)
{
    return sol::make_object(L, impl::collection_contains(collection, item));
}
};


struct is_in : base<is_in, impl::has_is_in>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, impl::collection_contains(y, x));
}
};


struct is_subset : base<is_subset, impl::has_is_subset>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& subset, T2&& superset)
{
    return sol::make_object(L, impl::subset_of(subset, superset));
}
};


struct set_equal : base<set_equal, impl::has_set_equal>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& lhs, T2&& rhs)
{
    return sol::make_object(L, impl::set_equal(lhs, rhs));
}
};


struct is_proper_subset : base<is_proper_subset, impl::has_is_proper_subset>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& subset, T2&& superset)
{
    return sol::make_object(L, impl::proper_subset_of(subset, superset));
}
};


struct is_superset : base<is_superset, impl::has_is_superset>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& superset, T2&& subset)
{
    return sol::make_object(L, impl::subset_of(subset, superset));
}
};


struct is_proper_superset : base<is_proper_superset, impl::has_is_proper_superset>
{
using base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& superset, T2&& subset)
{
    return sol::make_object(L, impl::proper_subset_of(subset, superset));
}
};

} // namespace op
} // namespace haikan
