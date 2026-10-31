/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/phoenix/phoenix.hpp>
#include <boost/spirit/include/karma.hpp>

#include "haikan/impl/expression.hpp"
#include "haikan/impl/keyword_grammar.hpp"

namespace haikan {
namespace impl {


template <typename OutputIterator>
struct ExpressionLuaGrammar : boost::spirit::karma::grammar<OutputIterator, Expression()> {
    ExpressionLuaGrammar() : ExpressionLuaGrammar::base_type(start)
    {
        namespace karma = boost::spirit::karma;
        namespace phoenix = boost::phoenix;

        using karma::_1;
        using karma::_val;
        using karma::eps;
        using karma::lit;
        using karma::string;

        auto const is_literal          = boost::phoenix::bind(&Expression::is_literal, _val);
        auto const is_preproc          = boost::phoenix::bind(&Expression::is_preproc, _val);
        auto const is_link             = boost::phoenix::bind(&Expression::is_link, _val);
        auto const is_valid_link       = boost::phoenix::bind(&Expression::is_valid_link, _val);
        auto const has_subexpr         = boost::phoenix::bind(&Expression::has_subexpr, _val);
        auto const serialize           = boost::phoenix::bind(&Expression::serialize, _val);
        auto const keyword_to_str      = boost::phoenix::bind(&Expression::keyword_to_str, _val);
        auto const subexpressions_vec = boost::phoenix::bind(&Expression::subexpressions_vec, _val);
        auto const link_parameters     = boost::phoenix::bind(&Expression::link_parameters, _val);
        auto const is_infix_pipe       = boost::phoenix::bind(&Expression::is_infix_pipe, _val);
        auto const is_infix_fork       = boost::phoenix::bind(&Expression::is_infix_fork, _val);
        auto const is_infix_tuple      = boost::phoenix::bind(&Expression::is_infix_tuple, _val);
        auto const is_complete_flip    = boost::phoenix::bind(&Expression::is_complete_flip, _val);

        start
            = eps(is_literal)      << karma::lazy(serialize)
            | eps(is_preproc)      << karma::lazy(serialize)
            | eps(is_link)         << karma::lazy(serialize)
            | eps(is_valid_link)   << link[_1 = link_parameters]
            | eps(is_infix_pipe)   << pipe[_1 = subexpressions_vec]
            | eps(is_infix_fork)   << fork[_1 = subexpressions_vec]
            | eps(is_infix_tuple)  << tuple[_1 = subexpressions_vec]
            | eps(is_complete_flip)<< flip[_1 = subexpressions_vec]
            | keyword;

        subexpr
            = eps(is_literal)       << karma::lazy(serialize)
            | eps(is_preproc)       << karma::lazy(serialize)
            | eps(is_link)          << karma::lazy(serialize)
            | eps(is_valid_link)    << nested_link[_1 = link_parameters]
            | eps(is_infix_pipe)    << nested_pipe[_1 = subexpressions_vec]
            | eps(is_infix_fork)    << nested_fork[_1 = subexpressions_vec]
            | eps(is_infix_tuple)   << tuple[_1 = subexpressions_vec]
            | eps(is_complete_flip) << flip[_1 = subexpressions_vec]
            | keyword;

        pipe    = subexpr % lit(" | ");
        fork    = subexpr % lit(" & ");
        link    = subexpr % lit(" << ");
        flip    = lit('~') << subexpr;
        tuple   = lit('(') << subexpr % lit(", ") << ')';
        nested_link  = lit('(') << subexpr % lit(" << ") << ')';
        nested_pipe  = lit('(') << subexpr % lit(" | ") << ')';
        nested_fork  = lit('(') << subexpr % lit(" & ") << ')';
        parameters   = lit('(') << -(start % lit(", "))  << ')';

        keyword = string[_1 = keyword_to_str] << -(eps(has_subexpr)  << parameters[_1 = subexpressions_vec]);
    }

    boost::spirit::karma::rule<OutputIterator, Expression()> start, subexpr, keyword;
    boost::spirit::karma::rule<OutputIterator, std::vector<Expression>()> parameters, fork, pipe, tuple, link, flip, nested_pipe, nested_fork, nested_link;
};

}  // namespace impl
}  // namespace haikan
