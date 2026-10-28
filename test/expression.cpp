/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/expression.hpp"

#include <cstdint>
#include <sstream>
#include <string>

#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "haikan/impl/lua_stream.hpp"
#include "haikan/impl/lua_json_conversion.hpp"
#include "haikan/impl/type_registry.hpp"
#include "haikan/reflection_meta.hpp"

#include "fixture/reflect_types.hpp"

#include "haikan/keywords.hpp"

#include <boost/math/constants/constants.hpp>

using haikan::impl::json_to_lua;
using haikan::impl::lua_to_json;
using haikan::impl::lua_to_json_stream;
using haikan::impl::lua_to_stream;

namespace
{

struct ExpressionLuaSuite
{
    using Expression = haikan::impl::Expression;

    ExpressionLuaSuite()
    {
        haikan::impl::TypeRegistry registry;
        registry.Add(haikan::type<Foo>).Add(haikan::type<Lol>);
        haikan::impl::lua::reset_global_state();
    }

    ~ExpressionLuaSuite()
    {
    }
};

} // namespace

BOOST_FIXTURE_TEST_SUITE(ExpressionLuaTests, ExpressionLuaSuite)

BOOST_AUTO_TEST_CASE(SmokeTest)
{
        Expression ex {42};
}

BOOST_AUTO_TEST_CASE(PrimitiveCtors)
{
    {
        Expression ex {42};
        BOOST_CHECK(ex.to_json() == 42);
    }
    {
        Expression ex {"42"};
        BOOST_CHECK(ex.to_json() == "42");
    }
    {
        Expression ex {haikan::as_enum(Foo::Lol)};
        BOOST_CHECK(ex.encoding_view().data_[0].is<haikan::impl::user_data_enum<Foo>>());
        auto obj = ex.encoding_view().to_object();
        BOOST_CHECK(!obj.is<haikan::impl::user_data_enum<Foo>>());

        BOOST_REQUIRE(obj.valid());

        BOOST_CHECK(obj.get_type() == sol::type::string);

        BOOST_CHECK(ex.to_json() == "Lol");
    }
    {
        Expression ex {Lol{Foo::Kek, 42}};
        BOOST_CHECK(ex.keyword() == haikan::impl::Keyword::_Literal);
        BOOST_CHECK(ex.encoding_view().data_[0].as<Lol>() == Lol(Foo::Kek, 42));

        auto const tbl = ex.encoding_view().to_object().as<sol::table>();
        BOOST_CHECK(tbl["x"].get<int>() == 42);
        BOOST_CHECK(tbl["foo"].get<std::string>() == "Kek");
    }
}


BOOST_AUTO_TEST_CASE(CopyMove)
{
    Expression a {42};
    Expression b = a;
    Expression c = std::move(a);
}

BOOST_AUTO_TEST_CASE(FoldingExpressions)
{

    using haikan::impl::Keyword;
    Expression a {42};
    Expression b {67};
    Expression c = a | b;



    BOOST_CHECK(c.is(haikan::impl::Keyword::Pipe));
    BOOST_CHECK(c.encoding_view().size() == 3);
    BOOST_CHECK(c.encoding_view().keywords_.size() == 3);
    BOOST_CHECK_EQUAL(c.encoding_view().data_[1].as<int>(), 42);
    BOOST_CHECK_EQUAL(c.encoding_view().data_[2].as<int>(), 67);

    auto o = c.encoding_view().to_object();
    BOOST_CHECK(o.valid());
    BOOST_CHECK(!o.is<sol::nil_t>());
    BOOST_CHECK(o.get_type() == sol::type::table);
    auto tbl = o.as<sol::table>();
    BOOST_CHECK(tbl["keywords"][1].get<std::string>() == "Pipe");
    BOOST_CHECK(tbl["keywords"][2].get<std::string>() == "_Literal");
    BOOST_CHECK(tbl["keywords"][3].get<std::string>() == "_Literal");
    BOOST_CHECK(tbl["data"][1].get<sol::object>() == sol::nil);
    BOOST_CHECK(tbl["data"][2].get<int>() == 42);
    BOOST_CHECK(tbl["data"][3].get<int>() == 67);

    BOOST_CHECK_NE(c.serialize(), "");
    BOOST_CHECK_NE(c.serialize(), "null");

    std::stringstream ss;
    lua_to_stream(ss, o, 0, true);
    BOOST_CHECK_NE(ss.str(), "");
    BOOST_CHECK_NE(ss.str(), "null");
    BOOST_CHECK_NE(ss.str(), "nil");
}

BOOST_AUTO_TEST_CASE(SyntacticSugar)
{
    using haikan::impl::Keyword;
    Expression a {42};
    Expression b {"word"};
    Expression c {Lol(Foo::Kek, 11)};

    Expression lol = ~(a & b | c);

    auto const p = lol.prettify();

    bool const match =
            (p == R"(~((42 & "word") | {["x"]=11,["foo"]="Kek"}))")
        ||  (p == R"(~((42 & "word") | {["foo"]="Kek",["x"]=11}))");

    BOOST_CHECK_MESSAGE(match, "Prettify assertion failed for " << p);

    std::cerr << '\n';
    lol.prettify_to(std::cerr);
    std::cerr << '\n';
}


BOOST_AUTO_TEST_CASE(ExprEval)
{
    using haikan::impl::EvalContextLua;

    EvalContextLua ctx{};
    Expression lit {42};
    sol::object result = lit.eval(sol::nil, ctx).data();
    BOOST_CHECK_EQUAL(result.as<int>(), 42);
}



BOOST_AUTO_TEST_CASE(ApiConst)
{
    using namespace haikan;
    using haikan::impl::EvalContextLua;


    EvalContextLua ctx{};
    sol::object result = Pi.eval({}, ctx).data();
    BOOST_CHECK_EQUAL(result.as<double>(), boost::math::constants::pi<double>());
}


BOOST_AUTO_TEST_CASE(ApiFunc)
{
    using namespace haikan;
    using namespace haikan::impl;

    EvalContextLua ctx{};

    {
        sol::object result = Sqrt.eval(16, ctx).data();
        BOOST_CHECK_EQUAL(result.as<int>(), 4);
    }

    {
        sol::object result = Sqrt.eval(std::complex<int>{16, 0}, ctx).data();

        BOOST_CHECK_EQUAL(result.as<int>(), 4);
    }
    {
        sol::object result = Abs.eval(std::complex<double>{-16.1, 0}, ctx).data();
        BOOST_CHECK_EQUAL(result.as<double>(), 16.1);
    }
}


BOOST_AUTO_TEST_CASE(ApiOp)
{
    using namespace haikan;
    using haikan::impl::EvalContextLua;

    {
        EvalContextLua ctx{};
        sol::object result = Div(11).eval(44, ctx).data();
        BOOST_CHECK_EQUAL(result.as<int>(), 4);
    }
    {
        EvalContextLua ctx{};
        sol::object result = (Pi|Mul(2)).eval({}, ctx).data();
        BOOST_CHECK_EQUAL(result.as<double>(), boost::math::constants::pi<double>()*2);
    }
    {
        EvalContextLua ctx{};
        sol::object result = Div(Pi).eval(Pi|Mul(2), ctx).data();
        BOOST_CHECK_EQUAL(result.as<int>(), 2);
    }
}


BOOST_AUTO_TEST_SUITE_END()
