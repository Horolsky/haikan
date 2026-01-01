/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include "haikan/impl/expression.hpp"
#include "haikan/impl/keyword.hpp"


namespace haikan {
namespace impl {

template <Keyword K>
struct EvalValidator
{

    EvalValidator(ExpressionView const& lhs, ExpressionView const& rhs) {}

    bool is_invalid() const
    {
        return false;
    }

    Expression status() const
    {
        return Expression();
    }
};

}  // namespace impl
}  // namespace haikan
