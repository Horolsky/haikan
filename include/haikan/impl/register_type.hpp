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

#include "haikan/impl/reflect.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/impl/register_type_fallback.hpp"


namespace haikan {
namespace impl {

template <class T, class Seen = mp_list<>, class = void>
struct register_type_impl;

namespace detail {
template <typename T> using register_type_callop_t = decltype(register_type_impl<T>::operator()(std::declval<sol::state_view>()));
}

template <typename T> using has_register_type_impl = boost::mp11::mp_valid<detail::register_type_callop_t, T>;


template <class T, class Seen = mp_list<>, class = void>
struct register_type;


template <class T, class Seen>
struct register_type<T, Seen, mp_if<has_register_type_impl<T>, void>>
    : register_type_impl<T, Seen>
{
};


template <class T, class Seen>
struct register_type<T, Seen, mp_if<mp_not<has_register_type_impl<T>>, void>> : register_type_fallback<T>
{
};

} // namespace impl
} // namespace haikan
