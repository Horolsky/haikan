/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/phoenix/phoenix.hpp>
#include <boost/spirit/include/karma.hpp>

#include "haikan/impl/expression.hpp"
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_grammar.hpp"

namespace haikan {
namespace impl {

template <typename OutputIterator>
struct ExpressionGrammar : boost::spirit::karma::grammar<OutputIterator, EncodingView()> {
    ExpressionGrammar() : ExpressionGrammar::base_type(start)
    {
        namespace karma = boost::spirit::karma;
        namespace phoenix = boost::phoenix;

        using karma::_1;
        using karma::_val;
        using karma::eps;
        using karma::lit;
        using karma::string;

        auto const is_literal         = phoenix::bind(&ExpressionGrammar::is_literal, _val);
        auto const is_preproc         = phoenix::bind(&ExpressionGrammar::is_preproc, _val);
        auto const is_link            = phoenix::bind(&ExpressionGrammar::is_link, _val);
        auto const is_valid_link      = phoenix::bind(&ExpressionGrammar::is_valid_link, _val);
        auto const has_subexpr        = phoenix::bind(&ExpressionGrammar::has_subexpr, _val);
        auto const serialize          = phoenix::bind(&ExpressionGrammar::serialize, _val);
        auto const keyword_to_string  = phoenix::bind(&ExpressionGrammar::keyword_to_string, _val);
        auto const children           = phoenix::bind(&EncodingView::children, _val);
        auto const is_infix_pipe      = phoenix::bind(&ExpressionGrammar::is_infix_pipe, _val);
        auto const is_infix_juxt      = phoenix::bind(&ExpressionGrammar::is_infix_juxt, _val);
        auto const is_infix_tuple     = phoenix::bind(&ExpressionGrammar::is_infix_tuple, _val);
        auto const is_complete_flip   = phoenix::bind(&ExpressionGrammar::is_complete_flip, _val);

        start
            = eps(is_literal)       << karma::lazy(serialize)
            | eps(is_preproc)       << karma::lazy(serialize)
            | eps(is_link)          << karma::lazy(serialize)
            | eps(is_valid_link)    << link[_1 = children]
            | eps(is_infix_pipe)    << pipe[_1 = children]
            | eps(is_infix_juxt)    << juxt[_1 = children]
            | eps(is_infix_tuple)   << tuple[_1 = children]
            | eps(is_complete_flip) << flip[_1 = children]
            | keyword;

        subexpr
            = eps(is_literal)       << karma::lazy(serialize)
            | eps(is_preproc)       << karma::lazy(serialize)
            | eps(is_link)          << karma::lazy(serialize)
            | eps(is_valid_link)    << nested_link[_1 = children]
            | eps(is_infix_pipe)    << nested_pipe[_1 = children]
            | eps(is_infix_juxt)    << nested_juxt[_1 = children]
            | eps(is_infix_tuple)   << tuple[_1 = children]
            | eps(is_complete_flip) << flip[_1 = children]
            | keyword;

        pipe         = subexpr % lit(" | ");
        juxt         = subexpr % lit(" & ");
        link         = subexpr % lit(" << ");
        flip         = lit('~') << subexpr;
        tuple        = lit('(') << subexpr % lit(", ") << ')';
        nested_link  = lit('(') << subexpr % lit(" << ") << ')';
        nested_pipe  = lit('(') << subexpr % lit(" | ") << ')';
        nested_juxt  = lit('(') << subexpr % lit(" & ") << ')';
        parameters   = lit('(') << -(start % lit(", ")) << ')';

        keyword = string[_1 = keyword_to_string]
            << -(eps(has_subexpr) << parameters[_1 = children]);
    }

private:
    static bool is(EncodingView const& expression, Keyword keyword)
    {
        return expression.opcode() == keyword;
    }

    static bool is_literal(EncodingView const& expression) { return is(expression, Keyword::_Literal); }
    static bool is_preproc(EncodingView const& expression) { return is(expression, Keyword::PreProc); }
    static bool is_link(EncodingView const& expression) { return is(expression, Keyword::Link); }
    static bool has_subexpr(EncodingView const& expression) { return expression.size() > 1; }

    static bool is_valid_link(EncodingView const& expression)
    {
        return is(expression, Keyword::Fn) && expression.arity() == 2;
    }

    static bool is_infix_pipe(EncodingView const& expression)
    {
        return is(expression, Keyword::Pipe) && expression.arity() > 1;
    }

    static bool is_infix_juxt(EncodingView const& expression)
    {
        return is(expression, Keyword::Juxt) && expression.arity() == 2;
    }

    static bool is_infix_tuple(EncodingView const& expression)
    {
        if (!is(expression, Keyword::Tuple) || expression.arity() <= 1)
            return false;
        auto const child = expression.child(0);
        return !child.empty() && child.opcode() != Keyword::Tuple;
    }

    static bool is_complete_flip(EncodingView const& expression)
    {
        return is(expression, Keyword::Flip) && expression.size() > 1;
    }

    static std::string serialize(EncodingView const& expression)
    {
        std::stringstream ss;
        impl::lua_to_stream(ss, expression.to_object(), 0, false);
        return ss.str();
    }

    static char const* keyword_to_string(EncodingView const& expression)
    {
        return impl::keyword_to_str(expression.opcode());
    }

    boost::spirit::karma::rule<OutputIterator, EncodingView()> start, subexpr, keyword;
    boost::spirit::karma::rule<OutputIterator, std::vector<EncodingView>()>
        parameters, juxt, pipe, tuple, link, flip, nested_pipe, nested_juxt, nested_link;
};


}  // namespace impl
}  // namespace haikan
