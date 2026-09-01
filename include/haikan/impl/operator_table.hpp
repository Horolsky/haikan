/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <type_traits>
#include <typeinfo>
#include <boost/mp11.hpp>
#include <boost/type_traits.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/error.hpp"
#include "haikan/impl/operators.hpp"
#include "haikan/impl/get_or_create_table.hpp"
#include "haikan/impl/type_tag.hpp"
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/expression_parameter.hpp"


namespace haikan {
namespace impl {


class OperatorTable
{
    using unary_transform = std::function<sol::object(sol::object)>;
    using binary_transform = std::function<sol::object(sol::object, sol::object)>;
    struct Handle {

        std::string annotation{""};

        struct C {
            binary_transform equal_to{};
            binary_transform less{};
            binary_transform less_equal{};
            binary_transform greater{};
            binary_transform greater_equal{};
        } comp;

        struct A {
            unary_transform neg{};
            binary_transform add{};
            binary_transform sub{};
            binary_transform mul{};
            binary_transform div{};
            binary_transform mod{};
        } arithmetics;

        struct B {
            unary_transform compl_{};
            binary_transform and_{};
            binary_transform or_{};
            binary_transform xor_{};
        } bitwise;

        struct S {
            binary_transform left{};
            binary_transform right{};
        } shift;

        struct L {
            unary_transform  bool_{};
            unary_transform  not_{};
            binary_transform and_{};
            binary_transform or_{};
        } logic;
    };

    template <class...> struct operands {};

    template <class T1, class T2>
    static Handle makeHandle(type_tag<T1>, type_tag<T2>) {
        Handle handle;

        handle.annotation = boost::typeindex::type_id_with_cvr<operands<T1, T2>>().pretty_name();

        handle.comp.equal_to      = op::equal_to::cast_and_evaluate<T1, T2>;
        handle.comp.less          = op::less::cast_and_evaluate<T1, T2>;
        handle.comp.less_equal    = op::less_equal::cast_and_evaluate<T1, T2>;
        handle.comp.greater       = op::greater::cast_and_evaluate<T1, T2>;
        handle.comp.greater_equal = op::greater_equal::cast_and_evaluate<T1, T2>;

        handle.arithmetics.neg = op::negate::cast_and_evaluate<T1>;
        handle.arithmetics.add = op::plus::cast_and_evaluate<T1, T2>;
        handle.arithmetics.sub = op::minus::cast_and_evaluate<T1, T2>;
        handle.arithmetics.mul = op::multiplie::cast_and_evaluate<T1, T2>;
        handle.arithmetics.div = op::divide::cast_and_evaluate<T1, T2>;
        handle.arithmetics.mod = op::modulo::cast_and_evaluate<T1, T2>;

        handle.bitwise.compl_ = op::complement::cast_and_evaluate<T1>;
        handle.bitwise.and_   = op::bit_and::cast_and_evaluate<T1, T2>;
        handle.bitwise.or_    = op::bit_or::cast_and_evaluate<T1, T2>;
        handle.bitwise.xor_   = op::bit_xor::cast_and_evaluate<T1, T2>;

        handle.shift.left  = op::left_shift::cast_and_evaluate<T1, T2>;
        handle.shift.right = op::right_shift::cast_and_evaluate<T1, T2>;

        handle.logic.bool_ = op::boolean::cast_and_evaluate<T1>;
        handle.logic.not_ = op::logical_not::cast_and_evaluate<T1>;
        handle.logic.and_  = op::logical_and::cast_and_evaluate<T1, T2>;
        handle.logic.or_   = op::logical_or::cast_and_evaluate<T1, T2>;

        return handle;
    }

    Handle handle_;

    public:

    template <class T1, class T2 = T1>
    OperatorTable(type_tag<T1> t1, type_tag<T2> t2 = {})
        : handle_{makeHandle(t1, t2)}
    {
    }

    sol::object apply(Keyword const& keyword, sol::object lhs) const
    {
        return apply(keyword, lhs, sol::make_object(lhs.lua_state(), sol::nil));
    }
    // sol::object apply(Keyword const& keyword, sol::object lhs, sol::object rhs) const;

    // sol::object OperatorHandler::apply(Keyword const& keyword, ExpressionParameter lhs, ExpressionParameter rhs) const
    sol::object apply(Keyword const& keyword, sol::object lhs, sol::object rhs) const
    try
    {
        // std::string cf = BOOST_CURRENT_FUNCTION;
        auto const negate = [&](sol::object val) -> sol::object
        {
            if (val.is<error>())
            {
                return val;
            }
            else if (!val.is<bool>())
            {
                // return make_error("invalid argument", cf);
                return sol::make_object(val.lua_state(), error("invalid argument", handle_.annotation));
            }
            // return make_object(not val.as<bool>());
            return sol::make_object(val.lua_state(), not val.as<bool>());

        };

        auto const conj = [&](sol::object lhs, sol::object rhs) -> sol::object
        {
            if (lhs.is<error>())
            {
                return lhs;
            }
            if (rhs.is<error>())
            {
                return rhs;
            }
            if (not (lhs.is<bool>() && rhs.is<bool>()))
            {
                // return make_error("invalid argument", cf);
                return sol::make_object(lhs.lua_state(), error("invalid argument", handle_.annotation));

            }
            // return make_object(lhs.as<bool>() && rhs.as<bool>());
            return sol::make_object(lhs.lua_state(), lhs.as<bool>() && rhs.as<bool>());
        };



        switch (keyword)
        {
        case Keyword::Bool: return (handle_.logic.bool_)(lhs);
        case Keyword::Not: return negate((handle_.logic.bool_)(lhs));
        case Keyword::And: return (handle_.logic.and_)(lhs, rhs);
        case Keyword::Or: return (handle_.logic.or_)(lhs, rhs);

        case Keyword::Eq: return (handle_.comp.equal_to)(lhs, rhs);
        case Keyword::Ne: return negate((handle_.comp.equal_to)(lhs, rhs));

        case Keyword::Le: return (handle_.comp.less_equal)(lhs, rhs);
        case Keyword::Gt: return negate((handle_.comp.less_equal)(lhs, rhs));
        case Keyword::Ge: return (handle_.comp.less_equal)(rhs, lhs);
        case Keyword::Lt: return negate((handle_.comp.less_equal)(rhs, lhs));

        case Keyword::Add: return (handle_.arithmetics.add)(lhs, rhs);
        case Keyword::Sub: return (handle_.arithmetics.sub)(lhs, rhs);
        case Keyword::Mul: return (handle_.arithmetics.mul)(lhs, rhs);
        case Keyword::Div: return (handle_.arithmetics.div)(lhs, rhs);
        case Keyword::Mod: return (handle_.arithmetics.mod)(lhs, rhs);
        case Keyword::Neg: return (handle_.arithmetics.neg)(lhs);

        case Keyword::BitNot: return (handle_.bitwise.compl_)(lhs);
        case Keyword::BitAnd: return (handle_.bitwise.and_)(lhs, rhs);
        case Keyword::BitOr : return (handle_.bitwise.or_)(lhs, rhs);
        case Keyword::BitXor: return (handle_.bitwise.xor_)(lhs, rhs);

        case Keyword::Lshift: return (handle_.shift.left)(lhs, rhs);
        case Keyword::Rshift: return (handle_.shift.right)(lhs, rhs);

        // case Keyword::SetEq: return conj(is_subset(lhs, rhs), is_subset(rhs, lhs)); // TODO: optimize
        // case Keyword::Subset: return is_subset(lhs, rhs);
        // case Keyword::Superset: return is_subset(rhs, lhs);
        // case Keyword::PSubset: return conj(is_subset(lhs, rhs), negate(is_subset(rhs, lhs))); // TODO: optimize
        // case Keyword::PSuperset: return conj(is_subset(rhs, lhs), negate(is_subset(lhs, rhs))); // TODO: optimize

        // case Keyword::In: return contains(rhs, lhs);
        // case Keyword::Ni: return contains(lhs, rhs);
        // case Keyword::NotIn: return negate(contains(rhs, lhs));
        // case Keyword::NotNi: return negate(contains(lhs, rhs));

        // case Keyword::Pow: return OperatorHandler::generic_pow(lhs, rhs);
        // case Keyword::Log: return OperatorHandler::generic_log(lhs, rhs);
        // case Keyword::Quot: return OperatorHandler::generic_quot(lhs, rhs);

        default:
            // return make_error("unsupported operator", BOOST_CURRENT_FUNCTION);
            return sol::make_object(lhs.lua_state(), error("unsupported operator", handle_.annotation));

        }
    }
    catch(const std::exception& e)
    {
        return sol::make_object(lhs.lua_state(), error(e.what(), handle_.annotation));
        // return make_error(e.what(), annotation());
    }


};

class OperatorRegistry
{
    sol::state_view state_view_;



    public:

    OperatorRegistry(sol::state_view sv)
        : state_view_{sv}
    {
    }

    sol::state_view state()
    {
        return state_view_;
    }

    OperatorRegistry(OperatorRegistry const&) = default;
    OperatorRegistry(OperatorRegistry &&) = default;
    OperatorRegistry& operator=(OperatorRegistry const&) = default;
    OperatorRegistry& operator=(OperatorRegistry &&) = default;

    ~OperatorRegistry() = default;


    template <class LHS, class... RHS>
    void register_operators(type_tag<LHS>, type_tag<RHS>...)
    {
        using auto_types = boost::mp11::mp_list<
            type_tag<LHS>,
            type_tag<bool>,
            type_tag<double>
            >;
        using rhs_types = boost::mp11::mp_unique<boost::mp11::mp_append<
            boost::mp11::mp_list<type_tag<RHS>...>, auto_types>>;


        sol::table root = get_or_create_table(state_view_, state_view_.globals(), "haikan");
        sol::table operators = get_or_create_table(state_view_, root, "operators");
        sol::table lhs2rhs = get_or_create_table(state_view_, operators, "lhs2rhs");
        sol::table rhs2lhs = get_or_create_table(state_view_, operators, "rhs2lhs");

        auto const lhs_hash = typeid(LHS).hash_code();
        sol::table lhs_entry = get_or_create_table(state_view_, lhs2rhs, lhs_hash);


        boost::mp11::mp_for_each<rhs_types>([&](auto rhs) {
            using rhs_type = typename decltype(rhs)::type;
            auto const rhs_hash = typeid(rhs_type).hash_code();
            {
                sol::table rhs_entry = get_or_create_table(state_view_, rhs2lhs, rhs_hash);
                rhs_entry[lhs_hash] = lhs_entry[rhs_hash] = OperatorTable{type<LHS>, type<rhs_type>};
            }

            // commute
            {
                sol::table entry = get_or_create_table(state_view_, lhs2rhs, rhs_hash);
                sol::table entry_r = get_or_create_table(state_view_, rhs2lhs, lhs_hash);
                entry_r[rhs_hash] = entry[lhs_hash] = OperatorTable{type<rhs_type>, type<LHS>};
            }
            // TODO: add OperatorTable impl

        });
    }
};

} // namespace impl
} // namespace haikan
