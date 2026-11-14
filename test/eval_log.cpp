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


using Vr = std::vector<double>;
using Vi = std::vector<std::int64_t>;
using Vb = std::list<bool>;

struct EvalLogSample
{
    Expression (*expr)();
    std::string expected;

    friend std::ostream& operator<<(std::ostream& os, EvalLogSample const& sample)
    {
        return os << sample.expr().prettify();
    }
};

std::vector<EvalLogSample> EvalLogSamples()
{
    return {
        {[] { return Vr{1,2,3,4.25E1} | Fold(Add) & Size | Div; }, R"(
         ┌── {1.0,2.0} | Add = 3.0
         ├── {3.0,3.0} | Add = 6.0
         ├── {6.0,42.5} | Add = 48.5
      ┌── {1.0,2.0,3.0,42.5} | Fold(Add) = 48.5
      ├── {1.0,2.0,3.0,42.5} | Size = 4
   ┌── {1.0,2.0,3.0,42.5} | Fold(Add) & Size = {48.5,4}
   ├── {48.5,4} | Div = 12.125
□  {1.0,2.0,3.0,42.5} | (Fold(Add) & Size) | Div = 12.125
)"},
        {[] { return Vr{2, 3} | Add | Mul(Pi | Add(Pi | Neg)) | Add(Q(42)); }, R"(
   ┌── {2.0,3.0} | Add = 5.0
   │            ┌── 3.1415926535898 | Neg = -3.1415926535898
   │         ┌── Pi | Neg = -3.1415926535898
   │      ┌── 3.1415926535898 | Add(Pi | Neg) = 0.0
   │   ┌── Pi | Add(Pi | Neg) = 0.0
   ├── 5.0 | Mul(Pi | Add(Pi | Neg)) = 0.0
   ├── 0.0 | Add(Q(42)) = 42.0
□  {2.0,3.0} | Add | Mul(Pi | Add(Pi | Neg)) | Add(Q(42)) = 42.0
)"},
        {[] { return 42 | If(Lt(11), "lol") | Elif(Eq(42), "kek") | Else("fail"); }, R"(
      ┌── 42 | Lt(11) = false
   ┌── 42 | If(Lt(11), "lol") = _Forward(42)
   │   ┌── 42 | Eq(42) = true
   ├── _Forward(42) | Elif(Eq(42), "kek") = _Select("kek")
   ├── _Select("kek") | Else("fail") = "kek"
□  42 | If(Lt(11), "lol") | Elif(Eq(42), "kek") | Else("fail") = "kek"
)"},
        {[] { return Vb{true,false,true,true} | Fold(And); }, R"(
      ┌── {true,false} | And = false
   ┌── {true,false,true,true} | Fold(And) = false
□  {true,false,true,true} | Fold(And) = false
)"},
    };
}


BOOST_DATA_TEST_CASE(EvalLog, utf::data::make(EvalLogSamples()), sample)
{
    OperatorRegistry()
        .AddSignature(type<double>, template_list<std::vector, std::set>)
        .Connects(type<int>, template_list<std::vector, std::set>);
    impl::TypeRegistry tr{};
    tr.Add(type<std::vector<double>>);
    tr.Add(type<Vb>);
    tr.init(lua::global_state());

    auto expr = sample.expr();
    EvalContext ctx = EvalContext::make();
    Interpreter vm {{}, expr, ctx};
    vm.evaluate();

    std::stringstream observed;
    observed << '\n' << ctx.log;
    BOOST_CHECK_EQUAL(observed.str(), sample.expected);
}
