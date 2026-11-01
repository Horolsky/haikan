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
        // Save original input (future RHS).
        frame.value = std::exchange(register_, lua::make_object(sol::nil));
        frame.index = 1;

        // if (child.subexpressions_vec().empty())
        // {
        //     // Immediate parameter.
        //     register_ = child.param();
        // }
        // else
        {
            // Evaluate complex parameter with Void input.
            frame.state = &Interpreter::ResumeFrame;
            stack.push_front(Frame(child.child(0), &Interpreter::Enter));
        }
        break;

    case 1:
        // Parameter is resolved: swap operand positions.
        {
            sol::table result = lua::create_table(2, 0);
            result[1] = std::exchange(register_, lua::make_object(sol::nil));
            result[2] = std::exchange(frame.value, lua::make_object(sol::nil));
            register_ =  result;
            frame.index = 2;
            frame.state = &Interpreter::ResumeFrame;
            // Skip Enter: operands are already prepared.
            stack.push_front(Frame(child, &Interpreter::Execute));
            break;
        }

    case 2:
        // Child returned its result in register_.
        frame.state = &Interpreter::Return;
        break;
    }
}


} // namespace impl
} // namespace haikan
