/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <algorithm>
#include <iterator>
#include <list>
#include <ostream>
#include <vector>

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

    bool empty() const
    {
        return keywords_.empty();
    }

};

class EncodingView
{
public:
    struct ExprRow
    {
        Keyword keyword;
        std::size_t depth;
        sol::object const* data;
        std::size_t index;
    };

    class Iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = ExprRow;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = ExprRow;

        Iterator(Keyword const* k, std::size_t const* d, sol::object const* v,
                 std::size_t index, std::size_t offset)
            : keywords_(k), depths_(d), data_(v), index_(index), offset_(offset) {}

        reference operator*() const { return {keywords_[index_], depths_[index_] - depths_[0], &data_[index_], index_ + offset_}; }
        Iterator& operator++() { ++index_; return *this; }
        Iterator operator++(int) { auto copy = *this; ++*this; return copy; }
        Iterator& operator--() { --index_; return *this; }
        Iterator operator--(int) { auto copy = *this; --*this; return copy; }
        Iterator& operator+=(difference_type n) { index_ += n; return *this; }
        Iterator& operator-=(difference_type n) { index_ -= n; return *this; }
        reference operator[](difference_type n) const { return *(*this + n); }
        Iterator operator+(difference_type n) const { return {keywords_, depths_, data_, index_ + n, offset_}; }
        Iterator operator-(difference_type n) const { return {keywords_, depths_, data_, index_ - n, offset_}; }
        difference_type operator-(Iterator const& o) const { return static_cast<difference_type>(index_) - static_cast<difference_type>(o.index_); }
        bool operator==(Iterator const& o) const { return index_ == o.index_; }
        bool operator!=(Iterator const& o) const { return !(*this == o); }
        bool operator<(Iterator const& o) const { return index_ < o.index_; }
        bool operator<=(Iterator const& o) const { return index_ <= o.index_; }
        bool operator>(Iterator const& o) const { return index_ > o.index_; }
        bool operator>=(Iterator const& o) const { return index_ >= o.index_; }

    private:
        Keyword const* keywords_;
        std::size_t const* depths_;
        sol::object const* data_;
        std::size_t index_;
        std::size_t offset_;
    };

    using const_iterator = Iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    EncodingView() = default;
    EncodingView(Encoding&&) = delete;
    EncodingView(Keyword const* keywords, std::size_t const* depths, sol::object const* data,
                 std::size_t size, std::size_t index_offset) noexcept;
    EncodingView(Encoding const& encoding) noexcept;

    const_iterator begin() const { return {keywords_, depths_, data_, 0, index_offset_}; }
    const_iterator end() const { return {keywords_, depths_, data_, size_, index_offset_}; }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }
    std::size_t index_offset() const noexcept { return index_offset_; }
    std::size_t depth_offset() const noexcept { return depths_ ? depths_[0] : 0; }
    ExprRow front() const;
    ExprRow back() const;
    ExprRow at(std::size_t i) const;
    ExprRow operator[](std::size_t i) const { return at(i); }

    bool operator==(EncodingView const& o) const;
    bool operator!=(EncodingView const& o) const { return !(*this == o); }
    Encoding freeze() const;
    sol::object to_object() const;
    boost::json::value to_json() const;
    EncodingView slice(std::size_t start, std::size_t count) const noexcept;
    EncodingView subtree(std::size_t node) const noexcept;
    EncodingView traverse_subtrees(std::size_t node, std::size_t& next) const noexcept;
    std::vector<EncodingView> children() const;
    std::size_t arity() const noexcept;
    Keyword head() const noexcept;
    std::size_t child_idx(int ord) const noexcept;
    EncodingView child(int ord) const noexcept;
    bool is_const() const;
    bool is_boolean() const;

    Keyword const* if_keywords() const noexcept { return keywords_; }
    std::size_t const* if_depths() const noexcept { return depths_; }
    sol::object const* if_data() const noexcept { return data_; }

private:
    Keyword const* keywords_ = nullptr;
    std::size_t const* depths_ = nullptr;
    sol::object const* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t index_offset_ = 0;
};

}  // namespace impl
}  // namespace haikan
