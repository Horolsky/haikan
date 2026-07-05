/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */
#include <algorithm>
#include <iterator>
#include <list>
#include <ostream>
#include <utility>
#include <vector>

#include <boost/optional.hpp>
#include <boost/utility/string_view.hpp>

#include "haikan/impl/encoding_lua.hpp"
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/keyword_tag_invoke.hpp"
#include "haikan/impl/keyword_grammar.hpp"



namespace haikan {
namespace impl {


bool EncodingLua::is_preproc_token(sol::object const& value)
{
    if (value.get_type() != sol::type::string) return false;
    boost::string_view str = value.as<char const*>(); // TODO: check me
    return str.starts_with("$[") && str.ends_with("]");
}

bool EncodingLua::is_link_token(sol::object const& value)
{
    if (value.get_type() != sol::type::string) return false;
    boost::string_view str = value.as<char const*>(); // TODO: check me

    if (str.size() <= 1 || !str.starts_with("$")) return false;
    auto const second = str.at(1);
    auto const end = str.back();

    bool const is_bracketed // TODO: regex
        =   (second == '[' && end == ']')
        ||  (second == '{' && end == '}')
        ||  (second == '(' && end == ')');
    return not is_bracketed && (second != '$');
}

EncodingLua::EncodingLua(sol::object value)
{
    auto const as_table = value.as<sol::optional<sol::table>>();
    if (as_table
        && as_table.value()["keywords"] != sol::nil
        && as_table.value()["depth"] != sol::nil
        && as_table.value()["data"] != sol::nil
    )
    {
        keywords = as_table.value()["keywords"].get<std::vector<Keyword>>();
        depth = as_table.value()["depth"].get<std::vector<std::size_t>>();
        data = as_table.value()["data"].get<std::vector<sol::object>>();
        preprocess();
    }
    else
    {
        auto k = Keyword::_Literal;
        if (is_preproc_token(value))
        {
            k = Keyword::PreProc;
        }
        else if (is_link_token(value))
        {
            k = Keyword::Link;
        }
        push_back(k, 0, std::move(value));
    }
}



std::size_t EncodingLua::size() const
{
    return keywords.size();
}

bool EncodingLua::operator==(EncodingLua const& o) const
{
    return (keywords == o.keywords)
    && (depth == o.depth)
    && (data == o.data);
}

bool EncodingLua::operator!=(EncodingLua const& o) const
{
    return not operator==(o);
}


void EncodingLua::push_back(Keyword const& k, std::size_t const d, sol::object v)
{
    keywords.push_back(k);
    depth.push_back(d);
    data.push_back(v);
}


void EncodingLua::append_to_root(EncodingLua tail)
{
    auto const offset = size();
    auto const tail_size = tail.size();
    if (tail_size == 0)
    {
        return;
    }

    keywords.reserve(offset + tail_size);
    keywords.insert(keywords.end(),
        std::make_move_iterator(tail.keywords.begin()),
        std::make_move_iterator(tail.keywords.end()));

    data.reserve(offset + tail_size);
    data.insert(data.end(),
        std::make_move_iterator(tail.data.begin()),
        std::make_move_iterator(tail.data.end()));

    depth.resize(offset + tail_size);
    std::transform(tail.depth.begin(), tail.depth.end(), depth.begin() + offset,
        [](std::size_t const value) { return value + 1; });
}



/// Single-pass preprocessing, return true if no multipass tokens left
bool EncodingLua::preprocess()
{
    bool complete{true};

    if (std::none_of(keywords.begin(), keywords.end(), [](Keyword k) { return k == Keyword::PreProc; }))
    {
        return true;
    }

    std::vector<Keyword> new_keywords;
    std::vector<std::size_t> new_depth;
    std::vector<sol::object> new_data;

    new_keywords.reserve(keywords.size());
    new_depth.reserve(depth.size());
    new_data.reserve(data.size());

    for (std::size_t i = 0; i < keywords.size(); ++i)
    {
        auto const kw = keywords[i];
        auto const d = depth[i];
        auto payload = data[i];

        if (kw == Keyword::PreProc)
        {
            if (is_preproc_token(payload))
            {
                complete = false;
                new_keywords.push_back(kw);
                new_depth.push_back(d);
                new_data.push_back(payload);
                continue;
            }

            auto subenc = EncodingLua(payload);
            for (auto& sub_depth : subenc.depth)
            {
                sub_depth += d;
            }

            new_keywords.insert(
                new_keywords.end(),
                std::make_move_iterator(subenc.keywords.begin()),
                std::make_move_iterator(subenc.keywords.end()));
            new_depth.insert(
                new_depth.end(),
                std::make_move_iterator(subenc.depth.begin()),
                std::make_move_iterator(subenc.depth.end()));
            new_data.insert(
                new_data.end(),
                std::make_move_iterator(subenc.data.begin()),
                std::make_move_iterator(subenc.data.end()));
        }
        else
        {
            new_keywords.push_back(kw);
            new_depth.push_back(d);
            new_data.push_back(payload);
        }
    }

    keywords = std::move(new_keywords);
    depth = std::move(new_depth);
    data = std::move(new_data);

    return complete;
}

sol::object EncodingLua::to_object(sol::state_view sv) const
{
    return sol::make_object(sv.lua_state(), sol::nil);
    // TODO: implement serialization to plain Lua

    // boost::json::array out_keywords;
    // boost::json::array out_depth;
    // boost::json::array out_data;

    // out_keywords.reserve(size_);
    // out_depth.reserve(size_);
    // out_data.reserve(size_);

    // auto const base = depth_ ? depth_[0] : 0U;
    // for (std::size_t i = 0; i < size_; ++i)
    // {
    //     out_keywords.push_back(static_cast<std::underlying_type_t<K>>(keywords_[i]));
    //     out_depth.push_back(depth_[i] - base);
    //     out_data.push_back(data_[i]);
    // }

    // return {
    //     {"keywords", std::move(out_keywords)},
    //     {"depth", std::move(out_depth)},
    //     {"data", std::move(out_data)},
    // };
}

EncodingLua EncodingLua::slice(std::size_t start, std::size_t count) const noexcept
{
    EncodingLua enc{};
    if ((count != 0) && (start <= size()) && (count <= size() - start))
    {
        enc.keywords.assign(keywords.begin() + start, keywords.begin() + start + count);
        enc.depth.assign(depth.begin() + start, depth.begin() + start + count);
        enc.data.assign(data.begin() + start, data.begin() + start + count);
    }
    return enc;
}

EncodingLua EncodingLua::subtree(std::size_t const node) const noexcept
{
    std::size_t ignored{};
    return traverse_subtrees(node, ignored);
}

std::size_t EncodingLua::arity() const
{
    return std::count(depth.begin(), depth.end(), 1);
}


std::vector<EncodingLua> EncodingLua::children() const
{
    std::vector<EncodingLua> st;
    if (size() < 2)
    {
        return st;
    }
    st.reserve(size() - 1);
    std::size_t next{1};
    while(next < size())
    {
        st.emplace_back(traverse_subtrees(next, next));
    }
    return st;
}


Keyword EncodingLua::head() const noexcept
{
    return keywords.size() ? keywords[0] : Keyword::Undefined;
}

EncodingLua EncodingLua::child(int ord) const noexcept
{
    return subtree(child_idx(ord));
}

std::size_t EncodingLua::child_idx(int ord) const noexcept
{
    auto const N = size();
    if (N == 0)
    {
        return 0;
    }
    std::size_t idx{N};

    if (ord < 0)
    {
        for (std::int64_t i = N-1; i >=0; --i)
        {
            if ((depth[static_cast<std::size_t>(i)] - depth[0]) == 1)
            {
                idx = i;
                if (++ord >= 0) break;
            }
        }
    }
    else
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if ((depth[i] - depth[0]) == 1)
            {
                idx = i;
                if (--ord < 0) break;
            }
        }
    }
    return idx;
}


bool EncodingLua::is_const() const
{
    auto const a = keyword_attributes(head());
    if (a & attr::is_pipe && size() > 1)
    {
        {
            // TODO: make it safe (maybe move to EncodingLua init)
            for (std::size_t i = 0; i < size(); i++)
            {
                if (keyword_attributes(keywords[i]) & attr::is_sideeffect)
                {
                    return false;
                }
            }
        }
        std::size_t next{1};
        auto st = traverse_subtrees(next, next);
        if (st.is_const() || st.head() == Keyword::_Literal) return true;
        while(next < size())
        {
            st = traverse_subtrees(next, next);
            if (st.is_const() && (st.head() != Keyword::_Literal))
            {
                return true;
            }
        }
        return false;
    }
    else if ((a & attr::is_fork) && (size() > 1))
    {
        std::size_t next{1};
        while(next < size())
        {
            if (not traverse_subtrees(next, next).is_const()) return false;
        }
        return true;
    }
    else if ((a & attr::is_overload) && (size() > 1))
    {
        return subtree(2).is_const();
    }
    else
    {
        return (a & attr::is_literal)
            || (a & attr::is_quote)
            || (a & attr::is_preproc)
            || (a & attr::is_noop)
            || (a & attr::is_error)
            || (a & attr::is_const);
    }
}

bool EncodingLua::is_boolean() const
{
    auto const a = keyword_attributes(head());

    if(a & attr::is_literal && size() > 1)
    {
        return data[1].get_type() == sol::type::boolean;
    }
    else if (a & attr::is_predicate)
    {
        return true;
    }
    else if (a & attr::is_overload && size() > 1)
    {
        return child(1).child(-1).is_boolean();
    }
    if (a & attr::is_pipe && size() > 1)
    {
        return child(-1).is_boolean();
    }
    return false;
}



EncodingLua EncodingLua::traverse_subtrees(std::size_t const node, std::size_t& next) const noexcept
{
    if (node >= size())
    {
        next = size();
        return {};
    }
    auto keywords_it = keywords.begin() + node;
    auto depth_it = depth.begin() + node;
    auto data_it = data.begin() + node;

    next = size();
    std::size_t const node_depth = *depth_it;
    std::size_t count {1};
    ++keywords_it;
    ++depth_it;
    ++data_it;
    while (keywords_it < keywords.cend())
    {
        if (*depth_it <= node_depth)
        {
            // TODO: test me
            next = node + count;
            break;
        }
        ++count;
        ++keywords_it;
        ++depth_it;
        ++data_it;
    }
    return slice(node, count);
}


}  // namespace impl
}  // namespace haikan
