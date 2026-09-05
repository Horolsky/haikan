/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <boost/describe.hpp>

#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/reflect_init_default.hpp"
#include "haikan/impl/reflect_utype_default_struct.hpp"
#include "haikan/impl/reflect_utype_default_enum.hpp"
#include "haikan/impl/reflect_serialize_default.hpp"
#include "haikan/impl/reflect_serialize_fallback.hpp"


namespace haikan {
namespace impl {


template <class T, class Seen = mp_list<>, class = void>
struct reflect_utype_default;

// DESCRIBED ENUMS AND STRUCTS

template <class T, class Seen>
struct reflect_utype_default<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, T>>, boost::describe::has_describe_members<T>>, void>>
 : default_reflect_struct_impl<T, Seen>
{
};

template <class T, class Seen>
struct reflect_utype_default<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, user_data_enum<T>>>, boost::describe::has_describe_enumerators<T>>, void>>
 : default_reflect_enum_impl<T, Seen>
{
};


// MISC

template <class T, class Seen>
struct reflect_utype_default<std::reference_wrapper<T>, Seen>
    : reflect_utype_default<remove_qualifiers_t<T>, Seen>
{
};

template <class T, class Seen>
struct reflect_utype_default<std::shared_ptr<T>, Seen>
    : reflect_utype_default<remove_qualifiers_t<T>, Seen>
{
};


template <class T, class Seen>
struct reflect_utype_default<T, Seen, mp_if<is_array_like_container<T>, void>>
{
    static void utype(ReflectionContextFactory& ctx)
    {
        return reflect<remove_qualifiers_t<typename T::value_type>>::utype(ctx);
    }
};

template <class T1, class T2, class Seen>
struct reflect_utype_default<std::pair<T1, T2>, Seen>
{
    static void utype(ReflectionContextFactory& ctx)
    {
        reflect<remove_qualifiers_t<T1>>::utype(ctx);
        reflect<remove_qualifiers_t<T2>>::utype(ctx);
    }
};



namespace detail {
template <typename T> using reflect_utype_impl_t = decltype(reflect_utype_default<T>::utype(std::declval<ReflectionContextFactory&>()));
}


// Registered type specialization
template <class T>
struct reflect_default<T, mp_if<mp_valid<detail::reflect_utype_impl_t, T>, void>>
    : reflect_init_default<T>
    , reflect_utype_default<T>
    , reflect_serialize_default<T>
{
};


// Fallback specialization
template <class T>
struct reflect_default<T, mp_if<mp_not<mp_valid<detail::reflect_utype_impl_t, T>>, void>>
    : reflect_init_default<T>
    , reflect_serialize_fallback<T>
{
    static void utype(ReflectionContextFactory&)
    {
    }
};


} // namespace impl
} // namespace haikan
