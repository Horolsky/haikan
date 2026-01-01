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

#include "haikan/impl/keyword.hpp"
#include "haikan/impl/lazy_param.hpp"
#include "haikan/impl/traits.hpp"
#include "haikan/impl/type_tag.hpp"
#include "haikan/impl/error_expr.hpp"
#include "haikan/monadic_cast.hpp"





#define HAIKAN_SOH_HANDLE_UNARY_TRANSFORM(OP, TRAIT)       \
template <class T>                                       \
static auto handle_##TRAIT(LV val)                       \
-> boost::mp11::mp_if<boost::has_##TRAIT<T>, V>                 \
try {                                                    \
    return boost::json::value_from(OP boost::json::value_to<T>(val()));  \
}                                                        \
catch(const std::exception& e)                           \
{                                                        \
    return make_error_expr(                              \
        e.what(), BOOST_CURRENT_FUNCTION);               \
}                                                        \
template <class T>                                       \
static auto handle_##TRAIT(LV)                           \
-> boost::mp11::mp_if<boost::mp11::mp_not<boost::has_##TRAIT<T>>, V>                      \
try {                                                    \
    return make_error_expr(                              \
        "invalid operand", BOOST_CURRENT_FUNCTION);      \
}                                                        \
catch(const std::exception& e)                           \
{                                                        \
    return make_error_expr(                              \
        e.what(), BOOST_CURRENT_FUNCTION);               \
}

#define HAIKAN_SOH_HANDLE_BIN_TRANSFORM(OP, TRAIT)                 \
template <class T>                                               \
static auto handle_##TRAIT(LV lhs, LV rhs)                       \
-> boost::mp11::mp_if<boost::has_##TRAIT<T>, V>                                      \
try {                                                            \
    static_assert(has_##TRAIT<T>::value, "lol");                 \
    return boost::json::value_from(boost::json::value_to<T>(lhs()) OP boost::json::value_to<T>(rhs()));    \
}                                                                \
catch(const std::exception& e)                                   \
{                                                                \
    return make_error_expr(                                      \
        e.what(), BOOST_CURRENT_FUNCTION);                       \
}                                                                \
template <class T>                                               \
static auto handle_##TRAIT(LV, LV)                               \
-> boost::mp11::mp_if<boost::mp11::mp_not<boost::has_##TRAIT<T>>, V>                              \
try {                                                            \
    return make_error_expr(                                      \
        "invalid operands", BOOST_CURRENT_FUNCTION);             \
}                                                                \
catch(const std::exception& e)                                   \
{                                                                \
    return make_error_expr(                                      \
        e.what(), BOOST_CURRENT_FUNCTION);                       \
}

namespace haikan {
namespace impl {

/// Signal transformation and comparison handler. Enables type erasure.
class Operator
{
  public:

    using V = boost::json::value;
    using LV = LazyParam;

    enum Config : std::uint32_t
    {
        Null,
        Decor       = 1U << 0,
        Comparison  = 1U << 1,
        Arithmetics = 1U << 2,
        Bitwise     = 1U << 3,
        Shift       = 1U << 4,
        Logic       = 1U << 5,
        Default     = Decor|Comparison|Arithmetics|Bitwise|Shift,
        Full        = Default|Logic,
    };

    static V generic_is_truth   (LV);
    static V generic_decorate   (LV);
    static V generic_negate     (LV);
    static V generic_complement (LV);
    static V generic_logical_not(LV);
    static V generic_equal_to   (LV, LV);
    static V generic_less       (LV, LV);
    static V generic_less_equal (LV, LV);
    static V generic_plus       (LV, LV);
    static V generic_minus      (LV, LV);
    static V generic_multiplies (LV, LV);
    static V generic_divides    (LV, LV);
    static V generic_modulus    (LV, LV);
    static V generic_bit_and    (LV, LV);
    static V generic_bit_or     (LV, LV);
    static V generic_bit_xor    (LV, LV);
    static V generic_left_shift (LV, LV);
    static V generic_right_shift(LV, LV);
    static V generic_logical_and(LV, LV);
    static V generic_logical_or (LV, LV);

    static V generic_pow (LV, LV);
    static V generic_log (LV, LV);
    static V generic_quot(LV, LV);


  private:

    using unary_transform = std::function<V(LV)>;
    using binary_transform = std::function<V(LV, LV)>;

    HAIKAN_SOH_HANDLE_UNARY_TRANSFORM(-, negate)
    HAIKAN_SOH_HANDLE_UNARY_TRANSFORM(~, complement)
    HAIKAN_SOH_HANDLE_UNARY_TRANSFORM(!, logical_not)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(==, equal_to)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(<, less)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(<=, less_equal)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(+, plus)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(-, minus)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(*, multiplies)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(/, divides)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(%, modulus)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(&, bit_and)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(|, bit_or)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(^, bit_xor)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(<<, left_shift)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(>>, right_shift)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(&&, logical_and)
    HAIKAN_SOH_HANDLE_BIN_TRANSFORM(||, logical_or)

#undef HAIKAN_SOH_HANDLE_UNARY_TRANSFORM
#undef HAIKAN_SOH_HANDLE_BIN_TRANSFORM



    template <class T>
    static boost::json::value handle_decorate(boost::json::value const& arg)
    try
    {
        monadic_cast<T> mc;
        switch (arg.kind())
        {
        case boost::json::kind::uint64 : { mc = mc.try_cast(arg.get_uint64()); break; }
        case boost::json::kind::int64  : { mc = mc.try_cast(arg.get_int64());  break; }
        case boost::json::kind::double_: { mc = mc.try_cast(arg.get_double()); break; }
        case boost::json::kind::string : { mc = mc.try_cast(arg.get_string()); break; }
        case boost::json::kind::array  : { mc = mc.try_cast(arg.get_array());  break; }
        case boost::json::kind::object : { mc = mc.try_cast(arg.get_object()); break; }
        case boost::json::kind::null   : { mc = mc.try_cast(nullptr);          break; }
        default:
            break;
        }
        return mc
            .try_cast(arg)
            .map([](T const& v) -> boost::json::value {
                return boost::json::value_from(v);
            })
            .value_or_eval([]{
                return make_error_expr("decoration is not deducible", BOOST_CURRENT_FUNCTION);
            });
    }
    catch(const std::exception& e)
    {
        return make_error_expr(e.what(), BOOST_CURRENT_FUNCTION);
    }

    template <class T>
    static auto handle_is_truth(LV const& val) -> mp_if<std::is_convertible<T, bool>, boost::json::value>
    try
    {
        return static_cast<bool>(boost::json::value_to<T>(val()));
    }
    catch(const std::exception& e)
    {
        return make_error_expr(e.what(), BOOST_CURRENT_FUNCTION);
    }

    template <class T>
    static auto handle_is_truth(LV const&) -> mp_if<mp_not<std::is_convertible<T, bool>>, boost::json::value>
    {
        return make_error_expr("invalid operand", BOOST_CURRENT_FUNCTION);
    }



    struct Handle {

        boost::json::string annotation{""};
        struct D {
            unary_transform decorate{generic_decorate};
        } decor;

        struct C {
            binary_transform equal_to{generic_equal_to};
            binary_transform less{generic_less};
            binary_transform less_equal{generic_less_equal};
        } comp;

        struct A {
            unary_transform neg{generic_negate};
            binary_transform add{generic_plus};
            binary_transform sub{generic_minus};
            binary_transform mul{generic_multiplies};
            binary_transform div{generic_divides};
            binary_transform mod{generic_modulus};
        } arithmetics;

        struct B {
            unary_transform compl_{generic_complement};
            binary_transform and_{generic_bit_and};
            binary_transform or_{generic_bit_or};
            binary_transform xor_{generic_bit_xor};
        } bitwise;

        struct S {
            binary_transform left{generic_left_shift};
            binary_transform right{generic_right_shift};
        } shift;

        struct L {
            unary_transform  bool_{generic_is_truth};
            binary_transform and_{generic_logical_and};
            binary_transform or_{generic_logical_or};
        } logic;
    };

    template <class T>
    static Handle makeHandle(type_tag<T>, Config const cfg = Default) {
        return {
            (boost::format("%s#%d") % boost::typeindex::type_id_with_cvr<T>().pretty_name() % cfg).str().c_str(),
            Decor & cfg ? Handle::D{handle_decorate<T>} : Handle::D{},
            Comparison & cfg ? Handle::C{
                handle_equal_to<T>,
                handle_less<T>,
                handle_less_equal<T>
            } : Handle::C{},
            Arithmetics & cfg ? Handle::A{
                handle_negate<T>,
                handle_plus<T>,
                handle_minus<T>,
                handle_multiplies<T>,
                handle_divides<T>,
                handle_modulus<T>
            } : Handle::A{},
            Bitwise & cfg ? Handle::B{
                handle_complement<T>,
                handle_bit_and<T>,
                handle_bit_or<T>,
                handle_bit_xor<T>
            } : Handle::B{},
            Shift & cfg ? Handle::S{
                handle_left_shift<T>,
                handle_right_shift<T>
            } : Handle::S{},
            Logic & cfg ? Handle::L{
                handle_is_truth<T>,
                handle_logical_and<T>,
                handle_logical_or<T>
            } : Handle::L{},
        };
    }

    Handle handle_;

    explicit Operator(
        Handle const handle
    );

    static bool exchangeHandle(Handle& handle, bool const retrieve);

public:

    /// Default operator with generic transforms
    Operator();

    /// Operator with T as type decorator
    template <class T>
    Operator(type_tag<T> tag, Config const cfg)
    : Operator{makeHandle(tag, cfg)}
    {
    }

    /// Operator with T as type decorator
    template <class T>
    Operator(type_tag<T> tag)
    : Operator{tag, Config::Default}
    {
    }

    /// Retrieve registered operator instance if it exists, throw otherwise
    Operator(boost::json::string_view annotation);

    Operator(Operator const&) = default;
    Operator(Operator &&) = default;
    virtual ~Operator() = default;
    Operator& operator=(Operator const&) = default;
    Operator& operator=(Operator &&) = default;

    bool is_generic() const
    {
        return annotation().empty();
    }

    /// decorated type name
    boost::json::string annotation() const
    {
        return handle_.annotation;
    }

    /// Reserialize as decorated type
    boost::json::value decorate(boost::json::value const& a) const
    {
        return handle_.decor.decorate(a);
    }

    /// \brief Apply operands
    /// \details For unary operators, lhs is nullptr
    boost::json::value apply(Keyword const& keyword, LV lhs, LV rhs) const;


private:

    /// Is subset of
    V is_subset(LV const& lhs, LV const& rhs) const;

    /// Is element of
    V contains(LV const& set, LV const& element) const;

};

} // namespace impl
} // namespace haikan
