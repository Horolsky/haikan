/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <cstddef>
#include <vector>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

namespace haikan
{

struct ReflectionMeta
{
    std::string type_name;
    std::size_t type_index_hash;
    std::vector<std::string> members;
    sol::protected_function serialize;
    sol::protected_function deserialize;
};

} // namespace haikan
