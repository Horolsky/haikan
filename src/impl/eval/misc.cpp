#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> sol::object Interpreter::Evaluate<Keyword::_Literal>(sol::object const&, sol::object const& rhs) const
{
    return rhs;
}

template <> sol::object Interpreter::Evaluate<Keyword::_Void>(sol::object const&, sol::object const&) const
{
    return sol::nil;
}

template <> sol::object Interpreter::Evaluate<Keyword::D>(sol::object const&lhs, sol::object const& rhs) const
{
    return lhs.valid() ? lhs : rhs;
}

template <> void Interpreter::Evaluate<Keyword::Id>(Interpreter::Stack& stack)
{
    stack.front().state = &Interpreter::Return;
}

template <> void Interpreter::Evaluate<Keyword::Q>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto child = frame.expr.child(0);
    register_ = child.to_object();
    frame.state = &Interpreter::Return;
}

} // namespace impl
} // namespace haikan
