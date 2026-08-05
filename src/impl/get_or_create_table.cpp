/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/get_or_create_table.hpp"


namespace haikan {
namespace impl {

sol::table get_or_create_table(sol::state_view L, sol::table parent, char const* name)
{
    sol::object obj = parent[name];
    if (obj.get_type() == sol::type::table)
    {
        return obj;
    }
    sol::table tbl = L.create_table();
    parent[name] = tbl;
    return tbl;
}

} // namespace impl
} // namespace haikan
