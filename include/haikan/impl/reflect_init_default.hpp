/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <type_traits>

#include <boost/optional.hpp>



namespace haikan {
namespace impl {

template <class T>
struct is_reference_wrapper : std::false_type {};

template <class T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};


template <class T, class = void>
struct reflect_init_default;

template <class T>
struct reflect_init_default<T, std::enable_if_t<std::is_default_constructible<T>::value && !is_reference_wrapper<T>::value>>
{
    static boost::optional<T> init()
    {
        return T{};
    }
};

template <class T>
struct reflect_init_default<T, std::enable_if_t<!std::is_default_constructible<T>::value && !is_reference_wrapper<T>::value>>
{
    static boost::optional<T> init()
    {
        return boost::none;
    }
};

template <class T>
struct reflect_init_default<std::reference_wrapper<T>, void>
{
    static boost::optional<std::reference_wrapper<T>> init()
    {
        return boost::none;
    }
};

} // namespace impl
} // namespace haikan
