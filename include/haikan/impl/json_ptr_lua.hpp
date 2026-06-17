/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <boost/utility/string_view.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


namespace haikan {
namespace impl {

/// @brief JSON Pointer Lua getter
/// @param obj target object
/// @param ptr JSON Pointer
/// @return Object node at given pointer
sol::object at_pointer(sol::object obj, boost::string_view ptr);


/// @brief JSON Pointer Lua setter
/// @param obj target object
/// @param ptr JSON Pointer
/// @param value New node object
/// @return Object node at given pointer
sol::object set_at_pointer(sol::object& obj, boost::string_view ptr, sol::object value);

}
}
