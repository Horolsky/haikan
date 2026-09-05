/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <sol/sol.hpp>


namespace haikan {
namespace impl {

sol::table get_or_create_table(sol::state_view L, sol::table parent, char const* name);
sol::table get_or_create_table(sol::state_view L, sol::table parent, std::size_t const key);

} // namespace impl
} // namespace haikan
