/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/optional.hpp>

#include "haikan/impl/cast.hpp"

namespace haikan {

template <class From, class To>
struct cast<From, To, impl::enable_default_cast<From, To>>
{
    auto operator()(From const& v) -> To
    {
        return impl::default_cast<From, To>()(v);
    }
};

template <class From, class To>
struct cast<From, To, impl::enable_custom_cast<From, To>>
{
    auto operator()(From const& v) -> To
    {
        return custom_cast<From, To>()(v);
    }
};

template <class From, class To>
struct cast<From, To, impl::enable_missing_cast<From, To>>
{
    auto operator()(From const& v) -> To
    {
        static_assert(
            std::is_same<From, To>::value && false,
            "No custom_cast defined for (From const&) -> To"
        );
        return {};
    }
};

} // namespace haikan
