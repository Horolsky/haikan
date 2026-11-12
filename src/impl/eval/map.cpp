#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> void Interpreter::Evaluate<Keyword::Map>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(0);

    if (child.empty())
    {
        register_ = lua::create_table(0, 0);
        frame.state = &Interpreter::Return;
        return;
    }

    if (0 == frame.index)
    {
        sol::table input = register_.as<sol::table>();
        auto const size = input.size();
        sol::table output = lua::create_table(size, 0);
        for (std::size_t i = 1; i <= size; ++i)
        {
            output[i] = input[i];
        }

        frame.value = output;

        if (0 == size)
        {
            register_ = output;
            frame.state = &Interpreter::Return;
            return;
        }

        frame.index = 1;
        register_ = output[1];
    }
    else
    {
        sol::table output = frame.value.as<sol::table>();
        output[frame.index] = register_;

        if (++frame.index > output.size())
        {
            register_ = output;
            frame.state = &Interpreter::Return;
            return;
        }

        register_ = output[frame.index];
    }

    frame.state = &Interpreter::ResumeFrame;
    stack.push_front(Frame(child));
}

template <> void Interpreter::Evaluate<Keyword::Filter>(Interpreter::Stack&)
{
}

template <> void Interpreter::Evaluate<Keyword::Count>(Interpreter::Stack&)
{
}

template <> void Interpreter::Evaluate<Keyword::Each>(Interpreter::Stack&)
{
}

} // namespace impl
} // namespace haikan
