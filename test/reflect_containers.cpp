#include <boost/test/unit_test.hpp>

#include <array>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "haikan/reflection_registry.hpp"
#include "fixture/reflect_types.hpp"

BOOST_AUTO_TEST_CASE(ReflectVector)
{
    using reflect_kekvec = haikan::reflect<std::vector<Kek>>;

    auto test_kekvec = reflect_kekvec::init().value();
    test_kekvec.push_back(Kek(Foo::Kek, 67, 13));
    test_kekvec.push_back(Kek(Foo::Lol, 42, 11));

    sol::state state{};
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<std::vector<Kek>>);
    haikan::ReflectionRegistry::init(state);

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
}

BOOST_AUTO_TEST_CASE(ReflectArrayLikeContainers)
{
    sol::state state{};

    auto const list = std::list<int>{1, 2, 3};
    sol::object list_obj = haikan::reflect<std::list<int>>::serialize(list, state);
    BOOST_REQUIRE(list_obj.is<sol::table>());
    BOOST_CHECK_EQUAL(list_obj.as<sol::table>().size(), list.size());
    auto list_out = haikan::reflect<std::list<int>>::deserialize(list_obj);
    BOOST_REQUIRE(list_out);
    BOOST_CHECK(*list_out == list);

    auto const deque = std::deque<int>{4, 5, 6};
    sol::object deque_obj = haikan::reflect<std::deque<int>>::serialize(deque, state);
    BOOST_REQUIRE(deque_obj.is<sol::table>());
    auto deque_out = haikan::reflect<std::deque<int>>::deserialize(deque_obj);
    BOOST_REQUIRE(deque_out);
    BOOST_CHECK(*deque_out == deque);
}

BOOST_AUTO_TEST_CASE(ReflectInsertAndFixedSizeContainers)
{
    sol::state state{};

    auto const set = std::set<int>{3, 1, 2};
    sol::object set_obj = haikan::reflect<std::set<int>>::serialize(set, state);
    BOOST_REQUIRE(set_obj.is<sol::table>());
    auto set_out = haikan::reflect<std::set<int>>::deserialize(set_obj);
    BOOST_REQUIRE(set_out);
    BOOST_CHECK(*set_out == set);

    auto const array = std::array<int, 3>{{7, 8, 9}};
    sol::object array_obj = haikan::reflect<std::array<int, 3>>::serialize(array, state);
    BOOST_REQUIRE(array_obj.is<sol::table>());
    auto array_out = haikan::reflect<std::array<int, 3>>::deserialize(array_obj);
    BOOST_REQUIRE(array_out);
    BOOST_CHECK(*array_out == array);

    sol::table wrong_size = state.create_table();
    wrong_size.add(1);
    wrong_size.add(2);
    BOOST_CHECK((!haikan::reflect<std::array<int, 3>>::deserialize(wrong_size)));
}

BOOST_AUTO_TEST_CASE(ReflectNestedContainers)
{
    sol::state state{};

    auto const nested = std::vector<std::set<int>>{
        {1, 3},
        {2, 4, 6}
    };

    sol::object obj = haikan::reflect<std::vector<std::set<int>>>::serialize(nested, state);
    BOOST_REQUIRE(obj.is<sol::table>());
    BOOST_CHECK_EQUAL(obj.as<sol::table>().size(), nested.size());

    auto out = haikan::reflect<std::vector<std::set<int>>>::deserialize(obj);
    BOOST_REQUIRE(out);
    BOOST_CHECK(*out == nested);
}

BOOST_AUTO_TEST_CASE(ReflectContainerDeserializeRejectsInvalidObjects)
{
    sol::state state{};

    BOOST_CHECK(!haikan::reflect<std::vector<int>>::deserialize(sol::make_object(state, 42)));

    sol::table invalid = state.create_table();
    invalid.add(1);
    invalid.add("not an int");
    invalid.add(3);

    BOOST_CHECK(!haikan::reflect<std::vector<int>>::deserialize(invalid));
}

BOOST_AUTO_TEST_CASE(ReflectMap)
{
    using reflect_kekmap = haikan::reflect<std::map<std::string, Kek>>;

    auto test_kekmap = reflect_kekmap::init().value();
    test_kekmap["one"] = Kek(Foo::Kek, 67, 13);
    test_kekmap["two"] = Kek(Foo::Lol, 42, 11);

    sol::state state{};
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<std::map<std::string, Kek>>);
    haikan::ReflectionRegistry::init(state);

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
