/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include <boost/mp11.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/traits.hpp"
namespace haikan {
namespace impl {
template <class T, class = void>
struct has_type_key_type : std::false_type {};

template <class T>
struct has_type_key_type<T, void_t<typename T::key_type>> : std::true_type {};

template <class T, class = void>
struct has_member_reserve : std::false_type {};

template <class T>
struct has_member_reserve<T, void_t<decltype(std::declval<T&>().reserve(std::declval<std::size_t>()))>> : std::true_type {};

template <class T, class = void>
struct has_member_push_back : std::false_type {};

template <class T>
struct has_member_push_back<T, void_t<decltype(std::declval<T&>().push_back(std::declval<typename T::value_type>()))>>
    : std::true_type {};

template <class T, class = void>
struct has_member_insert_value : std::false_type {};

template <class T>
struct has_member_insert_value<T, void_t<decltype(std::declval<T&>().insert(std::declval<typename T::value_type>()))>>
    : std::true_type {};

template <class T, class = void>
struct has_subscript : std::false_type {};

template <class T>
struct has_subscript<T, void_t<decltype(std::declval<T&>()[std::declval<std::size_t>()])>> : std::true_type {};

template <class T>
struct is_string_like : std::is_convertible<T, std::string> {};

template <class T>
using is_array_like_container = mp_and<
    sol::is_container<T>,
    mp_not<has_type_key_type<T>>,
    mp_not<is_string_like<T>>
>;



} // namespace impl
} // namespace haikan
