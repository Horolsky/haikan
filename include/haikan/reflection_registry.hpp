/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <utility>
#include <typeindex>
#include <unordered_set>
#include <deque>
#include <vector>

#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/reflect.hpp"
#include "haikan/impl/type_info.hpp"
#include "haikan/impl/type_tag.hpp"
#include "haikan/reflection_context.hpp"


namespace haikan {

class ReflectionRegistry
{

private:
    static void insert(
        std::type_index type_index,
        const char* type_name,
        void (*init_reflection_fn)(ReflectionContext&)
    );

    static sol::object get_utype(std::type_index type_index, sol::state_view L);

public:

    template <class T>
    static void insert(impl::type_tag<T>, void (*init_reflection_fn)(ReflectionContext&))
    {
        insert(typeid(T), impl::type_name<T>().data(), init_reflection_fn);
    }

    template <class T>
    static void insert_auto(impl::type_tag<T>)
    {
        insert(typeid(T), impl::type_name<T>().data(), &reflect<T>::utype);
    }

    static void init(sol::state_view L);

    static boost::optional<sol::table> get_utype(std::size_t ti, sol::state_view L)
    {
        return L["haikan"]["utypes"][ti];
    }

    template <class T>
    static boost::optional<sol::table> get_utype(impl::type_tag<T>, sol::state_view L)
    {
        try
        {
            return get_utype(typeid(T), L).as<sol::table>();
        }
        catch(...)
        {
            return boost::none;
        }
    }
};

} // namespace haikan
