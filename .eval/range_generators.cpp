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
#include "haikan/impl/slice.hpp"

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


HAIKAN_DEFINE_EVALUATE_IMPL(Repeat)
{
    auto const& param = rhs().data();
    ASSERT(param.is_number(), "invalid parameter");
    std::uint64_t count = boost::json::value_to<std::uint64_t>(param);
    boost::json::array ret {};

    if (!count)
    {
        return ret;
    }
    ret.reserve(count);
    for (std::uint64_t i = 0; i < count; i++)
    {
        ret.push_back(lhs().data());
    }
    return ret;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Sequence)
{
    auto const& arg = x().data();
    ASSERT(arg.is_number(), "invalid argument");
    std::uint64_t count = boost::json::value_to<std::uint64_t>(arg);
    boost::json::array ret {};

    if (!count)
    {
        return ret;
    }
    ret.reserve(count);
    for (std::uint64_t i = 0; i < count; i++)
    {
        ret.push_back(rhs().eval({}, curr_ctx()));
    }
    return ret;
}



HAIKAN_DEFINE_EVALUATE_IMPL(Arange)
{
    auto const& x = lhs().data();
    ASSERT(x.is_number() || x.is_array() || x.is_string(), "invalid argument");

    std::int64_t start = 0;
    std::int64_t stop = 0;
    std::int64_t step = 1;

    if (x.is_number())
    {
        stop = boost::json::value_to<int>(x);
    }
    else if (x.is_array())
    {
        auto const& params = x.get_array();
        ASSERT(params.size() >= 1 && params.size() <= 3, "invalid argument");
        if (params.size() == 1)
        {
            stop = boost::json::value_to<std::int64_t>(params.at(0));
        }
        else if (params.size() >= 2)
        {
            start = boost::json::value_to<std::int64_t>(params.at(0));
            stop = boost::json::value_to<std::int64_t>(params.at(1));
        }
        if (params.size() == 3)
        {
            step = boost::json::value_to<std::int64_t>(params.at(2));
        }
    }
    else if (x.is_string())
    {
        auto const slice_idx = str_to_slice_idx(x.get_string());
        start = slice_idx.at(0);
        stop = slice_idx.at(1);
        step = slice_idx.at(2);
    }

    ASSERT(step != 0, "invalid parameter")

    L out {};
    if ((step > 0 && start >= stop) || (step < 0 && start <= stop))
    {
        return out;
    }

    out.reserve(std::max(0l, (stop - start) / step));
    for (std::int64_t i = start; (stop > start) ? i < stop : i > stop; i += step)
    {
        out.push_back(i);
    }
    return out;
}


} // namespace impl
} // namespace haikan

