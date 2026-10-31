/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#include <type_traits>
#include <typeinfo>
#include <cstdint>

// #include <boost/container_hash/hash.hpp>
#include <boost/current_function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/mp11.hpp>
#include <boost/type_traits.hpp>


#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/error.hpp"
#include "haikan/logger.hpp"
#include "haikan/reflection_meta.hpp"
#include "haikan/impl/pp.hpp"
// #include "haikan/impl/operators.hpp"
// #include "haikan/impl/get_or_create_table.hpp"
// #include "haikan/type_tag.hpp"
// #include "haikan/impl/keyword.hpp"
// #include "haikan/impl/expression_parameter.hpp"


namespace haikan {
namespace impl {


struct GenericOperand : public sol::object
{
    using sol::object::object;
    using sol::object::operator=;
    using sol::object::operator bool;

    GenericOperand(sol::object const& o) : sol::object(o) {}
    GenericOperand(sol::object && o) : sol::object(std::move(o)) {}

    friend GenericOperand operator+(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator-(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator*(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator/(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator==(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator!=(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator<(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator>(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator<=(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator>=(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator&&(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator||(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator&(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator|(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator^(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator<<(GenericOperand const&, GenericOperand const&);
    friend GenericOperand operator>>(GenericOperand const&, GenericOperand const&);

    friend GenericOperand operator~(GenericOperand const&);
    friend GenericOperand operator!(GenericOperand const&);
    friend GenericOperand operator-(GenericOperand const&);
};


} // namespace impl
} // namespace haikan


template<>
template<>
inline decltype(auto) sol::basic_object_base<sol::reference>::as<haikan::impl::GenericOperand>() const {
    return haikan::impl::GenericOperand{*this};
}

template<>
template<>
inline bool sol::basic_object_base<sol::reference>::is<haikan::impl::GenericOperand>() const {
    return true;
}
