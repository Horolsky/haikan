/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>
#include <boost/optional.hpp>



namespace haikan {
namespace impl {


template <class T, class = void>
struct default_reflect_init;

template <class T>
struct default_reflect_init<T, std::enable_if_t<std::is_default_constructible<T>::value>>
{
    static boost::optional<T> init()
    {
        return T{};
    }
};

template <class T>
struct default_reflect_init<T, std::enable_if_t<not std::is_default_constructible<T>::value>>
{
    static boost::optional<T> init()
    {
        return boost::none;
    }
};

} // namespace impl
} // namespace haikan
