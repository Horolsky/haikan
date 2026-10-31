/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <memory>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#ifndef HAIKAN_LUA_STATE_INITIAL_CAPACITY
#define HAIKAN_LUA_STATE_INITIAL_CAPACITY (1024*1024)
#endif


namespace haikan {
namespace impl {

/// @brief Lua state handler with custom memory pool
class LuaState
{
  public:
    LuaState(
        std::size_t initial_capacity,
        std::size_t max_capacity);
    LuaState(LuaState const&) = delete;
    LuaState(LuaState&& other) noexcept;

    explicit LuaState(std::size_t initial_capacity)
        : LuaState(initial_capacity, 0)
    {
    }

    LuaState()
        : LuaState(HAIKAN_LUA_STATE_INITIAL_CAPACITY, 0)
    {
    }

    LuaState& operator=(LuaState const&) = delete;
    LuaState& operator=(LuaState&& other) noexcept;

    ~LuaState();

    lua_State* lua_state() noexcept
    {
        return state_;
    }

    lua_State const* lua_state() const noexcept
    {
        return state_;
    }

    sol::state_view view() noexcept
    {
        return sol::state_view(state_);
    }

    sol::state_view view() const noexcept
    {
        return sol::state_view(const_cast<lua_State*>(state_));
    }

    operator lua_State*() noexcept
    {
        return state_;
    }

    operator sol::state_view() noexcept
    {
        return view();
    }

    void reset();

    void open_libraries();

    std::size_t used_memory() const noexcept;

  private:
    struct MemoryPool;

    std::size_t initial_capacity_;
    std::size_t max_capacity_;
    std::unique_ptr<MemoryPool> pool_;
    lua_State* state_;
};

namespace lua
{
LuaState& global_state_handle();

sol::state_view global_state();


void reset_global_state();

template <class T>
sol::object make_object(T&& value)
{
    return sol::make_object(global_state(), value);
}


template <typename Name>
sol::table create_table(Name&& name, int narr = 0, int nrec = 0) {
    return global_state().create_table(std::forward<Name>(name), narr, nrec);
}

template <typename Name, typename Key, typename Value, typename... Args>
sol::table create_table(Name&& name, int narr, int nrec, Key&& key, Value&& value, Args&&... args) {
    return global_state().create_table(std::forward<Name>(name), narr, nrec, std::forward<Key>(key), std::forward<Value>(value), std::forward<Args>(args)...);
}

template <typename Name, typename... Args>
sol::table create_named_table(Name&& name, Args&&... args) {
    return global_state().create_table(name, std::forward<Args>(args)...);
}

inline sol::table create_table(int narr = 0, int nrec = 0)
{
    return global_state().create_table(narr, nrec);
}


} // namespace lua
} // namespace impl
} // namespace haikan
