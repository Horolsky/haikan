#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> sol::object VirtualMachine::Evaluate<Keyword::_Literal>(sol::object const&, sol::object const& rhs) const
{
    return rhs;
}

template <> sol::object VirtualMachine::Evaluate<Keyword::_Void>(sol::object const&, sol::object const&) const
{
    return sol::nil;
}

template <> sol::object VirtualMachine::Evaluate<Keyword::D>(sol::object const&lhs, sol::object const& rhs) const
{
    return lhs.valid() ? lhs : rhs;
}

template <> void VirtualMachine::Evaluate<Keyword::Id>(VirtualMachine::Stack& stack)
{
    stack.front().state = &VirtualMachine::Return;
}

template <> void VirtualMachine::Evaluate<Keyword::Q>(VirtualMachine::Stack& stack)
{
    // TODO: remove serialization, pass encoding or expr. view directly
    auto& frame = stack.front();
    // auto const c = frame.expr.subexpressions_vec();
    // register_ = c.empty() ? frame.expr.data() : c.front().encoding_view().to_object();

    auto child = frame.expr.child(0);
    register_ = child.to_object();

    frame.state = &VirtualMachine::Return;
}

} // namespace impl
} // namespace haikan
