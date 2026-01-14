/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <type_traits>
#include <tuple>

#include <boost/callable_traits.hpp>
#include <boost/mp11.hpp>
#include <boost/type_traits.hpp>

#define HAIKAN_CAT(a, b) a##b

#define HAIKAN_DEFINE_TRAIT_HAS_TYPE_IMPL(trait, name)     \
    template <class T, class = void>         \
    struct trait : std::false_type {};       \
    template <class T>                       \
    struct trait<                            \
        T, ::haikan::impl::void_t<typename T::name>>   \
    : std::true_type {};


#define HAIKAN_DEFINE_TRAIT_HAS_MEMBER_IMPL(trait, name)           \
    template <class T, class = void>                 \
    struct trait : std::false_type {};               \
    template <class T>                               \
    struct trait<T,                                  \
    ::haikan::impl::void_t<decltype(std::declval<T>().name())>>\
    : std::true_type {};


#define HAIKAN_DEFINE_TRAIT_HAS_TYPE(name) HAIKAN_DEFINE_TRAIT_HAS_TYPE_IMPL(HAIKAN_CAT(has_type_, name), name)
#define HAIKAN_DEFINE_TRAIT_HAS_MEMBER(name) HAIKAN_DEFINE_TRAIT_HAS_MEMBER_IMPL(HAIKAN_CAT(has_member_, name), name)


namespace haikan {
namespace impl {

using namespace boost::mp11;

template <class...> using void_t = void;

/// Variable template that checks if a type has begin() and end() member functions
template <class, class = void>
struct is_const_iterable : std::false_type {};

template <class T>
struct is_const_iterable<T, void_t<decltype(std::declval<T>().cbegin()), decltype(std::declval<T>().cend())>> : std::true_type {};

template <class T>
using is_tuple = mp_or< mp_similar<std::tuple<>, T>, mp_similar<std::pair<void, void>, T> >;



/// expand boost::has_operator to containers and tuples
#define HAIKAN_WRAP_BOOST_TRAIT(has_operator) \
template <class T, class E = void> struct has_operator; \
template <class T> \
struct has_operator<T, std::enable_if_t<not is_const_iterable<T>::value and not is_tuple<T>::value>> : boost::has_operator<T> {}; \
template <class T> \
struct has_operator<T, std::enable_if_t<is_const_iterable<T>::value>> : has_operator<typename T::value_type> {}; \
template <class T> \
struct has_operator<T, std::enable_if_t<is_tuple<T>::value>> : mp_all_of<T, has_operator> {};

HAIKAN_WRAP_BOOST_TRAIT(has_bit_and)
HAIKAN_WRAP_BOOST_TRAIT(has_bit_or)
HAIKAN_WRAP_BOOST_TRAIT(has_bit_xor)
HAIKAN_WRAP_BOOST_TRAIT(has_complement)
HAIKAN_WRAP_BOOST_TRAIT(has_divides)
HAIKAN_WRAP_BOOST_TRAIT(has_equal_to)
HAIKAN_WRAP_BOOST_TRAIT(has_greater)
HAIKAN_WRAP_BOOST_TRAIT(has_greater_equal)
HAIKAN_WRAP_BOOST_TRAIT(has_left_shift)
HAIKAN_WRAP_BOOST_TRAIT(has_right_shift)
HAIKAN_WRAP_BOOST_TRAIT(has_less)
HAIKAN_WRAP_BOOST_TRAIT(has_less_equal)
HAIKAN_WRAP_BOOST_TRAIT(has_logical_and)
HAIKAN_WRAP_BOOST_TRAIT(has_logical_not)
HAIKAN_WRAP_BOOST_TRAIT(has_logical_or)
HAIKAN_WRAP_BOOST_TRAIT(has_minus)
HAIKAN_WRAP_BOOST_TRAIT(has_modulus)
HAIKAN_WRAP_BOOST_TRAIT(has_multiplies)
HAIKAN_WRAP_BOOST_TRAIT(has_negate)
HAIKAN_WRAP_BOOST_TRAIT(has_not_equal_to)
HAIKAN_WRAP_BOOST_TRAIT(has_plus)


#undef HAIKAN_WRAP_BOOST_TRAIT

template <class T>
using remove_qualifiers = std::remove_const<std::remove_reference_t<T>>;

template <class T>
using remove_qualifiers_t = typename remove_qualifiers<T>::type;

/// expose type info for readable static assertions
template <class... T>
constexpr bool failing_on{false};

}  // namespace impl
}  // namespace haikan
