#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> void VirtualMachine::Evaluate<Keyword::Fork>(VirtualMachine::Stack& stack)
{
    auto& frame = stack.front();

    if (frame.index == 0)
    {
        sol::table args = lua::create_table(2, 0);
        args[1] = register_;
        args[2] = lua::create_table(frame.expr.arity(), 0);
        // Preserve original input and initialize output array.
        frame.value = args;
    }
    else
    {
        // Collect result of the previously evaluated branch.
        sol::object saved = frame.value.as<sol::table>()[2];
        saved.as<sol::table>()[frame.index] = register_;
    }

    auto const child = frame.expr.child(frame.index);
    if (!child.empty())
    {
        // Every branch receives the original input.
        register_ = frame.value.as<sol::table>()[1];
        frame.state = &VirtualMachine::ResumeFrame;
        ++frame.index;
        stack.push_front(Frame(child));
    }
    else
    {
        // All branches completed.
        register_ = frame.value.as<sol::table>()[2];
        frame.state = &VirtualMachine::Return;
    }
}

} // namespace impl
} // namespace haikan
