#pragma once

#include <boost/optional.hpp>

#include "haikan/impl/reflect.hpp"

namespace haikan {

template <class T>
struct reflect
    : impl::select_reflect_init<T>
    , impl::select_reflect_solify<T>
    , impl::select_reflect_desolify<T>
{
    using value_type = T;
    using optional_type = boost::optional<T>::value_type;

    // TODO: doxygen
    // static boost::optional<T> init();
    // static boost::optional<T> desolify(sol::object const& value);
    // static sol::object solify(T const& value, sol::state_view L);
};


template <class T>
inline decltype(auto) as_enum(T const e)
{
    return haikan::impl::user_data_enum<T>(e);
}

} // namespace haikan
