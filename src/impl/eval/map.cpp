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

namespace {

sol::table copy_sequence(sol::object const& value)
{
    sol::table input = value.as<sol::table>();
    auto const size = input.size();
    sol::table copy = lua::create_table(size, 0);
    for (std::size_t i = 1; i <= size; ++i)
    {
        copy[i] = input[i];
    }
    return copy;
}

sol::object to_predicate(sol::object predicate, char const* keyword)
{
    if (predicate.is<bool>())
    {
        return predicate;
    }

    return OperatorRegistry::get(predicate)
        .map([&](OperatorTable op) {
            return op.apply(Keyword::Bool, predicate);
        })
        .value_or_eval([keyword] {
            return lua::make_object(error("invalid operands", keyword));
        });
}

} // namespace

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
        sol::table source = copy_sequence(register_);
        auto const size = source.size();
        frame.value = source;
        frame.lhs = lua::create_table(size, 0);

        if (0 == size)
        {
            register_ = frame.lhs;
            frame.state = &Interpreter::Return;
            return;
        }

        frame.index = 1;
        register_ = source[1];
    }
    else
    {
        sol::object predicate = to_predicate(register_, "Filter");
        if (predicate.is<error>())
        {
            register_ = predicate;
            frame.state = &Interpreter::Return;
            return;
        }

        sol::table input = frame.value.as<sol::table>();
        if (predicate.as<bool>())
        {
            sol::table output = frame.lhs.as<sol::table>();
            output[output.size() + 1] = input[frame.index];
        }

        if (++frame.index > input.size())
        {
            register_ = frame.lhs;
            frame.state = &Interpreter::Return;
            return;
        }

        register_ = input[frame.index];
    }

    frame.state = &Interpreter::ResumeFrame;
    stack.push_front(Frame(child));
}

template <> void Interpreter::Evaluate<Keyword::Count>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(0);

    if (HAIKAN_UNLIKELY(child.empty()))
    {
        register_ = lua::make_object(std::size_t{0});
        frame.state = &Interpreter::Return;
        return;
    }

    if (HAIKAN_UNLIKELY(0 == frame.index))
    {
        sol::table source = copy_sequence(register_);
        frame.value = source;
        frame.lhs = lua::make_object(std::size_t{0});

        if (0 == source.size())
        {
            register_ = frame.lhs;
            frame.state = &Interpreter::Return;
            return;
        }

        frame.index = 1;
        register_ = source[1];
    }
    else
    {
        sol::object predicate = to_predicate(register_, "Count");
        if (predicate.is<error>())
        {
            register_ = predicate;
            frame.state = &Interpreter::Return;
            return;
        }

        sol::table input = frame.value.as<sol::table>();
        if (predicate.as<bool>())
        {
            frame.lhs = lua::make_object(frame.lhs.as<std::size_t>() + 1);
        }

        if (++frame.index > input.size())
        {
            register_ = frame.lhs;
            frame.state = &Interpreter::Return;
            return;
        }

        register_ = input[frame.index];
    }

    frame.state = &Interpreter::ResumeFrame;
    stack.push_front(Frame(child));
}

template <> void Interpreter::Evaluate<Keyword::Each>(Interpreter::Stack& stack)
{
    auto& frame = stack.front();
    auto const child = frame.expr.child(0);

    if (HAIKAN_UNLIKELY(child.empty()))
    {
        register_ = lua::make_object(true);
        frame.state = &Interpreter::Return;
        return;
    }

    if (HAIKAN_UNLIKELY(0 == frame.index))
    {
        sol::table source = copy_sequence(register_);
        frame.value = source;

        if (0 == source.size())
        {
            register_ = lua::make_object(true);
            frame.state = &Interpreter::Return;
            return;
        }

        frame.index = 1;
        register_ = source[1];
    }
    else
    {
        sol::object predicate = to_predicate(register_, "Each");
        if (predicate.is<error>())
        {
            register_ = predicate;
            frame.state = &Interpreter::Return;
            return;
        }

        if (!predicate.as<bool>())
        {
            register_ = lua::make_object(false);
            frame.state = &Interpreter::Return;
            return;
        }

        sol::table input = frame.value.as<sol::table>();
        if (++frame.index > input.size())
        {
            register_ = lua::make_object(true);
            frame.state = &Interpreter::Return;
            return;
        }

        register_ = input[frame.index];
    }

    frame.state = &Interpreter::ResumeFrame;
    stack.push_front(Frame(child));
}

} // namespace impl
} // namespace haikan
