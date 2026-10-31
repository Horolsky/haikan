#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> sol::object Interpreter::Evaluate<Keyword::Size>(sol::object const& lhs, sol::object const&) const
{
    return lua::make_object(lhs.as<sol::table>().size());
}


} // namespace impl
} // namespace haikan
