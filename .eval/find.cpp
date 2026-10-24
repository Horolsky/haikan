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
#include "haikan/impl/json_traverse.hpp"



namespace haikan {
namespace impl {

HAIKAN_DEFINE_EVALUATE_IMPL(FindPtr)
{
    Expression res = Err("not found", keyword_to_str());

    JsonTraverse([&](boost::json::value const& node, std::string const jp) -> bool {
        Expression sts;
        if (rhs().eval_as_predicate(node, sts, curr_ctx()))
        {
            res = jp;
            return true;
        }
        else if (sts.is_error())
        {
            res = sts;
            return true;
        }
        return false;
    })(lhs().data());

    return res;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Find)
{
    Expression res = Err("not found", keyword_to_str());

    JsonTraverse([&](boost::json::value const& node, std::string const) -> bool {
        Expression sts;
        if (rhs().eval_as_predicate(node, sts, curr_ctx()))
        {
            res = node;
            return true;
        }
        else if (sts.is_error())
        {
            res = node;
            return true;
        }
        return false;
    })(lhs().data());

    return res;
}

HAIKAN_DEFINE_EVALUATE_IMPL(FindIdx)
{
    Expression res = Err("not found", keyword_to_str());

    auto const if_array = lhs().if_array();
    auto const if_object = lhs().if_object();
    auto const if_string = lhs().if_string();
    ASSERT(if_array || if_object || if_string, "argument not indexable");


    if (if_array)
    {
        auto const& arr = *if_array;
        for (size_t i = 0; i < arr.size(); i++)
        {
            if (rhs().eval_as_predicate(arr[i], res, curr_ctx()))
            {
                res = i;
                break;
            }
            else if (res.is_error())
            {
                break;
            }
        }
    }
    else if (if_object)
    {
        auto const& obj = *if_object;
        std::size_t i = 0;
        for (auto const& item: obj)
        {
            if (rhs().eval_as_predicate({item.key(), item.value()}, res, curr_ctx()))
            {
                res = i;
                break;
            }
            else if (res.is_error())
            {
                break;
            }
            ++i;
        }
    }
    else if (if_string)
    {
        auto const& str = *if_string;
        std::size_t i = 0;
        for (auto const c: str)
        {
            if (rhs().eval_as_predicate(c, res, curr_ctx()))
            {
                res = i;
                break;
            }
            else if (res.is_error())
            {
                break;
            }
            ++i;
        }
    }

    return res;
}


} // namespace impl
} // namespace haikan

