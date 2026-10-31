#include "haikan/impl/vm.hpp"
#include "haikan/keywords.hpp"
#include "haikan/impl/expression.hpp"
#include "haikan/impl/operator_table.hpp"
#include "haikan/impl/type_registry.hpp"
#include "haikan/impl/keyword_to_str.hpp"

namespace haikan
{
namespace impl
{


void VirtualMachine::Enter()
{
    auto& frame = stack_.front();
    auto const kattr = keyword_attributes(frame.expr.head());
    frame.input_trace = register_;
    frame.state = (kattr & attr::is_hiord)
        ? &VirtualMachine::DispatchHiOrd
        : &VirtualMachine::EvalParam;
}

void VirtualMachine::EvalParam()
{
    auto& frame = stack_.front();
    auto const child = frame.expr.child(0);


    if (child.empty())
    {
        frame.state = &VirtualMachine::ResolveArgs;
    }
    else
    {
        frame.value = std::exchange(
            register_, lua::make_object(sol::nil)
        );

        frame.state = &VirtualMachine::ResumeFrame;
        stack_.push_front(Frame(child));
    }
}

void VirtualMachine::ResumeFrame()
{
    auto& frame = stack_.front();
    auto const kattr = keyword_attributes(frame.expr.head());
    frame.state = (kattr & attr::is_hiord)
        ? (&VirtualMachine::DispatchHiOrd)
        : (&VirtualMachine::ResolveArgs);
}

void VirtualMachine::ResolveArgs()
{
    auto& frame = stack_.front();
    auto const kattr = keyword_attributes(frame.expr.head());
    if (kattr & attr::is_binary)
    {
        // auto const& subs = frame.expr.size();
        auto const child = frame.expr.child(0);
        if (frame.expr.size() > 1) // Parameter was evaluated in a child frame.
        {
            args_[1] = frame.value;
            args_[2] = register_;
            register_ = args_;
        }
        else if (!child.empty()) // Immediate explicit parameter.
        {
            args_[1] = register_;
            // args_[2] = frame.expr.param().data();
            if (child.head() == Keyword::_Literal || child.head() == Keyword::PreProc)
            {
                args_[2] = *child.if_data(); // TODO: check me
            }
            else
            {
                args_[2] = *frame.expr.if_data(); // TODO: check me
            }
            register_ = args_;
        }
        else if (register_ != args_) // No parameter: unpack incoming [lhs, rhs].
        {
            sol::table args = register_.as<sol::table>();
            args_[1] = args[1];
            args_[2] = args[2];
            register_ = args_;
        }
    }

    frame.state = &VirtualMachine::Execute;
}

void VirtualMachine::Execute()
{
    auto& frame = stack_.front();
    frame.state = &VirtualMachine::Return;

    sol::object lhs, rhs;
    if (register_ == args_)
    {
        lhs = args_[1];
        rhs = args_[2];
    }
    else
    {
        lhs = register_;
        rhs = lua::make_object(sol::nil);
    }

    // handle error

    Keyword const k = frame.expr.head();
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


void VirtualMachine::Return()
{
    auto& frame = stack_.front();
    // context_.log << "\n"
    //     << frame.input_trace
    //     << " | " << frame.expr << " -> " << register_
    //     ;

    // TODO: optimize log by allowing encoding view
    context_.log.push(
        Expression(frame.expr.freeze()),
        frame.input_trace,
        register_,
        frame.expr.depth_offset());
    // void push(Expression const& expr, Expression const& x, Expression const& result, std::uint64_t const depth) const;
    stack_.pop_front();
}


void VirtualMachine::Step()
{
    auto& frame = stack_.front();
    (this->*frame.state)();
}


} // namespace impl
} // namespace haikan
