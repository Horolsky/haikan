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

#include "haikan/serialization.hpp"
#include "haikan/reflection_meta.hpp"
#include "haikan/impl/encoding.hpp"
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/keyword_tag_invoke.hpp"
#include "haikan/impl/keyword_grammar.hpp"
#include "haikan/impl/lua_json_conversion.hpp"
#include "haikan/logger.hpp"
#include "haikan/impl/keyword_to_str.hpp"
#include "haikan/impl/lua_state.hpp"



namespace haikan {
namespace impl {


Encoding::Encoding(sol::object value)
{

    if(value.get_type() == sol::type::table)
    {
        auto const as_table = value.as<sol::table>();
        if (as_table["keywords"] != sol::nil
            && as_table["depths"] != sol::nil
            && as_table["data"] != sol::nil
        )
        {
            keywords_ = as_table["keywords"].get<std::vector<Keyword>>();
            depths_ = as_table["depths"].get<std::vector<std::size_t>>();
            data_ = as_table["data"].get<std::vector<sol::object>>();
            preprocess();
            return;
        }
    }

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
    else if(value.is<error>())
    {
        k = Keyword::Err;
    }
    push_back(k, 0, value);
}



std::size_t Encoding::size() const
{
    return keywords_.size();
}

bool Encoding::operator==(Encoding const& o) const
{
    return (keywords_ == o.keywords_)
    && (depths_ == o.depths_)
    && (data_ == o.data_);
}

bool Encoding::operator!=(Encoding const& o) const
{
    return not operator==(o);
}


void Encoding::push_back(Keyword const& k, std::size_t const d, sol::object v)
{
    keywords_.push_back(k);
    depths_.push_back(d);
    data_.push_back(v);
}


void Encoding::append_to_root(Encoding tail)
{
    auto const offset = size();
    auto const tail_size = tail.size();
    if (tail_size == 0)
    {
        return;
    }

    keywords_.reserve(offset + tail_size);
    keywords_.insert(keywords_.end(),
        std::make_move_iterator(tail.keywords_.begin()),
        std::make_move_iterator(tail.keywords_.end()));

    data_.reserve(offset + tail_size);
    data_.insert(data_.end(),
        std::make_move_iterator(tail.data_.begin()),
        std::make_move_iterator(tail.data_.end()));

    depths_.resize(offset + tail_size);
    std::transform(tail.depths_.begin(), tail.depths_.end(), depths_.begin() + offset,
        [](std::size_t const value) { return value + 1; });
}



/// Single-pass preprocessing, return true if no multipass tokens left
bool Encoding::preprocess()
{
    bool complete{true};

    if (std::none_of(keywords_.begin(), keywords_.end(), [](Keyword k) { return k == Keyword::PreProc; }))
    {
        return true;
    }

    std::vector<Keyword> new_keywords;
    std::vector<std::size_t> new_depth;
    std::vector<sol::object> new_data;

    new_keywords.reserve(keywords_.size());
    new_depth.reserve(depths_.size());
    new_data.reserve(data_.size());

    for (std::size_t i = 0; i < keywords_.size(); ++i)
    {
        auto const kw = keywords_[i];
        auto const d = depths_[i];
        auto payload = data_[i];

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

            auto subenc = Encoding(payload);
            for (auto& sub_depth : subenc.depths_)
            {
                sub_depth += d;
            }

            new_keywords.insert(
                new_keywords.end(),
                std::make_move_iterator(subenc.keywords_.begin()),
                std::make_move_iterator(subenc.keywords_.end()));
            new_depth.insert(
                new_depth.end(),
                std::make_move_iterator(subenc.depths_.begin()),
                std::make_move_iterator(subenc.depths_.end()));
            new_data.insert(
                new_data.end(),
                std::make_move_iterator(subenc.data_.begin()),
                std::make_move_iterator(subenc.data_.end()));
        }
        else
        {
            new_keywords.push_back(kw);
            new_depth.push_back(d);
            new_data.push_back(payload);
        }
    }

    keywords_ = std::move(new_keywords);
    depths_ = std::move(new_depth);
    data_ = std::move(new_data);

    return complete;
}

sol::object Encoding::to_object() const
{

    if (size() == 0)
    {
        return lua::make_object(sol::nil);
    }

    switch (head())
    {
    case Keyword::_Literal:
    case Keyword::PreProc:
    case Keyword::Link:
    {
        // TODO: handle transfer error
        return to_plain_lua(data_[0]);
    }
    default:
        break;
    }

    auto keywords_out = lua::create_table(static_cast<int>(keywords_.size()), 0);
    auto depth_out = lua::create_table(static_cast<int>(depths_.size()), 0);
    auto data_out = lua::create_table(static_cast<int>(data_.size()), 0);

    for (std::size_t i = 0; i < keywords_.size(); ++i)
    {
        auto const lua_index = i + 1;

        keywords_out.set(lua_index, lua::make_object(keyword_to_str(keywords_[i])));
        depth_out.set(lua_index, depths_[i]);

        auto const& value = data_[i];
        if (value.valid() && value.get_type() == sol::type::userdata)
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

    return lua::global_state().create_table_with(
        "keywords", keywords_out,
        "depths", depth_out,
        "data", data_out
    );
}

boost::json::value Encoding::to_json() const
{
    return lua_to_json(to_object()).value_or(nullptr);
}

Encoding Encoding::slice(std::size_t start, std::size_t count) const noexcept
{
    Encoding enc{};
    if ((count != 0) && (start <= size()) && (count <= size() - start))
    {
        enc.keywords_.assign(keywords_.begin() + start, keywords_.begin() + start + count);
        enc.depths_.assign(depths_.begin() + start, depths_.begin() + start + count);
        enc.data_.assign(data_.begin() + start, data_.begin() + start + count);
    }
    return enc;
}

Encoding Encoding::subtree(std::size_t const node) const noexcept
{
    std::size_t ignored{};
    return traverse_subtrees(node, ignored);
}

std::size_t Encoding::arity() const
{
    if (depths_.empty())
    {
        return 0;
    }
    return std::count(depths_.begin(), depths_.end(), depths_.front() + 1);
}


std::vector<Encoding> Encoding::children() const
{
    std::vector<Encoding> st;
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


Keyword Encoding::head() const noexcept
{
    return keywords_.size() ? keywords_[0] : Keyword::Undefined;
}

Encoding Encoding::child(int ord) const noexcept
{
    return subtree(child_idx(ord));
}

std::size_t Encoding::child_idx(int ord) const noexcept
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
            if ((depths_[static_cast<std::size_t>(i)] - depths_[0]) == 1)
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
            if ((depths_[i] - depths_[0]) == 1)
            {
                idx = i;
                if (--ord < 0) break;
            }
        }
    }
    return idx;
}


bool Encoding::is_const() const
{
    auto const a = keyword_attributes(head());
    if (a & attr::is_pipe && size() > 1)
    {
        {
            // TODO: make it safe (maybe move to Encoding init)
            for (std::size_t i = 0; i < size(); i++)
            {
                if (keyword_attributes(keywords_[i]) & attr::is_sideeffect)
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

bool Encoding::is_boolean() const
{
    auto const a = keyword_attributes(head());

    if(a & attr::is_literal && size() > 1)
    {
        return data_[1].get_type() == sol::type::boolean;
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



Encoding Encoding::traverse_subtrees(std::size_t const node, std::size_t& next) const noexcept
{
    if (node >= size())
    {
        next = size();
        return {};
    }
    auto keywords_it = keywords_.begin() + node;
    auto depth_it = depths_.begin() + node;
    auto data_it = data_.begin() + node;

    next = size();
    std::size_t const node_depth = *depth_it;
    std::size_t count {1};
    ++keywords_it;
    ++depth_it;
    ++data_it;
    while (keywords_it < keywords_.cend())
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
