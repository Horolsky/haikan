#include <cstdint>
#include <sstream>
#include <string>

#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "haikan/impl/lua_json_conversion.hpp"

using haikan::impl::json_to_lua;
using haikan::impl::lua_to_json;
using haikan::impl::lua_to_json_stream;
using haikan::impl::lua_to_stream;

namespace
{

struct LuaJsonConversionSuite
{
    sol::state state{};
};

} // namespace

BOOST_FIXTURE_TEST_SUITE(LuaJsonConversionTests, LuaJsonConversionSuite)

BOOST_AUTO_TEST_CASE(LuaPrimitivesConvertToJson)
{
    auto const null_value = lua_to_json(sol::make_object(state, sol::nil));
    auto const boolean = lua_to_json(sol::make_object(state, true));
    auto const integer = lua_to_json(sol::make_object(state, std::int64_t{-42}));
    auto const number = lua_to_json(sol::make_object(state, 1.25));
    auto const string = lua_to_json(sol::make_object(state, std::string{"a\0b", 3}));

    BOOST_REQUIRE(null_value);
    BOOST_REQUIRE(boolean);
    BOOST_REQUIRE(integer);
    BOOST_REQUIRE(number);
    BOOST_REQUIRE(string);
    BOOST_CHECK(null_value->is_null());
    BOOST_CHECK_EQUAL(boolean->as_bool(), true);
    BOOST_CHECK_EQUAL(integer->as_int64(), -42);
    BOOST_CHECK_EQUAL(number->as_double(), 1.25);
    BOOST_CHECK((string->as_string() == boost::json::string{"a\0b", 3}));
}

BOOST_AUTO_TEST_CASE(LuaTablesConvertToNestedJson)
{
    sol::table items = state.create_table(3, 0);
    items.add(1);
    items.add("two");
    items.add(false);

    sol::table child = state.create_table(0, 1);
    child["answer"] = 42;

    sol::table root = state.create_table(0, 2);
    root["items"] = items;
    root["child"] = child;

    auto const converted = lua_to_json(sol::make_object(state, root));

    BOOST_REQUIRE(converted);
    boost::json::object const& object = converted->as_object();
    boost::json::array const& array = object.at("items").as_array();
    BOOST_CHECK_EQUAL(array.size(), 3);
    BOOST_CHECK_EQUAL(array[0].as_int64(), 1);
    BOOST_CHECK_EQUAL(array[1].as_string(), "two");
    BOOST_CHECK_EQUAL(array[2].as_bool(), false);
    BOOST_CHECK_EQUAL(object.at("child").as_object().at("answer").as_int64(), 42);
}

BOOST_AUTO_TEST_CASE(JsonConvertsToNestedLuaTables)
{
    boost::json::value const source = {
        {"name", "root"},
        {"items", {1, 2.5, true}},
        {"child", {{"value", -7}}},
    };

    auto const converted = json_to_lua(state, source);

    BOOST_REQUIRE(converted);
    BOOST_REQUIRE(converted->get_type() == sol::type::table);
    sol::table const root = converted->as<sol::table>();
    BOOST_CHECK_EQUAL(root["name"].get<std::string>(), "root");
    sol::table const items = root["items"];
    BOOST_CHECK_EQUAL(items.size(), 3);
    BOOST_CHECK_EQUAL(items[1].get<std::int64_t>(), 1);
    BOOST_CHECK_EQUAL(items[2].get<double>(), 2.5);
    BOOST_CHECK_EQUAL(items[3].get<bool>(), true);
    sol::table const child = root["child"];
    BOOST_CHECK_EQUAL(child["value"].get<std::int64_t>(), -7);
}

BOOST_AUTO_TEST_CASE(JsonPrimitivesConvertToLua)
{
    auto const null_value = json_to_lua(state, nullptr);
    auto const boolean = json_to_lua(state, true);
    auto const integer = json_to_lua(state, std::int64_t{-12});
    auto const unsigned_integer = json_to_lua(state, std::uint64_t{12});
    auto const number = json_to_lua(state, 4.5);
    auto const string = json_to_lua(state, boost::json::string{"x\0y", 3});

    BOOST_REQUIRE(null_value);
    BOOST_REQUIRE(boolean);
    BOOST_REQUIRE(integer);
    BOOST_REQUIRE(unsigned_integer);
    BOOST_REQUIRE(number);
    BOOST_REQUIRE(string);
    BOOST_CHECK(*null_value == sol::nil);
    BOOST_CHECK_EQUAL(boolean->as<bool>(), true);
    BOOST_CHECK_EQUAL(integer->as<std::int64_t>(), -12);
    BOOST_CHECK_EQUAL(unsigned_integer->as<std::int64_t>(), 12);
    BOOST_CHECK_EQUAL(number->as<double>(), 4.5);
    BOOST_CHECK((string->as<std::string>() == std::string{"x\0y", 3}));
}

BOOST_AUTO_TEST_CASE(UnsupportedLuaValuesReturnNone)
{
    state["function_value"] = [] {};

    lua_newuserdata(state.lua_state(), 1);
    sol::object const userdata = sol::stack::get<sol::object>(state.lua_state(), -1);
    lua_pop(state.lua_state(), 1);

    BOOST_CHECK(!lua_to_json(state["function_value"]).has_value());
    BOOST_CHECK(!lua_to_json(userdata).has_value());
}

BOOST_AUTO_TEST_CASE(SparseLuaTablesConvertToKeyValuePairs)
{
    sol::table sparse = state.create_table();
    sparse[1] = "one";
    sparse[3] = "three";

    BOOST_CHECK((lua_to_json(sol::make_object(state, sparse)) == boost::json::value{
        {1, "one"},
        {3, "three"},
    }));

    std::ostringstream lua;
    lua_to_stream(lua, sol::make_object(state, sparse), 0, false);
    BOOST_CHECK_EQUAL(lua.str(), R"({[1]="one",[3]="three"})");
}

BOOST_AUTO_TEST_CASE(InvalidLuaTablesReturnNone)
{
    sol::table mixed = state.create_table();
    mixed[1] = "item";
    mixed["name"] = "value";


    sol::table invalid_key = state.create_table();
    invalid_key[true] = "value";

    BOOST_CHECK(!lua_to_json(sol::make_object(state, mixed)).has_value());
    BOOST_CHECK(!lua_to_json(sol::make_object(state, invalid_key)).has_value());
}

BOOST_AUTO_TEST_CASE(CyclicLuaTablesReturnNone)
{
    sol::table table = state.create_table();
    table["self"] = table;

    BOOST_CHECK(!lua_to_json(sol::make_object(state, table)).has_value());
}

BOOST_AUTO_TEST_CASE(StreamConversionsCanBeCompactOrPrettyPrinted)
{
    sol::table value = state.create_table_with(1, 1, 2, "two");
    sol::object const object = sol::make_object(state, value);
    std::ostringstream lua_compact;
    std::ostringstream json_compact;
    std::ostringstream lua_pretty;
    std::ostringstream json_pretty;

    lua_to_stream(lua_compact, object, 1, false);
    lua_to_json_stream(json_compact, object, 1, false);
    lua_to_stream(lua_pretty, object, 1, true);
    lua_to_json_stream(json_pretty, object, 1, true);

    BOOST_CHECK_EQUAL(lua_compact.str(), R"({1,"two"})");
    BOOST_CHECK_EQUAL(json_compact.str(), R"([1,"two"])");
    BOOST_CHECK_EQUAL(lua_pretty.str(), "    {\n        1,\n        \"two\"\n    }");
    BOOST_CHECK_EQUAL(json_pretty.str(), "    [\n        1,\n        \"two\"\n    ]");
}


BOOST_AUTO_TEST_CASE(DemoTest)
{

auto const s = R"({
    "hello": [
        0,
        [],
        {
            "from": "JSON"
        },
        42,
        67
    ]
})";

    boost::json::value x = boost::json::parse(s);

    sol::object x_lua = json_to_lua(state, x).value();
    std::ostringstream ss;
    lua_to_json_stream(ss, x_lua, 0, true);
    BOOST_CHECK_EQUAL(ss.str(), s);
}

BOOST_AUTO_TEST_SUITE_END()
