#include <cstdint>
#include <boost/utility/string_view.hpp>
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

sol::object get_payload(sol::object const& object, PayloadType& type)
{
    type = PayloadType::PlainValue;
    if (!object.is<sol::table>())
    {
        return object;
    }

    sol::table const table = object.as<sol::table>();
    sol::object const keywords_object = table["keywords"];
    if (!keywords_object.is<sol::table>())
    {
        return object;
    }

    sol::object const opcode_object = keywords_object.as<sol::table>()[1];
    if (opcode_object.get_type() != sol::type::string)
    {
        return object;
    }

    boost::string_view const opcode = opcode_object.as<char const*>();
    if (opcode == "_Select")
    {
        type = PayloadType::Match;
    }
    else if (opcode == "_Forward")
    {
        type = PayloadType::Pass;
    }
    else
    {
        return object;
    }

    Encoding const encoding(object);
    EncodingView const payload(encoding);
    return payload.child(0).to_object();
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

        result = Encoding(Keyword::_Forward, {Encoding(frame.value)}).to_object();
        return false;
    }

    case 2:
    {
        result = Encoding(Keyword::_Select, {Encoding(result)}).to_object();
        return false;
    }

    default:
        return false;
    }
}

} // namespace


template <> sol::object Interpreter::Evaluate<Keyword::_Forward>(sol::object const&, sol::object const& rhs) const { return rhs; }
template <> sol::object Interpreter::Evaluate<Keyword::_Select>(sol::object const&, sol::object const& rhs) const { return rhs; }

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
