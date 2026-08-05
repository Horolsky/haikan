/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <string>
#include <utility>

#include <boost/describe.hpp>
#include <boost/optional.hpp>

#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/user_data_enum.hpp"
#include "haikan/reflection_context.hpp"
#include "haikan/reflection_meta.hpp"

namespace haikan {
namespace impl {


template <class T, class Seen = mp_list<>, class = void>
struct default_reflect_enum_impl;

template <class T, class Seen>
struct default_reflect_enum_impl<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, user_data_enum<T>>>, boost::describe::has_describe_enumerators<T>>, void>>
{
    using U = user_data_enum<T>;
    using ThisType = default_reflect_enum_impl<T, Seen>;

    static sol::object serialize_impl(U const& e, sol::state_view L)
    {
        return sol::make_object(L.lua_state(), e.to_string());
    }

    static boost::optional<U> deserialize_impl(sol::object const& obj)
    {
        if (obj.is<U>())
        {
            return obj.as<U>();
        }
        if (obj.is<T>())
        {
            return U{obj.as<T>()};
        }
        if (obj.is<typename U::underlying_type>())
        {
            return U{obj.as<typename U::underlying_type>()};
        }
        if (obj.is<std::string>())
        {
            auto value = enum_stringify<T>::from_string(obj.as<std::string>());
            if (value)
            {
                return U{value.value()};
            }
        }
        return boost::none;
    }

    static void utype(ReflectionContextFactory& ctx)
    {
        using ctors = typename U::sol_constructors;

        auto tbl = ctx.make_registration_context(impl::type<T>, impl::type<U>);
        tbl.usertype_set(sol::meta_function::construct, ctors());
        tbl.usertype_set(sol::call_constructor, ctors());
        tbl.usertype_set("str", &U::to_string);
        tbl.usertype_set("num", &U::value);
        tbl.usertype_set(sol::meta_function::to_string, &U::to_string);

        ReflectionMeta& meta = tbl.reflection_meta();
        meta.serialize = tbl.make_function(&ThisType::serialize_impl);
        meta.deserialize = tbl.make_function([](sol::object obj) -> sol::object {
            auto value = ThisType::deserialize_impl(obj);
            if (value)
            {
                return sol::make_object(obj.lua_state(), std::move(value.value()));
            }
            return sol::make_object(obj.lua_state(), sol::nil);
        });
    }
};


} // namespace impl
} // namespace haikan
