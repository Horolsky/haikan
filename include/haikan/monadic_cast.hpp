/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/optional.hpp>

#include "haikan/cast.hpp"

namespace haikan {

template <class To>
class monadic_cast : public boost::optional<To>
{
  public:

    using boost::optional<To>::optional;
    using boost::optional<To>::operator=;


    template <class From>
    auto try_cast(From const& arg) const& -> boost::mp11::mp_if<impl::has_cast<From, To>, monadic_cast<To>>
    {
        monadic_cast<To> mc{};
        if (this->is_initialized())
        {
            mc = *this;
        }
        else
        {
            mc = monadic_cast<To>{cast<From, To>()(arg)};
        }
        return mc;
    }

    template <class From>
    auto try_cast(From const& arg) && -> boost::mp11::mp_if<impl::has_cast<From, To>, monadic_cast<To>>
    {
        monadic_cast<To> mc{};
        if (this->is_initialized())
        {
            mc = std::move(*this);
        }
        else
        {
            mc = monadic_cast<To>{cast<From, To>()(arg)};
        }
        return mc;
    }

    template <class From>
    auto try_cast(From const&) const& -> boost::mp11::mp_if<boost::mp11::mp_not<impl::has_cast<From, To>>, monadic_cast<To>>
    {
        return *this;
    }
    template <class From>
    auto try_cast(From const&) && -> boost::mp11::mp_if<boost::mp11::mp_not<impl::has_cast<From, To>>, monadic_cast<To>>
    {
        return std::move(*this);
    }
};

} // namespace haikan
