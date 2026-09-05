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


        struct Set {
            binary_transform contains{};
            binary_transform is_in{};
            binary_transform is_subset{};
            binary_transform is_proper_subset{};
            binary_transform set_equal{};
            binary_transform intersect{};
            binary_transform union_{};
            binary_transform difference{};

        } set;
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

        handle.set.contains         = op::contains::cast_and_evaluate<T1, T2>;
        handle.set.is_in            = op::is_in::cast_and_evaluate<T1, T2>;
        handle.set.is_subset        = op::is_subset::cast_and_evaluate<T1, T2>;
        handle.set.is_proper_subset = op::is_proper_subset::cast_and_evaluate<T1, T2>;
        handle.set.set_equal        = op::set_equal::cast_and_evaluate<T1, T2>;

        handle.set.intersect  = op::intersection::cast_and_evaluate<T1, T2>;
        handle.set.union_     = op::set_union::cast_and_evaluate<T1, T2>;
        handle.set.difference = op::difference::cast_and_evaluate<T1, T2>;
        // handle.set.symmetric_difference = op::symmetric_difference::cast_and_evaluate<T1, T2>;

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

    sol::object apply(Keyword const& keyword, sol::object lhs, sol::object rhs) const
    try
    {
        auto const negate = [&](sol::object val) -> sol::object
        {
            if (val.is<error>())
            {
                return val;
            }
            else if (!val.is<bool>())
            {
                return sol::make_object(val.lua_state(), error("invalid argument", handle_.annotation));
            }
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
                return sol::make_object(lhs.lua_state(), error("invalid argument", handle_.annotation));

            }
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

        case Keyword::SetEq: return  handle_.set.set_equal(lhs, rhs);
        case Keyword::Subset: return  handle_.set.is_subset(lhs, rhs);
        case Keyword::Superset: return handle_.set.is_subset(rhs, lhs);
        case Keyword::PSubset: return handle_.set.is_proper_subset(lhs, rhs);
        case Keyword::PSuperset: return handle_.set.is_proper_subset(rhs, lhs);

        case Keyword::In: return (handle_.set.is_in)(lhs, rhs);
        case Keyword::NotIn: return negate((handle_.set.is_in)(lhs, rhs));
        case Keyword::Ni: return (handle_.set.contains)(lhs, rhs);
        case Keyword::NotNi: return negate((handle_.set.contains)(lhs, rhs));

        case Keyword::Union: return (handle_.set.union_)(lhs, rhs);
        case Keyword::Intersect: return (handle_.set.intersect)(lhs, rhs);
        case Keyword::Diff: return (handle_.set.difference)(lhs, rhs);

        // case Keyword::Pow: return OperatorHandler::generic_pow(lhs, rhs);
        // case Keyword::Log: return OperatorHandler::generic_log(lhs, rhs);
        // case Keyword::Quot: return OperatorHandler::generic_quot(lhs, rhs);

        default:
            return sol::make_object(lhs.lua_state(), error("unsupported operator", handle_.annotation));

        }
    }
    catch(const std::exception& e)
    {
        return sol::make_object(lhs.lua_state(), error(e.what(), handle_.annotation));
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


    template <class LHS, class... RHS, template <class...> class... Cont>
    void insert(type_tag<LHS>, type_list_t<RHS...> = {}, template_list_t<Cont...> = {})
    {
        using boost::mp11::mp_list;
        using boost::mp11::mp_product;
        using boost::mp11::mp_invoke_q;
        using boost::mp11::mp_append;
        using boost::mp11::mp_quote;
        using boost::mp11::mp_transform;
        using boost::mp11::mp_first;
        using boost::mp11::mp_second;


        using rhs_raw_types = mp_list<LHS, RHS...>;
        using rhs_cont_types = mp_product<mp_invoke_q, mp_list<mp_quote<Cont>...>, rhs_raw_types>;
        using rhs_types = mp_append<rhs_raw_types, rhs_cont_types>;
        using lhs_raw_types = mp_list<LHS>;
        using lhs_cont_types = mp_product<mp_invoke_q, mp_list<mp_quote<Cont>...>, lhs_raw_types>;
        using lhs_types = mp_append<lhs_raw_types, lhs_cont_types>;
        using raw_operand_pairs = mp_product<mp_list, lhs_types, rhs_types>;
        using operand_pairs = mp_transform<type_tag, raw_operand_pairs>;

        sol::table root = get_or_create_table(state_view_, state_view_.globals(), "haikan");
        sol::table operators = get_or_create_table(state_view_, root, "operators");
        sol::table lhs2rhs = get_or_create_table(state_view_, operators, "lhs2rhs");
        sol::table rhs2lhs = get_or_create_table(state_view_, operators, "rhs2lhs");

        boost::mp11::mp_for_each<operand_pairs>([&](auto x) {
            using operand_pair = typename decltype(x)::type;

            using lhs_type = mp_first<operand_pair>;
            using rhs_type = mp_second<operand_pair>;

            auto const lhs_hash = typeid(lhs_type).hash_code();
            auto const rhs_hash = typeid(rhs_type).hash_code();

            {
                sol::table rhs_entry = get_or_create_table(state_view_, rhs2lhs, rhs_hash);
                sol::table lhs_entry = get_or_create_table(state_view_, lhs2rhs, lhs_hash);
                rhs_entry[lhs_hash] = lhs_entry[rhs_hash] = OperatorTable{type<lhs_type>, type<rhs_type>};
            }

            {
                sol::table rhs_entry = get_or_create_table(state_view_, lhs2rhs, rhs_hash);
                sol::table lhs_entry = get_or_create_table(state_view_, rhs2lhs, lhs_hash);
                rhs_entry[lhs_hash] = lhs_entry[rhs_hash] = OperatorTable{type<rhs_type>, type<lhs_type>};
            }
        });
    }

    private:

};

} // namespace impl
} // namespace haikan
