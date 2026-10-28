#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> void VirtualMachine::Evaluate<Keyword::Fold>(VirtualMachine::Stack& stack)
{
    auto& frame = stack.front();
    Expression const& expr = frame.expr;

    // Initialize accumulator with first element.
    if (frame.index == 0)
    {
        frame.value = register_;
        sol::table source = frame.value.as<sol::table>();

        if (source.size() != 0)
        {
            register_ = source[1];
            frame.index = 1;
        }
        else
        {
            frame.state = &VirtualMachine::Return;
        }
    }

    if (frame.state != &VirtualMachine::Return)
    {
        sol::table source = frame.value.as<sol::table>();

        if (frame.index < source.size())
        {
            sol::table new_val = lua::create_table(2, 0);
            new_val[1] = register_; // accumulator
            new_val[2] = source[1+(frame.index++)]; // element

            register_ = new_val;

            frame.state = &VirtualMachine::ResumeFrame;
            Expression function = expr.subexpressions_vec().front();
            stack.push_front(Frame(function));
        }
        else
        {
            frame.state = &VirtualMachine::Return;
        }
    }
}

} // namespace impl
} // namespace haikan
