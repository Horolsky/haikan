/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */

#include <limits>
#include <boost/math/constants/constants.hpp>

#include "haikan/impl/operator.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/keywords.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/impl/eval_impl.hpp"
#include "haikan/impl/eval_impl_pp.hpp"

namespace
{
using V = boost::json::value;
using L = boost::json::array;
using E = haikan::impl::Expression;
using K = haikan::impl::Keyword;
using O = haikan::impl::Operator;

} // namespace


namespace haikan {
namespace impl {

HAIKAN_DEFINE_EVALUATE_IMPL(Fold)
{
    auto const if_arr = lhs().if_array();
    ASSERT(if_arr, "invalid argument")
    auto const& samples = *if_arr;
    if (samples.empty())
    {
        return nullptr;
    }
    auto const& F = rhs();
    auto it = samples.cbegin();

    // take init term
    boost::json::value ret(*it++);

    while (it != samples.cend())
    {
        ret = F.eval({ret, *it},curr_ctx());
        it++;
    }

    return ret;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Sum)
{
    ASSERT(lhs().if_array(), "invalid argument");
    return Fold(Add).eval_e(lhs(), curr_ctx());
}

HAIKAN_DEFINE_EVALUATE_IMPL(Prod)
{
    ASSERT(lhs().if_array(), "invalid argument");
    return Fold(Mul).eval_e(lhs(), curr_ctx());
}

HAIKAN_DEFINE_EVALUATE_IMPL(Avg)
{
    auto const if_arr = lhs().if_array();
    ASSERT(if_arr, "invalid argument");

    auto const N = if_arr->size();
    auto const sum = Fold(Add).eval_e(lhs(), curr_ctx()).to_json();
    return curr_ctx().op.apply(Keyword::Div, sum, N);
}

} // namespace impl
} // namespace haikan
