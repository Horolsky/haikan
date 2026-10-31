#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> void VirtualMachine::Evaluate<Keyword::Pipe>(VirtualMachine::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(frame.index);
    if (!child.empty())
    {
        ++frame.index;
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
