/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <boost/describe.hpp>
#include <boost/format.hpp>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include "haikan/impl/traits.hpp"
#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/reflect_default_init.hpp"
#include "haikan/impl/reflect_default_struct.hpp"
#include "haikan/impl/reflect_default_enum.hpp"
#include "haikan/impl/reflect_default_serialize.hpp"


namespace haikan {
namespace impl {


template <class T, class Seen = mp_list<>, class = void>
struct default_reflect_utype;

// DESCRIBED ENUMS AND STRUCTS

template <class T, class Seen>
struct default_reflect_utype<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, T>>, boost::describe::has_describe_members<T>>, void>>
 : default_reflect_struct_impl<T, Seen>
{
};

template <class T, class Seen>
struct default_reflect_utype<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, user_data_enum<T>>>, boost::describe::has_describe_enumerators<T>>, void>>
 : default_reflect_enum_impl<T, Seen>
{
};


// MISC

template <class T, class Seen>
struct default_reflect_utype<std::reference_wrapper<T>, Seen>
    : default_reflect_utype<remove_qualifiers_t<T>, Seen>
{
    static sol::object serialize(std::reference_wrapper<T> const& value, sol::state_view L)
    {
        return reflect<T>::serialize(value.get(), L);
    }

    static boost::optional<std::reference_wrapper<T>> deserialize(sol::object obj)
    {
        static_assert(failing_on<std::reference_wrapper<T>>, "Can't deserialize to non-owning types");
        return boost::none;
    }
};

template <class T, class Seen>
struct default_reflect_utype<std::shared_ptr<T>, Seen>
    : default_reflect_utype<remove_qualifiers_t<T>, Seen>
{
    static sol::object serialize(std::shared_ptr<T> const& value, sol::state_view L)
    {
        if (value)
        {
            return reflect<T>::serialize(*value, L);
        }
        return sol::nil;
    }

    static boost::optional<std::shared_ptr<T>> deserialize(sol::object obj)
    {
        return std::make_shared<T>(reflect<T>::deserialize(obj));
    }
};

namespace detail {

template <class T, class = void>
struct has_type_key_type : std::false_type {};

template <class T>
struct has_type_key_type<T, void_t<typename T::key_type>> : std::true_type {};

template <class T, class = void>
struct has_member_reserve : std::false_type {};

template <class T>
struct has_member_reserve<T, void_t<decltype(std::declval<T&>().reserve(std::declval<std::size_t>()))>> : std::true_type {};

template <class T, class = void>
struct has_member_push_back : std::false_type {};

template <class T>
struct has_member_push_back<T, void_t<decltype(std::declval<T&>().push_back(std::declval<typename T::value_type>()))>>
    : std::true_type {};

template <class T, class = void>
struct has_member_insert_value : std::false_type {};

template <class T>
struct has_member_insert_value<T, void_t<decltype(std::declval<T&>().insert(std::declval<typename T::value_type>()))>>
    : std::true_type {};

template <class T, class = void>
struct has_subscript : std::false_type {};

template <class T>
struct has_subscript<T, void_t<decltype(std::declval<T&>()[std::declval<std::size_t>()])>> : std::true_type {};

template <class T>
struct is_string_like : std::is_convertible<T, std::string> {};

template <class T>
using is_array_like_container = mp_and<
    sol::is_container<T>,
    mp_not<has_type_key_type<T>>,
    mp_not<is_string_like<T>>
>;

template <class T>
void reserve_if_possible(T& cont, std::size_t size, std::true_type)
{
    cont.reserve(size);
}

template <class T>
void reserve_if_possible(T&, std::size_t, std::false_type)
{
}

template <class T>
boost::optional<T> deserialize_array_like(sol::table tbl, std::true_type, std::false_type)
{
    T cont{};
    reserve_if_possible(cont, tbl.size(), has_member_reserve<T>{});
    for (std::size_t i = 1; i <= tbl.size(); i++)
    {
        auto item = reflect<typename T::value_type>::deserialize(tbl[i]);
        if (!item)
        {
            return boost::none;
        }
        cont.push_back(std::move(*item));
    }
    return cont;
}

template <class T>
boost::optional<T> deserialize_array_like(sol::table tbl, std::false_type, std::true_type)
{
    T cont{};
    reserve_if_possible(cont, tbl.size(), has_member_reserve<T>{});
    for (std::size_t i = 1; i <= tbl.size(); i++)
    {
        auto item = reflect<typename T::value_type>::deserialize(tbl[i]);
        if (!item)
        {
            return boost::none;
        }
        cont.insert(std::move(*item));
    }
    return cont;
}

template <class T>
boost::optional<T> deserialize_array_like(sol::table tbl, std::false_type, std::false_type)
{
    static_assert(has_subscript<T>::value, "Array-like containers must support push_back, insert, or operator[]");
    T cont{};
    if (tbl.size() != cont.size())
    {
        return boost::none;
    }
    for (std::size_t i = 1; i <= tbl.size(); i++)
    {
        auto item = reflect<typename T::value_type>::deserialize(tbl[i]);
        if (!item)
        {
            return boost::none;
        }
        cont[i - 1] = std::move(*item);
    }
    return cont;
}

}

template <class T, class Seen>
struct default_reflect_utype<T, Seen, mp_if<detail::is_array_like_container<T>, void>>
{
    static sol::object serialize(T const& cont, sol::state_view L)
    {
        sol::table tbl = L.create_table(cont.size(), 0);
        for (auto const& item: cont)
        {
            tbl.add(reflect<typename T::value_type>::serialize(item, L));
        }
        return tbl;
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        if (obj.get_type() == sol::type::table)
        {
            sol::table tbl = obj;
            return detail::deserialize_array_like<T>(tbl, detail::has_member_push_back<T>{}, detail::has_member_insert_value<T>{});
        }
        return boost::none;
    }

    static void utype(ReflectionContext& ctx)
    {
        return reflect<remove_qualifiers_t<typename T::value_type>>::utype(ctx);
    }
};

template <class T1, class T2, class Seen>
struct default_reflect_utype<std::pair<T1, T2>, Seen>
{
    static sol::object serialize(std::pair<T1, T2> const& pair, sol::state_view L)
    {
        sol::table tbl = L.create_table(2,0);
        tbl[1] = reflect<T1>::serialize(pair.first, L);
        tbl[2] = reflect<T2>::serialize(pair.second, L);
        return tbl;
    }

    static boost::optional<std::pair<T1, T2>> deserialize(sol::object obj)
    {
        if (obj.get_type() == sol::type::table)
        {
            sol::table tbl = obj;
            auto first = reflect<T1>::deserialize(tbl[1]);
            auto second = reflect<T2>::deserialize(tbl[2]);
            if (!first || !second)
            {
                return boost::none;
            }
            return std::make_pair(std::move(*first), std::move(*second));
        }
        return boost::none;
    }


    static void utype(ReflectionContext& ctx)
    {
        reflect<remove_qualifiers_t<T1>>::utype(ctx);
        reflect<remove_qualifiers_t<T2>>::utype(ctx);
    }
};



namespace detail {
template <typename T> using reflect_utype_impl_t = decltype(default_reflect_utype<T>::utype(std::declval<ReflectionContext&>()));
}


// Registered type specialization
template <class T>
struct default_reflect<T, mp_if<mp_valid<detail::reflect_utype_impl_t, T>, void>>
    : default_reflect_init<T>
    , default_reflect_utype<T>
    , default_reflect_serialize<T>
{
    static sol::object serialize(T const& value, sol::state_view L)
    {
        return default_reflect_utype<T>::serialize(value, L);
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        return default_reflect_utype<T>::deserialize(obj);
    }
};


// Fallback specialization
template <class T>
struct default_reflect<T, mp_if<mp_not<mp_valid<detail::reflect_utype_impl_t, T>>, void>>
    : default_reflect_init<T>
{
    // TODO: use binary serialization for fallback
    static sol::object serialize(T const& value, sol::state_view L)
    {
        return sol::make_object(L.lua_state(), value);
    }
    static boost::optional<T> deserialize(sol::object obj)
    {
        if (obj.is<T>())
        {
            return obj.as<T>();
        }
        return boost::none;
    }
    static void utype(ReflectionContext&)
    {
    }

};


} // namespace impl
} // namespace haikan
