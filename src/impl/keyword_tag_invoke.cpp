/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/keyword_tag_invoke.hpp"
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_grammar.hpp"


namespace haikan {
namespace impl {

void tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, Keyword const& kw)
{
    v = keyword_to_str(kw);
}

Keyword tag_invoke(boost::json::value_to_tag<Keyword> const&, boost::json::value const& v)
{

    if (auto as_uint64 = v.if_uint64())
    {
        return *as_uint64 < static_cast<std::uint64_t>(Keyword::_count) ? static_cast<Keyword>(*as_uint64) : Keyword::Undefined;
    }

    if (not v.is_string())
    {
        return Keyword::Undefined;
    }

    static KeywordGrammar const keyword_parser {};

    auto const& str = v.as_string();
    return keyword_from_str(str.data());
}

}
}