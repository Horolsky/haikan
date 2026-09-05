/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "haikan/config.hpp"
#include "haikan/keywords.hpp"
#include "haikan/impl/keyword_tag_invoke.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/encoding.hpp"

namespace utf = boost::unit_test;
using namespace haikan;

BOOST_AUTO_TEST_CASE(ExpressionSerialization)
{
    auto context = haikan::impl::EvalContext::make();

    std::vector<haikan::impl::Expression> expr_list {Id, Pi | Sub(42), "$f" << Q(42)};
    try
    {
        BOOST_TEST_INFO("expr_list value_from: " << boost::json::value_from(expr_list));
    }
    catch(const std::exception& e)
    {
        BOOST_FAIL("Exception thrown: " << e.what());
    }
}