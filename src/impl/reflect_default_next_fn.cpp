/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <vector>

#include "haikan/impl/reflect_default_next_fn.hpp"



namespace haikan {
namespace impl {

int reflect_default_next_fn(lua_State* L) {

    sol::userdata self = sol::stack::get<sol::userdata>(L, 1);
    std::vector<char const*> members = self["__haikan"]["members"];

    sol::stack_object current_key(L, 2);
    bool const current_key_is_nil = current_key.is<sol::lua_nil_t>();
    bool const current_key_is_str = (current_key.get_type() == sol::type::string);

    if (members.empty() || !(current_key_is_nil || current_key_is_str))
    {
        return 0;
    }

    if (current_key_is_nil)
    {
        sol::stack::push(L, members.front());
        sol::stack::push(L, self[members.front()]);
        return 2;
    }

    char const* key_str = current_key.as<char const*>();
    auto item = std::find_if(members.cbegin(), members.cend(), [key_str](char const* const m){
        return 0 == std::strcmp(m, key_str);
    });

    if(item != members.cend())
    {
        item++;
    }

    if (item == members.cend())
    {
        return 0;
    }
    char const* next_key = *item;
    sol::stack::push(L, next_key);
    sol::stack::push(L, self[next_key]);
    return 2;
}
}
}