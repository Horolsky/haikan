#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> void VirtualMachine::Evaluate<Keyword::Fork>(VirtualMachine::Stack& stack)
{
    auto& frame = stack.front();
    Expression const& expr = frame.expr;
    auto const& subs = expr.subexpressions_vec();

    if (frame.index == 0)
    {
        sol::table args = lua::create_table(2, 0);
        args[1] = register_;
        args[2] = lua::create_table(subs.size(), 0);
        // Preserve original input and initialize output array.
        frame.value = args;
    }
    else
    {
        // Collect result of the previously evaluated branch.
        sol::object saved = frame.value.as<sol::table>()[2];
        saved.as<sol::table>()[frame.index] = register_;
    }

    if (frame.index < subs.size())
    {
        // Every branch receives the original input.
        register_ = frame.value.as<sol::table>()[1];
        frame.state = &VirtualMachine::ResumeFrame;
        stack.push_front(Frame(subs[frame.index++]));
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
