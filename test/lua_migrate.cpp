#include <boost/test/unit_test.hpp>

#include "haikan/impl/lua_migrate.hpp"
#include "haikan/reflection_registry.hpp"
#include "fixture/reflect_types.hpp"



BOOST_AUTO_TEST_CASE(LuaMigrateState)
{

    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Foo>);

    sol::state target_state;
    haikan::ReflectionRegistry::init(target_state);

    sol::object target_obj;
    {
        sol::state source_state;
        haikan::ReflectionRegistry::init(source_state);

        sol::object source_obj = source_state.create_table_with(
            "value", 14,
            "table", source_state.create_table_with(
                    "aa", 42, "bb", 13, "cc", 75
                ),
            "array", sol::make_object(source_state, sol::as_table(std::vector<int>{11,22,33,44,55})),
            "utype", sol::make_object(source_state, Foo::Kek)
        );

        target_obj = haikan::impl::migrate_to_state(target_state, source_obj).value();
    }

    sol::table target_table = target_obj;

    BOOST_CHECK(target_table["value"] == 14);
    BOOST_CHECK(target_table["table"]["bb"] == 13);
    BOOST_CHECK(target_table["array"][3] == 33);
    BOOST_CHECK(target_table["utype"] == Foo::Kek);

}



BOOST_AUTO_TEST_CASE(LuaMigrateAllowsRepeatedTableReference)
{
    sol::state source_state;
    sol::state target_state;
    sol::table shared = source_state.create_table_with("value", 42);
    sol::table source = source_state.create_table_with(
        "first", shared,
        "second", shared
    );

    auto const migrated = haikan::impl::migrate_to_state(target_state, source);

    BOOST_REQUIRE(migrated);
    sol::table const target = migrated->as<sol::table>();
    BOOST_CHECK(target["first"]["value"] == 42);
    BOOST_CHECK(target["second"]["value"] == 42);
}


BOOST_AUTO_TEST_CASE(LuaMigrateRejectsTableCycle)
{
    sol::state source_state;
    sol::state target_state;
    sol::table source = source_state.create_table();
    source["self"] = source;

    BOOST_CHECK(!haikan::impl::migrate_to_state(target_state, source));
}