/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/expression_lua.hpp"
#include "haikan/impl/eval_context.hpp"


namespace haikan {
namespace impl {


EvalContextLua EvalContextLua::make()
{
    EvalContextLua next{};
    next.log = EvalLog::make();
    next.traces = std::make_shared<boost::json::array>();
    return next;
}
EvalContextLua::EvalContextLua()
    : log{}
    , traces{}
    , capture_links{std::make_shared<std::deque<boost::json::object>>()}
    , expr_links{std::make_shared<std::map<boost::json::string, ExpressionLua>>()}
    , depth{0}
{
    capture_links->push_back({});
}

EvalContextLua EvalContextLua::operator++() const
{
    EvalContextLua next = *this;
    next.depth++;
    return next;
}



}  // namespace impl
}  // namespace haikan
