/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <boost/optional.hpp>

#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/reflect_serialize_fallback.hpp"
#include "haikan/reflection_meta.hpp"

namespace haikan {
namespace impl {


template <class T, class=void>
struct reflect_serialize_default
{
    static boost::optional<ReflectionMeta&> get_meta(sol::state_view L, std::false_type)
    {
        return get_rmeta(L, type<T>);
    }

    static boost::optional<ReflectionMeta&> get_meta(sol::state_view L, std::true_type)
    {
        return get_rmeta(L, type<user_data_enum<T>>);
    }

    static sol::object make_serialize_value(T const& obj, sol::state_view L, std::false_type)
    {
        return sol::make_object(L.lua_state(), std::ref(const_cast<T&>(obj)));
    }

    static sol::object make_serialize_value(T const& obj, sol::state_view L, std::true_type)
    {
        return sol::make_object(L.lua_state(), user_data_enum<T>{obj});
    }

    static sol::object serialize(T const& obj, sol::state_view L)
    {
        auto const meta = get_meta(L, std::is_enum<T>{});
        if (meta)
        {
            sol::protected_function const& serialize_fn = meta->serialize;
            if (serialize_fn)
            {
                sol::object value_in = make_serialize_value(obj, L, std::is_enum<T>{});
                sol::protected_function_result result = serialize_fn(value_in);
                if (result.valid())
                {
                    return result.get<sol::object>();
                }
            }
        }
        return reflect_serialize_fallback<T>::serialize(obj, L);
    }

    static boost::optional<T> deserialize(sol::object obj)
    {
        sol::state_view L(obj.lua_state());
        auto const meta = get_meta(L, std::is_enum<T>{});
        if (meta)
        {
            sol::protected_function const& deserialize_fn = meta->deserialize;
            if (deserialize_fn)
            {
                sol::protected_function_result result = deserialize_fn(obj);
                if (result.valid())
                {
                    sol::object value = result.get<sol::object>();
                    auto deserialized = reflect_serialize_fallback<T>::deserialize(value);
                    if (deserialized)
                    {
                        return deserialized;
                    }
                }
            }
        }
        return reflect_serialize_fallback<T>::deserialize(obj);
    }
};


template <class T>
struct reflect_serialize_default<std::reference_wrapper<T>>
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

template <class T>
struct reflect_serialize_default<std::shared_ptr<T>>
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
        auto value = reflect<T>::deserialize(obj);
        if (!value)
        {
            return boost::none;
        }
        return std::make_shared<T>(std::move(value.value()));
    }
};

template <class T>
struct reflect_serialize_default<T, mp_if<is_array_like_container<T>, void>>
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
            return deserialize_array_like<T>(tbl, has_member_push_back<T>{}, has_member_insert_value<T>{});
        }
        return boost::none;
    }

    private:
    template <class U>
    static void reserve_if_possible(U& cont, std::size_t size, std::true_type)
    {
        cont.reserve(size);
    }

    template <class U>
    static void reserve_if_possible(U&, std::size_t, std::false_type)
    {
    }

    template <class U>
    static boost::optional<U> deserialize_array_like(sol::table tbl, std::true_type, std::false_type)
    {
        U cont{};
        reserve_if_possible(cont, tbl.size(), has_member_reserve<U>{});
        for (std::size_t i = 1; i <= tbl.size(); i++)
        {
            auto item = reflect<typename U::value_type>::deserialize(tbl[i]);
            if (!item)
            {
                return boost::none;
            }
            cont.push_back(std::move(*item));
        }
        return cont;
    }

    template <class U>
    static boost::optional<U> deserialize_array_like(sol::table tbl, std::false_type, std::true_type)
    {
        U cont{};
        reserve_if_possible(cont, tbl.size(), has_member_reserve<U>{});
        for (std::size_t i = 1; i <= tbl.size(); i++)
        {
            auto item = reflect<typename U::value_type>::deserialize(tbl[i]);
            if (!item)
            {
                return boost::none;
            }
            cont.insert(std::move(*item));
        }
        return cont;
    }

    template <class U>
    static boost::optional<U> deserialize_array_like(sol::table tbl, std::false_type, std::false_type)
    {
        static_assert(has_subscript<U>::value, "Array-like containers must support push_back, insert, or operator[]");
        U cont{};
        if (tbl.size() != cont.size())
        {
            return boost::none;
        }
        for (std::size_t i = 1; i <= tbl.size(); i++)
        {
            auto item = reflect<typename U::value_type>::deserialize(tbl[i]);
            if (!item)
            {
                return boost::none;
            }
            cont[i - 1] = std::move(*item);
        }
        return cont;
    }
};



template <class T1, class T2>
struct reflect_serialize_default<std::pair<T1, T2>>
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
};


} // namespace impl
} // namespace haikan
