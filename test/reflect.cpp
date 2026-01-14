#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/optional/optional_io.hpp>
#include <boost/describe.hpp>
#include <boost/json.hpp>

#include "haikan/reflect.hpp"
#include "haikan/impl/traits.hpp"


BOOST_AUTO_TEST_CASE(ReflectInt)
{
    static_assert(haikan::impl::has_default_reflect_init<int>::value    , "");
    static_assert(haikan::impl::has_default_reflect_solify<int>::value  , "");
    static_assert(haikan::impl::has_default_reflect_desolify<int>::value, "");

    using reflect_int = haikan::reflect<int>;
    BOOST_CHECK_EQUAL(reflect_int::init(), 0);

    sol::state state{};
    sol::object x = reflect_int::solify(42, state);
    BOOST_CHECK_EQUAL(reflect_int::desolify(x), 42);
}


enum class Foo {None, Lol = 42, Kek = 67 };
BOOST_DESCRIBE_ENUM(Foo, None, Lol, Kek)

BOOST_AUTO_TEST_CASE(ReflectEnum)
{
    static_assert(boost::describe::has_describe_enumerators<Foo>::value, "");
    static_assert(haikan::impl::has_default_reflect_solify<Foo>::value, "");

    using reflect_foo = haikan::reflect<Foo>;
    BOOST_CHECK(reflect_foo::init() == Foo::None);

    sol::state state{};
    sol::object x = reflect_foo::solify(Foo::Kek, state);
    BOOST_CHECK(reflect_foo::desolify(x) == Foo::Kek);

    using udenum = haikan::impl::user_data_enum<Foo>;
    BOOST_CHECK(state.script("return Foo()").get<udenum>().value() == Foo());
    BOOST_CHECK(state.script("return Foo(42)").get<udenum>().value() == Foo(42));
    BOOST_CHECK(state.script("return Foo('Kek')").get<udenum>().value() == Foo::Kek);
}


struct Lol
{
    Foo foo;
    int x;

    Lol() : foo{Foo::Lol}, x{42} {}

    Lol(Foo foo, int x) : foo{foo}, x{x} {}

    friend bool operator==(Lol const& l, Lol const& r)
    {
        return (l.foo == r.foo) && (l.x == r.x);
    }

    friend bool operator!=(Lol const& l, Lol const& r)
    {
        return !(l == r);
    }

    friend bool operator<(Lol const& l, Lol const& r)
    {
        return (l.foo < r.foo) && (l.x < r.x);
    }
};

BOOST_DESCRIBE_STRUCT(Lol, (), (foo, x))


BOOST_AUTO_TEST_CASE(ReflectClass)
{
    static_assert(boost::describe::has_describe_members<Lol>::value, "");
    static_assert(haikan::impl::has_default_reflect_solify<Lol>::value, "");

    using reflect_lol = haikan::reflect<Lol>;

    auto test_lol = Lol(Foo::Kek, 67);
    sol::state state{};
    sol::object x = reflect_lol::solify(Lol(Foo::Kek, 67), state);
    BOOST_CHECK(reflect_lol::desolify(x) == test_lol);
    // modify
    sol::table t = x;
    t["foo"] = haikan::as_enum(test_lol.foo = Foo::Lol);
    BOOST_CHECK(reflect_lol::desolify(x) == test_lol);
}

struct Kek : Lol
{
    double y;

    Kek() : Lol{}, y{3.14} {}

    Kek(Foo foo, int x, double y) : Lol{foo, x}, y{y} {}

    friend bool operator==(Kek const& l, Kek const& r)
    {
        return (static_cast<Lol>(l) == static_cast<Lol>(r)) && (l.y == r.y);
    }

    friend bool operator!=(Kek const& l, Kek const& r)
    {
        return !(l == r);
    }

    friend bool operator<(Kek const& l, Kek const& r)
    {
        return (static_cast<Lol>(l) < static_cast<Lol>(r)) && (l.y < r.y);
    }
};

BOOST_DESCRIBE_STRUCT(Kek, (Lol), (y))

// customizations should be defined before
// haikan::reflect<Kek> instantiation
template <>
struct ::haikan::custom_reflect<Kek>
{
    static boost::optional<Kek> init()
    {
        return Kek{Foo::Kek, 67, 13};
    }
};


BOOST_AUTO_TEST_CASE(ReflectSubclass)
{
    static_assert(boost::describe::has_describe_members<Kek>::value, "");
    static_assert(haikan::impl::has_custom_reflect_init<Kek>::value, "");
    static_assert(haikan::impl::has_default_reflect_solify<Kek>::value, "");

    using reflect_kek = haikan::reflect<Kek>;

    auto test_kek = reflect_kek::init().value();
    BOOST_CHECK(test_kek == Kek(Foo::Kek, 67, 13));
    sol::state state{};
    sol::object x = reflect_kek::solify(test_kek, state);
    BOOST_CHECK(reflect_kek::desolify(x) == test_kek);

    // modify
    sol::table t = x;
    t["foo"] = haikan::as_enum(test_kek.foo = Foo::Lol);
    t["y"] = test_kek.y = 0.1f;
    BOOST_CHECK(reflect_kek::desolify(x) == test_kek);
}

BOOST_AUTO_TEST_CASE(ReflectVector)
{

    using reflect_kekvec = haikan::reflect<std::vector<Kek>>;

    auto test_kekvec = reflect_kekvec::init().value();
    test_kekvec.push_back(Kek(Foo::Kek, 67, 13));
    test_kekvec.push_back(Kek(Foo::Lol, 42, 11));

    sol::state L{};
    sol::object x = reflect_kekvec::solify(test_kekvec, L);
    BOOST_CHECK(reflect_kekvec::desolify(x) == test_kekvec);

    // modify
    sol::table t = x;
    t[1]["y"] = test_kekvec[0].y = 0.1f;
    t[2]["foo"] = haikan::as_enum(test_kekvec[1].foo = Foo::Lol);
    BOOST_CHECK(reflect_kekvec::desolify(x) == test_kekvec);

    // modify from Lua
    L.set("x", x);
    L.open_libraries();
    L.script(R"(
        x[1].foo = 314
    )");
    BOOST_CHECK(t[1]["foo"] == haikan::as_enum(Foo(314)));
}


BOOST_AUTO_TEST_CASE(ReflectMap)
{

    using reflect_kekmap = haikan::reflect<std::map<std::string, Kek>>;

    auto test_kekmap = reflect_kekmap::init().value();
    test_kekmap["one"] = Kek(Foo::Kek, 67, 13);
    test_kekmap["two"] = Kek(Foo::Lol, 42, 11);

    sol::state state{};

    auto copy = test_kekmap;
    sol::object x = reflect_kekmap::solify(std::move(copy), state);
    BOOST_CHECK(reflect_kekmap::desolify(x) == test_kekmap);

    state.open_libraries();
    state.set("x", x);

    // new item
    state.script(R"(x.tre = Kek:new())");

    // modify
    sol::userdata t = x;
    t["one"]["y"]   = (test_kekmap["one"].y   = 0.1f);
    t["two"]["foo"] = haikan::as_enum(test_kekmap["two"].foo = Foo::None);

    auto const desolified = reflect_kekmap::desolify(x).value();

    BOOST_CHECK(desolified.count("tre"));
    // Looks like an issue with sol2 usertype for assoc conts -
    // elements of map are not modifiable and no error thrown
    BOOST_WARN(desolified.at("one") == test_kekmap["one"]);
    BOOST_WARN(desolified.at("two") == test_kekmap["two"]);

    auto out = state.script(R"(
        out = ""
        for k in pairs(x) do
            out = out .. ":" .. k
        end
        return out
    )").get<std::string>();

    BOOST_CHECK(out.find(":one") != std::string::npos);
    BOOST_CHECK(out.find(":two") != std::string::npos);
    BOOST_CHECK(out.find(":tre") != std::string::npos);
}


BOOST_AUTO_TEST_CASE(DesolifyInvalidObject)
{
    using reflect_kek = haikan::reflect<Kek>;
    sol::state state{};
    sol::table empty = state.create_table();
    BOOST_CHECK(not reflect_kek::desolify(empty).has_value());
}

BOOST_AUTO_TEST_CASE(ReflectReference)
{
    static_assert(haikan::impl::has_default_reflect_solify<std::reference_wrapper<Kek>>::value, "");

    using reflect_kek = haikan::reflect<std::reference_wrapper<Kek>>;
    Kek test_kek(Foo::Kek, 67, 13);

    sol::state state{};
    sol::object ref_handle = reflect_kek::solify(std::ref(test_kek), state);
    BOOST_CHECK(reflect_kek::desolify(ref_handle).value().get() == test_kek);

    // modify source
    test_kek.foo = Foo::Lol;
    test_kek.y = 0.1f;
    BOOST_CHECK(reflect_kek::desolify(ref_handle).value().get() == test_kek);

    ref_handle.as<sol::table>()["foo"] =  Foo::None;
    ref_handle.as<sol::table>()["y"] = 3.14;
    BOOST_CHECK(test_kek.foo == Foo::None);

    BOOST_CHECK(test_kek.y == 3.14);
    ref_handle.as<sol::table>()["foo"] = Foo(142);
    ref_handle.as<sol::table>()["y"] = 142;
    BOOST_CHECK(test_kek.foo == Foo(142));
    BOOST_CHECK(test_kek.y == 142);

    state["test_kek"] = ref_handle;
    state.open_libraries();
    auto out = state.script(R"(
        out = ""
        for k in pairs(test_kek) do
            out = out .. ":" .. k
        end
        return out
    )").get<std::string>();
    BOOST_CHECK_EQUAL(out, ":foo:x:y");


    BOOST_CHECK_NO_THROW(state.script("test_kek.foo = Foo()"));
    BOOST_CHECK_NO_THROW(state.script("test_kek.foo = Foo(42)"));
    BOOST_CHECK_NO_THROW(state.script("test_kek.foo = 42"));
    BOOST_CHECK_NO_THROW(state.script("test_kek.foo = 'Lol'"));
    BOOST_CHECK_EQUAL(state.script("return tostring(test_kek.foo)").get<std::string>(), "Lol");
    BOOST_CHECK_EQUAL(state.script("return test_kek.foo:str()").get<std::string>(), "Lol");
    BOOST_CHECK_EQUAL(state.script("return test_kek.foo:num()").get<int>(), 42);
    BOOST_CHECK(state.script("return test_kek.foo == Foo('Lol')").get<bool>());
    BOOST_CHECK(state.script("return test_kek.foo == Foo(42)").get<bool>());
    BOOST_CHECK(state.script("return test_kek.foo ~= 'Lol'").get<bool>());
    BOOST_CHECK(test_kek.foo == Foo::Lol);

    BOOST_CHECK_NO_THROW(ref_handle.as<sol::table>()["foo"] = "None");
    BOOST_CHECK_NO_THROW(state.script("test_kek.foo = 'None'"));

    BOOST_CHECK_THROW(ref_handle.as<sol::table>()["foo"] = "NoSuchEnum", sol::error);
    BOOST_CHECK_THROW(state.script("test_kek.foo = 'NoSuchEnum'"), sol::error);
}
