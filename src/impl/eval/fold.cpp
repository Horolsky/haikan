#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> void Interpreter::Evaluate<Keyword::Fold>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();

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
            frame.state = &Interpreter::Return;
        }
    }

    if (frame.state != &Interpreter::Return)
    {
        sol::table source = frame.value.as<sol::table>();

        if (frame.index < source.size())
        {
            sol::table new_val = lua::create_table(2, 0);
            new_val[1] = register_; // accumulator
            new_val[2] = source[1+(frame.index++)]; // element

            register_ = new_val;

            frame.state = &Interpreter::ResumeFrame;
            stack.push_front(Frame(frame.expr.child(0)));
        }
        else
        {
            frame.state = &Interpreter::Return;
        }
    }
}

} // namespace impl
} // namespace haikan
