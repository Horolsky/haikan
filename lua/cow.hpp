/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <exception>
#include <string>
#include <iostream>

#include <sol/sol.hpp>



namespace lauseke {
namespace impl {


class Cow
{
    sol::state_view lua_;
    sol::function make_cow_;

  public:
    Cow(lua_State* L);

    sol::table make(sol::table const& tbl)
    {
        return make_cow_(tbl);
    }
};

} // namespace impl
} // namespace lauseke
