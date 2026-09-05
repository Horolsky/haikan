/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <utility>

#include "haikan/impl/lazy_param.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/eval_context.hpp"



namespace haikan {
namespace impl {


LazyParam::LazyParam(std::function<V()> getter)
    : getter_{std::move(getter)}
{
    if (!getter_)
    {
        getter_ = []{ return V{}; };
    }
}

LazyParam::LazyParam(ExpressionView v, EvalContext ctx)
    : getter_{[view = std::move(v), context = std::move(ctx)]
        { return view.eval_e({}, context).to_json(); }}
{

}

LazyParam::LazyParam(V && v)
    : getter_{[value = std::move(v)]{return value;}}
{
}

LazyParam::LazyParam(V const& v)
    : getter_{[v]{return v;}}
{
}

LazyParam::LazyParam(std::reference_wrapper<V const> v)
    : getter_{[v]{return v;}}
{
}


LazyParam::LazyParam()
    : LazyParam(std::function<V()>{})
{
}

LazyParam::V LazyParam::operator()() const
{
    if (!cache_)
    {
        cache_ = getter_();
    }
    return *cache_;
}

LazyParam::operator V() const
{
    return operator()();
}

} // namespace impl
} // namespace haikan
