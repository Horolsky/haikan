/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>
#include <functional>
#include <boost/optional.hpp>


namespace haikan {

/// @brief SFINAE-guarded monadic dispatcher for function object template F
///
/// @details
/// A SFINAE-guarded first-match monadic switch over generic call operator chain.
///
/// The generic call operator instantiates the F<R(T...)> functor only if
/// the domain guard G<R(T...)>::value is satisfied, otherwise, the object is
/// propagated unchanged.
///
/// Then, if the value is uninitialized, the arguments are forwarded to the
/// F<R(T...)> call operator and result is and stored as boost::optional.
///
/// Once a value is initialized, all subsequent calls propagate
/// the existing value unchanged.
///
/// @tparam R result type.
/// @tparam G domain guard template for R(T...) parameter.
/// @tparam F function object template for R(T...) parameter.
template <class R, template <class> class G, template <class> class F>
class sfinae_switch : public boost::optional<R>
{
  public:

    using this_type = sfinae_switch<R, G, F>;
    using value_type = R;

    template <class... T>
    using functor = F<value_type(T...)>;

    template <class... T>
    using domain_guard = G<value_type(T...)>;

    template <class... T>
    static constexpr bool is_valid_domain = domain_guard<T...>::value;

    using boost::optional<value_type>::optional;
    using boost::optional<value_type>::operator=;

    template <class... T>
    auto operator()(T&&... args) const& -> std::enable_if_t<is_valid_domain<T...>, this_type>
    {
        this_type mc{};
        if (this->is_initialized())
        {
            mc = *this;
        }
        else
        {
            mc = this_type{functor<T...>()(std::forward<T>(args)...)};
        }
        return mc;
    }

    template <class... T>
    auto operator()(T&&... args) && -> std::enable_if_t<is_valid_domain<T...>, this_type>
    {
        this_type mc{};
        if (this->is_initialized())
        {
            mc = std::move(*this);
        }
        else
        {
            mc = this_type{functor<T...>()(std::forward<T>(args)...)};
        }
        return mc;
    }

    template <class... T>
    auto operator()(T&&... args) const& -> std::enable_if_t<not is_valid_domain<T...>, this_type>
    {
        return {*this};
    }

    template <class... T>
    auto operator()(T&&... args) && -> std::enable_if_t<not is_valid_domain<T...>, this_type>
    {
        return {std::move(*this)};
    }
};

} // namespace haikan
