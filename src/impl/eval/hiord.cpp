/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */

#include <limits>
#include <boost/math/constants/constants.hpp>

#include "haikan/impl/operator.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/impl/eval_impl.hpp"
#include "haikan/impl/eval_impl_pp.hpp"
#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/keywords.hpp"

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

HAIKAN_DEFINE_EVALUATE_IMPL(Kwrd)
{
    return lhs().keyword_to_str();
}

HAIKAN_DEFINE_EVALUATE_IMPL(Prms)
{
    auto enc = lhs().encoding_view().freeze();
    // TODO Pack?
    enc.keywords.front() = Keyword::Tuple;
    return Expression(enc);
}


HAIKAN_DEFINE_EVALUATE_IMPL(Sort)
{
    auto const& x = lhs().data();

    ASSERT(x.is_array(), "invalid argument");


    auto const& key_fn = rhs().is_literal() && rhs().is_null() ? Id : rhs();

    auto const& op = curr_ctx().op;
    std::function<bool(V const&, V const&)> is_less = [&key_fn, &op](V const& lhs, V const& rhs) ->bool {
        return op.apply(Keyword::Lt, key_fn.eval(lhs), key_fn.eval(rhs)).as_bool();
    };
    boost::json::array out(x.get_array());
    std::stable_sort(out.begin(), out.end(), is_less);
    return out;
}


HAIKAN_DEFINE_EVALUATE_IMPL(Eval)
{
    return lhs().eval_e(rhs(), curr_ctx());
}


HAIKAN_DEFINE_EVALUATE_IMPL(Bind)
{
    auto const enc = rhs().encoding_view().freeze();
    ASSERT(enc.size() == 1, "function has parameters")

    if (keyword_attributes(enc.keywords.front()) & attr::is_variadic)
    {
        auto const if_arr = lhs().if_array();
        ASSERT(if_arr, "invalid argument")
        std::vector<Expression> xx;
        xx.reserve(if_arr->size());
        for (auto const& item: *if_arr)
        {
            xx.emplace_back(Expression(item));
        }
        return Expression(Expression::encodeNested(enc.keywords.front(), std::move(xx)));

    }
    else
    {
        return Expression(Expression::encodeNested(enc.keywords.front(), {lhs()}));
    }
}

HAIKAN_DEFINE_EVALUATE_IMPL(Flip)
{
    auto const child = rhs().encoding_view().child(0);
    if(child.empty()) // expect argument pair
    {
        auto const if_array = lhs().data().if_array();
        ASSERT(if_array && (if_array->size() == 2), "invalid argument");
        auto const flip = Expression(Expression::encodeNested(rhs().keyword(), {if_array->at(0)}));
        return flip.eval(if_array->at(1), curr_ctx());
    }
    else
    {
        ExpressionView const roperand(child);
        auto const flip = Expression(Expression::encodeNested(rhs().keyword(), {lhs()}));
        return flip.eval_e(roperand.eval_e({}, curr_ctx()), curr_ctx());

    }

}


HAIKAN_DEFINE_EVALUATE_IMPL(Try)
{
    auto const result = rhs().eval_e(lhs(),curr_ctx());
    if (result.is_error()) return nullptr;
    return result;
}



HAIKAN_DEFINE_EVALUATE_IMPL(Count)
{
    auto const if_arr = lhs().if_array();
    auto const if_obj = lhs().if_object();
    ASSERT(if_arr || if_obj, "invalid argument")

    std::size_t count {0};

    Expression err_sts(nullptr);

    if (if_arr)
    {
        for (auto const& sample: *if_arr)
        {
            if (rhs().eval_as_predicate(sample, err_sts, curr_ctx()) && !err_sts.is_error())
            {
                count++;
            }
            else if(err_sts.is_error())
            {
                return err_sts;
            }
        }
    }
    else if (if_obj)
    {
        for (auto const& kv: *if_obj)
        {
            if (rhs().eval_as_predicate({kv.key(), kv.value()},err_sts, curr_ctx()) && !err_sts.is_error())
            {
                count++;
            }
            else if(err_sts.is_error())
            {
                return err_sts;
            }
        }
    }

    return count;
}

HAIKAN_DEFINE_EVALUATE_IMPL(Each)
{
    auto const if_arr = lhs().if_array();
    auto const if_obj = lhs().if_object();
    ASSERT(if_arr || if_obj, "invalid argument")

    Expression err_sts(nullptr);

    if (if_arr)
    {
        for (auto const& sample: *if_arr)
        {
            if (!rhs().eval_as_predicate(sample,err_sts,curr_ctx()) && !err_sts.is_error())
            {
                return false;
            }
            else if(err_sts.is_error())
            {
                return err_sts;
            }
        }
    }
    else if (if_obj)
    {
        for (auto const& kv: *if_obj)
        {
            if (!rhs().eval_as_predicate({kv.key(), kv.value()},err_sts,curr_ctx()) && !err_sts.is_error())
            {
                return false;
            }
            else if(err_sts.is_error())
            {
                return err_sts;
            }
        }
    }

    return true;
}


HAIKAN_DEFINE_EVALUATE_IMPL(Map)
{
    auto const if_arr = lhs().if_array();
    ASSERT(if_arr, "invalid argument")
    auto const& samples = *if_arr;
    auto const& F = rhs();
    boost::json::array ret {};
    if (samples.empty())
    {
        return ret;
    }

    ret.reserve(samples.size());

    for (auto const& el: samples)
    {
        ret.push_back(F.eval(el,curr_ctx()));
    }

    return ret;
}


HAIKAN_DEFINE_EVALUATE_IMPL(Filter)
{
    auto const if_arr = lhs().if_array();
    ASSERT(if_arr, "invalid argument")
    auto const& samples = *if_arr;
    boost::json::array ret {};
    auto const& F = rhs();

    Expression err_sts(nullptr);


    if (samples.empty())
    {
        return ret;
    }
    for (auto const& el: samples)
    {
        if (F.eval_as_predicate(el, err_sts, curr_ctx()) && !err_sts.is_error())
        {
            ret.push_back(el);
        }
        else if(err_sts.is_error())
        {
            return err_sts;
        }
    }
    return ret;
}

} // namespace impl
} // namespace haikan
