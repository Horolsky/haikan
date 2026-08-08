#include "haikan/expression_lua.hpp"

#include <cstdint>
#include <sstream>
#include <string>

#include <boost/json.hpp>
#include <boost/test/unit_test.hpp>

#include "haikan/impl/operator_handler.hpp"
#include "haikan/reflection_registry.hpp"

namespace
{

struct OperatorHandlerSuite
{
    sol::state state;
};

} // namespace

using haikan::impl::OperatorHandler;
using haikan::impl::Keyword;
using haikan::impl::ErrorObject;
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
        BOOST_CHECK_EQUAL(mod.as<ErrorObject>().what, "operator not supported");
    }

    {
        OperatorHandler op {type<Y>, state};
        sol::object modyy = op.apply(Keyword::Mod, Y{"lol"}, Y{"kek"});
        sol::object modxy = op.apply(Keyword::Mod, X{"lol"}, Y{"kek"});
        sol::object modyx = op.apply(Keyword::Mod, Y{"lol"}, X{"kek"});
        BOOST_CHECK_EQUAL(modyy.as<std::string>(), "lol % kek");
        BOOST_CHECK_EQUAL(modxy.as<ErrorObject>().what, "lua: error: stack index -1, expected userdata, received userdata");
        BOOST_CHECK_EQUAL(modyx.as<ErrorObject>().what, "lua: error: stack index -1, expected userdata, received userdata");
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
        BOOST_CHECK_EQUAL(error.as<haikan::impl::ErrorObject>().what, "operator not supported");
    }
    {
        sol::object error = state.script("return X('lol')");
        BOOST_CHECK_EQUAL(error.as<haikan::impl::ErrorObject>().what, "invalid ctor argument");
    }
}



BOOST_AUTO_TEST_SUITE_END()
