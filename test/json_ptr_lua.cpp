#include <stdexcept>
#include <system_error>

#include <boost/test/unit_test.hpp>

#include "haikan/impl/json_ptr_lua.hpp"

using haikan::impl::at_pointer;
using haikan::impl::set_at_pointer;

namespace
{

struct JsonPtrLuaSuite
{
    JsonPtrLuaSuite()
    {
        state.open_libraries();
    }

    sol::object make_document()
    {
        sol::table root = state.create_table();
        root["plain"] = "value";
        root["a/b"] = 12;
        root["tilde~key"] = 34;

        sol::table child = state.create_table();
        child["name"] = "child";
        child["value"] = 42;
        root["child"] = child;

        sol::table items = state.create_table();
        items.add("zero");
        items.add("one");
        items.add("two");
        root["items"] = items;

        return sol::make_object(state, root);
    }

    sol::state state{};
};

} // namespace

BOOST_FIXTURE_TEST_SUITE(JsonPtrLuaTests, JsonPtrLuaSuite)

BOOST_AUTO_TEST_CASE(GetRootObject)
{
    sol::object root = make_document();

    BOOST_CHECK(at_pointer(root, "") == root);
}

BOOST_AUTO_TEST_CASE(GetObjectKeysAndArrayItems)
{
    sol::object root = make_document();

    BOOST_CHECK_EQUAL(at_pointer(root, "/plain").value().as<std::string>(), "value");
    BOOST_CHECK_EQUAL(at_pointer(root, "/child/name").value().as<std::string>(), "child");
    BOOST_CHECK_EQUAL(at_pointer(root, "/child/value").value().as<int>(), 42);
    BOOST_CHECK_EQUAL(at_pointer(root, "/items/0").value().as<std::string>(), "zero");
    BOOST_CHECK_EQUAL(at_pointer(root, "/items/1").value().as<std::string>(), "one");
    BOOST_CHECK_EQUAL(at_pointer(root, "/items/2").value().as<std::string>(), "two");
}

BOOST_AUTO_TEST_CASE(GetEscapedKeys)
{
    sol::object root = make_document();

    BOOST_CHECK_EQUAL(at_pointer(root, "/a~1b").value().as<int>(), 12);
    BOOST_CHECK_EQUAL(at_pointer(root, "/tilde~0key").value().as<int>(), 34);
}

BOOST_AUTO_TEST_CASE(SetExistingAndCreateNestedPaths)
{
    sol::object root = make_document();

    set_at_pointer(root, "/child/value", sol::make_object(state, 100));
    set_at_pointer(root, "/created/path/0/name", sol::make_object(state, "new"));

    BOOST_CHECK_EQUAL(at_pointer(root, "/child/value").value().as<int>(), 100);
    BOOST_CHECK_EQUAL(at_pointer(root, "/created/path/0/name").value().as<std::string>(), "new");
}

BOOST_AUTO_TEST_CASE(SetRootReplacesObject)
{
    sol::object root = make_document();

    set_at_pointer(root, "", sol::make_object(state, "replacement"));

    BOOST_CHECK_EQUAL(root.as<std::string>(), "replacement");
    BOOST_CHECK_EQUAL(at_pointer(root, "").value().as<std::string>(), "replacement");
}

BOOST_AUTO_TEST_CASE(SetArrayNewItems)
{
    sol::object root = make_document();

    BOOST_CHECK_NO_THROW(set_at_pointer(root, "/items/3", sol::make_object(state, "four")));
    BOOST_CHECK_EQUAL(       at_pointer(root, "/items/3").value().as<std::string>(), "four");

    // Unlike Boost.JSON equivale, this is safe as it results in sparce Lua table,
    // however, the # operator will lie.
    BOOST_CHECK_NO_THROW(set_at_pointer(root, "/items/18446744073709551616", sol::make_object(state, "lol")));
    BOOST_CHECK_EQUAL(       at_pointer(root, "/items/18446744073709551616").value().as<std::string>(), "lol");
}

BOOST_AUTO_TEST_CASE(InvalidPointersThrow)
{
    sol::object root = make_document();

    BOOST_CHECK(!at_pointer(root, "plain").has_value());
    BOOST_CHECK(!at_pointer(root, "/missing").has_value());
    BOOST_CHECK(!at_pointer(root, "/plain/name").has_value());
    BOOST_CHECK(!at_pointer(root, "/tilde~2key").has_value());
    BOOST_CHECK(!set_at_pointer(root, "plain", sol::make_object(state, 1)).has_value());
}

BOOST_AUTO_TEST_SUITE_END()
