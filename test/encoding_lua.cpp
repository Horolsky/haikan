#include <boost/test/unit_test.hpp>
#include <boost/describe.hpp>

#include "haikan/reflect.hpp"
#include "haikan/impl/encoding_lua.hpp"
#include "haikan/reflection_registry.hpp"


struct EncodingPayload
{
    int value;

    friend bool operator==(EncodingPayload const& lhs, EncodingPayload const& rhs)
    {
        return lhs.value == rhs.value;
    }
};

BOOST_DESCRIBE_STRUCT(EncodingPayload, (), (value))

using haikan::impl::EncodingLua;
using haikan::impl::Keyword;

namespace
{

struct EncodingLuaSuite
{
    sol::state state;

    sol::object object(int value)
    {
        return sol::make_object(state, value);
    }
};

} // namespace

BOOST_FIXTURE_TEST_SUITE(EncodingLuaTests, EncodingLuaSuite)

BOOST_AUTO_TEST_CASE(ClassifiesStringTokens)
{
    BOOST_CHECK(EncodingLua::is_preproc_token(sol::make_object(state, "$[value]")));
    BOOST_CHECK(!EncodingLua::is_preproc_token(sol::make_object(state, "$value")));
    BOOST_CHECK(EncodingLua::is_link_token(sol::make_object(state, "$value")));
    BOOST_CHECK(!EncodingLua::is_link_token(sol::make_object(state, "$[value]")));
    BOOST_CHECK(!EncodingLua::is_link_token(object(1)));
}

BOOST_AUTO_TEST_CASE(ConstructsLiteralAndTokens)
{
    EncodingLua literal(object(7));
    EncodingLua preproc(sol::make_object(state, "$[value]"));
    EncodingLua link(sol::make_object(state, "$value"));

    BOOST_CHECK_EQUAL(literal.size(), 1U);
    BOOST_CHECK(literal.head() == Keyword::_Literal);
    BOOST_CHECK(preproc.head() == Keyword::PreProc);
    BOOST_CHECK(link.head() == Keyword::Link);
}

BOOST_AUTO_TEST_CASE(SlicesAndTraversesTrees)
{
    EncodingLua tree;
    tree.push_back(Keyword::Add, 0, object(0));
    tree.push_back(Keyword::Mul, 1, object(1));
    tree.push_back(Keyword::_Literal, 2, object(2));
    tree.push_back(Keyword::_Literal, 1, object(3));

    EncodingLua first = tree.child(0);
    EncodingLua last = tree.child(-1);

    BOOST_CHECK_EQUAL(tree.arity(), 2U);
    BOOST_CHECK_EQUAL(first.size(), 2U);
    BOOST_CHECK(first.head() == Keyword::Mul);
    BOOST_CHECK_EQUAL(last.size(), 1U);
    BOOST_CHECK_EQUAL(last.data.front().as<int>(), 3);
    BOOST_CHECK_EQUAL(tree.children().size(), 2U);
    BOOST_CHECK(tree.slice(4, 1).size() == 0U);
}

BOOST_AUTO_TEST_CASE(AppendsAtRootDepth)
{
    EncodingLua root(object(0));
    EncodingLua tail;
    tail.push_back(Keyword::Mul, 0, object(1));
    tail.push_back(Keyword::_Literal, 1, object(2));

    root.append_to_root(std::move(tail));

    BOOST_CHECK_EQUAL(root.size(), 3U);
    BOOST_CHECK_EQUAL(root.depth[1], 1U);
    BOOST_CHECK_EQUAL(root.depth[2], 2U);
}

BOOST_AUTO_TEST_CASE(EmptyAndMissingChildrenAreSafe)
{
    EncodingLua empty;

    BOOST_CHECK_EQUAL(empty.child_idx(0), 0U);
    BOOST_CHECK(empty.child(0).size() == 0U);
    BOOST_CHECK(empty.subtree(0).size() == 0U);
}

BOOST_AUTO_TEST_CASE(ToObjectPreservesPlainPayload)
{
    EncodingLua encoding;
    encoding.push_back(Keyword::Add, 0, object(17));

    sol::table result = encoding.to_object(state);
    sol::table data = result["data"];

    BOOST_CHECK_EQUAL(data.get<int>(1), 17);
}

BOOST_AUTO_TEST_CASE(ToObjectSolifiesRegisteredUserdata)
{
    EncodingPayload expected{42};
    sol::object payload = sol::make_object(state, expected);
    state["payload"] = payload;

    BOOST_CHECK_THROW(state.script("return payload.value == 42").get<bool>(), sol::error);
    haikan::ReflectionRegistry::insert_auto(haikan::impl::type<EncodingPayload>);
    haikan::ReflectionRegistry::init(state);
    BOOST_REQUIRE(state.script("return payload.value == 42").get<bool>());

    EncodingLua encoding;
    encoding.push_back(Keyword::Add, 0, payload);

    sol::table result = encoding.to_object(state);
    sol::table data = result["data"];
    sol::object as_object = data[1];

    BOOST_CHECK(as_object.as<EncodingPayload>() == expected);
    state["as_object"] = as_object;

    state.open_libraries();
    BOOST_CHECK(state.script("return tostring(payload) == tostring(as_object)").get<bool>());

}

BOOST_AUTO_TEST_SUITE_END()
