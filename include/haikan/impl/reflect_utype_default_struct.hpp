/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

#include <boost/describe.hpp>
#include <boost/optional.hpp>

#include "haikan/impl/default_next_fn.hpp"
#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/reflect_utype_default_enum.hpp"
#include "haikan/reflection_context.hpp"
#include "haikan/reflection_meta.hpp"
#include "haikan/logger.hpp"


namespace haikan {
namespace impl {


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

template <class MemT>
sol::object serialize_field(MemT const& value, sol::state_view L, std::false_type)
{
    return reflect<MemT>::serialize(value, L);
}

template <class MemT>
sol::object serialize_field(MemT const value, sol::state_view L, std::true_type)
{
    return sol::make_object(L.lua_state(), user_data_enum<MemT>{value}.to_string());
}


template <class T, class Seen = mp_list<>, class = void>
struct default_reflect_struct_impl;

template <class T, class Seen>
struct default_reflect_struct_impl<T, Seen, mp_if<mp_and<mp_not<mp_contains<Seen, T>>, mp_not<has_custom_reflect_utype<T>>>, void>>
{
    using ThisType = default_reflect_struct_impl<T, Seen>;
    using Descr = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_inherited>;

    static sol::object construct_impl(sol::variadic_args args)
    {
        sol::state_view L = args.lua_state();

        T value = reflect<T>::init().value();
        auto arg = args.begin();
        if (arg != args.end())
        {
            sol::object const first = *arg;
            sol::object const usertype = L[sol::usertype_traits<T>::name()];
            if (first == usertype)
            {
                ++arg;
            }
        }
        bool is_error{false};
        boost::mp11::mp_for_each<Descr>([&](auto descr) {
            if (is_error || arg == args.end())
            {
                return;
            }
            using MemT = remove_qualifiers_t<decltype(value.*descr.pointer)>;
            auto member = reflect<MemT>::deserialize(sol::object(*arg));
            if (member)
            {
                value.*descr.pointer = std::move(member.value());
            }
            else
            {
                is_error = true;
            }
            ++arg;
        });

        return is_error
            ? sol::make_object(L, ErrorObject("invalid ctor argument", BOOST_CURRENT_FUNCTION))
            : sol::make_object(L, value);
    }

    static sol::object serialize_impl(T const& obj, sol::this_state state)
    {
        sol::state_view L(state);
        sol::table tbl = L.create_table(0, static_cast<int>(mp_size<Descr>::value));
        boost::mp11::mp_for_each<Descr>([&](auto descr) {
            using MemT = std::remove_reference_t<decltype(std::declval<T>().*descr.pointer)>;
            tbl.set(descr.name, serialize_field(obj.*(descr.pointer), L, std::is_enum<MemT>{}));
        });
        return tbl;
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
            if (err)
            {
                return boost::none;
            }
            return t;
        }
        return boost::none;
    }

    static void utype(ReflectionContextFactory& factory)
    {
        static_assert(not std::is_union<T>::value, "union types are not supported by default, provide a specialization");
        using Bases = boost::describe::describe_bases<T, boost::describe::mod_public>;

        auto ctx = factory.make_registration_context(impl::type<T>);
        ReflectionMeta& meta = ctx.reflection_meta();

        ctx.usertype_set(sol::meta_function::construct, sol::factories(&ThisType::construct_impl));
        ctx.usertype_set(sol::call_constructor, sol::factories(&ThisType::construct_impl));
        ctx.usertype_set(sol::base_classes, describe_bases_for_sol<Bases>());

        meta.members.reserve(mp_size<Descr>::value);

        boost::mp11::mp_for_each<Descr>([&](auto descr) {
            using MemT = std::remove_reference_t<decltype(std::declval<T>().*descr.pointer)>;
            ctx.usertype_set(descr.name, make_field<MemT>::make(descr.pointer));
            meta.members.emplace_back(descr.name);
            reflect<remove_qualifiers_t<MemT>>::utype(factory);
        });

        sol::function next_fn = make_reflect_default_next_fn(meta.members, ctx.state_view());
        ctx.usertype_set(sol::meta_function::pairs, make_reflect_default_pairs_fn(next_fn));
        ctx.usertype_set(sol::meta_function::next, next_fn);

        meta.serialize = ctx.make_function(&ThisType::serialize_impl);

        meta.deserialize = ctx.make_function([](sol::object obj) -> sol::object {
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
