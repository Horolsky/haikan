/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/json.hpp>
#include <complex>

#include "haikan/impl/real_to_number.hpp"

namespace std {

// Boost JSON conversion from std::complex<T>
template <class T>
void tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, std::complex<T> const& t)
{
    auto const real = haikan::impl::real_to_number(t.real());
    auto const imag = haikan::impl::real_to_number(t.imag());
    v = !t.imag() ? real : boost::json::array{real, imag};
}

// Boost JSON conversion to std::complex<T>
template <class T>
std::complex<T> tag_invoke(boost::json::value_to_tag<std::complex<T>> const&, boost::json::value const& v)
{
    T real, imag;
    if (v.is_array() && v.get_array().size() == 2)
    {
        real = boost::json::value_to<T>(v.get_array().at(0));
        imag = boost::json::value_to<T>(v.get_array().at(1));
    }
    else if (v.is_number())
    {
        real = boost::json::value_to<T>(v);
        imag = 0;
    }
    else
    {
        boost::throw_exception(std::runtime_error((boost::format(
            "can't produce std::complex from `%s`") % v).str()));
    }
    return {real, imag};
}

}