#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> void VirtualMachine::Evaluate<Keyword::Pipe>(VirtualMachine::Stack& stack)
{
    auto& frame = stack.front();

    auto const& subs = frame.expr.subexpressions_vec();

    if (frame.index < subs.size())
    {
        auto const& child = subs[frame.index++];

        frame.state = &VirtualMachine::ResumeFrame;
        stack.push_front(Frame(child));
    }
    else
    {
        frame.state = &VirtualMachine::Return;
    }
}

} // namespace impl
} // namespace haikan
