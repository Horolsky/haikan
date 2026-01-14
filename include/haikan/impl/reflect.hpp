/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/traits.hpp"


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

namespace impl {

template <class T, class E = void> struct default_reflect;

HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(init, ())
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(desolify, (std::declval<sol::object const&>()))
HAIKAN_DEFINE_REFLECT_METHOD_SFINAE(solify, (std::declval<T const&>(), std::declval<sol::state_view>()))

#undef HAIKAN_DEFINE_REFLECT_METHOD_SFINAE

/// select_reflect_init

template <class T>
struct select_reflect_init<T, enable_custom_reflect_init<T>> {
    static boost::optional<T> init() { return ::haikan::custom_reflect<T>::init(); }
};

template <class T>
struct select_reflect_init<T, enable_default_reflect_init<T>> {
    static boost::optional<T> init() { return ::haikan::impl::default_reflect<T>::init(); }
};

template <class T>
struct select_reflect_init<T, enable_missing_reflect_init<T>> {
    static boost::optional<T> init() {
        static_assert(failing_on<T>, "No custom_reflect<T>::init defined");
        return {};
    };
};


/// select_reflect_solify

template <class T>
struct select_reflect_solify<T, enable_custom_reflect_solify<T>> {
    static sol::object solify(T const& value, sol::state_view L) { return ::haikan::custom_reflect<T>::solify(value, L); }
    static sol::object solify(T && value, sol::state_view L) { return ::haikan::custom_reflect<T>::solify(std::move(value), L); }
};

template <class T>
struct select_reflect_solify<T, enable_default_reflect_solify<T>> {
    static sol::object solify(T const& value, sol::state_view L) { return ::haikan::impl::default_reflect<T>::solify(value, L); }
    static sol::object solify(T && value, sol::state_view L) { return ::haikan::impl::default_reflect<T>::solify(std::move(value), L); }
};

template <class T>
struct select_reflect_solify<T, enable_missing_reflect_solify<T>> {
    static sol::object solify(T const&, sol::state_view) {
        static_assert(failing_on<T>, "No custom_reflect<T>::solify defined");
        return {};
    };
};


/// select_reflect_desolify

template <class T>
struct select_reflect_desolify<T, enable_custom_reflect_desolify<T>> {
    static boost::optional<T> desolify(sol::object const& value) { return ::haikan::custom_reflect<T>::desolify(value); }
};

template <class T>
struct select_reflect_desolify<T, enable_default_reflect_desolify<T>> {
    static boost::optional<T> desolify(sol::object const& value) { return ::haikan::impl::default_reflect<T>::desolify(value); }
};

template <class T>
struct select_reflect_desolify<T, enable_missing_reflect_desolify<T>> {
    static boost::optional<T> desolify(sol::object const&) {
        static_assert(failing_on<T>, "No custom_reflect<T>::desolify defined");
        return {};
    };
};

} // namespace impl
} // namespace haikan

#include "haikan/impl/reflect_default_describe.hpp"  // IWYU pragma: keep
