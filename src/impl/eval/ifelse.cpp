#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

namespace
{

enum class PayloadType : std::uint8_t
{
    PlainValue,
    Match,
    Pass,
};

sol::table payload_mt()
{
    static sol::table table = lua::create_table(0, 0);
    return table;
}

sol::object get_payload(sol::object const& object, PayloadType& type)
{
    type = PayloadType::PlainValue;
    if (!object.is<sol::table>())
    {
        return object;
    }

    sol::table const table = object.as<sol::table>();
    sol::object const metatable = table[sol::metatable_key];
    if (metatable != payload_mt())
    {
        return object;
    }

    sol::object const resolved = table["Match"];
    if (resolved != sol::nil)
    {
        type = PayloadType::Match;
        return resolved;
    }

    type = PayloadType::Pass;
    return table["Pass"];
}

sol::object make_payload(PayloadType type, sol::object const& value)
{
    char const* const key = type == PayloadType::Match
        ? "Match"
        : "Pass";
    sol::table table = lua::global_state().create_table_with(key, value);
    table[sol::metatable_key] = payload_mt();
    return table;
}

bool evaluate_clause(Interpreter::Stack& stack, sol::object& result)
{
    auto& frame = stack.front();

    switch (frame.index)
    {
    case 0: // Dispatch condition
    {
        frame.index = 1;
        stack.push_front(Interpreter::Frame(frame.expr.child(0)));
        return true;
    }

    case 1: // Check condition and branch
    {
        sol::object predicate = result;
        if (!predicate.is<bool>())
        {
            predicate = OperatorRegistry::get(predicate)
                .map([&](OperatorTable op) {
                    return op.apply(Keyword::Bool, predicate);
                })
                .value_or_eval([] {
                    return lua::make_object(error("invalid operands", "If/Elif"));
                });
        }

        if (predicate.is<error>())
        {
            result = predicate;
            return false;
        }

        EncodingView const alternative = frame.expr.child(2);
        if (predicate.as<bool>())
        {
            result = frame.value;
            frame.index = alternative.empty() ? 2 : 3;
            stack.push_front(Interpreter::Frame(frame.expr.child(1)));
            return true;
        }

        if (!alternative.empty())
        {
            result = frame.value;
            frame.index = 3;
            stack.push_front(Interpreter::Frame(alternative));
            return true;
        }

        result = make_payload(PayloadType::Pass, frame.value);
        return false;
    }

    case 2:
    {
        result = make_payload(PayloadType::Match, result);
        return false;
    }

    default:
        return false;
    }
}

} // namespace


template <> sol::object Interpreter::Evaluate<Keyword::_Continue>(sol::object const&, sol::object const& rhs) const { return rhs; }
template <> sol::object Interpreter::Evaluate<Keyword::_Resolve>(sol::object const&, sol::object const& rhs) const { return rhs; }

template <> void Interpreter::Evaluate<Keyword::If>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    frame.state = &Interpreter::Return;

    if (frame.index == 0)
    {
        PayloadType type;
        if (register_.is<error>())
        {
            return;
        }
        std::ignore = get_payload(register_, type);
        if (type != PayloadType::PlainValue)
        {
            register_ = lua::make_object(error("missing If", "Else"));
            return;
        }
        frame.value = register_;
    }

    if (evaluate_clause(stack, register_))
    {
        frame.state = &Interpreter::ResumeFrame;
    }
}

template <> void Interpreter::Evaluate<Keyword::Elif>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    frame.state = &Interpreter::Return;

    if (frame.index == 0)
    {
        PayloadType type;
        sol::object const value = get_payload(register_, type);
        if (type == PayloadType::PlainValue)
        {
            if (!register_.is<error>())
            {
                register_ = lua::make_object(error("missing If", "Else"));
            }
            return;
        }
        if (type == PayloadType::Match)
        {
            return;
        }
        register_ = value;
        frame.value = value;
    }

    if (evaluate_clause(stack, register_))
    {
        frame.state = &Interpreter::ResumeFrame;
    }
}

template <> void Interpreter::Evaluate<Keyword::Else>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    frame.state = &Interpreter::Return;

    if (frame.index != 0)
    {
        return;
    }

    PayloadType type;
    sol::object const value = get_payload(register_, type);
    if (type == PayloadType::PlainValue)
    {
        if (!register_.is<error>())
        {
            register_ = lua::make_object(error("missing If", "Else"));
        }
        return;
    }

    if (type == PayloadType::Match)
    {
        register_ = value;
        return;
    }

    register_ = value;
    frame.index = 1;
    stack.push_front(Interpreter::Frame(frame.expr.child(0)));
    frame.state = &Interpreter::ResumeFrame;
}

} // namespace impl
} // namespace haikan
