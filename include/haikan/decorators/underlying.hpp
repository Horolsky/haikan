/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <exception>
#include <type_traits>
#include <boost/json.hpp>

#include "haikan/impl/type_tag.hpp"


namespace haikan {
namespace decorators {

template <class E>
struct underlying
{
    static_assert(std::is_enum<E>::value, "");
    static_assert(boost::json::has_value_from<E>::value, "");
    static_assert(boost::json::has_value_to<E>::value, "");

    using decorated_type = std::underlying_type_t<E>;


    ~underlying() = default;
    underlying() = default;
    underlying(underlying const&) = default;
    underlying(underlying &&) = default;
    underlying& operator=(underlying const&) = default;
    underlying& operator=(underlying &&) = default;


    underlying(decorated_type const v) : value_{v} {}
    underlying& operator=(decorated_type const v) { value_ = v; return *this; }

    underlying(E const v) : value_{static_cast<decorated_type>(v)} {}
    underlying& operator=(E const v) { value_ = static_cast<decorated_type>(v); return *this; }

    underlying(boost::json::value const v)
    {
        value_ = static_cast<decorated_type>(boost::json::value_to<E>(v));
    }
    underlying& operator=(boost::json::value const& v)
    {
        value_ = static_cast<decorated_type>(boost::json::value_to<E>(v));
        return *this;
    }

    decorated_type value() const
    {
        return value_;
    }

    std::string stringify() const
    {
        return boost::json::value_from(static_cast<E>(value())).as_string().c_str();
    }

    operator decorated_type() const
    {
        return {value()};
    }

    auto operator!()  const { return !value(); }
    auto operator~()  const { return ~value(); }
    auto operator-()  const { return -value(); }

    template <class T> auto operator==(T const& other) const { return value() == other; }
    template <class T> auto operator!=(T const& other) const { return value() != other; }
    template <class T> auto operator>>(T const& other) const { return value() >> other; }
    template <class T> auto operator<<(T const& other) const { return value() << other; }
    template <class T> auto operator<=(T const& other) const { return value() <= other; }
    template <class T> auto operator>=(T const& other) const { return value() >= other; }
    template <class T> auto operator<(T const& other)  const { return value() <  other; }
    template <class T> auto operator>(T const& other)  const { return value() >  other; }
    template <class T> auto operator+(T const& other)  const { return value() +  other; }
    template <class T> auto operator-(T const& other)  const { return value() -  other; }
    template <class T> auto operator*(T const& other)  const { return value() *  other; }
    template <class T> auto operator/(T const& other)  const { return value() /  other; }
    template <class T> auto operator%(T const& other)  const { return value() %  other; }
    template <class T> auto operator&(T const& other)  const { return value() &  other; }
    template <class T> auto operator|(T const& other)  const { return value() |  other; }
    template <class T> auto operator^(T const& other)  const { return value() ^  other; }
    template <class T> auto operator&&(T const& other) const { return value() && other; }
    template <class T> auto operator||(T const& other) const { return value() || other; }

private:
    decorated_type value_;
};


/// @brief underlying<T> decorator
/// @tparam T
template <class T>
constexpr ::haikan::impl::type_tag<underlying<T>> Underlying;

// Boost JSON conversion from underlying<E>
template <class E>
void tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, underlying<E> const& t)
{
    v = t.value();
}

// Boost JSON conversion to underlying<E>
template <class E>
underlying<E> tag_invoke(boost::json::value_to_tag<underlying<E>> const&, boost::json::value const& v)
{
    return underlying<E>{v};
}

} // namespace decorators
} // namespace haikan
