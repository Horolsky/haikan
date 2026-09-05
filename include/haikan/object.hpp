#pragma once

#include <cstdint>

#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <boost/utility/string_view.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

// #include "haikan/reflect.hpp"

namespace haikan {


struct slice
{
    boost::optional<std::int64_t> start;
    boost::optional<std::int64_t> stop;
    boost::optional<std::int64_t> step;
};


struct object : public sol::object
{
    // 1. Key
    // 2. Index
    // 3. Slice
    // 4. JSON Pointer
    // 5. Array pack
    // 6. Object pack
    object operator[](object const query) const;

    object operator-() const;
    object operator+(object const& rhs) const;
    object operator-(object const& rhs) const;
    object operator*(object const& rhs) const;
    object operator/(object const& rhs) const;

    object operator~() const;
    object operator&(object const& rhs) const;
    object operator|(object const& rhs) const;
    object operator^(object const& rhs) const;

    object operator!() const;
    object operator&&(object const& rhs) const;
    object operator||(object const& rhs) const;

    object operator==(object const& rhs) const;
    object operator!=(object const& rhs) const;

    object operator<=(object const& rhs) const;
    object operator<(object const& rhs) const;
    object operator>=(object const& rhs) const;
    object operator>(object const& rhs) const;

    object operator%(object const& rhs) const;
    object operator<<(object const& rhs) const;
    object operator>>(object const& rhs) const;

    object set_union(object const& rhs) const;
    object set_intersection(object const& rhs) const;
    object set_difference(object const& rhs) const;
    object set_is_equal(object const& rhs) const;
    object set_is_subset(object const& rhs) const;
    object set_is_superset(object const& rhs) const;
    object set_is_proper_subset(object const& rhs) const;
    object set_is_proper_superset(object const& rhs) const;

};

} // namespace haikan
