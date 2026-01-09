/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/optional.hpp>

#include "haikan/impl/cast.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/sfinae_switch.hpp"

namespace haikan {

template <class To, class From>
struct cast<To, From, impl::enable_default_cast<To, From>>
{
    auto operator()(From const& v) -> To
    {
        return impl::default_cast<To, From>()(v);
    }
};

template <class To, class From>
struct cast<To, From, impl::enable_custom_cast<To, From>>
{
    auto operator()(From const& v) -> To
    {
        return custom_cast<To, From>()(v);
    }
};

template <class To, class From>
struct cast<To, From, impl::enable_missing_cast<To, From>>
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

template <class To>
using switch_cast = sfinae_switch<To, impl::switch_cast_guard, impl::switch_cast_fn>;


} // namespace haikan
