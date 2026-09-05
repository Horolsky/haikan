/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/json/string_view.hpp>
#include "haikan/impl/keyword.hpp"

namespace haikan {
namespace impl {

boost::json::string_view keyword_to_str(Keyword const& kw);

}  // namespace impl
}  // namespace haikan
