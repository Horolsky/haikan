/**
 * @file
 * @copyright (c) Copyright 2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

// #include "haikan/keywords.hpp"
#include "haikan/impl/keyword_tag_invoke.hpp"

namespace utf = boost::unit_test;
using haikan::impl::Keyword;

BOOST_AUTO_TEST_CASE(SerializationSpeed, *utf::timeout(1))
{
    using haikan::impl::tag_invoke;
    boost::json::string const test_kw = boost::json::value_from(Keyword::PSuperset).as_string();
    boost::json::value test_value{test_kw};
    for (int i = 0; i < 1000*1000; i++)
    {
        auto const kw = boost::json::value_to<Keyword>(test_value);
        if (kw != Keyword::PSuperset)
        {
            BOOST_FAIL("ITER FAILED = " << i);
            break;
        }
    }
}

BOOST_AUTO_TEST_CASE(SerializationUndefinedSpeed, *utf::timeout(1))
{
    boost::json::value const test_value{"ArbitraryStringNotPresentInKeywords"};
    for (int i = 0; i < 1000*1000; i++)
    {
        auto const kw = boost::json::value_to<Keyword>(test_value);
        if (kw != Keyword::Undefined)
        {
            BOOST_FAIL("ITER FAILED = " << i);
            break;
        }
    }
}
