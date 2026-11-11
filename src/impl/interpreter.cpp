#include "haikan/impl/interpreter.hpp"
#include "haikan/keywords.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/operator_table.hpp"
#include "haikan/impl/type_registry.hpp"
#include "haikan/impl/keyword.hpp"

namespace haikan
{
namespace impl
{


void Interpreter::Enter()
{
    auto& frame = stack_.front();
    auto const kattr = keyword_attributes(frame.expr.opcode());
    frame.input_trace = register_;
    frame.state = (kattr & attr::is_hiord)
        ? &Interpreter::DispatchHiOrd
        : &Interpreter::EvalParam;
}

void Interpreter::EvalParam()
{
    auto& frame = stack_.front();
    auto const child = frame.expr.child(0);


    if (child.empty())
    {
        frame.state = &Interpreter::ResolveArgs;
    }
    else
    {
        frame.value = std::exchange(
            register_, lua::make_object(sol::nil)
        );

        frame.state = &Interpreter::ResumeFrame;
        stack_.push_front(Frame(child));
    }
}

void Interpreter::ResumeFrame()
{
    auto& frame = stack_.front();
    auto const kattr = keyword_attributes(frame.expr.opcode());
    frame.state = (kattr & attr::is_hiord)
        ? (&Interpreter::DispatchHiOrd)
        : (&Interpreter::ResolveArgs);
}

void Interpreter::ResolveArgs()
{
    auto& frame = stack_.front();
    auto const child = frame.expr.child(0);
    auto const kattr = keyword_attributes(frame.expr.opcode());
    if (kattr & (attr::is_literal | attr::is_preproc))
    {
        frame.lhs = register_;
        frame.rhs = frame.expr.data();
        frame.args_resolved = true;
    }
    else if (kattr & attr::is_binary)
    {
        if (!child.empty())
        {
            frame.lhs = frame.value;
            frame.rhs = register_;
        }
        else if (register_.get_type() == sol::type::table
                 || (register_.get_type() == sol::type::userdata
                     && !register_.is<error>()))
        {
            sol::table const args = register_.as<sol::table>();
            frame.lhs = args[1];
            frame.rhs = args[2];
        }
        else
        {
            frame.lhs = register_;
            frame.rhs = lua::make_object(sol::nil);
        }
        frame.args_resolved = true;
    }

    frame.state = &Interpreter::Execute;
}

void Interpreter::Execute()
{
    auto& frame = stack_.front();
    frame.state = &Interpreter::Return;

    sol::object lhs, rhs;
    if (frame.args_resolved)
    {
        lhs = frame.lhs;
        rhs = frame.rhs;
    }
    else
    {
        lhs = register_;
        rhs = lua::make_object(sol::nil);
    }

    // handle error

    Keyword const k = frame.expr.opcode();
    auto const kattr = keyword_attributes(k);

    if (kattr & attr::is_operator)
    {
        register_ = OperatorRegistry::get(lhs, rhs)
            .map([&](OperatorTable op){ return op.apply(k, lhs, rhs); })
            .value_or_eval([k](){
                return lua::make_object(error("invalid operands", keyword_to_str(k)));
            });
    }
    else
    {
        register_ = Dispatch(k, lhs, rhs);
    }
}


void Interpreter::Return()
{
    auto& frame = stack_.front();
    Keyword const k = frame.expr.opcode();
    auto const kattr = keyword_attributes(k);
    if (not (kattr & (attr::is_literal | attr::is_const | attr::is_quote)))
    {
    // TODO: optimize log by allowing encoding view
        context_.log.push(
            Expression(frame.expr.freeze()),
            frame.input_trace,
            register_,
            frame.expr.depth_offset());
    }
    // void push(Expression const& expr, Expression const& x, Expression const& result, std::uint64_t const depth) const;
    stack_.pop_front();
}


void Interpreter::Step()
{
    auto& frame = stack_.front();
    (this->*frame.state)();
}


} // namespace impl
} // namespace haikan
