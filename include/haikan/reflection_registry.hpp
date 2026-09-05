/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <typeindex>

#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/reflect.hpp"
#include "haikan/impl/type_tag.hpp"
#include "haikan/reflection_context.hpp"


namespace haikan {

class ReflectionRegistry
{

private:
    static void insert(
        std::type_index type_index,
        void (*init_reflection_fn)(ReflectionContextFactory&)
    );

public:

    template <class T>
    static void insert(impl::type_tag<T>, void (*init_reflection_fn)(ReflectionContextFactory&))
    {
        insert(typeid(T), init_reflection_fn);
    }

    template <class T>
    static void insert_auto(impl::type_tag<T>)
    {
        insert(typeid(T), &reflect<T>::utype);
    }

    static void init(sol::state_view L);
};

} // namespace haikan
