/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/json.hpp>


namespace haikan {
namespace impl {

boost::json::value make_error_expr(boost::json::string_view msg, boost::json::string_view ctx);

}
}