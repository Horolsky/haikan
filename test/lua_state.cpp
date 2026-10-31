#include <boost/test/unit_test.hpp>

#include "haikan/impl/lua_state.hpp"

using haikan::impl::LuaState;


BOOST_AUTO_TEST_CASE(LuaStateCreatesUsableState)
{
    LuaState state;

    BOOST_REQUIRE(state.lua_state() != nullptr);

    sol::state_view L = state.view();
    sol::table table = L.create_table();
    table["answer"] = 42;

    BOOST_CHECK_EQUAL(table["answer"].get<int>(), 42);
}

BOOST_AUTO_TEST_CASE(LuaStateOpensLibraries)
{
    LuaState state;
    state.open_libraries();

    sol::state_view L = state;
    int const status = luaL_dostring(state.lua_state(), "result = table.concat({'a', 'b', 'c'}, '')");

    BOOST_REQUIRE_EQUAL(status, LUA_OK);
    BOOST_CHECK_EQUAL(L["result"].get<std::string>(), "abc");
}

BOOST_AUTO_TEST_CASE(LuaStateMoveConstructionTransfersOwnership)
{
    LuaState original;
    lua_State* const raw = original.lua_state();

    LuaState moved(std::move(original));

    BOOST_CHECK_EQUAL(moved.lua_state(), raw);
    BOOST_CHECK(original.lua_state() == nullptr);

    sol::state_view L = moved.view();
    L["value"] = 17;
    BOOST_CHECK_EQUAL(L["value"].get<int>(), 17);
}

BOOST_AUTO_TEST_CASE(LuaStateMoveAssignmentReplacesState)
{
    LuaState first;
    LuaState second;
    lua_State* const second_raw = second.lua_state();

    first = std::move(second);

    BOOST_CHECK_EQUAL(first.lua_state(), second_raw);
    BOOST_CHECK(second.lua_state() == nullptr);

    sol::state_view L = first.view();
    sol::table table = L.create_table();
    table.add("x");
    table.add("y");

    BOOST_CHECK_EQUAL(table.size(), 2);
}

BOOST_AUTO_TEST_CASE(LuaStateAllocatorHandlesManyAllocations)
{
    LuaState state;
    state.open_libraries();

    sol::state_view L = state.view();
    sol::table table = L.create_table();

    for (int idx = 1; idx <= 512; ++idx)
    {
        table[idx] = std::string(static_cast<std::size_t>(idx % 64 + 1), 'x');
    }

    BOOST_CHECK_EQUAL(table.size(), 512);
    BOOST_CHECK_EQUAL(table[64].get<std::string>().size(), 1);
    BOOST_CHECK_EQUAL(table[127].get<std::string>().size(), 64);
}

BOOST_AUTO_TEST_CASE(LuaStateTracksUsedMemory)
{
    LuaState state;
    std::size_t const initial_used_memory = state.used_memory();

    BOOST_CHECK(initial_used_memory > 0);

    lua_newuserdata(state.lua_state(), 64 * 1024);

    BOOST_CHECK(state.used_memory() > initial_used_memory);
}

BOOST_AUTO_TEST_CASE(MovedFromLuaStateReportsNoUsedMemory)
{
    LuaState original;
    LuaState moved(std::move(original));

    BOOST_CHECK_EQUAL(original.used_memory(), 0);
    BOOST_CHECK(moved.used_memory() > 0);
}

BOOST_AUTO_TEST_CASE(LuaStateAllocatorCanGrowWithoutLimit)
{
    LuaState state(0, 0);
    state.open_libraries();

    sol::state_view L = state.view();
    sol::table table = L.create_table();

    for (int idx = 1; idx <= 256; ++idx)
    {
        table[idx] = std::string(128, 'x');
    }

    BOOST_CHECK_EQUAL(table.size(), 256);
}

BOOST_AUTO_TEST_CASE(LuaStateAllocatorHonorsMaxCapacity)
{
    BOOST_CHECK_THROW(LuaState(1, 1), std::bad_alloc);
}

BOOST_AUTO_TEST_CASE(LuaStateAllocatorThrowsBadAllocOnRuntimeMaxCapacityOverflow)
{
    BOOST_CHECK_THROW(
        []{
            LuaState state(0, 1024 * 1024);
            lua_newuserdata(state.lua_state(), 2 * 1024 * 1024);
        }(),
        std::bad_alloc);
}
