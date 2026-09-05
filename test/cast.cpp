#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/optional/optional_io.hpp>
#include <boost/describe.hpp>
#include <boost/json.hpp>

#include "haikan/cast.hpp"


namespace utf = boost::unit_test;
using V = boost::json::value;
using L = boost::json::array;


static_assert((!boost::mp11::mp_valid<haikan::impl::has_cast, void>::value)     , "");
static_assert((boost::mp11::mp_valid<haikan::impl::has_cast, void, void>::value), "");

static_assert( (haikan::impl::has_cast<int, int>::value), "");
static_assert( (haikan::impl::has_cast<int, int&>::value), "");
static_assert( (haikan::impl::has_cast<int, int&&>::value), "");
static_assert( (haikan::impl::has_cast<int, int const&>::value), "");

static_assert( (haikan::impl::has_cast<double, int>::value), "");
static_assert( (haikan::impl::has_cast<int, double>::value), "");
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
    BOOST_CHECK((haikan::switch_cast<int>()(42)) == 42);
    BOOST_CHECK((haikan::switch_cast<double>()(0.5)) == 0.5);
    BOOST_CHECK((haikan::switch_cast<double>()(0.1f)) == double(0.1f));
    BOOST_CHECK((haikan::switch_cast<float>()(0.1)) == 0.1f);
    BOOST_CHECK((!haikan::switch_cast<std::string>()(0.5)));
}


BOOST_AUTO_TEST_CASE(MonadicCastEnum)
{
    BOOST_ASSERT((haikan::impl::has_default_cast<int, Foo>::value));
    BOOST_ASSERT((haikan::impl::has_default_cast<Foo, int>::value));
    BOOST_CHECK_EQUAL((haikan::switch_cast<int>()(Foo::Lol)), int(Foo::Lol));
    BOOST_CHECK_EQUAL((haikan::switch_cast<int>()(Foo::Kek)), int(Foo::Kek));
    BOOST_CHECK_EQUAL((haikan::switch_cast<Foo>()(int(Foo::Lol))), Foo::Lol);
    BOOST_CHECK_EQUAL((haikan::switch_cast<Foo>()(int(Foo::Kek))), Foo::Kek);
}


BOOST_AUTO_TEST_CASE(MonadicCastChain)
{
    BOOST_CHECK_EQUAL(Foo::Lol, haikan::switch_cast<Foo>()
        (0.5)
        ("Foo")
        (boost::none)
        (Foo::Lol)
        (Foo::Kek)
    );

    BOOST_CHECK_EQUAL(Foo::Lol, haikan::switch_cast<Foo>()
        (Foo::Lol)
        (Foo::Kek)
        (0.5)
        ("Foo")
        (boost::none)
    );

    BOOST_CHECK_EQUAL(Foo::Lol, haikan::switch_cast<Foo>()
        (0.5)
        ("Foo")
        (Foo::Lol)
        (Foo::Kek)
        (boost::none)
    );
}

BOOST_AUTO_TEST_CASE(MonadicCastJson)
{
    using json = boost::json::value;

    static_assert((haikan::impl::has_default_cast<json, Foo>::value), "");
    static_assert((haikan::impl::has_default_cast<Foo, json>::value), "");

    BOOST_CHECK_EQUAL(Foo::Lol, (haikan::switch_cast<Foo>()(json{"Lol"})));
    BOOST_CHECK_EQUAL(Foo::Kek, (haikan::switch_cast<Foo>()(json{"Kek"})));

    BOOST_CHECK_EQUAL(json{"Lol"}, (haikan::switch_cast<json>()(Foo::Lol)));
    BOOST_CHECK_EQUAL(json{"Kek"}, (haikan::switch_cast<json>()(Foo::Kek)));
}
