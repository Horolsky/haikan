/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstring>
#include <functional>
#include <memory>
#include <tuple>

#include "haikan/impl/reflect.hpp"
#include "haikan/impl/register_type.hpp"



namespace haikan {
namespace impl {


template <class T, class Seen>
struct register_type_impl<std::reference_wrapper<T>, Seen>
    : register_type_impl<remove_qualifiers_t<T>, Seen>
{
};

template <class T, class Seen>
struct register_type_impl<std::shared_ptr<T>, Seen>
    : register_type_impl<remove_qualifiers_t<T>, Seen>
{
};


template <class T1, class T2, class Seen>
struct register_type_impl<std::pair<T1, T2>, Seen>
{
    static void operator()(sol::state_view L)
    {
        register_type<remove_qualifiers_t<T1>>()(L);
        register_type<remove_qualifiers_t<T2>>()(L);
    }
};

template <class T, class Seen>
struct register_type_impl<T, Seen, mp_if<sol::is_container<T>, void>>
    : register_type_impl<typename T::value_type, Seen>
{
};

} // namespace impl
} // namespace haikan
