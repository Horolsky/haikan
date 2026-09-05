#include <algorithm>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "haikan/impl/lua_iter.hpp"

using haikan::impl::LuaIter;
using haikan::impl::LuaNwiseIter;
using haikan::impl::LuaProdIter;
using haikan::impl::LuaZipIter;

namespace
{

sol::object array_object(sol::state& state, std::initializer_list<int> values)
{
    sol::table table = state.create_table();
    for (int const value: values)
    {
        table.add(value);
    }
    return sol::make_object(state, table);
}

std::vector<int> to_row(LuaIter::Vector const& values)
{
    std::vector<int> row;
    row.reserve(values.size());
    for (auto const& value: values)
    {
        row.push_back(value.as<int>());
    }
    return row;
}

void check_values(LuaIter::Vector const& values, std::initializer_list<int> expected)
{
    std::vector<int> const actual = to_row(values);
    std::vector<int> const expected_values(expected);

    BOOST_CHECK_EQUAL_COLLECTIONS(
        actual.cbegin(),
        actual.cend(),
        expected_values.cbegin(),
        expected_values.cend());
}

} // namespace

BOOST_AUTO_TEST_CASE(LuaZipIterYieldsAlignedRowsUntilShortestSequenceEnds)
{
    sol::state state;
    LuaIter::Vector sequences{
        array_object(state, {1, 2}),
        array_object(state, {10, 20, 30}),
        array_object(state, {100, 200})
    };
    LuaZipIter iter(sequences);

    BOOST_REQUIRE(!iter.halt());
    check_values(*iter, {1, 10, 100});

    iter++;
    BOOST_REQUIRE(!iter.halt());
    check_values(*iter, {2, 20, 200});

    iter++;
    BOOST_CHECK(iter.halt());
    BOOST_CHECK((*iter).empty());
}

BOOST_AUTO_TEST_CASE(LuaProdIterYieldsCartesianProductWithLastSequenceFastest)
{
    sol::state state;
    LuaProdIter iter(LuaIter::Vector{
        array_object(state, {1, 2}),
        array_object(state, {10, 20, 30})
    });

    BOOST_REQUIRE(!iter.halt());
    check_values(*iter, {1, 10});

    iter++;
    check_values(*iter, {1, 20});

    iter++;
    check_values(*iter, {1, 30});

    iter++;
    check_values(*iter, {2, 10});

    iter++;
    check_values(*iter, {2, 20});

    iter++;
    check_values(*iter, {2, 30});

    iter++;
    BOOST_CHECK(iter.halt());
    BOOST_CHECK((*iter).empty());
}

BOOST_AUTO_TEST_CASE(LuaIterRejectsNonArrayInputs)
{
    sol::state state;
    sol::table table = state.create_table();
    table["key"] = 42;

    BOOST_CHECK_THROW(LuaZipIter(LuaIter::Vector{sol::make_object(state, table)}), std::runtime_error);
    BOOST_CHECK_THROW(LuaProdIter(LuaIter::Vector{sol::make_object(state, 42)}), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(LuaIterWithNoSequencesIsHalted)
{
    LuaZipIter zip(LuaIter::Vector{});
    LuaProdIter prod(LuaIter::Vector{});
    LuaZipIter default_zip;
    LuaProdIter default_prod;

    BOOST_CHECK(zip.halt());
    BOOST_CHECK(prod.halt());
    BOOST_CHECK(default_zip.halt());
    BOOST_CHECK(default_prod.halt());
    BOOST_CHECK((*zip).empty());
    BOOST_CHECK((*prod).empty());
    BOOST_CHECK((*default_zip).empty());
    BOOST_CHECK((*default_prod).empty());
}

BOOST_AUTO_TEST_CASE(LuaNwiseIterRejectsZeroDegree)
{
    sol::state state;

    BOOST_CHECK_THROW(LuaNwiseIter(LuaIter::Vector{array_object(state, {1})}, 0), std::invalid_argument);
    BOOST_CHECK_THROW(LuaNwiseIter(0), std::invalid_argument);
}


BOOST_AUTO_TEST_CASE(LuaNwiseIterYieldsExpectedPairwiseRowsForFourTernaryParams)
{
    sol::state state;
    LuaNwiseIter iter(LuaIter::Vector{
        array_object(state, {1, 2, 3}),
        array_object(state, {10, 20, 30}),
        array_object(state, {100, 200, 300}),
        array_object(state, {1000, 2000, 3000})
    }, 2);

    check_values(*iter, {1, 10, 100, 1000});
    iter++;
    check_values(*iter, {1, 20, 200, 2000});
    iter++;
    check_values(*iter, {1, 30, 300, 3000});
    iter++;
    check_values(*iter, {2, 10, 200, 3000});
    iter++;
    check_values(*iter, {2, 20, 300, 1000});
    iter++;
    check_values(*iter, {2, 30, 100, 2000});
    iter++;
    check_values(*iter, {3, 10, 300, 2000});
    iter++;
    check_values(*iter, {3, 20, 100, 3000});
    iter++;
    check_values(*iter, {3, 30, 200, 1000});
    iter++;

    BOOST_CHECK(iter.halt());
}
