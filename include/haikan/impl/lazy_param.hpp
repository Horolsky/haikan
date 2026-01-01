/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <functional>
#include <boost/optional.hpp>

#include <boost/json.hpp>


namespace haikan {
namespace impl {

class ExpressionView;
class EvalContext;

class LazyParam
{
  public:
    using V = boost::json::value;

    LazyParam();

    LazyParam(std::function<V()> getter);

    LazyParam(ExpressionView, EvalContext);

    LazyParam(V && v);

    LazyParam(V const& v);

    LazyParam(std::reference_wrapper<V const> v);

    template <class T>
    LazyParam(T const& v)
        : LazyParam(boost::json::value_from(v))
    {
    }

    ~LazyParam() = default;

    LazyParam(LazyParam const&) = default;
    LazyParam(LazyParam &&) = default;
    LazyParam& operator=(LazyParam const&) = default;
    LazyParam& operator=(LazyParam &&) = default;

    [[nodiscard]] V operator()() const;

    operator V() const;

  private:

    std::function<V()> getter_;
    mutable boost::optional<V> cache_;
};

} // namespace impl
} // namespace haikan
