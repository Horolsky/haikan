/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <boost/describe.hpp>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <tuple>
#include "haikan/impl/reflect_default_next_fn.hpp"

#include "haikan/impl/register_type.hpp"
#include "haikan/impl/register_enum_impl.hpp"


namespace haikan {
namespace impl {


template <class>
struct describe_bases_for_sol;

template <template <class...> class L, class... D>
struct describe_bases_for_sol<L<D...>> : sol::bases<typename D::type...> {};

template <class MemT, class=void>
struct make_field;

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

template <class MemT>
struct make_field<MemT, mp_if<mp_not<std::is_enum<MemT>>, void>>
{
    template <class T>
    static decltype(auto) make(MemT T::* memptr)
    {
        return memptr;
    }
};


template <class T, class Seen>
struct register_type_impl<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, T>>, boost::describe::has_describe_members<T>>, void>>
{
    using Descr = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_inherited>;

    static void operator()(sol::state_view L)
    {
        static_assert(not std::is_union<T>::value, "union types are not supported by default, provide a specialization");

        sol::reference maybe = L[sol::usertype_traits<T>::name()];
        if (maybe.get_type() == sol::type::userdata)
        {
            // TODO: check and raise error if already registered for different type
            return;
        }

        sol::simple_usertype<T> usertype = L.create_simple_usertype<T>();

        usertype.set(sol::meta_function::construct, [](){ return reflect<T>::init().value(); });
        usertype.set(sol::meta_function::call_function, [](){ return reflect<T>::init().value(); });
        usertype.set(sol::meta_function::garbage_collect, [](T& e){ e.~T(); });


        using Bases = boost::describe::describe_bases<T, boost::describe::mod_public>;
        usertype.set(sol::base_classes, describe_bases_for_sol<Bases>());

        std::vector<char const*> members;
        members.reserve(mp_size<Descr>::value);
        { // registering described member vars
            using Descr = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_inherited>;
            boost::mp11::mp_for_each<Descr>([&](auto descr) {
                using MemT = std::remove_reference_t<decltype(std::declval<T>().*descr.pointer)>;
                usertype.set(descr.name, make_field<MemT>::make(descr.pointer));
                // usertype.set(descr.name, descr.pointer);
                members.emplace_back(descr.name);
                register_type<MemT, mp_push_back<Seen, T>>()(L);
            });
        }

        {
            usertype.set(sol::meta_function::pairs, [](T& self) {
                return std::make_tuple(reflect_default_next_fn, std::ref(self), sol::lua_nil_t{});
            });
            usertype.set(sol::meta_function::next, reflect_default_next_fn);
        }

        usertype.set("__haikan", L.create_table_with(
            "members", members
        ));

        L.set_usertype(usertype);
        // L.traverse_set("haikan","utypes", sol::usertype_traits<T>::name(), 1);
    }
};


} // namespace impl
} // namespace haikan
