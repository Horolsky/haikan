#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "haikan/impl/copy_on_write.hpp"

using haikan::impl::Cow;

namespace std {
template <class T>
std::ostream& operator<<(std::ostream& os, set<T> const& s)
{
    os << "{";

    for (auto const& item: s)
    {
        os << item;
        os << ',';
    }
    os << "}";
    return os;
}
}

namespace
{

struct CowSuite
{
    struct TestTables
    {
        sol::table orig_root;
        sol::table proxy_root;
        sol::table orig_child;
        sol::table proxy_child;
    };

    CowSuite()
    {
        state.open_libraries();
    }

    sol::table make_object()
    {
        sol::table orig = state.create_table();
        orig["a"] = 42;
        orig["delete me"] = 1;
        return orig;
    }

    sol::table make_array()
    {
        sol::table orig = state.create_table();
        orig.add(10);
        orig.add(20);
        orig.add(30);
        return orig;
    }

    sol::table make_unsorted_array()
    {
        sol::table orig = state.create_table();
        orig.add(30);
        orig.add(10);
        orig.add(20);
        return orig;
    }

    sol::table make_array_with_object()
    {
        sol::table orig = state.create_table();
        sol::table child = make_object();
        orig.add(child);
        return orig;
    }

    sol::table make_object_with_array()
    {
        sol::table orig = state.create_table();
        orig["child"] = make_array();
        return orig;
    }

    sol::table make_object_with_unsorted_array()
    {
        sol::table orig = state.create_table();
        orig["child"] = make_unsorted_array();
        return orig;
    }

    TestTables object_case()
    {
        sol::table orig_root = make_array_with_object();
        sol::table proxy_root = Cow::make(orig_root);
        return {orig_root, proxy_root, orig_root[1], proxy_root[1]};
    }

    TestTables array_case()
    {
        sol::table orig_root = make_object_with_array();
        sol::table proxy_root = Cow::make(orig_root);
        return {orig_root, proxy_root, orig_root["child"], proxy_root["child"]};
    }

    TestTables unsorted_array_case()
    {
        sol::table orig_root = make_object_with_unsorted_array();
        sol::table proxy_root = Cow::make(orig_root);
        return {orig_root, proxy_root, orig_root["child"], proxy_root["child"]};
    }

    std::string pairs_keys(sol::table proxy)
    {
        state.set("proxy", proxy);
        return state.script(R"(
            out = ""
            for k in pairs(proxy) do
                out = out .. ":" .. k
            end
            return out
        )").get<std::string>();
    }

    sol::state state{};
};

} // namespace

BOOST_FIXTURE_TEST_SUITE(CowTests, CowSuite)

BOOST_AUTO_TEST_CASE(ObjectRead)
{
    TestTables const tables = object_case();
    sol::table proxy = tables.proxy_child;

    BOOST_CHECK(proxy.is<Cow>());
    BOOST_CHECK(proxy["a"] == 42);
    BOOST_CHECK(proxy["delete me"] == 1);
    BOOST_CHECK(proxy["missing"] == sol::nil);

    Cow const cow_snapshot = proxy;
    BOOST_CHECK_EQUAL(cow_snapshot.length(), 0);
    BOOST_CHECK_EQUAL(cow_snapshot.keys(), (std::set<std::string>{"a", "delete me"}));
    BOOST_CHECK(cow_snapshot.ikeys().empty());
}

BOOST_AUTO_TEST_CASE(ObjectWrite)
{
    TestTables const tables = object_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    proxy["a"] = 67;

    BOOST_CHECK(orig["a"] == 42);
    BOOST_CHECK(proxy["a"] == 67);

    Cow const cow_snapshot = proxy;
    BOOST_CHECK_EQUAL(cow_snapshot.keys(), (std::set<std::string>{"a", "delete me"}));
}

BOOST_AUTO_TEST_CASE(ObjectAdd)
{
    TestTables const tables = object_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    proxy["b"] = "hello";
    proxy[42] = true;

    BOOST_CHECK(orig["b"] == sol::nil);
    BOOST_CHECK(orig[42] == sol::nil);
    BOOST_CHECK(proxy["b"].get<std::string>() == "hello");
    BOOST_CHECK(proxy[42] == true);

    Cow const cow_snapshot = proxy;
    BOOST_CHECK_EQUAL(cow_snapshot.keys(), (std::set<std::string>{"a", "b", "delete me"}));
    BOOST_CHECK_EQUAL(cow_snapshot.ikeys(), (std::set<std::int64_t>{42}));
}

BOOST_AUTO_TEST_CASE(ObjectDelete)
{
    TestTables const tables = object_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    proxy["delete me"] = sol::nil;

    BOOST_CHECK(orig["delete me"] == 1);
    BOOST_CHECK(proxy["delete me"] == sol::nil);
    BOOST_CHECK_EQUAL(pairs_keys(proxy), ":a");

    Cow const cow_snapshot = proxy;
    BOOST_CHECK_EQUAL(cow_snapshot.keys(), (std::set<std::string>{"a"}));
}

BOOST_AUTO_TEST_CASE(ArrayRead)
{
    TestTables const tables = array_case();
    sol::table proxy = tables.proxy_child;

    BOOST_CHECK(proxy.is<Cow>());
    BOOST_CHECK(proxy[1] == 10);
    BOOST_CHECK(proxy[3] == 30);
    BOOST_CHECK(proxy[4] == sol::nil);

    state.set("proxy", proxy);
    BOOST_CHECK_EQUAL(state.script("return table.concat(proxy, ',')").get<std::string>(), "10,20,30");
    BOOST_CHECK_EQUAL(state.script("local a, b, c = table.unpack(proxy); return a + b + c").get<int>(), 60);

    Cow const cow_snapshot = proxy;
    BOOST_CHECK_EQUAL(cow_snapshot.length(), 3);
    BOOST_CHECK(cow_snapshot.keys().empty());
    BOOST_CHECK(cow_snapshot.ikeys().empty());
}


BOOST_AUTO_TEST_CASE(ArrayWriteAssign)
{
    TestTables const tables = array_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    proxy[2] = 25;

    BOOST_CHECK(orig[2] == 20);
    BOOST_CHECK(proxy[2] == 25);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 3);
}


BOOST_AUTO_TEST_CASE(ArrayWriteSort)
{
    TestTables const tables = unsorted_array_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    state["table"]["sort"](proxy);

    BOOST_CHECK(orig[1] == 30);
    BOOST_CHECK(orig[2] == 10);
    BOOST_CHECK(orig[3] == 20);
    BOOST_CHECK(proxy[1] == 10);
    BOOST_CHECK(proxy[2] == 20);
    BOOST_CHECK(proxy[3] == 30);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 3);
}


BOOST_AUTO_TEST_CASE(ArrayWriteMove)
{
    TestTables const tables = array_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    state["table"]["move"](proxy, 1, 2, 2);

    BOOST_CHECK(orig[1] == 10);
    BOOST_CHECK(orig[2] == 20);
    BOOST_CHECK(orig[3] == 30);
    BOOST_CHECK(proxy[1] == 10);
    BOOST_CHECK(proxy[2] == 10);
    BOOST_CHECK(proxy[3] == 20);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 3);
}


BOOST_AUTO_TEST_CASE(ArrayAddAssign)
{
    TestTables const tables = array_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    proxy[4] = 40;

    BOOST_CHECK(orig[4] == sol::nil);
    BOOST_CHECK(proxy[4] == 40);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 4);
}


BOOST_AUTO_TEST_CASE(ArrayAddInsert)
{
    TestTables const tables = array_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    state["table"]["insert"](proxy, 40);

    BOOST_CHECK(orig[4] == sol::nil);
    BOOST_CHECK(proxy[4] == 40);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 4);
}


BOOST_AUTO_TEST_CASE(ArrayAddMove)
{
    TestTables const tables = array_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    state["table"]["move"](proxy, 1, 3, 2);

    BOOST_CHECK(orig[4] == sol::nil);
    BOOST_CHECK(proxy[1] == 10);
    BOOST_CHECK(proxy[2] == 10);
    BOOST_CHECK(proxy[3] == 20);
    BOOST_CHECK(proxy[4] == 30);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 4);
}


BOOST_AUTO_TEST_CASE(ArrayDeleteAssign)
{
    TestTables const tables = array_case();
    sol::table orig = tables.orig_child;
    sol::table proxy = tables.proxy_child;

    proxy[2] = sol::nil;

    BOOST_CHECK(orig[2] == 20);
    BOOST_CHECK(proxy[2] == sol::nil);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 3);
}


BOOST_AUTO_TEST_CASE(ArrayDeleteRemove)
{
    TestTables const tables = array_case();
    sol::table proxy = tables.proxy_child;

    proxy[4] = 40;
    sol::object removed = state["table"]["remove"](proxy, 3);

    BOOST_CHECK_EQUAL(removed.as<int>(), 30);
    BOOST_CHECK(proxy[3] == 40);
    BOOST_CHECK(proxy[4] == sol::nil);
    BOOST_CHECK_EQUAL(Cow(proxy).length(), 3);
}


BOOST_AUTO_TEST_SUITE_END()
