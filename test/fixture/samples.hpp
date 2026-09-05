#pragma once

#include <vector>
#include <ostream>

#include <boost/describe.hpp>
#include <boost/json.hpp>

#include <haikan/keywords.hpp>
#include <haikan/config.hpp>
#include <haikan/decorators/underlying.hpp>
#include <haikan/impl/cast.hpp>



BOOST_DEFINE_FIXED_ENUM_CLASS(Foo, int, Bar, Baz)

struct TestEvalSample
{
    haikan::impl::Expression expr;
    haikan::impl::Expression expected;

    friend std::ostream& operator<<(std::ostream& os, TestEvalSample const& sample);
};


std::vector<TestEvalSample> const& TestSamples();