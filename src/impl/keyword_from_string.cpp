/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */


#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_grammar.hpp"
#include <boost/utility/string_view.hpp>


namespace haikan {
namespace impl {



Keyword keyword_from_str(boost::string_view const str)
{
    static KeywordGrammar const keyword_parser {};

    auto iter = str.cbegin();
    auto end = str.cend();
    Keyword keyword_out {Keyword::Undefined};
    static_cast<void>(boost::spirit::qi::parse(iter, end, keyword_parser, keyword_out));
    return keyword_out;
}

}
}