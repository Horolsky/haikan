/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <type_traits>
#include <typeinfo>
#include <cstdint>

// #include <boost/container_hash/hash.hpp>
#include <boost/functional/hash.hpp>
#include <boost/mp11.hpp>
#include <boost/type_traits.hpp>


#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/error.hpp"
#include "haikan/logger.hpp"
#include "haikan/reflection_meta.hpp"
#include "haikan/impl/operators.hpp"
#include "haikan/impl/generic_operand.hpp"
#include "haikan/impl/get_or_create_table.hpp"
#include "haikan/type_tag.hpp"
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
        handle.arithmetics.mul = op::multiply::cast_and_evaluate<T1, T2>;
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

    OperatorTable() : OperatorTable(type<GenericOperand>)
    {
    }

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

        // case Keyword::Pow: return generic_pow(lhs, rhs);
        // case Keyword::Log: return generic_log(lhs, rhs);
        // case Keyword::Quot: return generic_quot(lhs, rhs);

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
    struct GlobalIndex
    {
        using Key = std::pair<std::size_t, std::size_t>;
        using Index = std::unordered_map<Key, OperatorTable, boost::hash<Key>>;
        Index lhs2rhs;
        Index rhs2lhs;
    };


    static GlobalIndex& gstate()
    {
        static GlobalIndex s{};
        return s;
    }

    static std::unordered_map<std::size_t, std::string>& seen_registry()
    {
        static std::unordered_map<std::size_t, std::string> s{};
        return s;
    }

    static boost::optional<OperatorTable> get(std::size_t const t1_hash, std::size_t const t2_hash)
    {
        GlobalIndex const& s = gstate();

        auto maybe_tbl = s.lhs2rhs.find(std::make_pair(t1_hash, t2_hash));

        if (maybe_tbl != s.lhs2rhs.cend())
        {
            return maybe_tbl->second;
        }
        maybe_tbl = s.rhs2lhs.find(std::make_pair(t2_hash, t1_hash));
        if (maybe_tbl != s.rhs2lhs.cend())
        {
            return maybe_tbl->second;
        }

        return OperatorTable();
        // return boost::none;
    }

    template <class OperandPairs>
    static void register_pairs()
    {
        boost::mp11::mp_for_each<OperandPairs>([](auto x) {

            using operand_pair = typename decltype(x)::type;
            using lhs_type = mp_first<operand_pair>;
            using rhs_type = mp_second<operand_pair>;

            auto const lhs_hash = typeid(lhs_type).hash_code();
            auto const rhs_hash = typeid(rhs_type).hash_code();
            auto const l2r_key = std::make_pair(lhs_hash, rhs_hash);
            auto const r2l_key = std::make_pair(rhs_hash, lhs_hash);

            GlobalIndex& s = gstate();
            s.lhs2rhs[l2r_key] = s.rhs2lhs[r2l_key] = OperatorTable{type<lhs_type>, type<rhs_type>};
            s.lhs2rhs[r2l_key] = s.rhs2lhs[l2r_key] = OperatorTable{type<rhs_type>, type<lhs_type>};

            seen_registry().insert({lhs_hash, sol::usertype_traits<lhs_type>::name()});
            seen_registry().insert({rhs_hash, sol::usertype_traits<rhs_type>::name()});
        });
    }

  public:

    template <class LHS, template <class...> class... LhsCont>
    struct SignatureBuilder
    {

        template <class RHS, template <class...> class... RhsCont>
        static SignatureBuilder Connects(type_tag<RHS>, template_list_t<RhsCont...> = {})
        {
            using namespace boost::mp11;

            using rhs_raw_types = mp_list<RHS>;
            using rhs_cont_types = mp_product<mp_invoke_q, mp_list<mp_quote<RhsCont>...>, rhs_raw_types>;
            using rhs_types = mp_append<rhs_raw_types, rhs_cont_types>;

            using lhs_raw_types = mp_list<LHS>;
            using lhs_cont_types = mp_product<mp_invoke_q, mp_list<mp_quote<LhsCont>...>, lhs_raw_types>;
            using lhs_types = mp_append<lhs_raw_types, lhs_cont_types>;
            using raw_operand_pairs = mp_product<mp_list, lhs_types, rhs_types>;
            using operand_pairs = mp_transform<type_tag, raw_operand_pairs>;

            register_pairs<operand_pairs>();
            return {};
        }

        template <class T, template <class...> class... C>
        static SignatureBuilder<T, C...> AddSignature(type_tag<T> t, template_list_t<C...> l = {})
        {
            return OperatorRegistry::AddSignature(t, l);
        }
    };

    template <class T, template <class...> class... C>
    static SignatureBuilder<T, C...> AddSignature(type_tag<T>, template_list_t<C...> = {})
    {
        using namespace boost::mp11;

        using raw_types = mp_list<T>;
        using cont_types = mp_product<mp_invoke_q, mp_list<mp_quote<C>...>, raw_types>;
        using all_types = mp_append<raw_types, cont_types>;

        using raw_operand_pairs = mp_product<mp_list, all_types, all_types>;
        using operand_pairs = mp_transform<type_tag, raw_operand_pairs>;

        register_pairs<operand_pairs>();
        return {};
    }


    static std::unordered_map<std::size_t, std::string> const& types_seen()
    {
        return seen_registry();
    }

    static boost::optional<OperatorTable> get(sol::object x, sol::object y = sol::nil)
    {
        auto const get_hash = [](sol::object o) -> std::size_t
        {
            switch (o.get_type())
            {
            case sol::type::userdata:
                {
                    sol::userdata userdata = o.as<sol::userdata>();
                    sol::object metadata = userdata["meta"];
                    if (metadata.is<ReflectionMeta>())
                    {
                        ReflectionMeta const& meta = metadata.as<ReflectionMeta const&>();
                        return meta.type_index_hash;
                    }
                    else
                    {
                        return 0;
                    }
                }
                break;
            case sol::type::string: return typeid(std::string).hash_code();
            case sol::type::number: return typeid(double).hash_code();
            case sol::type::boolean: return typeid(bool).hash_code();
            default:
                return typeid(sol::object).hash_code();
            }
        };

        auto const x_hash = get_hash(x);
        auto const y_hash = y == sol::nil ? x_hash : x_hash;

        return get(x_hash, y_hash);
    }

    template <class T1, class T2 = T1>
    static boost::optional<OperatorTable> get(type_tag<T1>, type_tag<T2> = {})
    {
        return get(typeid(T1).hash_code(), typeid(T2).hash_code());
    }


    private:

};

} // namespace impl
} // namespace haikan
