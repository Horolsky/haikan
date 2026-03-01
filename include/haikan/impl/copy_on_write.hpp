/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <memory>
#include <set>
#include <ostream>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

namespace haikan {
namespace impl {

class Cow
{
  public:
    struct State
    {
        sol::table original;
        sol::table tracked;
        sol::table delta;
        sol::table deleted;
        sol::table proxies;
        std::set<std::string> keys{};
        std::set<std::int64_t> ikeys{};
        std::size_t length{0};
    };

    static sol::object make(sol::object original);

    Cow(Cow const&) = default;
    Cow(Cow &&) = default;

    Cow& operator=(Cow const&) = default;
    Cow& operator=(Cow &&) = default;

    ~Cow() = default;

    Cow(sol::table original)
        : Cow(original, sol::state_view::create_table(original.lua_state()))
    {
    }

    /// String keys for object-like table
    std::set<std::string> const& keys() const
    {
        return state_->keys;
    }

    /// Integer keys (non-sequential) for object-like table
    std::set<std::int64_t> const& ikeys() const
    {
        return state_->ikeys;
    }

    std::size_t length() const
    {
        return state_->length;
    }

private:

    Cow(sol::table original, sol::table tracked);

    static void register_utype(sol::state_view L);

    void mf_new_index(sol::object const key, sol::object value, sol::this_state _s);

    sol::object mf_index(sol::object const key, sol::this_state _s);

    void del_at_key(std::string const& key, sol::this_state _s);
    void del_at_idx(std::int64_t const idx, sol::this_state _s);

    void insert_at_key(std::string const& key, sol::object const value, sol::this_state _s);
    void insert_at_idx(std::int64_t const idx, sol::object const value, sol::this_state _s);


  private:
    std::shared_ptr<State> state_;
};


}  // namespace impl
}  // namespace haikan
