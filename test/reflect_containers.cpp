/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <boost/test/unit_test.hpp>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "haikan/impl/type_registry.hpp"
#include "haikan/impl/lua_stream.hpp"
#include "fixture/reflect_types.hpp"

BOOST_AUTO_TEST_CASE(ReflectVector)
{
    auto test_kekvec = std::vector<Kek>{};
    test_kekvec.push_back(Kek(Foo::Kek, 67, 13));
    test_kekvec.push_back(Kek(Foo::Lol, 42, 11));

    sol::state state{};
    haikan::impl::TypeRegistry registry;
    registry.Add(haikan::type<Foo>)
        .Add(haikan::type<Kek>)
        .Add(haikan::type<std::vector<Kek>>);
    registry.init(state);

    sol::object x = sol::make_object(state, test_kekvec);
    BOOST_CHECK(x.as<std::vector<Kek>>() == test_kekvec);

    sol::table t = x;
    t[1]["y"] = test_kekvec[0].y = 0.1f;
    t[2]["foo"] = haikan::as_enum(test_kekvec[1].foo = Foo::Lol);
    BOOST_CHECK(x.as<std::vector<Kek>>() == test_kekvec);

    state.set("x", x);
    state.open_libraries();
    state.script(R"(
        x[1].foo = 314
    )");
    BOOST_CHECK(t[1]["foo"] == haikan::as_enum(Foo(314)));

    sol::object xx = haikan::to_plain_lua(x);
    BOOST_REQUIRE(xx.is<sol::table>());
    sol::table serialized = xx;
    BOOST_CHECK_EQUAL(serialized.size(), test_kekvec.size());
    BOOST_CHECK_EQUAL(serialized[1]["x"].get<int>(), test_kekvec[0].x);
    BOOST_CHECK_EQUAL(serialized[1]["foo"].get<std::string>(), "314");
}

BOOST_AUTO_TEST_CASE(ReflectArrayLikeContainers)
{
    sol::state state{};

    auto const list = std::list<int>{1, 2, 3};
    sol::object list_obj = haikan::impl::TypeRegistry::Serialize(list, state);
    BOOST_REQUIRE(list_obj.is<sol::table>());
    BOOST_CHECK_EQUAL(list_obj.as<sol::table>().size(), list.size());
    auto list_out = haikan::impl::TypeRegistry::Deserialize(list_obj, haikan::type<std::list<int>>);
    BOOST_REQUIRE(list_out);
    BOOST_CHECK(*list_out == list);

    auto const deque = std::deque<int>{4, 5, 6};
    sol::object deque_obj = haikan::impl::TypeRegistry::Serialize(deque, state);
    BOOST_REQUIRE(deque_obj.is<sol::table>());
    auto deque_out = haikan::impl::TypeRegistry::Deserialize(deque_obj, haikan::type<std::deque<int>>);
    BOOST_REQUIRE(deque_out);
    BOOST_CHECK(*deque_out == deque);
}

BOOST_AUTO_TEST_CASE(ReflectInsertAndFixedSizeContainers)
{
    sol::state state{};

    auto const set = std::set<int>{3, 1, 2};
    sol::object set_obj = haikan::impl::TypeRegistry::Serialize(set, state);
    BOOST_REQUIRE(set_obj.is<sol::table>());
    auto set_out = haikan::impl::TypeRegistry::Deserialize(set_obj, haikan::type<std::set<int>>);
    BOOST_REQUIRE(set_out);
    BOOST_CHECK(*set_out == set);

    auto const array = std::array<int, 3>{{7, 8, 9}};
    sol::object array_obj = haikan::impl::TypeRegistry::Serialize(array, state);
    BOOST_REQUIRE(array_obj.is<sol::table>());
    auto array_out = haikan::impl::TypeRegistry::Deserialize(array_obj, haikan::type<std::array<int, 3>>);
    BOOST_REQUIRE(array_out);
    BOOST_CHECK(*array_out == array);

    sol::table wrong_size = state.create_table();
    wrong_size.add(1);
    wrong_size.add(2);
    BOOST_CHECK((!haikan::impl::TypeRegistry::Deserialize(wrong_size, haikan::type<std::array<int, 3>>)));
}

BOOST_AUTO_TEST_CASE(ReflectNestedContainers)
{
    sol::state state{};

    auto const nested = std::vector<std::set<int>>{
        {1, 3},
        {2, 4, 6}
    };

    sol::object obj = haikan::impl::TypeRegistry::Serialize(nested, state);
    BOOST_REQUIRE(obj.is<sol::table>());
    BOOST_CHECK_EQUAL(obj.as<sol::table>().size(), nested.size());

    auto out = haikan::impl::TypeRegistry::Deserialize(obj, haikan::type<std::vector<std::set<int>>>);
    BOOST_REQUIRE(out);
    BOOST_CHECK(*out == nested);
}

BOOST_AUTO_TEST_CASE(ReflectContainerDeserializeRejectsInvalidObjects)
{
    sol::state state{};

    BOOST_CHECK(!haikan::impl::TypeRegistry::Deserialize(sol::make_object(state, 42), haikan::type<std::vector<int>>));

    sol::table invalid = state.create_table();
    invalid.add(1);
    invalid.add("not an int");
    invalid.add(3);

    BOOST_CHECK(!haikan::impl::TypeRegistry::Deserialize(invalid, haikan::type<std::vector<int>>));
}

BOOST_AUTO_TEST_CASE(ReflectSharedPtr)
{
    sol::state state{};
    haikan::impl::TypeRegistry registry;
    registry.Add(haikan::type<std::shared_ptr<Lol>>);
    registry.init(state);

    auto const input = std::make_shared<Lol>(Foo::Kek, 67);
    sol::object serialized = haikan::impl::TypeRegistry::Serialize(input, state);
    BOOST_REQUIRE(serialized.is<sol::table>());

    auto deserialized = haikan::impl::TypeRegistry::Deserialize(serialized, haikan::type<std::shared_ptr<Lol>>);
    BOOST_REQUIRE(deserialized);
    BOOST_REQUIRE(*deserialized);
    BOOST_CHECK(**deserialized == *input);

    std::shared_ptr<Lol> const empty;
    BOOST_CHECK(haikan::impl::TypeRegistry::Serialize(empty, state) == sol::nil);
    BOOST_CHECK(!haikan::impl::TypeRegistry::Deserialize(sol::make_object(state, 42), haikan::type<std::shared_ptr<Lol>>));
}

BOOST_AUTO_TEST_CASE(ReflectPair)
{
    using Pair = std::pair<int, std::string>;

    sol::state state{};
    Pair const input{42, "Lol"};

    sol::object serialized = haikan::impl::TypeRegistry::Serialize(input, state);
    BOOST_REQUIRE(serialized.is<sol::table>());
    sol::table table = serialized;
    BOOST_CHECK_EQUAL(table[1].get<int>(), input.first);
    BOOST_CHECK_EQUAL(table[2].get<std::string>(), input.second);

    auto deserialized = haikan::impl::TypeRegistry::Deserialize(serialized, haikan::type<Pair>);
    BOOST_REQUIRE(deserialized);
    BOOST_CHECK(*deserialized == input);

    BOOST_CHECK(!haikan::impl::TypeRegistry::Deserialize(sol::make_object(state, 42), haikan::type<Pair>));
    table[2] = sol::nil;
    BOOST_CHECK(!haikan::impl::TypeRegistry::Deserialize(table, haikan::type<Pair>));
}

BOOST_AUTO_TEST_CASE(ReflectTuple)
{
    using Tuple = std::tuple<int, std::string, double>;

    sol::state state{};
    Tuple const input{42, "Lol", 3.5};

    sol::object serialized = haikan::impl::TypeRegistry::Serialize(input, state);
    BOOST_REQUIRE(serialized.is<sol::table>());
    sol::table table = serialized;
    BOOST_CHECK_EQUAL(table[1].get<int>(), std::get<0>(input));
    BOOST_CHECK_EQUAL(table[2].get<std::string>(), std::get<1>(input));
    BOOST_CHECK_EQUAL(table[3].get<double>(), std::get<2>(input));

    auto deserialized = haikan::impl::TypeRegistry::Deserialize(serialized, haikan::type<Tuple>);
    BOOST_REQUIRE(deserialized);
    BOOST_CHECK(*deserialized == input);
}

BOOST_AUTO_TEST_CASE(ReflectMap)
{
    auto test_kekmap = std::map<std::string, Kek>{};
    test_kekmap["one"] = Kek(Foo::Kek, 67, 13);
    test_kekmap["two"] = Kek(Foo::Lol, 42, 11);

    sol::state state{};
    haikan::impl::TypeRegistry registry;
    registry.Add(haikan::type<std::map<std::string, Kek>>);
    registry.init(state);

    auto copy = test_kekmap;
    sol::object x = sol::make_object(state, std::move(copy));
    BOOST_CHECK((x.as<std::map<std::string, Kek>>()) == test_kekmap);

    state.open_libraries();
    state.set("x", x);
    state.script(R"(x.tre = Kek:new())");

    sol::userdata t = x;
    t["one"]["y"]   = (test_kekmap["one"].y   = 0.1f);
    t["two"]["foo"] = haikan::as_enum(test_kekmap["two"].foo = Foo::None);

    auto const desolified = x.as<std::map<std::string, Kek>>();

    BOOST_CHECK(desolified.count("tre"));
    // Looks like an issue with sol2 usertype for assoc conts -
    // elements of map are not modifiable and no error thrown.
    BOOST_WARN(desolified.at("one") == test_kekmap["one"]);
    BOOST_WARN(desolified.at("two") == test_kekmap["two"]);

    auto out = state.script(R"(
        out = ""
        for k in pairs(x) do
            out = out .. ":" .. k
        end
        return out
    )").get<std::string>();

    BOOST_CHECK(out.find(":one") != std::string::npos);
    BOOST_CHECK(out.find(":two") != std::string::npos);
    BOOST_CHECK(out.find(":tre") != std::string::npos);
}
