/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <boost/format.hpp>
#include <algorithm>
#include <iterator>
// #include <vector>

#include "haikan/impl/json_iter.hpp"


void haikan::impl::JsonIterZipProdBase::init_iters(boost::json::array const& seqences)
{
    if (seqences.empty())
    {
        halt_ = true;
        return;
    }
    for (auto const& v: seqences)
    {
        if (!v.is_array())
        {
            boost::throw_exception(std::runtime_error((boost::format("not a sequence: `%s`") % v).str()));
        }
        ends_.push_back(v.as_array().cend());
        begins_.push_back(v.as_array().cbegin());
    }
    its_ = begins_;
}

boost::json::array haikan::impl::JsonIterZipProdBase::get()
{
    boost::json::array arr {};
    if (its_ == ends_)
    {
        halt_ = true;
    }
    if (halt_) { return arr; }
    for (auto const& it: its_)
    {
        arr.push_back(*it);
    }
    return arr;
}

void haikan::impl::JsonProdIter::step()
{
    auto iter   = its_.rbegin();
    auto begin  = begins_.crbegin();
    auto end    = ends_.crbegin();

    while (iter != its_.crend())
    {
        (*iter)++;

        if (*iter != *end) { break; }
        else if (std::next(iter) != its_.rend())
        {
            *iter = *begin;
        }
        else {
            halt_ = true;
            break;
        }
        iter++;
        begin++;
        end++;
    }
}

void haikan::impl::JsonZipIter::step()
{
    auto iter = its_.begin();
    auto end = ends_.cbegin();

    while(iter != its_.cend())
    {
        (*iter)++;
        if (*iter == *end)
        {
            halt_ = true;
            // if (its_ != ends_)
            // {
            // }
        }
        iter++;
        end++;
    }
}
