/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/describe.hpp>


#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/lua_state.hpp"


namespace haikan {
namespace impl {

struct Args
{
    sol::object lhs {lua::make_object("none")};
    sol::object rhs {lua::make_object("none")};
};


BOOST_DESCRIBE_STRUCT(Args, (), (lhs, rhs));

} // namespace impl
} // namespace haikan
