/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ostream>

#include <boost/json.hpp>

namespace haikan {


/**
 * @brief Pretty print JSON data
 *
 * @param os
 * @param jv
 * @param indent initial indent
 * @return std::ostream&
 */
std::ostream& pretty_print(std::ostream& os, boost::json::value const& jv, int const indent = 0);



} // namespace haikan
