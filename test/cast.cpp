#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/optional/optional_io.hpp>
#include <boost/describe.hpp>
#include <boost/json.hpp>

#include "haikan/monadic_cast.hpp"

namespace utf = boost::unit_test;
using V = boost::json::value;
using L = boost::json::array;

static_assert((!boost::mp11::mp_valid<haikan::impl::has_cast, void>::value)     , "");
static_assert((boost::mp11::mp_valid<haikan::impl::has_cast, void, void>::value), "");

static_assert( (haikan::impl::has_cast<int, int>::value), "");
static_assert( (haikan::impl::has_cast<int, double>::value), "");
static_assert( (haikan::impl::has_cast<double, int>::value), "");
static_assert((!haikan::impl::has_cast<double, std::string>::value), "");
static_assert((!haikan::impl::has_cast<std::string, double>::value), "");


enum class Foo { Lol = 42, Kek = 67 };
BOOST_DESCRIBE_ENUM(Foo, Lol, Kek)

std::ostream& operator<<(std::ostream& os, Foo const& foo)
{
    switch (foo)
    {
    case Foo::Lol: { os << "Lol"; break; }
    case Foo::Kek: { os << "Kek"; break; }
    default:
        os << "Foo(" << static_cast<std::underlying_type_t<Foo>>(foo) << ")";
        break;
    }
    return os;
}


BOOST_AUTO_TEST_CASE(MonadicCastNum)
{
    BOOST_CHECK((haikan::monadic_cast<int>().try_cast(42)) == 42);
    BOOST_CHECK((haikan::monadic_cast<double>().try_cast(0.5)) == 0.5);
    BOOST_CHECK((haikan::monadic_cast<double>().try_cast(0.1f)) == double(0.1f));
    BOOST_CHECK((haikan::monadic_cast<float>().try_cast(0.1)) == 0.1f);
    BOOST_CHECK((!haikan::monadic_cast<std::string>().try_cast(0.5)));
}


BOOST_AUTO_TEST_CASE(MonadicCastEnum)
{
    BOOST_ASSERT((haikan::impl::has_default_cast<Foo, int>::value));
    BOOST_ASSERT((haikan::impl::has_default_cast<int, Foo>::value));
    BOOST_CHECK_EQUAL((haikan::monadic_cast<int>().try_cast(Foo::Lol)), int(Foo::Lol));
    BOOST_CHECK_EQUAL((haikan::monadic_cast<int>().try_cast(Foo::Kek)), int(Foo::Kek));
    BOOST_CHECK_EQUAL((haikan::monadic_cast<Foo>().try_cast(int(Foo::Lol))), Foo::Lol);
    BOOST_CHECK_EQUAL((haikan::monadic_cast<Foo>().try_cast(int(Foo::Kek))), Foo::Kek);
}


BOOST_AUTO_TEST_CASE(MonadicCastChain)
{
    BOOST_CHECK_EQUAL(Foo::Lol, haikan::monadic_cast<Foo>()
        .try_cast(0.5)
        .try_cast("Foo")
        .try_cast(boost::none)
        .try_cast(Foo::Lol)
        .try_cast(Foo::Kek)
    );

    BOOST_CHECK_EQUAL(Foo::Lol, haikan::monadic_cast<Foo>()
        .try_cast(Foo::Lol)
        .try_cast(Foo::Kek)
        .try_cast(0.5)
        .try_cast("Foo")
        .try_cast(boost::none)
    );

    BOOST_CHECK_EQUAL(Foo::Lol, haikan::monadic_cast<Foo>()
        .try_cast(0.5)
        .try_cast("Foo")
        .try_cast(Foo::Lol)
        .try_cast(Foo::Kek)
        .try_cast(boost::none)
    );
}

BOOST_AUTO_TEST_CASE(MonadicCastJson)
{
    using json = boost::json::value;

    static_assert((haikan::impl::has_default_cast<Foo, json>::value), "");
    static_assert((haikan::impl::has_default_cast<json, Foo>::value), "");

    BOOST_CHECK_EQUAL(Foo::Lol, (haikan::monadic_cast<Foo>().try_cast(json{"Lol"})));
    BOOST_CHECK_EQUAL(Foo::Kek, (haikan::monadic_cast<Foo>().try_cast(json{"Kek"})));

    BOOST_CHECK_EQUAL(json{"Lol"}, (haikan::monadic_cast<json>().try_cast(Foo::Lol)));
    BOOST_CHECK_EQUAL(json{"Kek"}, (haikan::monadic_cast<json>().try_cast(Foo::Kek)));
}
