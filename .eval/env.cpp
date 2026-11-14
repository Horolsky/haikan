/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */


#include "haikan/impl/global_env.hpp"
#include "haikan/impl/operator.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/keywords.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/impl/eval_impl.hpp"
#include "haikan/impl/eval_impl_pp.hpp"



namespace haikan {
namespace impl {


HAIKAN_DEFINE_EVALUATE_IMPL(EnvLoad)
{
    auto const rhs_eval = rhs().eval_e({}, curr_ctx());
    auto const if_str_key = rhs_eval.if_string();
    ASSERT(if_str_key, "reference is not a string");

    return GlobalEnv().Load(*if_str_key);
}


HAIKAN_DEFINE_EVALUATE_IMPL(EnvStore)
{
    auto const rhs_eval = rhs().eval_e({}, curr_ctx());
    auto const if_str_key = rhs_eval.if_string();
    ASSERT(if_str_key, "reference is not a string");

    return GlobalEnv().Store(*if_str_key, lhs().data());
}

} // namespace impl
} // namespace haikan

