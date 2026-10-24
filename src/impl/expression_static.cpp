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
#include "haikan/impl/expression_lua.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/keywords_lua.hpp"


namespace
{

using V = boost::json::value;
using E = haikan::impl::ExpressionLua;
using Keyword = haikan::impl::Keyword;


}


namespace haikan {
namespace impl {

// EncodingLua ExpressionLua::encodeLiteral(boost::json::value const& params)
// {
//     EncodingLua enc {};
//     enc.push_back(Keyword::_Literal, 0, params);
//     return enc;
// }

ExpressionLua ExpressionLua::encodePreProc(std::string && params)
{
    ExpressionLua r {EncodingLua{}};
    r.encoding_.push_back(Keyword::PreProc, 0, lua::make_object(std::move(params)));
    return r;
}

template <class T>
EncodingLua ExpressionLua::encodeNested(Keyword const& keyword, std::move_iterator<T> begin, std::move_iterator<T> const end)
{
    EncodingLua enc {};
    enc.push_back(keyword, 0, nullptr);

    while(begin != end)
    {
        enc.append_to_root(std::move((*begin++).encoding_));
    }
    return enc;
}

EncodingLua ExpressionLua::encodeNested(Keyword const& keyword, std::initializer_list<ExpressionLua> subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}

EncodingLua ExpressionLua::encodeNested(Keyword const& keyword, std::vector<ExpressionLua>&& subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}


bool ExpressionLua::to_predicate_if_const(ExpressionLua& e)
{
    if (!e.is_noop() && e.is_const())
    {
        e = ExpressionLua(encodeNested(Keyword::Eq, {e}));
        return true;
    }
    return false;
}

}  // namespace impl
}  // namespace haikan
