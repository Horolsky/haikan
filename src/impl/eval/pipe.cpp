#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> void Interpreter::Evaluate<Keyword::Pipe>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(frame.index);
    if (!child.empty())
    {
        ++frame.index;
        frame.state = &Interpreter::ResumeFrame;
        stack.push_front(Frame(child));
    }
    else
    {
        frame.state = &Interpreter::Return;
    }
}

} // namespace impl
} // namespace haikan
