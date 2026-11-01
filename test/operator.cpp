/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <cstdint>
#include <sstream>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "haikan/impl/operator_table.hpp"
#include "haikan/impl/generic_operand.hpp"
#include "haikan/impl/operators.hpp"
#include "haikan/impl/type_registry.hpp"
#include "haikan/impl/lua_stream.hpp"


namespace
{

struct OperatorHandlerSuite
{
    sol::state state;
};

} // namespace

using haikan::error;
using haikan::impl::Keyword;
using haikan::type;
using haikan::type_list;
using haikan::template_list;

BOOST_FIXTURE_TEST_SUITE(ExpressionLuaTests, OperatorHandlerSuite)

struct X
{
    double val;

    X() = default;
    X(double v) : val{v} {}

    X operator+(X const& rhs) const
    {
        return X{val + rhs.val};
    }

    friend X operator-(X const& lhs, X const& rhs)
    {
        return X{lhs.val - rhs.val};
    }
};

BOOST_DESCRIBE_STRUCT(X, (), (val));

BOOST_AUTO_TEST_CASE(OperatorsReflected)
{
    haikan::impl::TypeRegistry registry;
    registry.Add(type<X>);
    registry.init(state);
    haikan::impl::OperatorRegistry().AddSignature(type<X>).Connects(type<double>);


    state.open_libraries();


    {
        sol::object result = state.script(
            "return X(3) + X(2) - X(.5)"
        );
        BOOST_CHECK_EQUAL(result.as<X>().val, 4.5);
    }
    {
        sol::object error = state.script(
            "return X(3) * X(2)"
        );
        BOOST_CHECK_EQUAL(error.as<haikan::error>().what, "operator not supported");
    }
    {
        sol::object error = state.script("return X('lol')");
        BOOST_CHECK_EQUAL(error.as<haikan::error>().what, "invalid ctor argument");
    }
}



BOOST_AUTO_TEST_CASE(Ops)
{
    namespace op = haikan::op;
    state.open_libraries();

    {
        sol::object result = op::negate()(state.lua_state(), 42);
        BOOST_CHECK_EQUAL(result.as<int>(), -42);
    }

    {
        sol::object result = op::complement()(state.lua_state(), 42);
        BOOST_CHECK_EQUAL(result.as<int>(), ~42);
    }

    {
        sol::object result = op::minus()(state.lua_state(), 42, 12);
        BOOST_CHECK_EQUAL(result.as<int>(), 42 - 12);
    }

    {
        sol::object result = op::modulo()(state.lua_state(), 42, 12);
        BOOST_CHECK_EQUAL(result.as<int>(), 42 % 12);
    }

    {
        sol::object result = op::divide()(state.lua_state(), 42, 0);
        BOOST_CHECK(result.is<error>());
    }
    {
        struct lol_type {
            std::string operator%(int) const
            {
                return "kek";
            }
        } lol;
        sol::object result = op::modulo()(state.lua_state(), lol, 0);
        BOOST_CHECK(!result.is<error>());
        BOOST_CHECK_EQUAL(result.as<std::string>(), "kek");
    }

    {
        sol::object result = op::boolean()(state.lua_state(), 42);
        BOOST_CHECK_EQUAL(result.as<bool>(), true);
    }
    {
        sol::object result = op::boolean()(state.lua_state(), 0);
        BOOST_CHECK_EQUAL(result.as<bool>(), false);
    }
}

BOOST_AUTO_TEST_CASE(GenericOperandTest)
{
    using GenericOperand = haikan::impl::GenericOperand;
    auto const lol = sol::make_object(state, "lol").as<GenericOperand>();
    auto const kek = sol::make_object(state, "kek").as<GenericOperand>();

    {
        sol::object result = haikan::op::plus()(state.lua_state(), lol, kek);
        BOOST_CHECK_EQUAL(result.as<std::string>(), "lolkek");
    }
    {
        sol::object result = haikan::op::greater()(state.lua_state(), lol, kek);
        BOOST_CHECK(result.as<bool>());
    }
}

BOOST_AUTO_TEST_CASE(OpMissing)
{
    namespace op = haikan::op;

    struct none_t {} none;
    state.open_libraries();

    {
        sol::object result = op::minus()(state.lua_state(), none, none);
        BOOST_CHECK(result.is<error>());
    }

    {
        sol::object result = op::boolean()(state.lua_state(), none);
        BOOST_CHECK(result.is<error>());
    }
}


X operator-(X const& lhs, double const& rhs)
{
    return X{lhs.val - rhs};
}

X operator-(double const& lhs, X const& rhs)
{
    return X{lhs - rhs.val};
}

X operator+(X const& lhs, double const& rhs)
{
    return X{lhs.val + rhs};
}

X operator+(double const& lhs, X const& rhs)
{
    return X{lhs + rhs.val};
}

BOOST_AUTO_TEST_CASE(OpTable)
{
    using haikan::impl::OperatorRegistry;
    using haikan::impl::OperatorTable;

    OperatorRegistry().AddSignature(type<X>).Connects(type<double>);

    state.open_libraries();
    OperatorTable x2d = OperatorRegistry::get(type<X>, type<double>).value();
    OperatorTable d2x = OperatorRegistry::get(type<double>, type<X>).value();

    sol::object x = sol::make_object(state.lua_state(), X{42});
    sol::object y = sol::make_object(state.lua_state(), 11.0);

    {
        sol::object result = x2d.apply(Keyword::Add, x, y);
        BOOST_CHECK_EQUAL(result.as<X>().val, 53);
    }

    {
        sol::object result = x2d.apply(Keyword::Sub, x, y);
        BOOST_CHECK_EQUAL(result.as<X>().val, 31);
    }

    {
        sol::object result = d2x.apply(Keyword::Add, y, x);
        BOOST_CHECK_EQUAL(result.as<X>().val, 53);
    }

    {
        sol::object result = d2x.apply(Keyword::Sub, y, x);
        BOOST_CHECK_EQUAL(result.as<X>().val, -31);
    }

    {
        sol::object result = x2d.apply(Keyword::BitNot, x);
        BOOST_REQUIRE(result.is<error>());
        BOOST_CHECK_EQUAL(result.as<error>().what, "operator not implemented");
        BOOST_CHECK_EQUAL(result.as<error>().where, "haikan::op::complement");
    }

}


BOOST_AUTO_TEST_CASE(SetOperators)
{
    std::set<int> x{1,2,3,4,5,6,7,8};
    std::set<int> subset{1,3,7};
    std::set<int> equal{x};
    std::set<int> not_subset{1,9};
    std::set<int> empty;
    int y {7};
    double z {42};

    {
        sol::object result = haikan::op::contains()(state.lua_state(), x, y);
        BOOST_CHECK_EQUAL(result.as<bool>(), true);
    }

    {
        sol::object result = haikan::op::contains()(state.lua_state(), x, z);
        BOOST_CHECK_EQUAL(result.as<bool>(), false);
    }


    {
        sol::object result = haikan::op::is_in()(state.lua_state(), y, x);
        BOOST_CHECK_EQUAL(result.as<bool>(), true);
    }

    {
        sol::object result = haikan::op::is_in()(state.lua_state(), z, x);
        BOOST_CHECK_EQUAL(result.as<bool>(), false);
    }

    BOOST_CHECK(haikan::op::is_subset()(state.lua_state(), subset, x).as<bool>());
    BOOST_CHECK(haikan::op::is_subset()(state.lua_state(), equal, x).as<bool>());
    BOOST_CHECK(haikan::op::is_subset()(state.lua_state(), empty, x).as<bool>());
    BOOST_CHECK(!haikan::op::is_subset()(state.lua_state(), x, subset).as<bool>());
    BOOST_CHECK(!haikan::op::is_subset()(state.lua_state(), not_subset, x).as<bool>());

    BOOST_CHECK(haikan::op::set_equal()(state.lua_state(), x, equal).as<bool>());
    BOOST_CHECK(haikan::op::set_equal()(state.lua_state(), empty, empty).as<bool>());
    BOOST_CHECK(!haikan::op::set_equal()(state.lua_state(), subset, x).as<bool>());
    BOOST_CHECK(!haikan::op::set_equal()(state.lua_state(), not_subset, x).as<bool>());

    BOOST_CHECK(haikan::op::is_proper_subset()(state.lua_state(), subset, x).as<bool>());
    BOOST_CHECK(haikan::op::is_proper_subset()(state.lua_state(), empty, x).as<bool>());
    BOOST_CHECK(!haikan::op::is_proper_subset()(state.lua_state(), equal, x).as<bool>());
    BOOST_CHECK(!haikan::op::is_proper_subset()(state.lua_state(), empty, empty).as<bool>());
    BOOST_CHECK(!haikan::op::is_proper_subset()(state.lua_state(), not_subset, x).as<bool>());

    BOOST_CHECK(haikan::op::is_superset()(state.lua_state(), x, subset).as<bool>());
    BOOST_CHECK(haikan::op::is_superset()(state.lua_state(), x, equal).as<bool>());
    BOOST_CHECK(!haikan::op::is_superset()(state.lua_state(), subset, x).as<bool>());

    BOOST_CHECK(haikan::op::is_proper_superset()(state.lua_state(), x, subset).as<bool>());
    BOOST_CHECK(haikan::op::is_proper_superset()(state.lua_state(), x, empty).as<bool>());
    BOOST_CHECK(!haikan::op::is_proper_superset()(state.lua_state(), x, equal).as<bool>());
    BOOST_CHECK(!haikan::op::is_proper_superset()(state.lua_state(), empty, empty).as<bool>());

    std::set<int> lhs{1, 2, 3, 4};
    std::vector<short> rhs{3, 4, 5, 6};

    {
        auto const result = haikan::op::set_union()(state.lua_state(), lhs, rhs);
        auto const actual = result.as<std::set<int>>();
        BOOST_CHECK(actual == (std::set<int>{1, 2, 3, 4, 5, 6}));
    }

    {
        auto const result = haikan::op::intersection()(state.lua_state(), lhs, rhs);
        auto const actual = result.as<std::set<int>>();
        BOOST_CHECK(actual == (std::set<int>{3, 4}));
    }

    {
        auto const result = haikan::op::difference()(state.lua_state(), lhs, rhs);
        auto const actual = result.as<std::set<int>>();
        BOOST_CHECK(actual == (std::set<int>{1, 2}));
    }

    {
        auto const result = haikan::op::symmetric_difference()(state.lua_state(), lhs, rhs);
        auto const actual = result.as<std::set<int>>();
        BOOST_CHECK(actual == (std::set<int>{1, 2, 5, 6}));
    }

    {
        auto const result = haikan::op::set_union()(state.lua_state(), rhs, lhs);
        auto const actual = result.as<std::vector<short>>();
        BOOST_CHECK(actual == (std::vector<short>{3, 4, 5, 6, 1, 2}));
    }

    {
        auto const result = haikan::op::intersection()(state.lua_state(), rhs, lhs);
        auto const actual = result.as<std::vector<short>>();
        BOOST_CHECK(actual == (std::vector<short>{3, 4}));
    }

    {
        std::string a, b;
        a = "aabbccddeeaa";
        b = "bcdef";
        auto const result = haikan::op::intersection()(state.lua_state(), a, b);
        auto const actual = result.as<std::string>();
        BOOST_CHECK_EQUAL(actual, "bbccddee");
    }

    {
        std::string const input = "aabbccddee";
        auto const result = haikan::op::uniques()(state.lua_state(), input);
        BOOST_CHECK(result.is<std::string>());
        BOOST_CHECK_EQUAL(result.as<std::string>(), "abcde");
    }

    {
        std::vector<short> const input{3, 4, 3, 5, 4, 6};
        auto const result = haikan::op::uniques()(state.lua_state(), input);
        BOOST_CHECK(result.is<std::vector<short>>());
        BOOST_CHECK(result.as<std::vector<short>>() == (std::vector<short>{3, 4, 5, 6}));
    }

    {
        std::set<int> const input{1, 2, 3};
        auto const result = haikan::op::uniques()(state.lua_state(), input);
        BOOST_CHECK(result.is<std::set<int>>());
        BOOST_CHECK(result.as<std::set<int>>() == input);
    }

    {
        std::set<int> const a{1, 2, 3, 4};
        std::set<int> const b{3, 4, 5, 6};
        BOOST_CHECK(haikan::op::set_union()(state.lua_state(), a, b).as<std::set<int>>() ==
                    (std::set<int>{1, 2, 3, 4, 5, 6}));
        BOOST_CHECK(haikan::op::intersection()(state.lua_state(), a, b).as<std::set<int>>() ==
                    (std::set<int>{3, 4}));
        BOOST_CHECK(haikan::op::difference()(state.lua_state(), a, b).as<std::set<int>>() ==
                    (std::set<int>{1, 2}));
        BOOST_CHECK(haikan::op::symmetric_difference()(state.lua_state(), a, b).as<std::set<int>>() ==
                    (std::set<int>{1, 2, 5, 6}));
    }

    {
        std::unordered_set<int> const a{1, 2, 3, 4};
        std::unordered_set<int> const b{3, 4, 5, 6};
        BOOST_CHECK(haikan::op::set_union()(state.lua_state(), a, b).as<std::unordered_set<int>>() ==
                    (std::unordered_set<int>{1, 2, 3, 4, 5, 6}));
        BOOST_CHECK(haikan::op::intersection()(state.lua_state(), a, b).as<std::unordered_set<int>>() ==
                    (std::unordered_set<int>{3, 4}));
        BOOST_CHECK(haikan::op::difference()(state.lua_state(), a, b).as<std::unordered_set<int>>() ==
                    (std::unordered_set<int>{1, 2}));
        BOOST_CHECK(haikan::op::symmetric_difference()(state.lua_state(), a, b).as<std::unordered_set<int>>() ==
                    (std::unordered_set<int>{1, 2, 5, 6}));
        BOOST_CHECK(haikan::op::uniques()(state.lua_state(), a).as<std::unordered_set<int>>() == a);
    }

}

BOOST_AUTO_TEST_CASE(SetOpTblArr)
{
    std::set<int> lhs{1, 2, 3, 4};
    std::vector<short> rhs{3, 4, 5, 6};
    sol::table ltable = sol::make_object(state.lua_state(), sol::as_table(lhs));
    sol::table rtable = sol::make_object(state.lua_state(), sol::as_table(rhs));
    using haikan::impl::lua_to_stream;

    static_assert(haikan::impl::has_contains<sol::table, sol::table::const_iterator>::value, "");
    static_assert(haikan::impl::has_set_equal<sol::table, sol::table>::value, "");
    static_assert(haikan::impl::has_set_operation<sol::table, sol::table>::value, "");

    {
        auto const result = haikan::op::set_union()(state.lua_state(), ltable, rtable).as<sol::table>();
        std::ostringstream lua;
        lua_to_stream(lua, result, 0, false);
        BOOST_CHECK_EQUAL(lua.str(), "{1,2,3,4,5,6}");
    }

    {
        auto const result = haikan::op::intersection()(state.lua_state(), ltable, rtable).as<sol::table>();
        std::ostringstream lua;
        lua_to_stream(lua, result, 0, false);
        BOOST_CHECK_EQUAL(lua.str(), "{3,4}");
    }

    {
        auto const result = haikan::op::difference()(state.lua_state(), ltable, rtable).as<sol::table>();
        std::ostringstream lua;
        lua_to_stream(lua, result, 0, false);
        BOOST_CHECK_EQUAL(lua.str(), "{1,2}");
    }

    {
        auto const result = haikan::op::symmetric_difference()(state.lua_state(), ltable, rtable).as<sol::table>();
        std::ostringstream lua;
        lua_to_stream(lua, result, 0, false);
        BOOST_CHECK_EQUAL(lua.str(), "{1,2,5,6}");
    }
}


BOOST_AUTO_TEST_CASE(SetOpTblMap)
{
    #define TBL(s) state.script("return " s).get<sol::table>()

    sol::table ltable = TBL("{a=1, b=2, c=3}");
    sol::table rtable = TBL("{c=1, d=2}");
    using haikan::impl::lua_to_stream;


    {
        BOOST_CHECK((haikan::op::set_equal()(state.lua_state(), ltable, ltable).as<bool>()));
        BOOST_CHECK((haikan::op::set_equal()(state.lua_state(), rtable, rtable).as<bool>()));
        BOOST_CHECK(!(haikan::op::set_equal()(state.lua_state(), ltable, rtable).as<bool>()));
    }

    {
        auto const result = haikan::op::set_union()(state.lua_state(), ltable, rtable).as<sol::table>();
        BOOST_ASSERT(result != sol::nil);
        auto const expected = TBL("{a=1, b=2, c=3, d=2}");
        BOOST_CHECK((haikan::op::set_equal()(state.lua_state(), result, expected).as<bool>()));
    }

    {
        auto const result = haikan::op::intersection()(state.lua_state(), ltable, rtable).as<sol::table>();
        auto const expected = TBL("{c=3}");
        // auto log = HAIKAN_LOG(INFO);
        // lua_to_stream(log, result, 0, false);
        BOOST_CHECK((haikan::op::set_equal()(state.lua_state(), result, expected).as<bool>()));
    }

    {
        auto const result = haikan::op::difference()(state.lua_state(), ltable, rtable).as<sol::table>();
        auto const expected = TBL("{a=1,b=2}");
        BOOST_CHECK((haikan::op::set_equal()(state.lua_state(), result, expected).as<bool>()));
    }

    {
        auto const result = haikan::op::symmetric_difference()(state.lua_state(), ltable, rtable).as<sol::table>();
        auto const expected = TBL("{a=1,b=2,d=2}");
        HAIKAN_LOG(INFO) << result;
        BOOST_CHECK((haikan::op::set_equal()(state.lua_state(), result, expected).as<bool>()));
    }
}


BOOST_AUTO_TEST_CASE(ContTemplates)
{
    std::vector<int> const a{1, 2, 3, 4};
    std::vector<int> const b{3, 4, 5, 6};

    // lexicographical comparison provided by std::vector::operator<
    BOOST_CHECK(haikan::op::less()(state.lua_state(), a, b).as<bool>());
    BOOST_CHECK(!haikan::op::greater()(state.lua_state(), a, b).as<bool>());

    using haikan::impl::OperatorRegistry;
    using haikan::impl::OperatorTable;

    OperatorRegistry()
        .AddSignature(type<int>, template_list<std::vector, std::set>)
        .Connects(type<double>, template_list<std::vector, std::set>);
    haikan::impl::TypeRegistry{}.init(state);

    auto x = sol::make_object(state, a);
    auto y = sol::make_object(state, b);

    OperatorTable optbl = OperatorRegistry::get(x).value();

    sol::object result = optbl.apply(Keyword::Lt, x, y);
    BOOST_CHECK(result.as<bool>());
}

BOOST_AUTO_TEST_SUITE_END()
