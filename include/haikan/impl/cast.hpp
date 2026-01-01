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

template <class From, class To, class = void>
struct cast;

template <class From, class To, class = void>
struct custom_cast;

namespace impl
{
template <class From, class To, class = void>
struct default_cast;


namespace detail
{
template<class From, class To>
using custom_cast_valid = decltype(&custom_cast<From, To>::operator());

template<class From, class To>
using default_cast_valid = decltype(&default_cast<From, To>::operator());
} // namespace detail

template<class From, class To>
using has_custom_cast = mp_valid<detail::custom_cast_valid, From, To>;

template<class From, class To>
using has_default_cast = mp_valid<detail::default_cast_valid, From, To>;

template<class From, class To>
using has_cast = mp_or<has_custom_cast<From, To>, has_default_cast<From, To>>;

template <class From, class To, class R = void>
using enable_custom_cast = mp_if<has_custom_cast<From, To>, R>;

template <class From, class To, class R = void>
using enable_default_cast = mp_if<mp_and<
    has_default_cast<From, To>,
    mp_not<has_custom_cast<From, To>>
>, R>;

template <class From, class To, class R = void>
using enable_missing_cast = mp_if<mp_not<has_cast<From, To>>, R>;

template <>
struct default_cast<void, void> {};

template <class From, class To>
struct default_cast<From, To, mp_if<std::is_convertible<From, To>, void>>
{
    auto operator()(From const& v) -> To
    {
        return static_cast<To>(v);
    }
};

template <class From, class To>
struct default_cast<From, To, mp_if<mp_and<
    std::is_enum<From>,
    std::is_integral<To>
>, void>>
{
    auto operator()(From const& v) -> To
    {
        using U = std::underlying_type_t<From>;
        return cast<U, To>()(static_cast<U>(v));
    }
};

template <class From, class To>
struct default_cast<From, To, mp_if<mp_and<
    std::is_enum<To>,
    std::is_integral<From>
>, void>>
{
    auto operator()(From const& v) -> To
    {
        using U = std::underlying_type_t<To>;
        return static_cast<To>(cast<From, U>()(v));
    }
};


template <class To>
struct default_cast<boost::json::value, To, mp_if<mp_and<
    mp_not<std::is_convertible<boost::json::value, To>>,
    boost::json::has_value_to<To>>, void>>
{
    auto operator()(boost::json::value const& v) -> To
    {
        return boost::json::value_to<To>(v);
    }
};


template <class From>
struct default_cast<From, boost::json::value, mp_if<mp_and<
    mp_not<std::is_convertible<boost::json::value, From>>,
    boost::json::has_value_from<From>>, void>>
{
    auto operator()(From const& v) -> boost::json::value
    {
        return boost::json::value_from(v);
    }
};

} // namespace impl
} // namespace haikan