/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


namespace haikan {
namespace impl {

sol::function make_reflect_default_next_fn(std::vector<std::string> const& members, sol::state_view state);
sol::function make_reflect_default_pairs_fn(sol::function const& next_fn);

}
}
