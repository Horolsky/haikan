/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <cstddef>
#include <string>
#include <vector>

#include "haikan/impl/default_next_fn.hpp"



namespace haikan {
namespace impl {

namespace {

int reflect_default_next_fn(lua_State* L)
{
    auto const& members = sol::stack::get<std::vector<std::string>>(L, lua_upvalueindex(1));
    int const key_type = lua_type(L, 2);
    if (members.empty() || (key_type != LUA_TNIL && key_type != LUA_TSTRING))
    {
        return 0;
    }

    std::size_t next_index = 0;
    if (key_type == LUA_TSTRING)
    {
        std::string const key = sol::stack::get<std::string>(L, 2);
        next_index = members.size();
        for (std::size_t i = 0; i + 1 < members.size(); ++i)
        {
            if (members[i] == key)
            {
                next_index = i + 1;
                break;
            }
        }
    }

    if (next_index == members.size())
    {
        return 0;
    }

    sol::stack::push(L, members[next_index]);
    lua_pushvalue(L, -1);
    lua_gettable(L, 1);
    return 2;
}

int reflect_default_pairs_fn(lua_State* L)
{
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}

} // namespace

sol::function make_reflect_default_next_fn(std::vector<std::string> const& members, sol::state_view state)
{
    lua_State* L = state.lua_state();
    sol::stack::push(L, members);
    lua_pushcclosure(L, reflect_default_next_fn, 1);
    sol::function result(L, -1);
    lua_pop(L, 1);
    return result;
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
