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
using namespace haikan::impl;
using V = boost::json::value;
using L = boost::json::array;

BOOST_AUTO_TEST_CASE(ExprFromKeyword)
{
    {
        Expression e(Keyword::Noop);
        BOOST_TEST_INFO("    e encoding: " << boost::json::value_from(e.encoding()));
        BOOST_TEST_INFO(" Noop encoding: " << boost::json::value_from(Noop.encoding()));
        BOOST_CHECK_EQUAL(e, Noop);
    }
    {
        Expression e = Keyword::Noop;
        BOOST_TEST_INFO("    e encoding: " << boost::json::value_from(e.encoding()));
        BOOST_TEST_INFO(" Noop encoding: " << boost::json::value_from(Noop.encoding()));
        BOOST_CHECK_NE(e, Noop);
    }
}

BOOST_AUTO_TEST_CASE(NaNConst)
{
    BOOST_CHECK(std::isnan((*NaN).as_double()));
}


BOOST_AUTO_TEST_CASE(InvalidSetError)
{
    BOOST_CHECK(Expression( Subset  ({1,2,3}).eval(13)).is_error());
    BOOST_CHECK(Expression( Superset({1,2,3}).eval(13)).is_error());
    BOOST_CHECK(Expression(PSubset  ({1,2,3}).eval(13)).is_error());
    BOOST_CHECK(Expression(PSuperset({1,2,3}).eval(13)).is_error());
}




BOOST_AUTO_TEST_CASE(SetOperationsOnObject)
{

    BOOST_CHECK(Subset({{"lol", 42}, {"kek", 13}}).match({{"lol", 42}}));
    BOOST_CHECK(In({{"lol", 42}, {"kek", 13}}).match({"lol", 42}));

    boost::json::object set {{"lol", 42}, {"kek", 13}};
    boost::json::object obj {{"lol", 42}, {"lol", 0}}; // will create {{"lol", 42}}
    boost::json::array  kvp {"lol", 42};
    boost::json::string key {"lol"};

    BOOST_CHECK(Subset(set).match(obj));
    BOOST_CHECK(In    (set).match(kvp));
    BOOST_CHECK(In    (set).match(key));

    BOOST_CHECK(Expression(Subset(set).eval(kvp)).is_error());
    BOOST_CHECK(Expression(Subset(set).eval(key)).is_error());
    BOOST_CHECK(Expression(In    (set).eval(obj)).is_error());
}


BOOST_AUTO_TEST_CASE(AtQueryMatch)
{
    auto const obj = boost::json::parse(R"({
        "home": {
            "user": {
                "Desktop":    ["foo", "bar", "baz"],
                "Downloads":  ["lol", "kek"]
            }
        }
    })");


    BOOST_CHECK_MESSAGE((At("/home/user/Downloads/1")| "kek").match(obj), obj);
    BOOST_CHECK((At("/home/user/Desktop/2")|"baz").match(obj));
    BOOST_CHECK((At("/home/user/Desktop") | At(1) | Eq("bar")).match(obj));

    // recursive At
    BOOST_CHECK((At("/home")|At("/user")|At("/Desktop")   | At(0) | Eq("foo")).match(obj));
    BOOST_CHECK((At("/home")|At("/user")|At("/Downloads") | Contains("kek")).match(obj));
    BOOST_CHECK((At("/undefined") | nullptr).match(obj));


    // joining At
    // query array
    BOOST_CHECK((At({"/home/user/Downloads/1", "/home/user/Desktop/2"}) | Eq({"kek", "baz"})).match(obj));
    // query nested array
    BOOST_CHECK((At({{"/home/user/Downloads/1", "/home/user/Desktop/2"}, "/undefined" }) | Eq({{"kek", "baz"}, nullptr})).match(obj));
    // query object
    BOOST_CHECK((At({{"a", "/home/user/Downloads/1"}, {"b", "/home/user/Desktop/2"}}) | Eq({{"a", "kek"}, {"b", "baz"}})).match(obj));
    // query nested structures, a: array, b: object
    BOOST_CHECK((At({{"a", {"/home/user/Downloads/1", "/home/user/Desktop/2"}}, {"b", {{"null", "/undefined"}}}}) | Eq({{"a", {"kek", "baz"}}, {"b", {{"null", nullptr}}}})).match(obj));

    // dynamic keys
    BOOST_CHECK((At({{"$/home/user/Desktop/0", "/home/user/Desktop/1"}}) | Eq({{"foo", "bar"}}) ).match(obj));

}


BOOST_AUTO_TEST_CASE(TestComposePipe)
{
    auto const test = boost::json::parse(R"({
        "foo": {
            "bar": {
                "baz": 42
            }
        }
    })");

    BOOST_CHECK_EQUAL(42, Pipe(At("/foo") | At("/bar") | At("/baz")).eval(test));
    BOOST_CHECK_EQUAL(42, (At("/foo") | At("/bar") | At("/baz")).eval(test));
}


BOOST_AUTO_TEST_CASE(TestComposeMapFilterAt)
{
    auto const AllTrueFirst = Pipe(Filter(At(1)|true), Map(At(0)));
    auto const AllFalseFirst  = Filter(At(1) | Eq(false)) | Map(At(0));

    boost::json::array const pairs {
        {"lol", true},
        {"kek", true},
        {"foo", false},
        {"bar", false},
    };

    BOOST_CHECK_EQUAL(AllTrueFirst.eval(pairs), V({"lol", "kek"}));
    BOOST_CHECK_EQUAL(AllFalseFirst.eval(pairs), V({"foo", "bar"}));
}


BOOST_AUTO_TEST_CASE(ExpressionEvalLog)
{
    auto ctx = EvalContext::make();

    auto const f = Debug(Reduce(Add) & Size | Div);
    auto const x = L{1,2,3,42.5};
    f.eval(x, ctx);
    BOOST_CHECK(!ctx.log.str().empty());
}




BOOST_AUTO_TEST_CASE(DebugExample)
{
    {
        auto const f = Debug(Reduce(Add) & Size | Div);
        auto const x = L{1,2,3,42.5};
        BOOST_CHECK_EQUAL(f.eval(x), 12.125);
    }
    {
        auto const f = Trace("foo") | Reduce(Add) & Size | Div;
        auto const x = L{1,2,3,42.5};
        BOOST_CHECK_EQUAL(f.eval(x), 12.125);
    }
}



BOOST_AUTO_TEST_CASE(BracketInit)
{
    Expression const e1{Add}; // API
    // Expression const e2{e1}; // error: conflicts with init list ctor
    BOOST_CHECK_EQUAL(e1, Add);
}
