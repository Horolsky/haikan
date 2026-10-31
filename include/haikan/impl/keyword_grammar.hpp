/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <boost/json.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix/operator.hpp>

#include "haikan/impl/keyword.hpp"


namespace haikan {
namespace impl {

/// Boost.Sirit.Qi Symbol for haikan::impl::Keyword parsing
struct KeywordSymbol : public boost::spirit::qi::symbols<char, Keyword>
{
    KeywordSymbol();
};

/// Boost.Sirit.Qi Grammar for haikan::impl::Keyword parsing
struct KeywordGrammar : public boost::spirit::qi::grammar<boost::json::string::const_iterator, Keyword()>
{
    KeywordGrammar();

    KeywordSymbol keyword_symbol;
    boost::spirit::qi::rule<boost::json::string::const_iterator, Keyword()> start;
};


// Boost JSON conversion from Keyword
void tag_invoke(boost::json::value_from_tag const&, boost::json::value&, Keyword const& t);

// Boost JSON conversion to Keyword
Keyword tag_invoke(boost::json::value_to_tag<Keyword> const&, boost::json::value const&);


} // namespace impl
} // namespace haikan
