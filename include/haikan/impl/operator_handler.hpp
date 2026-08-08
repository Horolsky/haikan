/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <limits>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <boost/mp11.hpp>
#include <boost/current_function.hpp>
#include <boost/type_index.hpp>


#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/keyword.hpp"
#include "haikan/impl/lazy_lua_object.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/impl/type_tag.hpp"
#include "haikan/impl/error_expr.hpp"
#include "haikan/cast.hpp"
#include "haikan/impl/error_object.hpp"




#define HAIKAN_OPERATOR_HANDLE_UNARY(OP, TRAIT)                 \
template <class T>                                              \
auto handle_##TRAIT(LazyLuaObject val) const noexcept           \
-> boost::mp11::mp_if<boost::has_##TRAIT<T>, sol::object>       \
try {                                                           \
    return make_object(OP val.load(L).as<T>());                 \
}                                                               \
catch(const std::exception& e)                                  \
{                                                               \
    return make_error(e.what(), BOOST_CURRENT_FUNCTION);        \
}                                                               \
template <class T>                                              \
auto handle_##TRAIT(LazyLuaObject) const noexcept               \
-> boost::mp11::mp_if<                                          \
    boost::mp11::mp_not<boost::has_##TRAIT<T>>, sol::object>    \
{                                                               \
    return make_error("operator not supported", BOOST_CURRENT_FUNCTION);\
}                                                               \

#define HAIKAN_OPERATOR_HANDLE_BINARY(OP, TRAIT)                        \
template <class T>                                                      \
auto handle_##TRAIT(LazyLuaObject lhs, LazyLuaObject rhs) const noexcept\
-> boost::mp11::mp_if<boost::has_##TRAIT<T>, sol::object>               \
try {                                                                   \
    return make_object(lhs.load(L).as<T>() OP rhs.load(L).as<T>());     \
}                                                                       \
catch(const std::exception& e)                                          \
{                                                                       \
    return make_error(e.what(), BOOST_CURRENT_FUNCTION);                \
}                                                                       \
template <class T>                                                      \
auto handle_##TRAIT(LazyLuaObject, LazyLuaObject) const noexcept        \
-> boost::mp11::mp_if<                                                  \
    boost::mp11::mp_not<boost::has_##TRAIT<T>>, sol::object>            \
{                                                                       \
    return make_error("operator not supported", BOOST_CURRENT_FUNCTION);      \
}                                                                       \

namespace haikan {
namespace impl {



class OperatorHandler
{
    mutable sol::state_view L;

    sol::object make_error(std::string what, std::string where) const
    {
        return sol::make_object(L, ErrorObject(what, where));
    }

    template <class T>
    sol::object make_object(T&& obj) const
    {
        return sol::make_object(L, std::forward<T>(obj));
    }

  public:

    sol::object generic_is_truth   (LazyLuaObject) const;
    sol::object generic_decorate   (LazyLuaObject) const;
    sol::object generic_negate     (LazyLuaObject) const;
    sol::object generic_complement (LazyLuaObject) const;
    sol::object generic_logical_not(LazyLuaObject) const;
    sol::object generic_equal_to   (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_less       (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_less_equal (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_plus       (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_minus      (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_multiplies (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_divides    (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_modulus    (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_bit_and    (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_bit_or     (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_bit_xor    (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_left_shift (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_right_shift(LazyLuaObject, LazyLuaObject) const;
    sol::object generic_logical_and(LazyLuaObject, LazyLuaObject) const;
    sol::object generic_logical_or (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_pow        (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_log        (LazyLuaObject, LazyLuaObject) const;
    sol::object generic_quot       (LazyLuaObject, LazyLuaObject) const;

    template <class T>
    static void register_utype_ops(sol::simple_usertype<T>& ut)
    {

        ut.set(sol::meta_function::addition,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_plus<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::subtraction,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_minus<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::multiplication,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_multiplies<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::division,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_divides<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::modulus,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_modulus<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::unary_minus,
        [](sol::object val, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_negate<T>(val);
        });

        ut.set(sol::meta_function::equal_to,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_equal_to<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::less_than,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_less<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::less_than_or_equal_to,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_less_equal<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::bitwise_not,
        [](sol::object val, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_complement<T>(val);
        });

        ut.set(sol::meta_function::bitwise_and,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_bit_and<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::bitwise_or,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_bit_or<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::bitwise_xor,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_bit_xor<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::bitwise_left_shift,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_left_shift<T>(lhs, rhs);
        });

        ut.set(sol::meta_function::bitwise_right_shift,
        [](sol::object lhs, sol::object rhs, sol::this_state ts) -> sol::object {
            return OperatorHandler(type<T>, ts).handle_right_shift<T>(lhs, rhs);
        });

    }

  private:

    using unary_transform = sol::object(OperatorHandler::*)(LazyLuaObject) const;
    using binary_transform = sol::object(OperatorHandler::*)(LazyLuaObject, LazyLuaObject) const;

    HAIKAN_OPERATOR_HANDLE_UNARY(-, negate)
    HAIKAN_OPERATOR_HANDLE_UNARY(~, complement)
    HAIKAN_OPERATOR_HANDLE_UNARY(!, logical_not)
    HAIKAN_OPERATOR_HANDLE_BINARY(==, equal_to)
    HAIKAN_OPERATOR_HANDLE_BINARY(<, less)
    HAIKAN_OPERATOR_HANDLE_BINARY(<=, less_equal)
    HAIKAN_OPERATOR_HANDLE_BINARY(+, plus)
    HAIKAN_OPERATOR_HANDLE_BINARY(-, minus)
    HAIKAN_OPERATOR_HANDLE_BINARY(*, multiplies)
    HAIKAN_OPERATOR_HANDLE_BINARY(/, divides)
    HAIKAN_OPERATOR_HANDLE_BINARY(%, modulus)
    HAIKAN_OPERATOR_HANDLE_BINARY(&, bit_and)
    HAIKAN_OPERATOR_HANDLE_BINARY(|, bit_or)
    HAIKAN_OPERATOR_HANDLE_BINARY(^, bit_xor)
    HAIKAN_OPERATOR_HANDLE_BINARY(<<, left_shift)
    HAIKAN_OPERATOR_HANDLE_BINARY(>>, right_shift)
    HAIKAN_OPERATOR_HANDLE_BINARY(&&, logical_and)
    HAIKAN_OPERATOR_HANDLE_BINARY(||, logical_or)

#undef HAIKAN_OPERATOR_HANDLE_UNARY
#undef HAIKAN_OPERATOR_HANDLE_BINARY


    template <class T>
    auto handle_is_truth(LazyLuaObject val) const
        -> mp_if<std::is_convertible<T, bool>, sol::object>
    try
    {
        return static_cast<bool>(val.load(L).as<T>());
    }
    catch(const std::exception& e)
    {
        return make_error(e.what(), BOOST_CURRENT_FUNCTION);
    }

    template <class T>
    auto handle_is_truth(LazyLuaObject) const
        -> mp_if<mp_not<std::is_convertible<T, bool>>, sol::object>
    {
        return make_error("invalid operand", BOOST_CURRENT_FUNCTION);
    }


    struct Handle {

        boost::json::string annotation{""};
        struct D {
            unary_transform decorate{&OperatorHandler::generic_decorate};
        } decor;

        struct C {
            binary_transform equal_to{&OperatorHandler::generic_equal_to};
            binary_transform less{&OperatorHandler::generic_less};
            binary_transform less_equal{&OperatorHandler::generic_less_equal};
        } comp;

        struct A {
            unary_transform neg{&OperatorHandler::generic_negate};
            binary_transform add{&OperatorHandler::generic_plus};
            binary_transform sub{&OperatorHandler::generic_minus};
            binary_transform mul{&OperatorHandler::generic_multiplies};
            binary_transform div{&OperatorHandler::generic_divides};
            binary_transform mod{&OperatorHandler::generic_modulus};
        } arithmetics;

        struct B {
            unary_transform compl_{&OperatorHandler::generic_complement};
            binary_transform and_{&OperatorHandler::generic_bit_and};
            binary_transform or_{&OperatorHandler::generic_bit_or};
            binary_transform xor_{&OperatorHandler::generic_bit_xor};
        } bitwise;

        struct S {
            binary_transform left{&OperatorHandler::generic_left_shift};
            binary_transform right{&OperatorHandler::generic_right_shift};
        } shift;

        struct L {
            unary_transform  bool_{&OperatorHandler::generic_is_truth};
            binary_transform and_{&OperatorHandler::generic_logical_and};
            binary_transform or_{&OperatorHandler::generic_logical_or};
        } logic;
    };

    template <class T>
    static Handle makeHandle(type_tag<T>) {
        Handle handle;

        handle.annotation = boost::typeindex::type_id_with_cvr<T>().pretty_name();

        handle.comp.equal_to   = &OperatorHandler::handle_equal_to<T>;
        handle.comp.less       = &OperatorHandler::handle_less<T>;
        handle.comp.less_equal = &OperatorHandler::handle_less_equal<T>;

        handle.arithmetics.neg = &OperatorHandler::handle_negate<T>;
        handle.arithmetics.add = &OperatorHandler::handle_plus<T>;
        handle.arithmetics.sub = &OperatorHandler::handle_minus<T>;
        handle.arithmetics.mul = &OperatorHandler::handle_multiplies<T>;
        handle.arithmetics.div = &OperatorHandler::handle_divides<T>;
        handle.arithmetics.mod = &OperatorHandler::handle_modulus<T>;

        handle.bitwise.compl_ = &OperatorHandler::handle_complement<T>;
        handle.bitwise.and_   = &OperatorHandler::handle_bit_and<T>;
        handle.bitwise.or_    = &OperatorHandler::handle_bit_or<T>;
        handle.bitwise.xor_   = &OperatorHandler::handle_bit_xor<T>;

        handle.shift.left  = &OperatorHandler::handle_left_shift<T>;
        handle.shift.right = &OperatorHandler::handle_right_shift<T>;

        handle.logic.bool_ = &OperatorHandler::handle_is_truth<T>;
        handle.logic.and_  = &OperatorHandler::handle_logical_and<T>;
        handle.logic.or_   = &OperatorHandler::handle_logical_or<T>;

        return handle;
    }

    Handle handle_;

    explicit OperatorHandler(
        Handle const handle, sol::state_view sv
    );

    static bool exchangeHandle(Handle& handle, bool const retrieve);

public:

    /// Default operator with generic transforms
    OperatorHandler(sol::state_view);

    template <class T>
    OperatorHandler(type_tag<T> tag, sol::state_view sv)
    : OperatorHandler{makeHandle(tag), sv}
    {
    }

    /// Retrieve registered operator instance if it exists, throw otherwise
    OperatorHandler(boost::json::string_view annotation, sol::state_view sv);

    OperatorHandler(OperatorHandler const&) = default;
    OperatorHandler(OperatorHandler &&) = default;
    virtual ~OperatorHandler() = default;
    OperatorHandler& operator=(OperatorHandler const&) = default;
    OperatorHandler& operator=(OperatorHandler &&) = default;

    bool is_generic() const
    {
        return annotation().empty();
    }

    boost::json::string_view annotation() const
    {
        return handle_.annotation.data();
    }


    /// \brief Apply operands
    sol::object apply(Keyword const& keyword, LazyLuaObject lhs, LazyLuaObject rhs) const;
    sol::object apply(Keyword const& keyword, LazyLuaObject rhs) const
    {
        return apply(keyword, {}, rhs);
    }


private:

    /// Is subset of
    sol::object is_subset(LazyLuaObject llhs, LazyLuaObject lrhs) const;

    /// Is element of
    sol::object contains(LazyLuaObject set, LazyLuaObject element) const;

};

} // namespace impl
} // namespace haikan
