/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/traits.hpp"
#include "haikan/reflection_context.hpp"



#define HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(method, args)                                                                     \
template <class T, class E = void> struct select_reflect_ ## method;                                                          \
namespace detail {                                                                                                            \
template <class T> using mfp_custom_ ## method = decltype(::haikan::custom_reflect<T>::method args);                          \
template <class T> using mfp_default_ ## method = decltype(::haikan::impl::default_reflect<T>::method args);}                 \
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

// class ReflectionContext;
namespace impl {

template <class T, class E = void> struct default_reflect;
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(utype, (std::declval<::haikan::ReflectionContext&>()))
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(init, ())
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(serialize, (std::declval<T const&>(), std::declval<sol::state_view>()))
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(deserialize, (std::declval<sol::object const&>()))

#undef HAIKAN_DEFINE_REFLECT_METHOD_SFINAE


} // namespace impl
} // namespace haikan
