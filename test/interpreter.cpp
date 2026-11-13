/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/variant2.hpp>


#include "haikan/keywords.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/operator_table.hpp"
#include "haikan/impl/type_registry.hpp"
#include "haikan/impl/interpreter.hpp"

namespace utf = boost::unit_test;
using namespace haikan;
using namespace haikan::impl;

// For ordinary complex parameters:
// Enter → EvalParam → ResumeFrame → ExecuteResolved → Return

/*
[1,2,3,4.25E1] -> (Fold(Add) & Size | Div)

           ┌── Add $ [1,2] = 3
           ├── Add $ [3,3] = 6
           ├── Add $ [6,4.25E1] = 4.85E1
        ┌── Fold(Add) $ [1,2,3,4.25E1] = 4.85E1
        ├── Size $ [1,2,3,4.25E1] = 4
     ┌── Fold(Add) & Size $ [1,2,3,4.25E1] = [4.85E1,4]
     ├── Div $ [4.85E1,4] = 1.2125E1
  □  (Fold(Add) & Size) | Div $ [1,2,3,4.25E1] = 1.2125E1

*/

using Vr = std::vector<double>;
using Vi = std::vector<std::int64_t>;
using Vb = std::list<bool>;
struct TestEvalSample
{
    Expression expr;
    Expression expected;

    friend std::ostream& operator<<(std::ostream& os, TestEvalSample const& sample)
    {
        return os << "{expr: " << sample.expr.prettify()
                  << ", expected: " << sample.expected.prettify() << "}";
    }

};

std::vector<TestEvalSample> const& TestSamples()
{
static std::vector<TestEvalSample> const samples
{
    {
        Vr {1,2,3,4.25E1} | Fold(Add) & Size | Div,
        1.2125E1
    },
    {
        2 | Div(3),
        2.0/3
    },
    {
        2 | ~Div(3),
        3.0/2
    },
    {
        Pi | Mul(2) | Add(Pi | Neg),
        3.1415926535898
    },

    {
        42 | If(Gt(100), "Kek") | Else("Lol"),
        "Lol"
    },
    {
        42 | If(Lt(100), "Kek") | Else("Lol"),
        "Kek"
    },

    {
        42 | If(Gt(100), "Kek") | Elif(Ne("string"), 13) | Else("Lol"),
        13
    },
    {
        Vr{1,2,3,4} | Map(Mul(10)),
        Vr{10,20,30,40}
    },
    {
        5 | Mod(3) | Eq(2),
        true
    },
    {
        Vr{1,2,3,4,5} | Filter(Mod(2) | Eq(1)),
        Vr{1,3,5}
    },
    {
        Vi{1,2,3,4,5} | Filter(Mod(2) | Eq(1)),
        Vi{1,3,5}
    },
    {
        Vi{2,4,8,17} | Map(
            Fork(Gt(0), (Mod(2)|Eq(0)), Lt(100)) | Fold(And)),
        Vb{true,true,true,false}
    }
    // TODO: change Unfold and Recur syntax to [x, n] | Keyword(Fn)
    // {Unfold(0, Add(1))          , 4                      , {0,1,2,3,4}          },
    // {Unfold(1, Add(1))          , 4                      , {1,2,3,4,5}          },
    // {Recur( 4, Add(1))          ,  Ge(12)                , 11                   },
    // {Unfold(8, Add(1))          ,  Ge(12)                , {8,9,10,11}          },
};
    return samples;
}

BOOST_DATA_TEST_CASE(Evaluate, TestSamples(), sample)
{
    impl::TypeRegistry tr{};
        tr.Add(type<Vr>);
        tr.Add(type<Vi>);
        tr.Add(type<Vb>); 
        tr.init(lua::global_state());

    Logger::set_max_level(Logger::DEBUG);


    std::stringstream ss;
    sol::object result;
    EvalContext ctx = EvalContext::make();
    Interpreter vm {{}, sample.expr, ctx};

    try
    {

        BOOST_TEST_INFO("Expression: " << sample.expr.prettify());
        BOOST_TEST_INFO("Expected: " << sample.expected.prettify());

        result = vm.evaluate();

        BOOST_TEST_INFO("Result: " << Expression(result).prettify());
        BOOST_TEST_INFO("Log: \n" << ctx.log);
        BOOST_CHECK_EQUAL(Expression(result).prettify(), sample.expected.prettify());
    }
    catch(const std::exception& e)
    {
        BOOST_FAIL("Exception thrown: " << e.what());
    }
}

BOOST_AUTO_TEST_CASE(Demo)
{
    using namespace haikan;

    OperatorRegistry()
        .AddSignature(type<double>, template_list<std::vector, std::set>)
        .Connects(type<int>, template_list<std::vector, std::set>)
        ;
    impl::TypeRegistry tr{};
        tr.Add(type<std::vector<double>>);
        tr.init(lua::global_state());


    Expression expr = Fold(Add) & Size | Div;
    sol::object arg = lua::make_object(std::vector<double> {1,2,3,4.25E1});

    std::stringstream ss;
    sol::object result;
    {
        EvalContext ctx = EvalContext::make();
        Interpreter vm {arg, expr, ctx};
        result = vm.evaluate();
        ss << '\n' << ctx.log;
    }
    auto expected = R"(
         ┌── {1.0,2.0} | Add = 3.0
         ├── {3.0,3.0} | Add = 6.0
         ├── {6.0,42.5} | Add = 48.5
      ┌── {1.0,2.0,3.0,42.5} | Fold(Add) = 48.5
      ├── {1.0,2.0,3.0,42.5} | Size = 4
   ┌── {1.0,2.0,3.0,42.5} | Fold(Add) & Size = {48.5,4}
   ├── {48.5,4} | Div = 12.125
□  {1.0,2.0,3.0,42.5} | (Fold(Add) & Size) | Div = 12.125
)";

    BOOST_CHECK_EQUAL(ss.str(), expected);

    BOOST_ASSERT(result.is<double>());
    BOOST_CHECK_EQUAL(result.as<double>(), 1.2125E1);
}


BOOST_AUTO_TEST_CASE(Demo2)
{
    using namespace haikan;

    OperatorRegistry()
        .AddSignature(type<double>, template_list<std::vector, std::set>)
        .Connects(type<int>, template_list<std::vector, std::set>)
        ;
    impl::TypeRegistry tr{};
        tr.Add(type<std::vector<double>>);
        tr.init(lua::global_state());

    using Vr = std::vector<double>;
    Expression expr = Vr{1,2,3,4.25E1} | Fold(Add) & Size | Div;

    std::stringstream ss;
    sol::object result;
    {
        EvalContext ctx = EvalContext::make();
        Interpreter vm {{}, expr, ctx};
        result = vm.evaluate();
        ss << '\n' << ctx.log;
    }
    auto expected = R"(
         ┌── {1.0,2.0} | Add = 3.0
         ├── {3.0,3.0} | Add = 6.0
         ├── {6.0,42.5} | Add = 48.5
      ┌── {1.0,2.0,3.0,42.5} | Fold(Add) = 48.5
      ├── {1.0,2.0,3.0,42.5} | Size = 4
   ┌── {1.0,2.0,3.0,42.5} | Fold(Add) & Size = {48.5,4}
   ├── {48.5,4} | Div = 12.125
□  {1.0,2.0,3.0,42.5} | (Fold(Add) & Size) | Div = 12.125
)";

    BOOST_CHECK_EQUAL(ss.str(), expected);

    BOOST_ASSERT(result.is<double>());
    BOOST_CHECK_EQUAL(result.as<double>(), 1.2125E1);
}



BOOST_AUTO_TEST_CASE(Demo3)
{
    using namespace haikan;

    OperatorRegistry()
        .AddSignature(type<double>, template_list<std::vector, std::set>)
        .Connects(type<int>, template_list<std::vector, std::set>)
        ;
    impl::TypeRegistry tr{};
        tr.Add(type<std::vector<double>>);
        tr.init(lua::global_state());

    using Vr = std::vector<double>;
    Expression expr = Vr{2, 3} | Add | Mul(Pi | Add(Pi | Neg)) | Add(Q(42));

    std::stringstream ss;
    sol::object result;
    {
        EvalContext ctx = EvalContext::make();
        Interpreter vm {{}, expr, ctx};
        result = vm.evaluate();
        ss << '\n' << ctx.log;
    }
    auto expected = R"(
   ┌── {2.0,3.0} | Add = 5.0
   │            ┌── 3.1415926535898 | Neg = -3.1415926535898
   │         ┌── Pi | Neg = -3.1415926535898
   │      ┌── 3.1415926535898 | Add(Pi | Neg) = 0.0
   │   ┌── Pi | Add(Pi | Neg) = 0.0
   ├── 5.0 | Mul(Pi | Add(Pi | Neg)) = 0.0
   ├── 0.0 | Add(Q(42)) = 42.0
□  {2.0,3.0} | Add | Mul(Pi | Add(Pi | Neg)) | Add(Q(42)) = 42.0
)";

    BOOST_CHECK_EQUAL(ss.str(), expected);

    BOOST_ASSERT(result.is<double>());
    BOOST_CHECK_CLOSE(result.as<double>(), 42, 1E-12);
}


BOOST_AUTO_TEST_CASE(IfElif)
{
    using namespace haikan;

    Expression expr = 42 | If(Lt(11), "lol") | Elif(Eq(42), "kek") | Else("fail");

    std::stringstream ss;
    sol::object result;
    {
        EvalContext ctx = EvalContext::make();
        Interpreter vm {{}, expr, ctx};
        result = vm.evaluate();
        ss << '\n' << ctx.log;
    }
    auto expected = R"(
      ┌── 42 | Lt(11) = false
   ┌── 42 | If(Lt(11), "lol") = _Forward(42)
   │   ┌── 42 | Eq(42) = true
   ├── _Forward(42) | Elif(Eq(42), "kek") = _Select("kek")
   ├── _Select("kek") | Else("fail") = "kek"
□  42 | If(Lt(11), "lol") | Elif(Eq(42), "kek") | Else("fail") = "kek"
)";

    BOOST_CHECK_EQUAL(ss.str(), expected);

    BOOST_ASSERT(result.is<std::string>());
    BOOST_CHECK_EQUAL(result.as<std::string>(), "kek");
}
