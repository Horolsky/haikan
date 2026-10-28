/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <boost/variant2.hpp>


#include "haikan/keywords_lua.hpp"
#include "haikan/impl/expression_lua.hpp"
#include "haikan/impl/operator_table.hpp"
#include "haikan/impl/type_registry.hpp"
#include "haikan/impl/vm.hpp"

namespace utf = boost::unit_test;
using namespace haikan;
using namespace haikan::impl;


// For ordinary complex parameters:
// Enter → EvalParam → ResumeFrame → ExecuteResolved → Return

/*
[1,2,3,4.25E1] -> (Fold(Add) & Size | Div)

           ┌── Add $ [1,2] = 3
           ├── Add $ [3,3] = 6
           ├── Add $ [6,4.25E1] = 4.85E1
        ┌── Fold(Add) $ [1,2,3,4.25E1] = 4.85E1
        ├── Size $ [1,2,3,4.25E1] = 4
     ┌── Fold(Add) & Size $ [1,2,3,4.25E1] = [4.85E1,4]
     ├── Div $ [4.85E1,4] = 1.2125E1
  □  (Fold(Add) & Size) | Div $ [1,2,3,4.25E1] = 1.2125E1

*/
namespace
{

struct Frame
{
    ExpressionLua expr;
    sol::object value{lua::make_object(sol::nil)};
    sol::object input_trace{lua::make_object(sol::nil)};

    enum class State : uint8_t {
        Enter,
        EvalParam,
        ResumeFrame,
        ResolveArgs,
        Execute,
        HigherOrder,
        Return
    } state;
    std::size_t index{0};


    Frame(ExpressionLua e, sol::object v, State const s)
        : expr{e}
        , value{v}
        , state{s}
    {
    }

    Frame() : Frame(ExpressionLua(), lua::make_object(sol::nil), State::Enter)
    {
    }

    Frame(ExpressionLua e) : Frame(e, lua::make_object(sol::nil), State::Enter)
    {
    }

    Frame(ExpressionLua e, State const s) : Frame(e, lua::make_object(sol::nil), s)
    {
    }

    Frame(Frame const&) = default;
    Frame(Frame &&) = default;
    Frame& operator=(Frame const&) = default;
    Frame& operator=(Frame &&) = default;
    ~Frame() = default;
};

using Stack = std::deque<Frame>;

using Args = haikan::impl::Args;


struct Evaluator {
    haikan::Logger& log_;
    Stack stack_{};
    sol::object register_;
    void step();

    void stepHigherOrder();

    void stepPipe();
    void stepFlip();
    void stepFold();
    void stepFork();

    Evaluator(haikan::Logger& l, sol::object arg, ExpressionLua expr)
        : log_{l}, register_{std::move(arg)}
    {
        stack_.push_front(Frame{std::move(expr)});
    }


    sol::object execute(Keyword const, sol::object const) const;
};




sol::object Evaluator::execute(Keyword const k, sol::object const a) const
{
    sol::object lhs, rhs;
    if (a.is<Args>())
    {
        Args& args = a.as<Args&>();
        lhs = args.lhs;
        rhs = args.rhs;
    }
    else
    {
        lhs = a;
        rhs = lua::make_object(sol::nil);
    }

    auto maybe_op = OperatorRegistry::get(lhs, rhs);
    sol::object result = lua::make_object(error("keyword not found", "execute"));
    switch (k)
    {
    case Keyword::Add:  result = maybe_op.value().apply(Keyword::Add, lhs, rhs); break;
    case Keyword::Div:  result = maybe_op.value().apply(Keyword::Div, lhs, rhs); break;
    case Keyword::Size: result = lua::make_object(lhs.as<sol::table>().size()); break;
    default:
        break;
    }
    return result;
}

void Evaluator::step()
{
    using State = Frame::State;
    auto& frame = stack_.front();
    ExpressionLua const& expr = frame.expr;
    auto const kattr = keyword_attributes(expr.keyword());


    switch (frame.state)
    {
    case State::Enter:
    {
        // if (trace enables)
        {
            frame.input_trace = register_;
        }
        frame.state = (kattr & attr::is_hiord)
            ? State::HigherOrder
            : State::EvalParam;
        break;
    }

    case State::EvalParam:
    {
        auto subs = expr.subexpressions_vec();
        if (subs.empty())
        {
            frame.state = State::ResolveArgs;
        }
        else
        {
            frame.value = std::exchange(
                register_, lua::make_object(sol::nil)
            );

            frame.state = State::ResumeFrame;
            stack_.push_front(Frame(subs.front()));
        }


        break;
    }

    case State::ResumeFrame:
    {
        frame.state = (kattr & attr::is_hiord)
            ? (State::HigherOrder)
            : (State::ResolveArgs);
        break;
    }

    case State::ResolveArgs:
    {

        if (kattr & attr::is_binary)
        {
            auto const& subs = expr.subexpressions_vec();

            if (!subs.empty())
            {
                // Parameter was evaluated in a child frame.
                register_ = lua::make_object(Args{
                    std::move(frame.value),
                    std::move(register_)
                });
            }
            else if (!expr.param().is(Keyword::_Void))
            {
                // Immediate explicit parameter.
                register_ = lua::make_object(Args{
                    std::move(register_),
                    expr.param().data()
                });
            }
            else if (!register_.is<Args>())
            {
                // No parameter: unpack incoming [lhs, rhs].
                sol::table args = register_.as<sol::table>();

                // Validate that args contains exactly two elements.

                register_ = lua::make_object(Args{
                    args[1],
                    args[2]
                });
            }
        }

        frame.state = State::Execute;
        break;
    }

    case State::Execute:
    {
        register_ = execute(expr.keyword(), register_);
        frame.state = State::Return;
        break;
    }
    case State::HigherOrder:
    {
        stepHigherOrder();
        break;
    }
    case State::Return:
    {
        log_ << '\n'
            << frame.input_trace
            << " | " << expr << " -> " << register_
            ;
        // auto log = HAIKAN_LOG(INFO)
        //     << frame.input_trace
        //     << " | " << expr << " -> " << register_
        //     ;

        stack_.pop_front();
        break;
    }
    }
}


void Evaluator::stepHigherOrder()
{
    auto& frame = stack_.front();
    ExpressionLua const& expr = frame.expr;

    switch(expr.keyword())
    {
    case Keyword::Pipe: stepPipe(); break;
    case Keyword::Flip: stepFlip(); break;
    case Keyword::Fold: stepFold(); break;
    case Keyword::Fork: stepFork(); break;
    default: break;
    }
}


void Evaluator::stepFlip()
{
    using State = Frame::State;

    auto& frame = stack_.front();
    ExpressionLua const child = frame.expr.subexpressions_vec().front();

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
            frame.state = State::ResumeFrame;
            stack_.push_front(Frame(child.subexpressions_vec().front()));
        }
        break;

    case 1:
        // Parameter is resolved: swap operand positions.
        register_ = lua::make_object(Args{
            std::exchange(register_, lua::make_object(sol::nil)),
            std::exchange(frame.value, lua::make_object(sol::nil))
        });

        frame.index = 2;
        frame.state = State::ResumeFrame;

        // Skip Enter: operands are already prepared.
        stack_.push_front(Frame(child, State::Execute));
        break;

    case 2:
        // Child returned its result in register_.
        frame.state = State::Return;
        break;
    }
}


void Evaluator::stepPipe()
{
    auto& frame = stack_.front();
    ExpressionLua const& expr = frame.expr;

    auto const& subs = expr.subexpressions_vec();

    if (frame.index < subs.size())
    {
        auto const& child = subs[frame.index++];

        frame.state = Frame::State::ResumeFrame;
        stack_.push_front(Frame(child));
    }
    else
    {
        frame.state = Frame::State::Return;
    }
}


void Evaluator::stepFold()
{
    auto& frame = stack_.front();
    ExpressionLua const& expr = frame.expr;

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
            frame.state = Frame::State::Return;
        }
    }

    if (frame.state != Frame::State::Return)
    {
        sol::table source = frame.value.as<sol::table>();

        if (frame.index < source.size())
        {
            sol::object accumulator = register_;
            sol::object element = source[1+(frame.index++)];

            register_ = lua::make_object(Args{
                std::move(accumulator),
                std::move(element)
            });

            frame.state = Frame::State::ResumeFrame;
            ExpressionLua function = expr.subexpressions_vec().front();
            stack_.push_front(Frame(function));
        }
        else
        {
            frame.state = Frame::State::Return;
        }
    }
}


void Evaluator::stepFork()
{
    auto& frame = stack_.front();
    ExpressionLua const& expr = frame.expr;
    auto const& subs = expr.subexpressions_vec();

    if (frame.index == 0)
    {
        // Preserve original input and initialize output array.
        frame.value = lua::make_object(Args{
            register_,
            lua::create_table(subs.size(), 0)
        });
    }
    else
    {
        // Collect result of the previously evaluated branch.
        Args& saved = frame.value.as<Args&>();

        saved.rhs.as<sol::table>()[frame.index] = register_;
    }

    if (frame.index < subs.size())
    {
        Args& saved = frame.value.as<Args&>();

        // Every branch receives the original input.
        register_ = saved.lhs;

        frame.state = Frame::State::ResumeFrame;
        stack_.push_front(Frame(subs[frame.index++]));
    }
    else
    {
        // All branches completed.
        Args& saved = frame.value.as<Args&>();

        register_ = std::move(saved.rhs);
        frame.state = Frame::State::Return;
    }
}

} // namespace


BOOST_AUTO_TEST_CASE(Demo)
{
    // [1,2,3,4.25E1] -> (Fold(Add) & Size | Div)

    using namespace haikan;

    OperatorRegistry()
        .AddSignature(type<double>, template_list<std::vector, std::set>)
        .Connects(type<int>, template_list<std::vector, std::set>)
        ;
    impl::TypeRegistry tr{};
        tr.Add(type<Args>);
        tr.Add(type<std::vector<double>>);
        tr.init(lua::global_state());


    ExpressionLua expr = Fold(Add) & Size | Div;
    sol::object arg = lua::make_object(std::vector<double> {1,2,3,4.25E1});

    std::stringstream ss;
    {
        Logger log;
        Evaluator e {log, arg, expr};
        while (!e.stack_.empty())
        {
            e.step();
        }
        log.drain(ss, true);
        BOOST_ASSERT(e.register_.is<double>());
        BOOST_CHECK_EQUAL(e.register_.as<double>(), 1.2125E1);
    }
    auto expected = R"(
{1.0, 2.0} | Add -> 3.0
{3.0, 3.0} | Add -> 6.0
{6.0, 42.5} | Add -> 48.5
{1.0,2.0,3.0,42.5} | Fold(Add) -> 48.5
{1.0,2.0,3.0,42.5} | Size -> 4
{1.0,2.0,3.0,42.5} | Fold(Add) & Size -> {48.5,4}
{48.5,4} | Div -> 12.125
{1.0,2.0,3.0,42.5} | (Fold(Add) & Size) | Div -> 12.125
)";

    BOOST_CHECK_EQUAL(ss.str(), expected);


}
