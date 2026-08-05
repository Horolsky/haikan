#include <boost/test/unit_test.hpp>

#include <boost/optional/optional_io.hpp>
#include <cctype>
#include <cstdint>
#include <string>

#include "haikan/impl/traits.hpp"
#include "haikan/reflection_registry.hpp"
#include "haikan/reflection_meta.hpp"
#include "fixture/reflect_types.hpp"

struct BinaryReflectPayload
{
    std::uint32_t value;

    friend bool operator==(BinaryReflectPayload const& lhs, BinaryReflectPayload const& rhs)
    {
        return lhs.value == rhs.value;
    }
};

BOOST_DESCRIBE_STRUCT(BinaryReflectPayload, (), (value))

BOOST_AUTO_TEST_CASE(ReflectInt)
{
    static_assert(haikan::impl::has_default_reflect_init<int>::value, "");
    static_assert(haikan::impl::has_default_reflect_init<int>::value, "");
    static_assert(haikan::impl::has_default_reflect_utype<int>::value, "");

    using reflect_int = haikan::reflect<int>;
    BOOST_CHECK_EQUAL(reflect_int::init(), 0);
}

BOOST_AUTO_TEST_CASE(ReflectEnum)
{
    static_assert(boost::describe::has_describe_enumerators<Foo>::value, "");
    static_assert(haikan::impl::has_default_reflect_init<Foo>::value, "");
    static_assert(haikan::impl::has_default_reflect_utype<Foo>::value, "");

    using reflect_foo = haikan::reflect<Foo>;
    BOOST_CHECK(reflect_foo::init() == Foo::None);

    sol::state state{};
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Foo>);
    haikan::ReflectionRegistry::init(state);

    sol::object x = sol::make_object(state, Foo::Kek);
    BOOST_CHECK(x.as<Foo>() == Foo::Kek);

    using udenum = haikan::impl::user_data_enum<Foo>;
    BOOST_CHECK(state.script("return Foo()").get<udenum>().value() == Foo());
    BOOST_CHECK(state.script("return Foo(42)").get<udenum>().value() == Foo(42));
    BOOST_CHECK(state.script("return Foo('Kek')").get<udenum>().value() == Foo::Kek);
}

BOOST_AUTO_TEST_CASE(ReflectClass)
{
    static_assert(boost::describe::has_describe_members<Lol>::value, "");
    static_assert(haikan::impl::has_default_reflect_init<Lol>::value, "");

    auto test_lol = Lol(Foo::Kek, 67);
    sol::state state{};
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Lol>);
    haikan::ReflectionRegistry::init(state);

    sol::object x = sol::make_object(state, Lol(Foo::Kek, 67));
    BOOST_CHECK(x.as<Lol>() == test_lol);

    sol::table t = x;
    t["foo"] = haikan::as_enum(test_lol.foo = Foo::Lol);
    BOOST_CHECK(x.as<Lol>() == test_lol);
}

BOOST_AUTO_TEST_CASE(ReflectDescribedStructMembersAreAccessibleInLua)
{
    sol::state state{};
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Lol>);
    haikan::ReflectionRegistry::init(state);

    sol::object value = sol::make_object(state, Lol{Foo::Kek, 42});
    state["value"] = value;

    BOOST_CHECK_EQUAL(state.script("return value.x").get<int>(), 42);
    BOOST_CHECK_EQUAL(state.script("return value.foo:num()").get<int>(), 67);

    state.script(R"(
        value.x = 43
        value.foo = "Lol"
    )");

    BOOST_CHECK(value.as<Lol>() == Lol(Foo::Lol, 43));
}

BOOST_AUTO_TEST_CASE(ReflectSubclass)
{
    static_assert(boost::describe::has_describe_members<Kek>::value, "");
    static_assert(haikan::impl::has_custom_reflect_init<Kek>::value, "");

    using reflect_kek = haikan::reflect<Kek>;

    auto test_kek = reflect_kek::init().value();
    BOOST_CHECK(test_kek == Kek(Foo::Kek, 67, 13));
    sol::state state{};
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Kek>);
    haikan::ReflectionRegistry::init(state);
    sol::object x = sol::make_object(state, test_kek);
    BOOST_CHECK(x.as<Kek>() == test_kek);

    sol::table t = x;
    t["foo"] = haikan::as_enum(test_kek.foo = Foo::Lol);
    t["y"] = test_kek.y = 0.1f;
    BOOST_CHECK(x.as<Kek>() == test_kek);
}

namespace haikan {
BOOST_DESCRIBE_STRUCT(ReflectionMeta, (), (
    type_name, type_index_hash, wrapper_type_index_hash, members, serialize, deserialize
))
}

BOOST_AUTO_TEST_CASE(ReflectDescribedStructSerializationUsesUtypeMetadata)
{
    using haikan::ReflectionMeta;
    sol::state state{};
    // static_assert(boost::describe::has_describe_members<ReflectionMeta>::value, "");

    // haikan::ReflectionRegistry::insert_auto(haikan::impl::type<ReflectionMeta>);
    // haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Foo>);
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<Lol>);
    haikan::ReflectionRegistry::init(state);
    sol::object haikan_table = state["haikan"];
    sol::object utypes_table = state["haikan"]["utypes"];
    sol::object lol_metadata = state["haikan"]["utypes"][typeid(Lol).hash_code()];
    // sol::object foo_metadata = state["haikan"]["utypes"][typeid(Foo).hash_code()];
    BOOST_REQUIRE(haikan_table.get_type() == sol::type::table);
    BOOST_REQUIRE(utypes_table.get_type() == sol::type::table);
    BOOST_REQUIRE(lol_metadata.is<haikan::ReflectionMeta>());


    Lol const input{Foo::Kek, 67};
    sol::object serialized = haikan::reflect<Lol>::serialize(input, state);

    BOOST_REQUIRE(serialized.is<sol::table>());
    sol::table tbl = serialized;
    BOOST_CHECK_EQUAL(tbl["x"].get<int>(), 67);
    auto serialized_foo = haikan::reflect<Foo>::deserialize(tbl["foo"]);
    BOOST_REQUIRE(serialized_foo);
    BOOST_CHECK(*serialized_foo == Foo::Kek);

    tbl["foo"] = "Lol";
    tbl["x"] = 42;

    auto deserialized = haikan::reflect<Lol>::deserialize(tbl);
    //  utypes_table
    BOOST_REQUIRE(deserialized);
    BOOST_CHECK(*deserialized == Lol(Foo::Lol, 42));
}

BOOST_AUTO_TEST_CASE(ReflectDescribedStructSerializationFallsBackToHexdumpWithoutMetadata)
{
    sol::state state{};

    BinaryReflectPayload const input{0x1234ABCD};
    sol::object serialized = haikan::reflect<BinaryReflectPayload>::serialize(input, state);

    BOOST_REQUIRE(serialized.is<std::string>());
    std::string const hexdump = serialized.as<std::string>();
    BOOST_CHECK_EQUAL(hexdump.size(), sizeof(BinaryReflectPayload) * 2);
    for (char c: hexdump)
    {
        BOOST_CHECK(std::isxdigit(static_cast<unsigned char>(c)));
    }

    auto deserialized = haikan::reflect<BinaryReflectPayload>::deserialize(serialized);
    BOOST_REQUIRE(deserialized);
    BOOST_CHECK(*deserialized == input);
}
