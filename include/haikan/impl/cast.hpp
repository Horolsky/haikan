/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>
#include <boost/mp11.hpp>
#include <boost/json.hpp>

#include "haikan/impl/traits.hpp"
#include "haikan/decorators/underlying.hpp"


namespace haikan {

template <class To, class From, class = void>
struct cast;

template <class To, class From, class = void>
struct custom_cast;

namespace impl
{
template <class To, class From, class = void>
struct default_cast;


namespace detail
{
template <class To, class From>
using custom_cast_valid = decltype(&custom_cast<To, From>::operator());

template <class To, class From>
using default_cast_valid = decltype(&default_cast<To, From>::operator());
} // namespace detail

template <class To, class From>
using has_custom_cast = mp_valid<detail::custom_cast_valid, To, From>;

template <class To, class From>
using has_default_cast = mp_valid<detail::default_cast_valid, To, From>;

template <class To, class From>
using has_cast = mp_or<has_custom_cast<To, From>, has_default_cast<To, From>>;

template <class To, class From, class R = void>
using enable_custom_cast = mp_if<has_custom_cast<To, From>, R>;

template <class To, class From, class R = void>
using enable_default_cast = mp_if<mp_and<
    has_default_cast<To, From>,
    mp_not<has_custom_cast<To, From>>
>, R>;

template <class To, class From, class R = void>
using enable_missing_cast = mp_if<mp_not<has_cast<To, From>>, R>;


template<class F>
struct switch_cast_fn;

template<class To, class From>
struct switch_cast_fn<To(From)> : cast<To, From> {};

template<class F>
struct switch_cast_guard;

template<class To, class From>
struct switch_cast_guard<To(From)> : has_cast<To, From> {};


template <>
struct default_cast<void, void> {};



// template <class To, class From>
// struct default_cast<To, From, mp_if<std::is_reference<From>, void>> : cast<To, std::remove_reference_t<From>> {};

// template <class To, class From>
// struct default_cast<To, From, mp_if<std::is_const<From>, void>> : cast<To, std::remove_const_t<From>> {};

template <class To, class From>
struct default_cast<To, From, mp_if<std::is_convertible<From, To>, void>>
{
    auto operator()(remove_qualifiers_t<From> const& v) -> To
    {
        return static_cast<To>(v);
    }
};

template <class To, class From>
struct default_cast<To, From, mp_if<mp_and<
    std::is_enum<remove_qualifiers_t<From>>,
    std::is_integral<To>
>, void>>
{
    auto operator()(remove_qualifiers_t<From> const& v) -> To
    {
        using U = std::underlying_type_t<remove_qualifiers_t<From>>;
        return cast<To, U>()(static_cast<U>(v));
    }
};

template <class To, class From>
struct default_cast<To, From, mp_if<mp_and<
    std::is_enum<To>,
    std::is_integral<remove_qualifiers_t<From>>
>, void>>
{
    auto operator()(remove_qualifiers_t<From> const& v) -> To
    {
        using U = std::underlying_type_t<To>;
        return static_cast<To>(cast<U, remove_qualifiers_t<From>>()(v));
    }
};


template <class To, class From>
struct default_cast<To, From, mp_if<mp_and<
    std::is_same<remove_qualifiers_t<From>, boost::json::value>,
    mp_not<std::is_convertible<boost::json::value, To>>,
    boost::json::has_value_to<To>>, void>>
{
    auto operator()(boost::json::value const& v) -> To
    {
        return boost::json::value_to<To>(v);
    }
};


template <class From>
struct default_cast<boost::json::value, From, mp_if<mp_and<
    mp_not<std::is_convertible<From, boost::json::value>>,
    boost::json::has_value_from<From>>, void>>
{
    auto operator()(remove_qualifiers_t<From> const& v) -> boost::json::value
    {
        return boost::json::value_from(v);
    }
};

// template <class To>
// struct default_cast<To, boost::json::value const> 

// template <class To, class From>
// struct default_cast<To, From, mp_if<std::is_reference<From>, void>> : ::haikan::cast<To, std::remove_reference_t<From>> {};

// template <class To, class From>
// struct default_cast<To, From, mp_if<std::is_const<From>, void>> : ::haikan::cast<To, std::remove_const_t<From>> {};

} // namespace impl
} // namespace haikan