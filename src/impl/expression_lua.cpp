#include <boost/current_function.hpp>
#include "haikan/impl/expression_lua.hpp"
#include "haikan/logger.hpp"


namespace haikan {
namespace impl {

template <class T>
impl::EncodingLua ExpressionLua::encodeNested(impl::Keyword const& keyword, std::move_iterator<T> begin, std::move_iterator<T> const end)
{
    impl::EncodingLua enc {};
    enc.push_back(keyword, 0, sol::nil);

    while(begin != end)
    {
        enc.append_to_root(std::move((*begin++).encoding_));
    }
    return enc;
}

impl::EncodingLua ExpressionLua::encodeNested(Keyword const& keyword, std::initializer_list<ExpressionLua> subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}

impl::EncodingLua ExpressionLua::encodeNested(Keyword const& keyword, std::vector<ExpressionLua>&& subexpressions)
{
    return encodeNested(keyword, std::make_move_iterator(subexpressions.begin()), std::make_move_iterator(subexpressions.end()));
}


ExpressionLua ExpressionLua::unfold_left_assoc(Keyword const keyword, ExpressionLua&& lhs, ExpressionLua&& rhs)
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
        return ExpressionLua(ExpressionLua::encodeNested(keyword, {std::move(lhs), std::move(rhs)}));
    }
}


ExpressionLua operator<<(ExpressionLua link, ExpressionLua referent)
{
    // TODO: binding
    // TODO: handle righ-assoc infix unfold
    static_cast<void>(link);
    static_cast<void>(referent);

    auto const& link_value = link.encoding_.data_[0];
    if (link_value.get_type() != sol::type::string)
    {
        return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Err, {"Symbolic reference shall be a string", BOOST_CURRENT_FUNCTION}));
        // return Err("Symbolic reference shall be a string", BOOST_CURRENT_FUNCTION);
    }
    else if(not is_link_token(link_value))
    {
        return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Err, {"Invalid symbolic reference format", BOOST_CURRENT_FUNCTION}));
        // return Err("Invalid symbolic reference format", BOOST_CURRENT_FUNCTION);
    }
    return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Fn,
        {std::move(link), std::move(referent)}
    ));
}




// ExpressionLua operator|(ExpressionLua lhs, ExpressionLua rhs)
// {
//     return ExpressionLua::unfold_left_assoc(Keyword::Pipe, std::move(lhs), std::move(rhs));
// }

// ExpressionLua operator,(ExpressionLua lhs, ExpressionLua rhs)
// {
//     return ExpressionLua::unfold_left_assoc(Keyword::Tuple, std::move(lhs), std::move(rhs));
// }

// ExpressionLua operator&(ExpressionLua lhs, ExpressionLua rhs)
// {
//     return ExpressionLua(ExpressionLua::encodeNested(Keyword::Fork, {lhs, rhs}));

// }

// ExpressionLua operator<<(ExpressionLua link, ExpressionLua referent)
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
//     return ExpressionLua(ExpressionLua::encodeNested(Keyword::Fn,
//         {std::move(link), std::move(referent)}
//     ));
// }

// ExpressionLua operator~(ExpressionLua expr)
// {
//     return Flip(expr);
// }



// bool ExpressionLua::eval_as_predicate(boost::json::value const& x, ExpressionLua& err_sts, EvalContext ctx) const
// {
//     auto arg = make_literal_argument_view(x);
//     return eval_as_predicate(arg, err_sts, std::move(ctx));
// }

// ExpressionLua ExpressionLua::eval_maybe_predicate(ExpressionLua const& x, EvalContext ctx) const
// {
//     if (!is_noop() && !is_quote() && is_const())
//     {
//         if (!const_predicate_cache_)
//         {
//             auto cached = ExpressionLua(ExpressionLua::encodeNested(
//                 Keyword::Eq,
//                 {ExpressionLua(encoding_view().freeze())}
//             ));
//             const_predicate_cache_ = std::make_shared<ExpressionLua>(std::move(cached));
//         }
//         return const_predicate_cache_->eval_e(x, std::move(ctx));
//     }
//     return eval_e(x, std::move(ctx));
// }


// bool ExpressionLua::match(boost::json::value const& x, Operator const& op) const
// {
//     auto result = eval(x, EvalContext::make(op));
//     auto const if_bool = result.if_bool();
//     return if_bool ? *if_bool : false;
// }

// boost::json::value operator*(ExpressionLua lhs, ExpressionLua const& rhs)
// {
//     return lhs.eval_e(rhs, {}).to_json();
// }

// boost::json::value operator*(ExpressionLua expr)
// {
//     return expr.eval();
// }


}  // namespace impl
}  // namespace haikan
