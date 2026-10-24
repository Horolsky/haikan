/**
 * @file
 * @copyright (c) Copyright 2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/json_traverse.hpp"

#include <boost/format.hpp>


void haikan::impl::JsonTraverse::traverse(boost::json::value const& v, std::string const jptr) const
{
    if (visit_(v, jptr)) return;

    if (v.is_array())
    {
        size_t i {0};
        for (auto const& el: v.get_array())
        {
            traverse(el, (boost::format("%s/%i") % jptr % i++).str());
        }
    }
    else if (v.is_object())
    {
        for (auto const& el: v.get_object())
        {
            traverse(el.value(), (boost::format("%s/%i") % jptr % el.key()).str());
        }
    }
}
