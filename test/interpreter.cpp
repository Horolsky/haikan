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


#include "fixture/samples.hpp"

namespace utf = boost::unit_test;
using namespace haikan;
using namespace haikan::impl;


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
