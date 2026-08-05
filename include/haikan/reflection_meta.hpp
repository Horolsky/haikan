/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/type_tag.hpp"

namespace haikan
{

struct ReflectionMeta
{
    std::string type_name;
    std::size_t type_index_hash;
    std::size_t wrapper_type_index_hash;
    std::vector<std::string> members;
    sol::protected_function serialize;
    sol::protected_function deserialize;
};

namespace impl
{

boost::optional<ReflectionMeta&> get_rmeta(sol::state_view L, std::size_t type_index_hash);

template <class T>
boost::optional<ReflectionMeta&> get_rmeta(sol::state_view L, type_tag<T>)
{
    return get_rmeta(L, typeid(T).hash_code());
}

} // namespace impl

} // namespace haikan
