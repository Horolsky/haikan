/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "haikan/keywords.hpp"

namespace utf = boost::unit_test;
using namespace haikan;

BOOST_AUTO_TEST_CASE(TestCapture)
{
    auto inv = "$x" | Ne(0) | And("$x" | Flip(Div(1))) | Or("$x");

    BOOST_TEST_INFO(inv.to_json());
    BOOST_CHECK_EQUAL(*(42 | inv), 1.0/42);
    BOOST_CHECK_EQUAL(*(0 | inv), 0);
}

BOOST_AUTO_TEST_CASE(SymbolicLink)
{
    auto const e = Dbg("$f" << Add(1) | "$f"  | "$f");
    BOOST_CHECK_EQUAL(e.eval(0),   3);
}

BOOST_AUTO_TEST_CASE(SymbolicLinkRecursion)
{
    auto const fact = "$f" << (
        "$x"
        | Assert(Ge(0))
        | Lt(2)
        | And(1)
        | Or("$x" | Sub(1) | "$f" | Mul("$x"))
    );

    BOOST_TEST_INFO(fact.prettify());
    BOOST_CHECK_EQUAL(fact.eval(0),   1);
    BOOST_CHECK_EQUAL(fact.eval(1),   1);
    BOOST_CHECK_EQUAL(fact.eval(2),   2);
    BOOST_CHECK_EQUAL(fact.eval(3),   6);
    BOOST_CHECK_EQUAL(fact.eval(4),  24);
    BOOST_CHECK_EQUAL(fact.eval(5), 120);
    BOOST_CHECK_EQUAL((fact | IsErr).eval(-5), true);
}

BOOST_AUTO_TEST_CASE(Closure)
{
    auto const closure = "$closed"
        | ("$f" << ("$local"
        | Get("$closed")
        | Add("$local")
        | If(Abs|Ge(31), Id)
        | Else("$f")
        ))
    ;

    BOOST_TEST_INFO(closure.prettify());
    BOOST_CHECK_EQUAL(Dbg(closure).eval(5),   35);
    BOOST_CHECK_EQUAL(Dbg(closure).eval(4),   32);
    BOOST_CHECK_EQUAL(Dbg(closure).eval(3),   33);
}

BOOST_AUTO_TEST_CASE(SideEffectsNeverConst)
{
    BOOST_CHECK(!(Noop | EnvLoad("x")).is_const());
    BOOST_CHECK(!(Noop | EnvStore("x")).is_const());
    BOOST_CHECK(!(Noop | Rand).is_const());
    BOOST_CHECK(!(Noop | RandInt(0, 25)).is_const());
}
