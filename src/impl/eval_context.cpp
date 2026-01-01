/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/expression.hpp"
#include "haikan/impl/eval_context.hpp"


namespace haikan {
namespace impl {

EvalContext EvalContext::make(Operator const& op)
{
    EvalContext next{};
    next.log = EvalLog::make();
    next.op = op;
    next.traces = std::make_shared<boost::json::array>();
    return next;
}
EvalContext::EvalContext()
    : op{}
    , log{}
    , traces{}
    , capture_links{std::make_shared<std::deque<boost::json::object>>()}
    , expr_links{std::make_shared<std::map<boost::json::string, ExpressionView>>()}
    , depth{0}
{
    capture_links->push_back({});
}

EvalContext EvalContext::operator++() const
{
    EvalContext next = *this;
    next.depth++;
    return next;
}



}  // namespace impl
}  // namespace haikan
