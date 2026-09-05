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

E if_continue(
    haikan::impl::ExpressionView const& arg,
    haikan::impl::ExpressionView const& F,
    haikan::impl::ExpressionView const& then_value,
    haikan::impl::EvalContext ctx
)
{
    auto keyword = K::_Continue;
    haikan::impl::Expression payload(arg);

    E sts;
    if (F.eval_as_predicate(payload, sts, ctx))
    {
        keyword = K::_Resolve;
        payload = then_value.eval_e(payload, ctx);
    }
    else if (sts.is_error())
    {
        payload = sts;
    }
    return haikan::impl::Expression::encodeNested(keyword, {payload});
}

} // namespace


namespace haikan {
namespace impl {


HAIKAN_DEFINE_EVALUATE_IMPL(If)
{
    auto const subexpressions = self().subexpressions_list();
    auto const N = subexpressions.size();
    ASSERT((N == 2) || (N == 3),
    "invalid parameters, expected (predicate, then_expr) or (predicate, then_expr, else_expr)");
    ExpressionView const& arg = x();
    ExpressionView const& F = subexpressions.at(0);
    ExpressionView const& then_expr = subexpressions.at(1);

    Expression const result = if_continue(arg, F, then_expr, curr_ctx());
    if (N == 3)
    {
        switch (result.keyword())
        {
        case K::_Resolve:
            return result.data();
        case K::_Continue:
            return subexpressions.at(2).eval_e(arg, curr_ctx()); // else_expr
        default:
            return result;
        }
    }
    return result;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Elif)
{
    if(x().is(Keyword::_Resolve))
    {
        return x();
    }
    ASSERT(x().is(Keyword::_Continue), "missing If");

    auto const subexpressions = self().subexpressions_list();

    ASSERT(subexpressions.size() == 2,
    "invalid parameters, expected (predicate, then_value)");
    Expression payload = x().data();
    return if_continue(payload, subexpressions.at(0), subexpressions.at(1), curr_ctx());
}

HAIKAN_DEFINE_EVALUATE_IMPL(Else)
{
    if(x().is(Keyword::_Resolve))
    {
        return x().data();
    }
    else if (x().is(Keyword::_Continue))
    {
        auto const view = x().encoding_view();
        if (view.child(1).head() == Keyword::Err)
        {
            return ExpressionView{view.subtree(1).freeze()};
        }
        return rhs().eval(x().data(), curr_ctx());
    }
    else
    {
        ASSERT(false, "missing If");
    }
    return nullptr;
}



} // namespace impl
} // namespace haikan
