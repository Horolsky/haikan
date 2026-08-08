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

sol::object OperatorHandler::generic_decorate(LazyLuaObject lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_equal_to(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_less(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_less_equal(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_left_shift(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_right_shift(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_negate(LazyLuaObject lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_complement(LazyLuaObject lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_logical_and(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_logical_or(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_plus(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_minus(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_multiplies(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}


sol::object OperatorHandler::generic_modulus(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_bit_and(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_bit_or(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_bit_xor(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_divides(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_pow(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_log(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_quot(LazyLuaObject llhs, LazyLuaObject lrhs) const
{
    std::ignore = llhs = lrhs;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_logical_not(LazyLuaObject lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}

sol::object OperatorHandler::generic_is_truth(LazyLuaObject lx) const
{
    std::ignore = lx;
    RETURN_ERROR("not implemented")
}


} // namespace impl
} // namespace haikan
