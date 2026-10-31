/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <boost/describe.hpp>


#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/lua_state.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/logger.hpp"
#include "haikan/impl/type_registry.hpp"


namespace haikan {
namespace impl {


class VirtualMachine
{


    using State = void (VirtualMachine::*)();

    void Enter();
    void EvalParam();
    void ResumeFrame();
    void ResolveArgs();
    void Execute();
    void DispatchHiOrd();
    void Return();

    void Step();

    void stepPipe();
    void stepFlip();
    void stepFold();
    void stepFork();

    sol::object Dispatch(Keyword const k, sol::object const& lhs, sol::object const& rhs) const;


    struct Frame
    {
        EncodingView expr;
        sol::object value{lua::make_object(sol::nil)};
        sol::object input_trace{lua::make_object(sol::nil)};
        State state {&VirtualMachine::Enter};
        std::size_t index{0};


        Frame(EncodingView e, sol::object v, State const s)
            : expr{e}
            , value{v}
            , state{s}
        {
        }

        Frame() : Frame(EncodingView(), lua::make_object(sol::nil), &VirtualMachine::Enter) {}
        Frame(EncodingView e) : Frame(e, lua::make_object(sol::nil), &VirtualMachine::Enter) {}
        Frame(EncodingView e, State const s) : Frame(e, lua::make_object(sol::nil), s) {}

        Frame(Frame const&) = default;
        Frame(Frame &&) = default;
        Frame& operator=(Frame const&) = default;
        Frame& operator=(Frame &&) = default;
        ~Frame() = default;
    };

public:
    using Stack = std::deque<Frame>;

    VirtualMachine(sol::object arg, EncodingView expr, EvalContext ctx = {})
        : register_{std::move(arg)}, context_{ctx}
    {
        stack_.push_front(Frame{expr});
        args_ = lua::create_table(2, 0);
    }

    VirtualMachine(sol::object arg, Expression const& expr, EvalContext ctx = {})
        : VirtualMachine(arg, expr.encoding_view(), ctx)
    {
    }

    sol::object evaluate()
    {
        while (!stack_.empty())
        {
            Step();
        }
        return register_;
    }

private:

    sol::object register_;
    EvalContext context_;
    sol::table args_;
    Stack stack_{};

    template <Keyword K>
    sol::object Evaluate(sol::object const& lhs, sol::object const& rhs) const;

    // Higher Order eval
    template <Keyword K>
    void Evaluate(VirtualMachine::Stack& stack);
};

} // namespace impl
} // namespace haikan
