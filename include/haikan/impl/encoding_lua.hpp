/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <algorithm>
#include <list>
#include <ostream>

#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/keyword_grammar.hpp"

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

namespace haikan {
namespace impl {

struct EncodingLua
{
    std::vector<Keyword> keywords;
    std::vector<std::size_t> depth;
    std::vector<sol::object> data;
    // sol::table data;

    static bool is_preproc_token(sol::object const& value);
    static bool is_link_token(sol::object const& value);

    EncodingLua() = default;
    explicit EncodingLua(sol::object value);

    std::size_t size() const;

    bool operator==(EncodingLua const& o) const;

    bool operator!=(EncodingLua const& o) const;

    void push_back(Keyword const& k, std::size_t const d, sol::object v);

    void append_to_root(EncodingLua tail);

    /// Single-pass preprocessing, return true if no multipass tokens left
    bool preprocess();


    /// Deepcopy Lua data
    EncodingLua clone() const;


    // Serialize data to plain Lua
    sol::object to_object(sol::state_view sv = nullptr) const;


    sol::object to_json() const
    {
        return to_object();
    }

    /// Create a subview [start, start+count)
    EncodingLua slice(std::size_t start, std::size_t count) const noexcept;

    /// Get a subview at specified node
    EncodingLua subtree(std::size_t const node) const noexcept;

    /// Traverse subtrees from starting position.
    /// `next` arg points to the next subtree index at the same depth,
    /// or to size() if none.
    EncodingLua traverse_subtrees(std::size_t const node, std::size_t& next) const noexcept;

    /// Children subviews
    std::vector<EncodingLua> children() const;

    /// Number of children
    std::size_t arity() const;

    /// Root node keyword
    Keyword head() const noexcept;

    /// Get child # ord node index.
    /// Negative ord resolves as reverse.
    /// If not found, returns size()
    std::size_t child_idx(int ord) const noexcept;

    /// Get subview on child # ord
    /// If not found, returns empty view
    EncodingLua child(int ord) const noexcept;

    bool is_const() const;
    bool is_boolean() const;
};


}  // namespace impl
}  // namespace haikan
