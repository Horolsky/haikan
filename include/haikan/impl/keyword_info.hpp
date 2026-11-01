/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/json.hpp>

#include "haikan/impl/keyword.hpp"
#include "haikan/impl/keyword_attributes.hpp"
#include "haikan/impl/keyword_to_str.hpp"


namespace haikan {
namespace impl {

template <Keyword k>
struct KeywordInfo
{
    constexpr Keyword keyword() { return k; }
    constexpr std::uint32_t attributes() { return keyword_attributes(k); }
    static boost::json::string_view keyword_to_str() { return ::haikan::impl::keyword_to_str(k); }
};

}  // namespace impl
}  // namespace haikan
