/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/generic_operand.hpp"
#include "haikan/impl/operators.hpp"


namespace haikan {
namespace impl {

namespace
{

template <class F>
GenericOperand evaluate_nil(F, GenericOperand const& lhs, GenericOperand const&)
{
    return sol::make_object(lhs.lua_state(), error("invalid operands", type_name<F>()));
};

GenericOperand evaluate_nil(op::equal_to, GenericOperand const& lhs, GenericOperand const& rhs)
{
    return sol::make_object(lhs.lua_state(), lhs.is<sol::nil_t>() && rhs.is<sol::nil_t>());
};

GenericOperand evaluate_nil(op::not_equal_to, GenericOperand const& lhs, GenericOperand const& rhs)
{
    return sol::make_object(lhs.lua_state(), !(lhs.is<sol::nil_t>() && rhs.is<sol::nil_t>()));
};

template <class F>
GenericOperand evaluate(F&& f, GenericOperand const& lhs, GenericOperand const& rhs)
{
    if (HAIKAN_UNLIKELY((!lhs.valid() || !rhs.valid())))
    {
        return evaluate_nil(f, lhs, rhs);
    }

    auto log = HAIKAN_LOG(DEBUG)
        << type_name<F>() << "()(" << lhs << ", "  << rhs << ") -> ";

    sol::object result;
    switch (lhs.get_type())
    {
    case sol::type::nil:   result = f(lhs.lua_state(), sol::nil, sol::nil); break;
    case sol::type::number: result = f(lhs.lua_state(), lhs.as<double>(), rhs.as<double>()); break;
    case sol::type::string: result = f(lhs.lua_state(), lhs.as<std::string>(), rhs.as<std::string>()); break;
    case sol::type::boolean: result = f(lhs.lua_state(), lhs.as<bool>(), rhs.as<bool>()); break;
    case sol::type::table: result = f(lhs.lua_state(), lhs.as<sol::table>(), rhs.as<sol::table>());

    default:
        result = sol::make_object(lhs.lua_state(), error("invalid operands", BOOST_CURRENT_FUNCTION)); break;
    }
    log << result;
    return GenericOperand{result};
}

template <class F>
GenericOperand evaluate(F&& f, GenericOperand const& arg)
{
    switch (arg.get_type())
    {
    case sol::type::number: return f(arg.lua_state(), arg.as<double>());
    case sol::type::string: return f(arg.lua_state(), arg.as<std::string>());
    case sol::type::boolean: return f(arg.lua_state(), arg.as<bool>());
    case sol::type::table: return f(arg.lua_state(), arg.as<sol::table>());

    default:
        return sol::make_object(arg.lua_state(), error("invalid operands", BOOST_CURRENT_FUNCTION));
    }
}
} // namespace


GenericOperand operator+(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::plus()         , lhs, rhs); }
GenericOperand operator-(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::minus()        , lhs, rhs); }
GenericOperand operator*(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::multiply()    , lhs, rhs); }
GenericOperand operator/(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::divide()       , lhs, rhs); }
GenericOperand operator==(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::equal_to()     , lhs, rhs); }
GenericOperand operator!=(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::not_equal_to() , lhs, rhs); }
GenericOperand operator<(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::less()         , lhs, rhs); }
GenericOperand operator>(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::greater()      , lhs, rhs); }
GenericOperand operator<=(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::less_equal()   , lhs, rhs); }
GenericOperand operator>=(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::greater_equal(), lhs, rhs); }
GenericOperand operator&&(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::logical_and()  , lhs, rhs); }
GenericOperand operator||(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::logical_or()   , lhs, rhs); }
GenericOperand operator&(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::bit_and()      , lhs, rhs); }
GenericOperand operator|(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::bit_or()       , lhs, rhs); }
GenericOperand operator^(GenericOperand const& lhs, GenericOperand const& rhs)  { return evaluate(op::bit_xor()      , lhs, rhs); }
GenericOperand operator<<(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::left_shift()   , lhs, rhs); }
GenericOperand operator>>(GenericOperand const& lhs, GenericOperand const& rhs) { return evaluate(op::right_shift()  , lhs, rhs); }

GenericOperand operator~(GenericOperand const& arg) { return evaluate(op::complement(), arg); }
GenericOperand operator!(GenericOperand const& arg) { return evaluate(op::logical_not(), arg); }
GenericOperand operator-(GenericOperand const& arg) { return evaluate(op::negate(), arg); }



} // namespace impl
} // namespace haikan
