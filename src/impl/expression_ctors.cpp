/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */
#include <cstdint>
#include <ostream>
#include <sstream>

#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/operator.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/keywords.hpp"

namespace
{
haikan::impl::EncodingView lazy_token_view()
{
    static haikan::impl::Encoding const lazy_token = []{
        haikan::impl::Encoding enc;
        enc.push_back(haikan::impl::Keyword::_Void, 0, {});
        return enc;
    }();
    return haikan::impl::EncodingView(lazy_token);
}
} // namespace


namespace haikan {
namespace impl {

ExpressionView::ExpressionView()
    : ExpressionView(lazy_token_view())
{
}

Expression::Expression(Encoding&& encoding)
    : encoding_{std::move(encoding)}
{
    encoding_view_ = EncodingView(encoding_);
}

Expression::Expression(Encoding const& encoding)
    : encoding_{encoding}
{
    encoding_view_ = EncodingView(encoding_);
}

Expression::Expression(Keyword const keyword)
    : Expression(encodeNested(keyword, {}))
{
}


Expression::Expression(ExpressionView const view)
    : Expression(view.encoding_view().freeze())
{
}


Expression::Expression(boost::json::value const& expr)
    : Expression(Encoding(expr))
{
}

Expression::Expression(boost::json::value&& expr)
    : Expression(Encoding(std::move(expr)))
{
}


Expression::Expression()
    : Expression(nullptr)
{
}


Expression::Expression(std::initializer_list<boost::json::value_ref> items)
    : Expression(boost::json::value(items))
{
}

Expression::Expression(Expression const& other)
    : Expression(other.encoding_)
{
}

Expression::Expression(Expression&& other)
    : Expression(std::move(other.encoding_))
{
}

Expression& Expression::operator=(Expression const& other)
{
    encoding_ = other.encoding_;
    encoding_view_ = EncodingView(encoding_);
    return *this;
}

Expression& Expression::operator=(Expression&& other)
{
    encoding_ = std::move(other.encoding_);
    encoding_view_ = EncodingView(encoding_);
    return *this;
}

}  // namespace impl
}  // namespace haikan
