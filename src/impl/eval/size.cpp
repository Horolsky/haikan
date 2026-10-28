#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> sol::object VirtualMachine::Evaluate<Keyword::Size>(sol::object const& lhs, sol::object const&) const
{
    return lua::make_object(lhs.as<sol::table>().size());
}


} // namespace impl
} // namespace haikan
