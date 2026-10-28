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

#include <boost/json.hpp>

#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/keyword_grammar.hpp"
#include "haikan/impl/expression_parameter.hpp"

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

namespace haikan {
namespace impl {

struct Encoding
{
    std::vector<Keyword> keywords_;
    std::vector<std::size_t> depths_;
    std::vector<sol::object> data_;

    Encoding() = default;

    explicit Encoding(std::nullptr_t) = delete;

    explicit Encoding(sol::object value);

    std::size_t size() const;

    bool operator==(Encoding const& o) const;

    bool operator!=(Encoding const& o) const;

    void push_back(Keyword const& k, std::size_t const d, sol::object v);

    void append_to_root(Encoding tail);

    /// Single-pass preprocessing, return true if no multipass tokens left
    bool preprocess();

    /// Deepcopy Lua data
    Encoding clone() const;

    // Serialize data to plain Lua
    sol::object to_object() const;

    // Serialize data to plain JSON
    boost::json::value to_json() const;

    /// Create a subview [start, start+count)
    Encoding slice(std::size_t start, std::size_t count) const noexcept;

    /// Get a subview at specified node
    Encoding subtree(std::size_t const node) const noexcept;

    /// Traverse subtrees from starting position.
    /// `next` arg points to the next subtree index at the same depth,
    /// or to size() if none.
    Encoding traverse_subtrees(std::size_t const node, std::size_t& next) const noexcept;

    /// Children subviews
    std::vector<Encoding> children() const;

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
    Encoding child(int ord) const noexcept;

    bool empty() const
    {
        return keywords_.empty();
    }

    bool is_const() const;
    bool is_boolean() const;

};


}  // namespace impl
}  // namespace haikan
