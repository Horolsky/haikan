/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */

#include <limits>
#include <boost/math/constants/constants.hpp>

#include "haikan/impl/operator.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/keywords.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/impl/eval_impl.hpp"
#include "haikan/impl/eval_impl_pp.hpp"

namespace
{
using V = boost::json::value;
using L = boost::json::array;
using E = haikan::impl::Expression;
using K = haikan::impl::Keyword;
using O = haikan::impl::Operator;

} // namespace


namespace haikan {
namespace impl {



// HAIKAN_DEFINE_EVALUATE_IMPL(Arange)


HAIKAN_DEFINE_EVALUATE_IMPL(Items)
{
    auto const if_obj = lhs().if_object();
    ASSERT(if_obj, "invalid argument");
    auto const& obj = *if_obj;
    boost::json::array out {};
    out.reserve(obj.size());
    for (auto const& kv: obj)
    {
        out.push_back({kv.key(), kv.value()});
    }
    return out;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Keys)
{
    auto const if_obj = lhs().if_object();
    ASSERT(if_obj, "invalid argument");
    auto const& obj = *if_obj;
    boost::json::array out {};
    out.reserve(obj.size());
    for (auto const& kv: obj)
    {
        out.push_back(kv.key());
    }
    return out;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Values)
{
    auto const if_obj = lhs().if_object();
    ASSERT(if_obj, "invalid argument");
    auto const& obj = *if_obj;
    boost::json::array out {};
    out.reserve(obj.size());
    for (auto const& kv: obj)
    {
        out.push_back(kv.value());
    }
    return out;
}


HAIKAN_DEFINE_EVALUATE_IMPL(Size)
{
    ASSERT(rhs().is_null(), "invalid parameter")
    auto const if_obj = lhs().if_object();
    auto const if_arr = lhs().if_array();

    V size = nullptr;
    if (if_arr)
    {
        return if_arr->size();
    }
    else if (if_obj)
    {
        return if_obj->size();
    }
    else
    {
        ASSERT(false, "invalid argument")
    }
}


HAIKAN_DEFINE_EVALUATE_IMPL(Card)
{
    ASSERT(rhs().is_null(), "invalid parameter")
    auto const if_obj = lhs().if_object();
    auto const if_arr = lhs().if_array();

    if (if_arr)
    {
        // TODO: optimize
        return (lhs() | Uniques).eval().as_array().size();
    }
    else if (if_obj)
    {
        return if_obj->size();
    }
    else
    {
        ASSERT(false, "invalid argument")
    }
}



} // namespace impl
} // namespace haikan

