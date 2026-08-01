/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <string>

#include "haikan/impl/reflect_default_next_fn.hpp"



namespace haikan {
namespace impl {

namespace {

int reflect_default_next_fn(lua_State* L)
{
    sol::userdata self = sol::stack::get<sol::userdata>(L, 1);
    sol::table members = sol::stack::get<sol::table>(L, lua_upvalueindex(1));

    sol::stack_object current_key(L, 2);
    bool const current_key_is_nil = current_key.is<sol::lua_nil_t>();
    bool const current_key_is_str = (current_key.get_type() == sol::type::string);

    if (members.empty() || !(current_key_is_nil || current_key_is_str))
    {
        return 0;
    }

    if (current_key_is_nil)
    {
        std::string const first_key = members.get<std::string>(1);
        sol::stack::push(L, first_key);
        sol::stack::push(L, self[first_key]);
        return 2;
    }

    std::string const key = current_key.as<std::string>();
    for (std::size_t i = 1; i < members.size(); ++i)
    {
        if (members.get<std::string>(i) != key)
        {
            continue;
        }
        std::string const next_key = members.get<std::string>(i + 1);
        sol::stack::push(L, next_key);
        sol::stack::push(L, self[next_key]);
        return 2;
    }
    return 0;
}

int reflect_default_pairs_fn(lua_State* L)
{
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}

sol::function make_closure(sol::table const& members, lua_CFunction fn)
{
    lua_State* L = members.lua_state();
    members.push();
    lua_pushcclosure(L, fn, 1);
    sol::function result(L, -1);
    lua_pop(L, 1);
    return result;
}

} // namespace

sol::function make_reflect_default_next_fn(sol::table const& members)
{
    return make_closure(members, reflect_default_next_fn);
}

sol::function make_reflect_default_pairs_fn(sol::function const& next_fn)
{
    lua_State* L = next_fn.lua_state();
    next_fn.push();
    lua_pushcclosure(L, reflect_default_pairs_fn, 1);
    sol::function result(L, -1);
    lua_pop(L, 1);
    return result;
}

}
}
