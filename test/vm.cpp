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
#include "haikan/impl/vm.hpp"

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


BOOST_AUTO_TEST_CASE(Demo)
{
    // [1,2,3,4.25E1] -> (Fold(Add) & Size | Div)

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
        VirtualMachine vm {arg, expr, ctx};
        result = vm.evaluate();
        ss << '\n' << ctx.log;
        // log.drain(ss, true);
    }
    auto expected = R"(
         ┌── Add $ {1.0,2.0} = 3.0
         ├── Add $ {3.0,3.0} = 6.0
         ├── Add $ {6.0,42.5} = 48.5
      ┌── Fold(Add) $ {1.0,2.0,3.0,42.5} = 48.5
      ├── Size $ {1.0,2.0,3.0,42.5} = 4
   ┌── Fold(Add) & Size $ {1.0,2.0,3.0,42.5} = {48.5,4}
   ├── Div $ {48.5,4} = 12.125
□  (Fold(Add) & Size) | Div $ {1.0,2.0,3.0,42.5} = 12.125
)";

    BOOST_CHECK_EQUAL(ss.str(), expected);

    BOOST_ASSERT(result.is<double>());
    BOOST_CHECK_EQUAL(result.as<double>(), 1.2125E1);
}
