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

#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/reflect_default.hpp"

namespace haikan {
namespace impl {


template <class T>
struct select_reflect_utype<T, enable_custom_reflect_utype<T>> {
    static void utype(ReflectionContext& ctx) { ::haikan::custom_reflect<T>::utype(ctx); }
};

template <class T>
struct select_reflect_utype<T, enable_default_reflect_utype<T>> {
    static void utype(ReflectionContext& ctx) { ::haikan::impl::default_reflect<T>::utype(ctx); }
};

template <class T>
struct select_reflect_utype<T, enable_missing_reflect_utype<T>> {
    static void utype(ReflectionContext&) {
        static_assert(failing_on<T>, "No custom_reflect<T>::utype defined");
    };
};



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



template <class T>
struct select_reflect_serialize<T, enable_custom_reflect_serialize<T>> {
    static sol::object serialize(T const& value, sol::state_view L) { return ::haikan::custom_reflect<T>::serialize(value, L); }
    static sol::object serialize(T && value, sol::state_view L) { return ::haikan::custom_reflect<T>::serialize(std::move(value), L); }
};

template <class T>
struct select_reflect_serialize<T, enable_default_reflect_serialize<T>> {
    static sol::object serialize(T const& value, sol::state_view L) { return ::haikan::impl::default_reflect<T>::serialize(value, L); }
    static sol::object serialize(T && value, sol::state_view L) { return ::haikan::impl::default_reflect<T>::serialize(std::move(value), L); }
};

template <class T>
struct select_reflect_serialize<T, enable_missing_reflect_serialize<T>> {
    static sol::object serialize(T const&, sol::state_view) {
        static_assert(failing_on<T>, "No custom_reflect<T>::serialize defined");
        return {};
    };
};



template <class T>
struct select_reflect_deserialize<T, enable_custom_reflect_deserialize<T>> {
    static boost::optional<T> deserialize(sol::object const& value) { return ::haikan::custom_reflect<T>::deserialize(value); }
};

template <class T>
struct select_reflect_deserialize<T, enable_default_reflect_deserialize<T>> {
    static boost::optional<T> deserialize(sol::object const& value) { return ::haikan::impl::default_reflect<T>::deserialize(value); }
};

template <class T>
struct select_reflect_deserialize<T, enable_missing_reflect_deserialize<T>> {
    static boost::optional<T> deserialize(sol::object const&) {
        static_assert(failing_on<T>, "No custom_reflect<T>::deserialize defined");
        return {};
    };
};


} // namespace impl
} // namespace haikan
