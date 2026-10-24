#pragma once

#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


#include "haikan/impl/select_reflect.hpp"


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

    // static void utype(ReflectionContextFactory& ctx)
};


template <class T>
inline decltype(auto) as_enum(T const e)
{
    return haikan::impl::user_data_enum<T>(e);
}


/// @brief Serialize generic object (possibly userdata)
/// @param value
/// @return Plain Lua object or haikan::error
sol::object to_plain_lua(sol::object value);


} // namespace haikan
