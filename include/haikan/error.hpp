/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <ostream>
#include <boost/describe.hpp>

// IWYU pragma: no_include <__ostream/basic_ostream.h>

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

    friend std::ostream& operator<<(std::ostream& os, error const& e)
    {
        os << "haikan::error(\"" << e.what << "\", \"" << e.where << "\")";
        return os;
    }
};

BOOST_DESCRIBE_STRUCT(error, (), (what, where));

} // namespace haikan
