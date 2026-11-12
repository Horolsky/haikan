/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <exception>
#include <string>

#include <boost/current_function.hpp>

#include "haikan/error.hpp"
#include "haikan/reflection_meta.hpp"
#include "haikan/serialization.hpp"


namespace haikan {

sol::object to_plain_lua(sol::object value)
{
    if (value.get_type() != sol::type::userdata)
    {
        return value;
    }
    lua_State* const state = value.lua_state();
    sol::object metadata;

    value.push();
    if (lua_getmetatable(state, -1) != 0)
    {
        lua_pushliteral(state, "meta");
        lua_rawget(state, -2);
        metadata = sol::stack::get<sol::object>(state, -1);
        lua_pop(state, 3);
    }
    else
    {
        lua_pop(state, 1);
    }

    if (!metadata.is<ReflectionMeta>())
    {
        return sol::make_object(
            state, error("no reflection found", BOOST_CURRENT_FUNCTION));
    }

    try
    {
        ReflectionMeta const& meta = metadata.as<ReflectionMeta const&>();
        if (meta.serialize)
        {
            return meta.serialize(std::move(value));
        }
    }
    catch(const std::exception&)
    {
    }

    return sol::make_object(
        state, error("runtime serialization failure", BOOST_CURRENT_FUNCTION));
}
}  // namespace haikan
