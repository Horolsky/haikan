/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <memory>
#include <string>

#include <boost/current_function.hpp>
#include <boost/json.hpp>
#define BOOST_UNORDERED_DISABLE_REENTRANCY_CHECK
#include <boost/unordered/concurrent_flat_map.hpp>

#include "haikan/impl/type_info.hpp"
#include "haikan/impl/operator_handler.hpp"
#include "haikan/keywords.hpp"
#include "haikan/error.hpp"



namespace haikan {
namespace impl {

bool OperatorHandler::exchangeHandle(Handle& handle, bool const retrieve)
{
    using Table = boost::concurrent_flat_map<boost::json::string, Handle>;

    static std::shared_ptr<Table> table = std::make_shared<Table>();

    if (!retrieve)
    {
        return table->emplace(handle.annotation, handle);
    }
    else
    {
        return 1 == table->visit(handle.annotation, [&handle](auto& rec){
            handle = rec.second;
        });
    }
}

OperatorHandler::OperatorHandler(sol::state_view sv) : OperatorHandler{Handle{}, sv}
{
}

OperatorHandler::OperatorHandler(Handle const handle, sol::state_view sv)
    : L{sv}
    , handle_{handle}
{
    exchangeHandle(handle_, false);
}

OperatorHandler::OperatorHandler(boost::json::string_view annotation, sol::state_view sv)
    : L{sv}
    , handle_{}
{
    handle_.annotation = annotation;
    // TODO: handle as valid()
    if (!exchangeHandle(handle_, true))
    {
        boost::throw_exception(std::runtime_error((boost::format("`%s` operator not found") % annotation).str()));
    }
}


sol::object OperatorHandler::apply(Keyword const& keyword, ExpressionParameter lhs, ExpressionParameter rhs) const
try
{
    std::string cf = BOOST_CURRENT_FUNCTION;
    auto const negate = [&](sol::object val) -> sol::object
    {
        if (val.is<error>())
        {
            return val;
        }
        else if (!val.is<bool>())
        {
            return make_error("invalid argument", cf);
        }
        return make_object(not val.as<bool>());
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
            return make_error("invalid argument", cf);
        }
        return make_object(lhs.as<bool>() && rhs.as<bool>());
    };



    switch (keyword)
    {
    case Keyword::Bool: return (this->*handle_.logic.bool_)(lhs);
    case Keyword::Not: return negate((this->*handle_.logic.bool_)(lhs));
    case Keyword::And: return (this->*handle_.logic.and_)(lhs, rhs);
    case Keyword::Or: return (this->*handle_.logic.or_)(lhs, rhs);

    case Keyword::Eq: return (this->*handle_.comp.equal_to)(lhs, rhs);
    case Keyword::Ne: return negate((this->*handle_.comp.equal_to)(lhs, rhs));

    case Keyword::Le: return (this->*handle_.comp.less_equal)(lhs, rhs);
    case Keyword::Gt: return negate((this->*handle_.comp.less_equal)(lhs, rhs));
    case Keyword::Ge: return (this->*handle_.comp.less_equal)(rhs, lhs);
    case Keyword::Lt: return negate((this->*handle_.comp.less_equal)(rhs, lhs));

    case Keyword::Add: return (this->*handle_.arithmetics.add)(lhs, rhs);
    case Keyword::Sub: return (this->*handle_.arithmetics.sub)(lhs, rhs);
    case Keyword::Mul: return (this->*handle_.arithmetics.mul)(lhs, rhs);
    case Keyword::Div: return (this->*handle_.arithmetics.div)(lhs, rhs);
    case Keyword::Mod: return (this->*handle_.arithmetics.mod)(lhs, rhs);
    case Keyword::Neg: return (this->*handle_.arithmetics.neg)(lhs);

    case Keyword::BitNot: return (this->*handle_.bitwise.compl_)(lhs);
    case Keyword::BitAnd: return (this->*handle_.bitwise.and_)(lhs, rhs);
    case Keyword::BitOr : return (this->*handle_.bitwise.or_)(lhs, rhs);
    case Keyword::BitXor: return (this->*handle_.bitwise.xor_)(lhs, rhs);

    case Keyword::Lshift: return (this->*handle_.shift.left)(lhs, rhs);
    case Keyword::Rshift: return (this->*handle_.shift.right)(lhs, rhs);

    case Keyword::SetEq: return conj(is_subset(lhs, rhs), is_subset(rhs, lhs)); // TODO: optimize
    case Keyword::Subset: return is_subset(lhs, rhs);
    case Keyword::Superset: return is_subset(rhs, lhs);
    case Keyword::PSubset: return conj(is_subset(lhs, rhs), negate(is_subset(rhs, lhs))); // TODO: optimize
    case Keyword::PSuperset: return conj(is_subset(rhs, lhs), negate(is_subset(lhs, rhs))); // TODO: optimize

    case Keyword::In: return contains(rhs, lhs);
    case Keyword::Ni: return contains(lhs, rhs);
    case Keyword::NotIn: return negate(contains(rhs, lhs));
    case Keyword::NotNi: return negate(contains(lhs, rhs));

    case Keyword::Pow: return OperatorHandler::generic_pow(lhs, rhs);
    case Keyword::Log: return OperatorHandler::generic_log(lhs, rhs);
    case Keyword::Quot: return OperatorHandler::generic_quot(lhs, rhs);

    default:
        return make_error("unsupported operator", BOOST_CURRENT_FUNCTION);
    }
}
catch(const std::exception& e)
{
    return make_error(e.what(), annotation());
}

/// Is subset of
sol::object OperatorHandler::is_subset(ExpressionParameter llhs, ExpressionParameter lrhs) const
{
    sol::object const lhs = llhs.load(L);
    sol::object const rhs = lrhs.load(L);

    if (lhs.get_type() != rhs.get_type())
    {
        return make_object(false);
    }

    if (lhs.is<sol::table>())
    {

        // sol::table const a = lhs.as<sol::table>();
        // sol::table const b = rhs.as<sol::table>();

        // for (auto const& item_a: a)
        // {
        //     try
        //     {
        //         bool a_subset{true};
        //         b.for_each([&](auto const& item_b){
        //             if (!(this->*handle_.comp.equal_to)(item_a, item_b).template as<bool>())
        //             {
        //                 a_subset = false;
        //             }
        //         });
        //         return make_object(a_subset);
        //     }
        //     catch(const std::exception& e)
        //     {
        //         return make_error("invalid operands", BOOST_CURRENT_FUNCTION);
        //     }
        // }
        return make_object(false);
    }
    // else if (lhs.is_object() && rhs.is_object())
    // {
    //     boost::json::object const& a = lhs.get_object();
    //     boost::json::object const& b = rhs.get_object();

    //     if (a.size() > b.size())
    //     {
    //         return false;
    //     }

    //     for (auto const& kvp: a)
    //     {
    //         auto const& key = kvp.key();
    //         auto const& value = kvp.value();
    //         if (!b.contains(key) || value != b.at(key))
    //         {
    //             return false;
    //         }
    //     }
    //     return true;
    // }
    // string is treated as set of substrings
    else if (lhs.get_type() == sol::type::string)
    {
        std::string ls = lhs.as<std::string>();
        std::string rs = rhs.as<std::string>();
        if (ls.empty()) { return make_object(true); }
        return make_object(std::string::npos != rs.find(ls));
    }
    return make_error("invalid operands", BOOST_CURRENT_FUNCTION);
}


/// Is element of
sol::object OperatorHandler::contains(ExpressionParameter lset, ExpressionParameter lelement) const
{
    static_cast<void>(lset);
    static_cast<void>(lelement);
    return make_object(false);
    // array item
    // if (lset().if_array())
    // {
    //     return is_subset(boost::json::array{lelement()}, lset);
    // }
    // else if(lelement().is_string() && lset().is_string())
    // {
    //     return is_subset(lelement, lset);
    // }

    // auto const& set = lset();
    // auto const& element = lelement();

    // // object key
    // if(element.is_string() && set.is_object())
    // {
    //     return set.get_object().contains(element.get_string());
    // }
    // // object key-value pair
    // else if(set.is_object() && element.is_array() && element.get_array().size() == 2 && element.get_array().front().is_string())
    // {
    //     auto const& kvp = element.get_array();
    //     auto const& obj = set.get_object();
    //     auto const& key = kvp.front().as_string();
    //     auto const& value = kvp.at(1);
    //     if (!obj.contains(key)) { return false; }
    //     return (this->*handle_.comp.equal_to)(obj.at(key), value);
    // }
    // return make_error("invalid operands", BOOST_CURRENT_FUNCTION);
}


} // namespace impl
} // namespace haikan
