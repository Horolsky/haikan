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

#include "haikan/reflect.hpp"
#include "haikan/expression_lua.hpp"
#include "haikan/impl/encoding_lua.hpp"
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/keyword_tag_invoke.hpp"
#include "haikan/impl/keyword_grammar.hpp"
#include "haikan/impl/lua_json_conversion.hpp"
#include "haikan/logger.hpp"
#include "haikan/impl/keyword_to_str.hpp"



namespace haikan {
namespace impl {

namespace
{
sol::object serialize_impl(sol::object value)
{
    if (value.get_type() != sol::type::userdata)
    {
        return value;
    }
    sol::userdata userdata = value.as<sol::userdata>();
    sol::protected_function const serialize_fn = userdata["serialize"];
    if (serialize_fn)
    {
        sol::protected_function_result result = serialize_fn(value);
        if (result.valid())
        {
            return result.get<sol::object>();
        }
    }
    sol::object metadata = userdata["meta"];
    if (metadata.is<ReflectionMeta>())
    {
        ReflectionMeta const& meta = metadata.as<ReflectionMeta const&>();
        if (meta.serialize.valid())
        {
            sol::protected_function_result result = meta.serialize(value);
            if (result.valid())
            {
                return result.get<sol::object>();
            }
            else
            {
                HAIKAN_LOG_CERR(ERROR) << "serialization failure";
            }
        }
    }
    else
    {
        HAIKAN_LOG_CERR(ERROR) << "bad metadata";
    }
    return sol::nil;
}
} // namespace


EncodingLua::EncodingLua(LazyLuaObject value)
{
    auto k = Keyword::_Literal;
    // TODO: handle special string tokens
    if (value.is_preproc_token())
    {
        k = Keyword::PreProc;
    }
    else if (value.is_link_token())
    {
        k = Keyword::Link;
    }
    push_back(k, 0, LazyLuaObject{std::move(value)});
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
        data = as_table.value()["data"].get<std::vector<LazyLuaObject>>();
        preprocess();
    }
    else
    {
        auto k = Keyword::_Literal;
        if (value.get_type() == sol::type::string)
        {
            boost::string_view str = value.as<char const*>();
            if (is_preproc_token(str))
            {
                k = Keyword::PreProc;
            }
            else if (is_link_token(str))
            {
                k = Keyword::Link;
            }
        }
        push_back(k, 0, LazyLuaObject{std::move(value)});
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


void EncodingLua::push_back(Keyword const& k, std::size_t const d, LazyLuaObject v)
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
    std::vector<LazyLuaObject> new_data;

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
            if (payload.is_preproc_token())
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

sol::object EncodingLua::to_object() const
{
    if (!data.empty() && data.front().source_state() != nullptr)
    {
        return to_object(sol::state_view(data.front().source_state()));
    }
    return to_object(ExpressionLua::lua_state());
}

sol::object EncodingLua::to_object(sol::state_view L) const
{

    if (size() == 0)
    {
        return sol::make_object(L.lua_state(), sol::nil);
    }

    switch (head())
    {
    case Keyword::_Literal:
    case Keyword::PreProc:
    case Keyword::Link:
    {
        return serialize_impl(data[0].load(L));
    }
    default:
        break;
    }

    auto keywords_out = L.create_table(static_cast<int>(keywords.size()), 0);
    auto depth_out = L.create_table(static_cast<int>(depth.size()), 0);
    auto data_out = L.create_table(static_cast<int>(data.size()), 0);

    for (std::size_t i = 0; i < keywords.size(); ++i)
    {
        auto const lua_index = i + 1;

        keywords_out.set(lua_index, sol::make_object(L,  keyword_to_str(keywords[i]).data()));
        depth_out.set(lua_index, depth[i]);

        auto value = data[i].load(L);
        if (value.get_type() == sol::type::userdata)
        {
            sol::userdata userdata = value.as<sol::userdata>();
            sol::object metadata = userdata["meta"];
            if (metadata.is<ReflectionMeta>())
            {
                ReflectionMeta const& meta = metadata.as<ReflectionMeta const&>();
                if (meta.serialize)
                {
                    sol::protected_function_result result = meta.serialize(value);
                    if (result.valid() && (result.get<sol::object>() != sol::nil))
                    {
                        data_out.set(lua_index, result.get<sol::object>());
                        continue;
                    }
                }
            }
        }
        data_out.set(lua_index, value);
    }

    return L.create_table_with(
        "keywords", keywords_out,
        "depth", depth_out,
        "data", data_out
    );
}

boost::json::value EncodingLua::to_json(sol::state_view sv) const
{
    return lua_to_json(to_object(sv)).value_or(nullptr);
}

boost::json::value EncodingLua::to_json() const
{
    return lua_to_json(to_object()).value_or(nullptr);
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
    if (depth.empty())
    {
        return 0;
    }
    return std::count(depth.begin(), depth.end(), depth.front() + 1);
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
        return data[1].sol_type() == sol::type::boolean;
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
