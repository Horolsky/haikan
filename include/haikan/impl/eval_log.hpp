/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <ostream>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/expression.hpp"

namespace haikan {
namespace impl {

/// Expression evaluation log
struct EvalLog
{
    struct Record
    {
        Expression expr;
        sol::object x;
        sol::object result;
        std::size_t depth;
    };

    mutable std::shared_ptr<std::vector<Record>> stack;

    /// Default instance with null log stack
    EvalLog() = default;

    /// Stringify log
    std::string str(int const indent = 0) const;

    /// Push record to log stack
    void push(Expression const& expr, sol::object const& x, sol::object const& result, std::uint64_t const depth) const;


    static void format(std::ostream& os, std::vector<Record> const& log, int const indent = 0);

    friend std::ostream& operator<<(std::ostream& os, EvalLog const& log);

    /// Make non-empty EvalLog
    static EvalLog make();
};

}  // namespace impl
}  // namespace haikan
