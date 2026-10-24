/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "haikan/config.hpp"
#include "haikan/keywords_lua.hpp"
#include "haikan/impl/expression_lua.hpp"

namespace utf = boost::unit_test;
using haikan::impl::ExpressionLua;
using namespace haikan;



BOOST_AUTO_TEST_CASE(ExpressionSerialization)
{
    std::vector<ExpressionLua> expr_list {Id, Pi | Sub(42), "$f" << Q(42)};
    try
    {
        BOOST_TEST_INFO("expr_list value_from: " << boost::json::value_from(expr_list));
    }
    catch(const std::exception& e)
    {
        BOOST_FAIL("Exception thrown: " << e.what());
    }
}
