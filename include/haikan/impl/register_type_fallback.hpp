/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <boost/format.hpp>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <tuple>

#include <iostream>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


namespace haikan {
namespace impl {

template <class T>
struct register_type_fallback {

static void operator()(sol::state_view L) {
    sol::object maybe = L[sol::usertype_traits<T>::name()];
    if (maybe.get_type() == sol::type::userdata)
    {
        sol::userdata u = maybe;
        sol::object mt_obj = u[sol::metatable_key];
        if (mt_obj.valid())
        {
            sol::table mt = mt_obj;
            sol::object registered_obj = mt["__haikan_type_name"];
            if (registered_obj.valid() && registered_obj.get_type() == sol::type::string)
            {
                static std::string const k_type_name = type_name<T>();
                std::string const registered = registered_obj.as<std::string>();
                if (registered != k_type_name)
                {
                    throw std::runtime_error(
                        (boost::format("usertype '%s' already registered for '%s', expected '%s'")
                            % sol::usertype_traits<T>::name()
                            % registered
                            % k_type_name)
                            .str());
                }
            }
        }
        return;
    }
    // TODO: register operators
    sol::simple_usertype<T> usertype = L.create_simple_usertype<T>();
    static std::string const k_type_name = type_name<T>();
    usertype.set("__haikan_type_name", k_type_name);
    L.set_usertype(usertype);
}

};
} // namespace impl
} // namespace haikan
