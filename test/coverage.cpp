/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "fixture/samples.hpp"

#include <set>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace utf = boost::unit_test;
using haikan::impl::Keyword;

std::set<Keyword> const& CoveredInTestEval()
{
    static std::set<Keyword>* covered {nullptr};
    if (!covered)
    {
        covered = new std::set<Keyword>;
        for (auto const& sample : TestSamples())
        {
            auto const& keywords = sample.expr.encoding().keywords;
            covered->insert(keywords.cbegin(), keywords.cend());
        }
    }
    return *covered;
};

BOOST_DATA_TEST_CASE(EvalTestCoverage, utf::data::xrange(std::size_t{1ul}, static_cast<std::size_t>(Keyword::_count)))
{
    Keyword const keyword = static_cast<Keyword>(sample);
    if (keyword == Keyword::_Void) return;
    if (keyword == Keyword::_Resolve) return;
    if (keyword == Keyword::_Continue) return;
    if (CoveredInTestEval().count(keyword) == 0)
    {
        BOOST_FAIL("Keyword " << boost::json::value_from(keyword) << " is not covered in TestEval");
    }
}
