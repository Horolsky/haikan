/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/json.hpp>
#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

namespace haikan {
namespace impl {

boost::optional<boost::json::value> lua_to_json(sol::object const value);
boost::optional<sol::object> json_to_lua(sol::state_view L, boost::json::value const& value);



} // namespace impl
} // namespace haikan
