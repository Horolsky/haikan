/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */
#include <cstdint>
#include <iterator>
#include <ostream>
#include <sstream>

#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/operator.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/keywords.hpp"


namespace
{

using V = boost::json::value;
using O = haikan::impl::Operator;
using E = haikan::impl::Expression;
using Keyword = haikan::impl::Keyword;


}


namespace haikan {
namespace impl {

Encoding Expression::encodeLiteral(boost::json::value const& params)
{
    Encoding enc {};
    enc.push_back(Keyword::_Literal, 0, params);
    return enc;
}

Encoding Expression::encodePreProc(boost::json::value const& params)
{
    Encoding enc {};
    enc.push_back(Keyword::PreProc, 0, params);
    return enc;
}

template <class T>
Encoding Expression::encodeNested(Keyword const& keyword, std::move_iterator<T> begin, std::move_iterator<T> const end)
{
    Encoding enc {};
    enc.push_back(keyword, 0, nullptr);

    while(begin != end)
    {
        enc.append_to_root(std::move((*begin++).encoding_));
    }
    return enc;
}

Encoding Expression::encodeNested(Keyword const& keyword, std::initializer_list<Expression> subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}

Encoding Expression::encodeNested(Keyword const& keyword, std::vector<Expression>&& subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}


bool Expression::to_predicate_if_const(Expression& e)
{
    if (!e.is_noop() && e.is_const())
    {
        e = Expression(encodeNested(Keyword::Eq, {e}));
        return true;
    }
    return false;
}

}  // namespace impl
}  // namespace haikan
