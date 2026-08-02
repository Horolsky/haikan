/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <boost/describe.hpp>
#include <cstddef>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include "haikan/impl/reflect_default_next_fn.hpp"

#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/reflect_default_enum.hpp"
#include "haikan/reflection_context.hpp"
#include "haikan/reflection_meta.hpp"


namespace haikan {
namespace impl {

template <class T>
struct default_reflect_serialize;


template <class>
struct describe_bases_for_sol;

template <template <class...> class L, class... D>
struct describe_bases_for_sol<L<D...>> : sol::bases<typename D::type...> {};

template <class MemT, class=void>
struct make_field;

template <class MemT>
struct make_field<MemT, mp_if<mp_not<std::is_enum<MemT>>, void>>
{
    template <class T>
    static decltype(auto) make(MemT T::* memptr)
    {
        return memptr;
    }
};

template <class MemT>
struct make_field<MemT, mp_if<std::is_enum<MemT>, void>>
{
    template <class T>
    static decltype(auto) make(MemT T::* memptr)
    {
        using udenum = haikan::impl::user_data_enum<MemT>;
        return sol::property(
            [memptr](T const& obj) -> udenum
            {
                return udenum(obj.*memptr);
            },
            [memptr](T& obj, sol::object value) -> MemT
            {
                MemT ee{};
                if (value.is<udenum>())
                {
                    ee = static_cast<MemT>(value.as<udenum>());
                }
                else if(value.get_type() == sol::type::number)
                {
                    ee = static_cast<MemT>(value.as<typename udenum::underlying_type>());
                }
                else if(value.get_type() == sol::type::string)
                {
                    ee = static_cast<MemT>(udenum(value.as<std::string>()));
                }
                obj.*memptr = ee;
                return obj.*memptr;
            }
        );
    }
};


template <class T, class Seen = mp_list<>, class = void>
struct default_reflect_struct_impl;

template <class T, class Seen>
struct default_reflect_struct_impl<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, T>>, mp_not<has_custom_reflect_utype<T>>>, void>>
{
    using ThisType = default_reflect_struct_impl<T, Seen>;
    using Descr = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_inherited>;

    static sol::object serialize(T const& obj, sol::state_view L)
    {
        if (default_reflect_serialize<T>::metadata(L))
        {
            return serialize_impl(obj, L);
        }
        return default_reflect_serialize<T>::serialize_fallback(obj, L);
    }

    static sol::object serialize_impl(T const& obj, sol::state_view L)
    {
        sol::table tbl = L.create_table(0, static_cast<int>(mp_size<Descr>::value));
        boost::mp11::mp_for_each<Descr>([&](auto descr) {
            using MemT = std::remove_reference_t<decltype(std::declval<T>().*descr.pointer)>;
            tbl.set(descr.name, reflect<MemT>::serialize(obj.*(descr.pointer), L));
        });
        return tbl;
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        sol::state_view L(obj.lua_state());
        if (default_reflect_serialize<T>::metadata(L))
        {
            return deserialize_impl(obj);
        }
        return default_reflect_serialize<T>::deserialize_fallback(obj);
    }

    static boost::optional<T> deserialize_impl(sol::object obj)
    {
        if (obj.get_type() == sol::type::table)
        {
            bool err {false};
            sol::table tbl = obj;
            auto init = reflect<T>::init();
            if (!init)
            {
                return boost::none;
            }
            T t {init.value()};
            boost::mp11::mp_for_each<Descr>([&](auto descr) {
                if (err) return;
                using MemT = std::remove_reference_t<decltype(std::declval<T>().*descr.pointer)>;
                auto new_member = reflect<MemT>::deserialize(tbl[descr.name]);
                if (new_member)
                {
                    t.*descr.pointer = new_member.value();
                }
                else
                {
                    err = true;
                }
            });
            return t;
        }
        return boost::none;
    }

    static void utype(ReflectionContext& ctx)
    {
        static_assert(not std::is_union<T>::value, "union types are not supported by default, provide a specialization");
        using Bases = boost::describe::describe_bases<T, boost::describe::mod_public>;

        auto usertype = ctx.get_registration_table(impl::type<T>);
        sol::table meta = usertype.hmeta();

        usertype.set(sol::meta_function::construct, [](){ return reflect<T>::init().value(); });
        usertype.set(sol::meta_function::call_function, [](){ return reflect<T>::init().value(); });
        usertype.set(sol::base_classes, describe_bases_for_sol<Bases>());

        sol::table members = sol::state_view(meta.lua_state()).create_table(static_cast<int>(mp_size<Descr>::value), 0);
        // registering described member vars
        std::size_t member_index = 1;
        boost::mp11::mp_for_each<Descr>([&](auto descr) {
            using MemT = std::remove_reference_t<decltype(std::declval<T>().*descr.pointer)>;
            usertype.set(descr.name, make_field<MemT>::make(descr.pointer));
            // usertype.set(descr.name, descr.pointer);
            members[member_index++] = descr.name;
            reflect<remove_qualifiers_t<MemT>>::utype(ctx);
        });
        meta["members"] = members;

        sol::function next_fn = make_reflect_default_next_fn(members);
        usertype.set(sol::meta_function::pairs, make_reflect_default_pairs_fn(next_fn));
        usertype.set(sol::meta_function::next, next_fn);

        meta.set("serialize", sol::as_function([](T const& obj, sol::this_state state) {
            return ThisType::serialize_impl(obj, sol::state_view(state));
        }));

        meta.set("deserialize", sol::as_function([](sol::object obj) {
            return ThisType::deserialize_impl(obj);
        }));
    }
};


} // namespace impl
} // namespace haikan
