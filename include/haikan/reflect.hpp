#pragma once

#include <boost/optional.hpp>

#include "haikan/impl/reflect.hpp"

namespace haikan {

template <class T>
struct reflect
    : impl::select_reflect_utype<T>
    , impl::select_reflect_init<T>
    , impl::select_reflect_serialize<T>
    , impl::select_reflect_deserialize<T>
{
    using value_type = T;
    using optional_type = boost::optional<T>::value_type;

    // TODO: doxygen
    // static boost::optional<T> init();

    // serialization to plain Lua types
    // static sol::object serialize(T const& value, sol::state_view L);
    // static boost::optional<T> deserialize(sol::object const& value);

    // serialization to plain Lua types
    // static void utype(ReflectionContext& ctx)
};


template <class T>
inline decltype(auto) as_enum(T const e)
{
    return haikan::impl::user_data_enum<T>(e);
}

} // namespace haikan
