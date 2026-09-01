/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <boost/describe.hpp>


namespace haikan {

struct error
{
    std::string what;
    std::string where;

    error() = default;
    error(std::string what,
        std::string where)
        : what{what}
        , where{where}
    {
    }
};

BOOST_DESCRIBE_STRUCT(error, (), (what, where));

} // namespace haikan
