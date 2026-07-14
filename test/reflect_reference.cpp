#include <boost/test/unit_test.hpp>

#include <functional>
#include <string>

#include "haikan/impl/traits.hpp"
#include "haikan/reflection_registry.hpp"
#include "fixture/reflect_types.hpp"

BOOST_AUTO_TEST_CASE(ReflectReference)
{
    static_assert(haikan::impl::has_default_reflect_init<std::reference_wrapper<Kek>>::value, "");

    Kek test_kek(Foo::Kek, 67, 13);

    sol::state state{};
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<std::reference_wrapper<Kek>>);
    haikan::ReflectionRegistry::init(state);

    sol::object ref_handle = sol::make_object(state, std::ref(test_kek));
    BOOST_CHECK(ref_handle.as<std::reference_wrapper<Kek>>() == test_kek);

    test_kek.foo = Foo::Lol;
    test_kek.y = 0.1f;
    BOOST_CHECK(ref_handle.as<std::reference_wrapper<Kek>>() == test_kek);

    ref_handle.as<sol::table>()["foo"] = Foo::None;
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
