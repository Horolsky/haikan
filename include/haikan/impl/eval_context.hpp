/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <map>
#include <memory>
#include <deque>


#include <boost/json.hpp>
#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


#include "haikan/impl/eval_log.hpp"
#include "haikan/impl/lua_state.hpp"



namespace haikan {
namespace impl {

struct EvalContextLua
{
    static EvalContextLua make();

    /// Evaluation log
    EvalLog log;
    /// reference -> value map
    std::shared_ptr<boost::json::array> traces;
    std::shared_ptr<std::deque<boost::json::object>> capture_links;
    std::shared_ptr<std::map<boost::json::string, Expression>> expr_links;
    // boost::json::object captures;
    /// Evaluation stack depth
    std::uint64_t depth;

    EvalContextLua();

    /// Copy context with depth increment
    EvalContextLua operator++() const;
};


}  // namespace impl
}  // namespace haikan
