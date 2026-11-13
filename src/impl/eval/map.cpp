#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"
#include "haikan/impl/operator_table.hpp"

namespace haikan {
namespace impl {

template <> void Interpreter::Evaluate<Keyword::Map>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(0);

    if (HAIKAN_UNLIKELY(child.empty()))
    {
        register_ = lua::create_table(0, 0);
        frame.state = &Interpreter::Return;
        return;
    }

    if (HAIKAN_UNLIKELY(0 == frame.index))
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

template <> void Interpreter::Evaluate<Keyword::Filter>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(0);

    if (HAIKAN_UNLIKELY(child.empty()))
    {
        register_ = lua::create_table(0, 0);
        frame.state = &Interpreter::Return;
        return;
    }

    if (HAIKAN_UNLIKELY(0 == frame.index))
    {
        sol::table input = register_.as<sol::table>();
        auto const size = input.size();
        sol::table source = lua::create_table(size, 0);
        sol::table output = lua::create_table(size, 0);
        for (std::size_t i = 1; i <= size; ++i)
        {
            source[i] = input[i];
        }

        frame.value = source;
        frame.lhs = output;

        if (0 == size)
        {
            register_ = output;
            frame.state = &Interpreter::Return;
            return;
        }

        frame.index = 1;
        register_ = source[1];
    }
    else
    {
        sol::object predicate = register_;
        if (!predicate.is<bool>())
        {
            predicate = OperatorRegistry::get(predicate)
                .map([&](OperatorTable op) {
                    return op.apply(Keyword::Bool, predicate);
                })
                .value_or_eval([] {
                    return lua::make_object(error("invalid operands", "Filter"));
                });
        }

        if (predicate.is<error>())
        {
            register_ = predicate;
            frame.state = &Interpreter::Return;
            return;
        }

        sol::table input = frame.value.as<sol::table>();
        sol::table output = frame.lhs.as<sol::table>();
        if (predicate.as<bool>())
        {
            output[output.size() + 1] = input[frame.index];
        }

        if (++frame.index > input.size())
        {
            register_ = output;
            frame.state = &Interpreter::Return;
            return;
        }

        register_ = input[frame.index];
    }

    frame.state = &Interpreter::ResumeFrame;
    stack.push_front(Frame(child));
}

template <> void Interpreter::Evaluate<Keyword::Count>(Interpreter::Stack&)
{
}

template <> void Interpreter::Evaluate<Keyword::Each>(Interpreter::Stack&)
{
}

} // namespace impl
} // namespace haikan
