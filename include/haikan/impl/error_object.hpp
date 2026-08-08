/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <boost/describe.hpp>


namespace haikan {
namespace impl {

struct ErrorObject
{
    std::string what;
    std::string where;

    ErrorObject() = default;
    ErrorObject(std::string what,
        std::string where)
        : what{what}
        , where{where}
    {
    }
};

BOOST_DESCRIBE_STRUCT(ErrorObject, (), (what, where));

} // namespace impl
} // namespace haikan
