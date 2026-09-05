/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


namespace haikan {
namespace impl {


boost::optional<sol::object> migrate_to_state(sol::state_view target_state, sol::object object);

} // namespace impl
} // namespace haikan
