/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <exception>
#include <string>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}
#include <boost/current_function.hpp>

#include "haikan/error.hpp"
#include "haikan/serialization.hpp"


namespace haikan {

sol::object to_plain_lua(sol::object value)
{
    if (value.get_type() != sol::type::userdata)
    {
        return value;
    }
    sol::userdata userdata = value.as<sol::userdata>();

    sol::protected_function serialize_fn;
    lua_State* const L = value.lua_state();
    value.push();
    if (lua_getmetatable(L, -1) != 0)
    {
        lua_pushliteral(L, "serialize");
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1))
        {
            serialize_fn = sol::stack::get<sol::protected_function>(L, -1);
        }
        lua_pop(L, 3);
    }
    else
    {
        lua_pop(L, 1);
    }

    try
    {
        if (!serialize_fn)
        {
            serialize_fn = userdata["serialize"];
        }
    }
    catch(const std::exception&)
    {
        return sol::make_object(value.lua_state(), error("no reflection found", BOOST_CURRENT_FUNCTION));
    }

    if (serialize_fn)
    {
        sol::protected_function_result result = serialize_fn(value);
        if (result.valid())
        {
            return result.get<sol::object>();
        }
        // else
        // {
        // }
    }
    return sol::make_object(value.lua_state(), error("runtime serialization failure", BOOST_CURRENT_FUNCTION));

    // sol::object metadata = userdata["meta"];
    // if (metadata.is<ReflectionMeta>())
    // {
    //     ReflectionMeta const& meta = metadata.as<ReflectionMeta const&>();
    //     if (meta.serialize.valid())
    //     {
    //         sol::protected_function_result result = meta.serialize(value);
    //         if (result.valid())
    //         {
    //             return result.get<sol::object>();
    //         }
    //     }
    //     return sol::make_object(value.lua_state(), error("runtime serialization failure", BOOST_CURRENT_FUNCTION));
    // }
    // return impl::lua::make_object(sol::nil);
    // return sol::make_object(value.lua_state(), error("corrupted runtime serialization", BOOST_CURRENT_FUNCTION));
}
}  // namespace haikan
