/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "fixture/samples.hpp"

#include <set>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "haikan/config.hpp"
#include "haikan/logger.hpp"

namespace utf = boost::unit_test;

BOOST_DATA_TEST_CASE(ExpressionEval, TestSamples())
{
    haikan::Config().SetRngSeed(42);
    haikan::Logger::set_max_level(haikan::Logger::DEBUG);

    auto context = haikan::impl::EvalContext::make();

    try
    {
        BOOST_TEST_INFO("Expression: " << sample.expr.prettify());
        BOOST_TEST_INFO("  encoding: " << boost::json::value_from(sample.expr.encoding()));
        BOOST_TEST_INFO("Input: " << sample.x.prettify());
        BOOST_TEST_INFO("Expected: " << sample.expected.prettify());

        auto const result = sample.expr.eval_e(sample.x, context);
        BOOST_TEST_INFO("Observed: " << result.prettify());
        BOOST_TEST_INFO("Eval log: \n" << context.log);
        BOOST_CHECK_EQUAL(result.prettify(), sample.expected.prettify());
    }
    catch(const std::exception& e)
    {
        BOOST_FAIL("Exception thrown: " << e.what());
    }
}