/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>

#include <boost/current_function.hpp>

#include "haikan/impl/traits.hpp"
#include "haikan/impl/operator_handler.hpp"
#include "haikan/impl/real_to_number.hpp"
#include "haikan/impl/tag_invoke_complex.hpp"
#include "haikan/impl/error_expr.hpp"



#define RETURN_ERROR(msg) return make_error(msg, BOOST_CURRENT_FUNCTION);



namespace haikan {
namespace impl {

sol::object OperatorHandler::generic_decorate(ExpressionParameter lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_equal_to(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_less(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_less_equal(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_left_shift(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_right_shift(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_negate(ExpressionParameter lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_complement(ExpressionParameter lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_logical_and(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_logical_or(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_plus(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_minus(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_multiplies(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}


sol::object OperatorHandler::generic_modulus(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_bit_and(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_bit_or(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_bit_xor(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_divides(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_pow(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_log(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_quot(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_logical_not(ExpressionParameter lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_is_truth(ExpressionParameter lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}


} // namespace impl
} // namespace haikan
