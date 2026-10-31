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
#include "haikan/logger.hpp"

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


HAIKAN_DEFINE_EVALUATE_IMPL(Dbg)
{
    EvalContext local_ctx {};
    local_ctx.op = curr_ctx().op;
    local_ctx.log = EvalLog::make();
    local_ctx.capture_links = curr_ctx().capture_links;
    local_ctx.expr_links = curr_ctx().expr_links;

    auto const result = rhs().eval_e(lhs(), local_ctx);

    HAIKAN_LOG_JSON(INFO).WithSrcLoc("haikan::Dbg") << *local_ctx.log.stack;
    HAIKAN_LOG_CERR(DEBUG).WithSrcLoc("haikan::Dbg") << "\n" << local_ctx.log.str(2);

    if (curr_ctx().log.stack)
    {
        curr_ctx().log.stack->reserve(curr_ctx().log.stack->capacity() + local_ctx.log.stack->size());
        for (auto line_it = std::make_move_iterator(local_ctx.log.stack->begin()),
        log_end = std::make_move_iterator(local_ctx.log.stack->end());
        line_it != log_end; ++line_it)
        {
            line_it->as_array().at(0).as_uint64() += (curr_ctx().depth);
            curr_ctx().log.stack->push_back(*line_it);
        }
    }
    return result;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Trace)
{
    auto const trace_id = rhs().eval_e({}, curr_ctx()).prettify();
    HAIKAN_LOG(INFO).WithSrcLoc("haikan::Trace") << trace_id << " " << lhs().data();
    if( auto traces = curr_ctx().traces)
    {
        traces->push_back(boost::json::object{{trace_id, lhs().data()}});
    }
    return lhs();
}

HAIKAN_DEFINE_EVALUATE_IMPL(Assert)
{
    Expression err_sts(nullptr);

    if (rhs().eval_as_predicate(lhs(), err_sts, curr_ctx()))
    {
        return lhs();
    }
    else if(err_sts.is_error())
    {
        return err_sts;
    }
    else
    {
        return Err("assertion failure", rhs().prettify());
    }
}

} // namespace impl
} // namespace haikan

