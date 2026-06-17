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
    explicit LuaState(
        std::size_t initial_capacity = HAIKAN_LUA_STATE_INITIAL_CAPACITY,
        std::size_t max_capacity = 0);
    LuaState(LuaState const&) = delete;
    LuaState(LuaState&& other) noexcept;

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

    void open_libraries();

    std::size_t used_memory() const noexcept;

  private:
    struct MemoryPool;

    std::unique_ptr<MemoryPool> pool_;
    lua_State* state_;
};

}
}
