/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "haikan/keywords.hpp"

namespace utf = boost::unit_test;
using namespace haikan;
using namespace haikan::impl;


BOOST_AUTO_TEST_CASE(TestPreprocessing)
{
    auto p1 = PreProc(1);
    auto p2 = PreProc(2);
    auto p3 = PreProc(3);

    auto const expr = Filter(At(p1)|false) | p2 | Eq({p3});
    BOOST_TEST_INFO(expr.prettify());
    BOOST_TEST_INFO(expr.to_json());
    auto const pp = expr.preprocessing_parameters();
    BOOST_ASSERT(pp.size() == 3);
    auto as_json = expr.to_json();

    boost::json::object values {
        {p1.eval().as_string(), 42                   },
        {p2.eval().as_string(), Map(At(13)).to_json()},
        {p3.eval().as_string(), "lol"                },
    };

    for (auto const& kp: pp)
    {
        auto const& key = kp.first;
        auto const& json_pointer = kp.second;
        as_json.set_at_pointer(json_pointer, values.at(key));
    }

    BOOST_TEST_INFO(as_json);
    BOOST_CHECK_EQUAL(
        Expression(as_json).prettify(),
        (Filter(At(42)|false) | Map(At(13)) | Eq({"lol"})).prettify()
    );
}

