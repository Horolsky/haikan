#include "haikan/expression_lua.hpp"

#include <cstdint>
#include <sstream>
#include <string>

#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "haikan/impl/lua_json_conversion.hpp"
#include "haikan/reflection_registry.hpp"
#include "haikan/reflection_meta.hpp"

#include "fixture/reflect_types.hpp"



using haikan::impl::json_to_lua;
using haikan::impl::lua_to_json;
using haikan::impl::lua_to_json_stream;
using haikan::impl::lua_to_stream;

namespace
{

struct ExpressionLuaSuite
{
    using ExpressionLua = haikan::ExpressionLua;

    ExpressionLuaSuite()
    {
        haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Foo>);
        haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Lol>);
        haikan::ReflectionRegistry::init(ExpressionLua::lua_state());
    }

    ~ExpressionLuaSuite()
    {
        ExpressionLua::lua_state().reset();
    }
};

} // namespace

BOOST_FIXTURE_TEST_SUITE(ExpressionLuaTests, ExpressionLuaSuite)

BOOST_AUTO_TEST_CASE(PrimitiveCtors)
{
    sol::state_view L = ExpressionLua::lua_state();
    {
        ExpressionLua ex {42};
        BOOST_CHECK(ex.to_json() == 42);
    }
    {
        ExpressionLua ex {"42"};
        BOOST_CHECK(ex.to_json() == "42");
    }
    {
        ExpressionLua ex {haikan::as_enum(Foo::Lol)};
        BOOST_CHECK(ex.encoding_view().data[0].load(L).is<haikan::impl::user_data_enum<Foo>>());
        auto obj = ex.encoding_view().to_object();
        BOOST_CHECK(!obj.is<haikan::impl::user_data_enum<Foo>>());

        BOOST_REQUIRE(obj.valid());

        BOOST_CHECK(obj.get_type() == sol::type::string);

        BOOST_CHECK(ex.to_json() == "Lol");
    }
    {
        ExpressionLua ex {Lol{Foo::Kek, 42}};
        BOOST_CHECK(ex.keyword() == haikan::impl::Keyword::_Literal);
        BOOST_CHECK(ex.encoding_view().data[0].load(L).as<Lol>() == Lol(Foo::Kek, 42));

        auto const tbl = ex.encoding_view().to_object().as<sol::table>();
        BOOST_CHECK(tbl["x"].get<int>() == 42);
        BOOST_CHECK(tbl["foo"].get<std::string>() == "Kek");
    }
}


BOOST_AUTO_TEST_CASE(CopyMove)
{
    ExpressionLua a {42};
    ExpressionLua b = a;
    ExpressionLua c = std::move(a);
}

BOOST_AUTO_TEST_CASE(FoldingExpressions)
{
    sol::state_view L = ExpressionLua::lua_state();

    using haikan::impl::Keyword;
    ExpressionLua a {42};
    ExpressionLua b {67};
    ExpressionLua c = a | b;



    BOOST_CHECK(c.is(haikan::impl::Keyword::Pipe));
    BOOST_CHECK(c.encoding_view().size() == 3);
    BOOST_CHECK(c.encoding_view().keywords.size() == 3);
    BOOST_CHECK_EQUAL(c.encoding_view().data[1].load(L).as<int>(), 42);
    BOOST_CHECK_EQUAL(c.encoding_view().data[2].load(L).as<int>(), 67);

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
    ExpressionLua a {42};
    ExpressionLua b {"word"};
    ExpressionLua c {Lol(Foo::Kek, 11)};

    ExpressionLua wtf = ~(a & b | c);
    std::cerr << '\n';
    wtf.prettify_to(std::cerr);
    std::cerr << '\n';

}

BOOST_AUTO_TEST_SUITE_END()
