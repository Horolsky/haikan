#include "haikan/expression_lua.hpp"

#include <cstdint>
#include <sstream>
#include <set>
#include <string>

#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "haikan/impl/operator_handler.hpp"
#include "haikan/impl/operator_table.hpp"
#include "haikan/impl/operators.hpp"
#include "haikan/reflection_registry.hpp"

// injecting operator overloads to sol::object
struct GenericOp : public sol::object
{
    using sol::object::object;
    using sol::object::operator=;
    using sol::object::operator bool;

    friend GenericOp operator+(GenericOp const& lhs, GenericOp const&)
    {
        return sol::make_object(lhs.lua_state(), "success!");
    }

    GenericOp(sol::object const& o) : sol::object(o) {}
    GenericOp(sol::object && o) : sol::object(std::move(o)) {}
};

template<>
template<>
decltype(auto) sol::basic_object_base<sol::reference>::as<GenericOp>() const {
    return GenericOp{*this};
}

template<>
template<>
bool sol::basic_object_base<sol::reference>::is<GenericOp>() const {
        return true;
}

namespace
{

struct OperatorHandlerSuite
{
    sol::state state;
};

} // namespace

using haikan::error;
using haikan::impl::OperatorHandler;
using haikan::impl::Keyword;
using haikan::impl::type;

BOOST_FIXTURE_TEST_SUITE(ExpressionLuaTests, OperatorHandlerSuite)

BOOST_AUTO_TEST_CASE(PrimitiveCtors)
{

    struct X
    {
        std::string msg;

        X(std::string msg) : msg{msg} {}

        std::string operator+(X const& rhs)
        {
            return msg + " + " + rhs.msg;
        }
        std::string operator*(X const& rhs)
        {
            return msg + " * " + rhs.msg;
        }
    };

    struct Y : public X
    {
        Y(std::string msg) : X(msg) {}

        std::string operator%(X const& rhs)
        {
            return msg + " % " + rhs.msg;
        }
    };

    {
        OperatorHandler op {type<X>, state};
        sol::object add = op.apply(Keyword::Add, X{"lol"}, X{"kek"});
        sol::object mul = op.apply(Keyword::Mul, X{"lol"}, X{"kek"});
        sol::object mod = op.apply(Keyword::Mod, X{"lol"}, X{"kek"});

        BOOST_CHECK_EQUAL(add.as<std::string>(), "lol + kek");
        BOOST_CHECK_EQUAL(mul.as<std::string>(), "lol * kek");
        BOOST_CHECK_EQUAL(mod.as<error>().what, "operator not supported");
    }

    {
        OperatorHandler op {type<Y>, state};
        sol::object modyy = op.apply(Keyword::Mod, Y{"lol"}, Y{"kek"});
        sol::object modxy = op.apply(Keyword::Mod, X{"lol"}, Y{"kek"});
        sol::object modyx = op.apply(Keyword::Mod, Y{"lol"}, X{"kek"});
        BOOST_CHECK_EQUAL(modyy.as<std::string>(), "lol % kek");
        BOOST_CHECK_EQUAL(modxy.as<error>().what, "lua: error: stack index -1, expected userdata, received userdata");
        BOOST_CHECK_EQUAL(modyx.as<error>().what, "lua: error: stack index -1, expected userdata, received userdata");
    }

}

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
    haikan::ReflectionRegistry::insert_auto(type<X>);
    haikan::ReflectionRegistry::init(state);

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


BOOST_AUTO_TEST_CASE(GenericOpTest)
{
    state.open_libraries();
    {
        sol::object x = sol::make_object(state, 42);
        BOOST_CHECK_EQUAL(x.as<GenericOp>().as<int>(), 42);
    }
    {
        OperatorHandler op {type<GenericOp>, state};
        sol::object result = op.apply(Keyword::Add, "lol", "kek");
        BOOST_CHECK_EQUAL(result.as<std::string>(), "success!");
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

    OperatorRegistry(state).insert(type<X>);

    state.open_libraries();

    sol::table lhs2rhs = state["haikan"]["operators"]["lhs2rhs"];
    sol::table rhs2lhs = state["haikan"]["operators"]["rhs2lhs"];

    OperatorTable x2d_l = lhs2rhs[typeid(X).hash_code()][typeid(double).hash_code()].get<OperatorTable>();
    OperatorTable x2d_r = rhs2lhs[typeid(double).hash_code()][typeid(X).hash_code()].get<OperatorTable>();

    OperatorTable d2x_l = lhs2rhs[typeid(double).hash_code()][typeid(X).hash_code()].get<OperatorTable>();
    OperatorTable d2x_r = rhs2lhs[typeid(X).hash_code()][typeid(double).hash_code()].get<OperatorTable>();

    sol::object x = sol::make_object(state.lua_state(), X{42});
    sol::object y = sol::make_object(state.lua_state(), 11.0);

    {
        sol::object result = x2d_l.apply(Keyword::Add, x, y);
        BOOST_CHECK_EQUAL(result.as<X>().val, 53);
    }

    {
        sol::object result = x2d_r.apply(Keyword::Sub, x, y);
        BOOST_CHECK_EQUAL(result.as<X>().val, 31);
    }

    {
        sol::object result = d2x_l.apply(Keyword::Add, y, x);
        BOOST_CHECK_EQUAL(result.as<X>().val, 53);
    }

    {
        sol::object result = d2x_r.apply(Keyword::Sub, y, x);
        BOOST_CHECK_EQUAL(result.as<X>().val, -31);
    }

    {
        sol::object result = x2d_l.apply(Keyword::BitNot, x);
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

}

BOOST_AUTO_TEST_SUITE_END()
