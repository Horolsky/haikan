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



HAIKAN_DEFINE_EVALUATE_IMPL(Op)
{
    auto const enc = self().encoding_view();
    ASSERT(enc.arity() == 2, "invalid parameters, expected [type, expression]");
    auto const operator_reference = ExpressionView(enc.child(0)).eval({}, curr_ctx());
    ExpressionView const F(enc.child(1));

    auto const if_str = operator_reference.if_string();
    ASSERT(if_str, "invalid parameter");
    EvalContext ctx = curr_ctx();
    ctx.op = O{*if_str};
    return F.eval_e(lhs(), ctx);
}

HAIKAN_DEFINE_EVALUATE_IMPL(Cast)
{
    auto const operator_reference = rhs().eval({}, curr_ctx());
    auto const if_str = operator_reference.if_string();
    ASSERT(if_str, "invalid parameter");
    auto const op = O{*if_str};
    return op.decorate(lhs().data());
}

} // namespace impl
} // namespace haikan

