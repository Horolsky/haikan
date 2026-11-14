#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> void Interpreter::Evaluate<Keyword::Flip>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(0);

    switch (frame.index)
    {
    case 0:
        frame.value = std::exchange(register_, lua::make_object(sol::nil));
        if (child.child(0).empty())
        {
            Frame operand(child, &Interpreter::Execute);
            operand.args_resolved = true;

            if (frame.value.get_type() == sol::type::table
                || (frame.value.get_type() == sol::type::userdata
                    && !frame.value.is<error>()))
            {
                sol::table const args = frame.value.as<sol::table>();
                operand.lhs = args[2];
                operand.rhs = args[1];
            }
            else
            {
                operand.lhs = lua::make_object(sol::nil);
                operand.rhs = frame.value;
            }

            frame.index = 2;
            frame.state = &Interpreter::ResumeFrame;
            stack.push_front(std::move(operand));
        }
        else
        {
            frame.index = 1;
            frame.state = &Interpreter::ResumeFrame;
            stack.push_front(Frame(child.child(0)));
        }
        break;

    case 1:
        {
            Frame operand(child, &Interpreter::Execute);
            operand.args_resolved = true;
            operand.lhs = std::exchange(register_, lua::make_object(sol::nil));
            operand.rhs = std::exchange(frame.value, lua::make_object(sol::nil));

            frame.index = 2;
            frame.state = &Interpreter::ResumeFrame;
            stack.push_front(std::move(operand));
            break;
        }

    case 2:
        frame.state = &Interpreter::Return;
        break;
    }
}


} // namespace impl
} // namespace haikan
