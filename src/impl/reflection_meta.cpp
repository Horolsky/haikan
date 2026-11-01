/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "include/haikan/reflection_meta.hpp"

#include <boost/none.hpp>
#include <boost/none_t.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


namespace haikan {
namespace impl {


boost::optional<ReflectionMeta&> get_rmeta(sol::state_view L, std::size_t type_hash)
{
    sol::object root = L.globals()["haikan"];
    if (root.get_type() != sol::type::table)
    {
        return boost::none;
    }

    sol::object utypes = root.as<sol::table>()["utypes"];
    if (utypes.get_type() != sol::type::table)
    {
        return boost::none;
    }

    sol::object meta = utypes.as<sol::table>()[type_hash];
    if (!meta.is<ReflectionMeta>())
    {
        return boost::none;
    }
    return meta.as<ReflectionMeta&>();
}


} // namespace impl
} // namespace haikan
