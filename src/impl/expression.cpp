#include <boost/current_function.hpp>
#include "haikan/impl/expression.hpp"
#include "haikan/logger.hpp"


namespace haikan {
namespace impl {

template <class T>
impl::Encoding Expression::encodeNested(impl::Keyword const& keyword, std::move_iterator<T> begin, std::move_iterator<T> const end)
{
    impl::Encoding enc {};
    enc.push_back(keyword, 0, sol::nil);

    while(begin != end)
    {
        enc.append_to_root(std::move((*begin++).encoding_));
    }
    return enc;
}

impl::Encoding Expression::encodeNested(Keyword const& keyword, std::initializer_list<Expression> subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}

impl::Encoding Expression::encodeNested(Keyword const& keyword, std::vector<Expression>&& subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}


Expression Expression::unfold_left_assoc(Keyword const keyword, Expression&& lhs, Expression&& rhs)
{

    // TODO: add SIOF guard here (can be detected by empty encoding_)
    // It may occur if expr builtin keywords are used in complex expressions with
    // static storage, s.t. this function is evaluated before operands initialization.
    if (!lhs.encoding_.keywords_.empty() && lhs.encoding_.keywords_.front() == keyword)
    {
        lhs.encoding_.append_to_root(std::move(rhs.encoding_));
        return lhs;
    }
    else
    {
        return Expression(Expression::encodeNested(keyword, {std::move(lhs), std::move(rhs)}));
    }
}


Expression operator<<(Expression link, Expression referent)
{
    // TODO: binding
    // TODO: handle righ-assoc infix unfold
    static_cast<void>(link);
    static_cast<void>(referent);

    auto const& link_value = link.encoding_.data_[0];
    if (link_value.get_type() != sol::type::string)
    {
        return Expression(Expression::encodeNested(impl::Keyword::Err, {"Symbolic reference shall be a string", BOOST_CURRENT_FUNCTION}));
        // return Err("Symbolic reference shall be a string", BOOST_CURRENT_FUNCTION);
    }
    else if(not is_link_token(link_value))
    {
        return Expression(Expression::encodeNested(impl::Keyword::Err, {"Invalid symbolic reference format", BOOST_CURRENT_FUNCTION}));
        // return Err("Invalid symbolic reference format", BOOST_CURRENT_FUNCTION);
    }
    return Expression(Expression::encodeNested(impl::Keyword::Fn,
        {std::move(link), std::move(referent)}
    ));
}




// Expression operator|(Expression lhs, Expression rhs)
// {
//     return Expression::unfold_left_assoc(Keyword::Pipe, std::move(lhs), std::move(rhs));
// }

// Expression operator,(Expression lhs, Expression rhs)
// {
//     return Expression::unfold_left_assoc(Keyword::Tuple, std::move(lhs), std::move(rhs));
// }

// Expression operator&(Expression lhs, Expression rhs)
// {
//     return Expression(Expression::encodeNested(Keyword::Fork, {lhs, rhs}));

// }

// Expression operator<<(Expression link, Expression referent)
// {
//     // TODO: binding
//     // TODO: handle righ-assoc infix unfold
//     static_cast<void>(link);
//     static_cast<void>(referent);

//     auto const if_string = link.if_string();
//     if (!if_string)
//     {
//         return Err("Symbolic reference shall be a string", BOOST_CURRENT_FUNCTION);
//     }
//     else if(not Encoding::is_link_token(*if_string))
//     {
//         return Err("Invalid symbolic reference format", BOOST_CURRENT_FUNCTION);
//     }
//     return Expression(Expression::encodeNested(Keyword::Fn,
//         {std::move(link), std::move(referent)}
//     ));
// }

// Expression operator~(Expression expr)
// {
//     return Flip(expr);
// }



// bool Expression::eval_as_predicate(boost::json::value const& x, Expression& err_sts, EvalContext ctx) const
// {
//     auto arg = make_literal_argument_view(x);
//     return eval_as_predicate(arg, err_sts, std::move(ctx));
// }

// Expression Expression::eval_maybe_predicate(Expression const& x, EvalContext ctx) const
// {
//     if (!is_noop() && !is_quote() && is_const())
//     {
//         if (!const_predicate_cache_)
//         {
//             auto cached = Expression(Expression::encodeNested(
//                 Keyword::Eq,
//                 {Expression(encoding_view().freeze())}
//             ));
//             const_predicate_cache_ = std::make_shared<Expression>(std::move(cached));
//         }
//         return const_predicate_cache_->eval_e(x, std::move(ctx));
//     }
//     return eval_e(x, std::move(ctx));
// }


// bool Expression::match(boost::json::value const& x, Operator const& op) const
// {
//     auto result = eval(x, EvalContext::make(op));
//     auto const if_bool = result.if_bool();
//     return if_bool ? *if_bool : false;
// }

// boost::json::value operator*(Expression lhs, Expression const& rhs)
// {
//     return lhs.eval_e(rhs, {}).to_json();
// }

// boost::json::value operator*(Expression expr)
// {
//     return expr.eval();
// }


}  // namespace impl
}  // namespace haikan
