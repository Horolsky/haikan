/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <ostream>
#include <boost/json.hpp>

namespace haikan {
namespace impl {

class ExpressionLua;

/// Expression evaluation log
struct EvalLog
{
    mutable std::shared_ptr<boost::json::array> stack;

    /// Default instance with null log stack
    EvalLog() = default;

    /// Stringify log
    boost::json::string str(int const indent = 0) const;

    /// Push record to log stack
    void push(ExpressionLua const& expr, ExpressionLua const& x, ExpressionLua const& result, std::uint64_t const depth) const;


    static void format(std::ostream& os, boost::json::array const& log, int const indent = 0);

    friend std::ostream& operator<<(std::ostream& os, EvalLog const& log);

    /// Make non-empty EvalLog
    static EvalLog make();
};

}  // namespace impl
}  // namespace haikan
