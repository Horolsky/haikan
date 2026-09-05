/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "haikan/keywords_lua.hpp"

namespace utf = boost::unit_test;
using namespace haikan;

using namespace haikan::lua;

BOOST_AUTO_TEST_CASE(PrettifyExpressionTostaticBuffer)
{
    auto e = (Fold(Add) & Size) | Div | Eq(2.5E0) | Not;
    char buff[100];

    e.prettify_to(buff);
    BOOST_CHECK_EQUAL(buff, "(Fold(Add) & Size) | Div | Eq(2.5E0) | Not");
}


BOOST_AUTO_TEST_CASE(PrettifyExpression)
{

    #define TEST_PRETIFY(e) \
    BOOST_TEST_INFO("Expected: " #e); \
    BOOST_TEST_INFO("     Got: " << (e).prettify()); \
    BOOST_TEST_INFO(" to_json: " << (e).to_json()); \
    {BOOST_CHECK_EQUAL((e).prettify(), #e); }\
    { std::stringstream ss; (e).prettify_to(ss); \
        BOOST_CHECK_EQUAL(ss.str(), #e); }

    TEST_PRETIFY(   (Fold(Add) & Size) | Div | Eq(2.5E0) | Not      )
    TEST_PRETIFY(   Eq(Pi | Div(2))                                 )
    TEST_PRETIFY(   "%s%d" | Fmt(Pi | Div(2), 2 | Add(2))           )
    TEST_PRETIFY(   Recur(42, Map(Add(2) | Div(E)))                 )
    TEST_PRETIFY(   Unfold(13, Recur(42, Map(Add(2) | Div(E))))     )

    TEST_PRETIFY(   Min                                             )
    TEST_PRETIFY(   Min(At("/%s" | Fmt("foo")))                     )

    // check parentheses
    TEST_PRETIFY(   All                                             )
    // TEST_PRETIFY(All()) // FIXME: empty variadic != incomplete kw

    TEST_PRETIFY(   Q(Q | (Q | Q) | Q)                              )
    TEST_PRETIFY(   Q((Q & (Q & Q)) & (Q | (Q | (Q | Q))))          )

    TEST_PRETIFY(   (Q | Q) & (Q | Q)                               )
    TEST_PRETIFY(   (Q & Q) | (Q & Q)                               )

     // right grouping - parentheses preserved
    TEST_PRETIFY(   Q & (Q & (Q & Q))                               )
     // left associativity - parentheses preserved
    TEST_PRETIFY(   ((Q & Q) & Q) & Q                               )
    // Explicit keywords is necessary to preserve left grouping
    // for non-associative Tuple
    TEST_PRETIFY( Tuple(Tuple((Q, Q), Q), Q)                        )

    // no infix sugar for singleton Pipe and Fork
    TEST_PRETIFY(   Pipe(Fork(All))                                 )
    TEST_PRETIFY(   Pipe(All & Any)                                 )
    TEST_PRETIFY(   Saturate(Eq(23), 42, Any(27, 13) | Not)         )

    TEST_PRETIFY(   Size | 3                                        )

    TEST_PRETIFY(   ~Div(1)                                         )

    // Symbolic links
    TEST_PRETIFY(
        "$f" << ("$x" | Assert(Ge(0)) | Lt(2) | And(1) | Or(("$x" & ("$x" | Sub(1) | "$f")) | Mul))
    )
    TEST_PRETIFY(
        Q("$f" << ("$x" | Assert(Ge(0)) | Lt(2) | And(1) | Or(("$x" & ("$x" | Sub(1) | "$f")) | Mul)))
    )

    BOOST_CHECK_EQUAL((~Diff({2,3,4})).prettify(), "~Diff({2,3,4})"); // Lua table syntax
    BOOST_CHECK_EQUAL((Q & Q | Q & Q).prettify(), "(Q & Q) | (Q & Q)"); // prec(&) > prec(|)
    BOOST_CHECK_EQUAL((Q | Q & Q | Q).prettify(), "Q | (Q & Q) | Q"); // prec(&) > prec(|)

}
