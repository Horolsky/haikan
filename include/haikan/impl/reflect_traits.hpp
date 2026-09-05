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

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/reflect_default_fwd.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/reflection_context.hpp"



#define HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(method, args)                                                                     \
template <class T, class E = void> struct select_reflect_ ## method;                                                          \
namespace detail {                                                                                                            \
template <class T> using mfp_custom_ ## method = decltype(::haikan::custom_reflect<T>::method args);                          \
template <class T> using mfp_default_ ## method = decltype(::haikan::impl::reflect_default<T>::method args);}                 \
template <class T> using has_custom_reflect_ ## method = mp_valid<detail::mfp_custom_ ## method, T>;                          \
template <class T> using has_default_reflect_ ## method = mp_valid<detail::mfp_default_ ## method, T>;                        \
template <class T> using has_reflect_ ## method = mp_or<has_custom_reflect_ ## method<T>, has_default_reflect_ ## method<T>>; \
template <class T, class R = void> using enable_custom_reflect_ ## method  = mp_if<has_custom_reflect_ ## method<T>, R>;      \
template <class T, class R = void> using enable_default_reflect_ ## method =                                                  \
    mp_if<mp_and<has_default_reflect_ ## method<T>, mp_not<has_custom_reflect_ ## method<T>>>, R>;                            \
template <class T, class R = void> using enable_missing_reflect_ ## method = mp_if<mp_not<has_reflect_ ## method<T>>, R>;



namespace haikan {

template <class T> struct reflect;
template <class T, class E = void> struct custom_reflect;

// class ReflectionContextFactory;
namespace impl {

HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(utype, (std::declval<::haikan::ReflectionContextFactory&>()))
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(init, ())
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(serialize, (std::declval<T const&>(), std::declval<sol::state_view>()))
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(deserialize, (std::declval<sol::object const&>()))

#undef HAIKAN_DEFINE_REFLECT_METHOD_SFINAE




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
