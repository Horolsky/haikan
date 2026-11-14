/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ostream>

#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

namespace haikan {
namespace impl {

std::ostream& lua_to_stream(
    std::ostream& os, sol::object const value, int const indent, bool const pretty_print = true);


} // namespace impl
} // namespace haikan
