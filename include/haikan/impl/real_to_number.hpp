/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <boost/json.hpp>


namespace haikan {
namespace impl {


/// Convert a double to a JSON number,
/// storing result as uint64_t or int64_t if possible.
boost::json::value real_to_number(double value);

/// Convert a signed integer to JSON number,
/// storing result as uint64_t if possible.
boost::json::value real_to_number(std::int64_t value);

}  // namespace impl
}  // namespace haikan
