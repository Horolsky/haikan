/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <set>
#include <type_traits>
#include <unordered_set>
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

template <class Lhs, class Rhs, class = void>
struct has_set_operation : std::false_type {};

template <class Lhs, class Rhs>
struct has_set_operation<Lhs, Rhs, void_t<
    typename Lhs::value_type,
    decltype(std::begin(std::declval<Rhs const&>())),
    decltype(std::end(std::declval<Rhs const&>())),
    decltype(Lhs(std::declval<Lhs const&>())),
    decltype(std::declval<Lhs&>().clear()),
    decltype(std::declval<Lhs&>().insert(
        std::end(std::declval<Lhs&>()),
        *std::begin(std::declval<Rhs const&>())))>>
    : std::integral_constant<bool,
        std::is_convertible<
            std::remove_cv_t<std::remove_reference_t<
                decltype(*std::begin(std::declval<Rhs const&>()))>>,
            typename Lhs::value_type>::value &&
        has_contains<Lhs, std::remove_cv_t<std::remove_reference_t<
            decltype(*std::begin(std::declval<Rhs const&>()))>>>::value &&
        has_contains<Rhs, typename Lhs::value_type>::value> {};

template <class Collection>
using has_uniques = has_set_operation<Collection, Collection>;

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

template <class Collection>
static Collection uniques(Collection const& collection)
{
    Collection result(collection);
    result.clear();
    for (auto const& item : collection)
    {
        if (!collection_contains(result, item))
        {
            result.insert(std::end(result), item);
        }
    }
    return result;
}

template <class Key, class Compare, class Allocator>
static std::set<Key, Compare, Allocator> uniques(
    std::set<Key, Compare, Allocator> const& collection)
{
    return collection;
}

template <class Key, class Hash, class KeyEqual, class Allocator>
static std::unordered_set<Key, Hash, KeyEqual, Allocator> uniques(
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& collection)
{
    return collection;
}

template <class Key, class Allocator>
static std::set<Key, std::less<Key>, Allocator> set_union(
    std::set<Key, std::less<Key>, Allocator> const& lhs,
    std::set<Key, std::less<Key>, Allocator> const& rhs)
{
    std::set<Key, std::less<Key>, Allocator> result(lhs.key_comp(), lhs.get_allocator());
    std::set_union(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        std::inserter(result, result.end()), lhs.key_comp());
    return result;
}

template <class Key, class Allocator>
static std::set<Key, std::less<Key>, Allocator> set_intersection(
    std::set<Key, std::less<Key>, Allocator> const& lhs,
    std::set<Key, std::less<Key>, Allocator> const& rhs)
{
    std::set<Key, std::less<Key>, Allocator> result(lhs.key_comp(), lhs.get_allocator());
    std::set_intersection(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        std::inserter(result, result.end()), lhs.key_comp());
    return result;
}

template <class Key, class Allocator>
static std::set<Key, std::less<Key>, Allocator> set_difference(
    std::set<Key, std::less<Key>, Allocator> const& lhs,
    std::set<Key, std::less<Key>, Allocator> const& rhs)
{
    std::set<Key, std::less<Key>, Allocator> result(lhs.key_comp(), lhs.get_allocator());
    std::set_difference(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        std::inserter(result, result.end()), lhs.key_comp());
    return result;
}

template <class Key, class Allocator>
static std::set<Key, std::less<Key>, Allocator> set_symmetric_difference(
    std::set<Key, std::less<Key>, Allocator> const& lhs,
    std::set<Key, std::less<Key>, Allocator> const& rhs)
{
    std::set<Key, std::less<Key>, Allocator> result(lhs.key_comp(), lhs.get_allocator());
    std::set_symmetric_difference(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
        std::inserter(result, result.end()), lhs.key_comp());
    return result;
}

template <class Key, class Hash, class KeyEqual, class Allocator>
static std::unordered_set<Key, Hash, KeyEqual, Allocator> set_union(
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& lhs,
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& rhs)
{
    auto result = lhs;
    result.reserve(lhs.size() + rhs.size());
    result.insert(rhs.begin(), rhs.end());
    return result;
}

template <class Key, class Hash, class KeyEqual, class Allocator>
static std::unordered_set<Key, Hash, KeyEqual, Allocator> set_intersection(
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& lhs,
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& rhs)
{
    std::unordered_set<Key, Hash, KeyEqual, Allocator> result(
        0, lhs.hash_function(), lhs.key_eq(), lhs.get_allocator());
    result.max_load_factor(lhs.max_load_factor());
    result.reserve(std::min(lhs.size(), rhs.size()));
    for (auto const& item : lhs)
    {
        if (rhs.find(item) != rhs.end())
        {
            result.insert(item);
        }
    }
    return result;
}

template <class Key, class Hash, class KeyEqual, class Allocator>
static std::unordered_set<Key, Hash, KeyEqual, Allocator> set_difference(
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& lhs,
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& rhs)
{
    std::unordered_set<Key, Hash, KeyEqual, Allocator> result(
        0, lhs.hash_function(), lhs.key_eq(), lhs.get_allocator());
    result.max_load_factor(lhs.max_load_factor());
    result.reserve(lhs.size());
    for (auto const& item : lhs)
    {
        if (rhs.find(item) == rhs.end())
        {
            result.insert(item);
        }
    }
    return result;
}

template <class Key, class Hash, class KeyEqual, class Allocator>
static std::unordered_set<Key, Hash, KeyEqual, Allocator> set_symmetric_difference(
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& lhs,
    std::unordered_set<Key, Hash, KeyEqual, Allocator> const& rhs)
{
    auto result = set_difference(lhs, rhs);
    result.reserve(lhs.size() + rhs.size());
    for (auto const& item : rhs)
    {
        if (lhs.find(item) == lhs.end())
        {
            result.insert(item);
        }
    }
    return result;
}

template <class Lhs, class Rhs>
static Lhs set_union(Lhs const& lhs, Rhs const& rhs)
{
    Lhs result(lhs);
    for (auto const& item : rhs)
    {
        if (!collection_contains(result, item))
        {
            result.insert(std::end(result), item);
        }
    }
    return result;
}

template <class Lhs, class Rhs>
static Lhs set_intersection(Lhs const& lhs, Rhs const& rhs)
{
    Lhs result(lhs);
    result.clear();
    for (auto const& item : lhs)
    {
        if (collection_contains(rhs, item))
        {
            result.insert(std::end(result), item);
        }
    }
    return result;
}

template <class Lhs, class Rhs>
static Lhs set_difference(Lhs const& lhs, Rhs const& rhs)
{
    Lhs result(lhs);
    result.clear();
    for (auto const& item : lhs)
    {
        if (!collection_contains(rhs, item))
        {
            result.insert(std::end(result), item);
        }
    }
    return result;
}

template <class Lhs, class Rhs>
static Lhs set_symmetric_difference(Lhs const& lhs, Rhs const& rhs)
{
    Lhs result = set_difference(lhs, rhs);
    for (auto const& item : rhs)
    {
        if (!collection_contains(lhs, item) && !collection_contains(result, item))
        {
            result.insert(std::end(result), item);
        }
    }
    return result;
}
} // namespace impl


namespace op {

struct uniques : impl::operator_base<uniques, impl::has_uniques>
{
using operator_base::evaluate;

template <class T>
static sol::object evaluate(std::true_type, lua_State* L, T&& collection)
{
    return sol::make_object(L, impl::uniques(collection));
}
};

struct contains : impl::operator_base<contains, impl::has_contains>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& collection, T2&& item)
{
    return sol::make_object(L, impl::collection_contains(collection, item));
}
};


struct is_in : impl::operator_base<is_in, impl::has_is_in>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& x, T2&& y)
{
    return sol::make_object(L, impl::collection_contains(y, x));
}
};


struct is_subset : impl::operator_base<is_subset, impl::has_is_subset>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& subset, T2&& superset)
{
    return sol::make_object(L, impl::subset_of(subset, superset));
}
};


struct set_equal : impl::operator_base<set_equal, impl::has_set_equal>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& lhs, T2&& rhs)
{
    return sol::make_object(L, impl::set_equal(lhs, rhs));
}
};


struct is_proper_subset : impl::operator_base<is_proper_subset, impl::has_is_proper_subset>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& subset, T2&& superset)
{
    return sol::make_object(L, impl::proper_subset_of(subset, superset));
}
};


struct is_superset : impl::operator_base<is_superset, impl::has_is_superset>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& superset, T2&& subset)
{
    return sol::make_object(L, impl::subset_of(subset, superset));
}
};


struct is_proper_superset : impl::operator_base<is_proper_superset, impl::has_is_proper_superset>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& superset, T2&& subset)
{
    return sol::make_object(L, impl::proper_subset_of(subset, superset));
}
};

struct set_union : impl::operator_base<set_union, impl::has_set_operation>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& lhs, T2&& rhs)
{
    return sol::make_object(L, impl::set_union(lhs, rhs));
}
};

using union_ = set_union;


struct intersection : impl::operator_base<intersection, impl::has_set_operation>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& lhs, T2&& rhs)
{
    return sol::make_object(L, impl::set_intersection(lhs, rhs));
}
};


struct difference : impl::operator_base<difference, impl::has_set_operation>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& lhs, T2&& rhs)
{
    return sol::make_object(L, impl::set_difference(lhs, rhs));
}
};


struct symmetric_difference : impl::operator_base<symmetric_difference, impl::has_set_operation>
{
using operator_base::evaluate;

template <class T1, class T2>
static sol::object evaluate(std::true_type, lua_State* L, T1&& lhs, T2&& rhs)
{
    return sol::make_object(L, impl::set_symmetric_difference(lhs, rhs));
}
};
} // namespace op

}
