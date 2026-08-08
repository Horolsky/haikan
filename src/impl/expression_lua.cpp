#include "haikan/expression_lua.hpp"
#include "haikan/logger.hpp"


namespace haikan {

template <class T>
impl::EncodingLua ExpressionLua::encodeNested(impl::Keyword const& keyword, std::move_iterator<T> begin, std::move_iterator<T> const end)
{
    impl::EncodingLua enc {};
    enc.push_back(keyword, 0, sol::nil);

    while(begin != end)
    {
        enc.append_to_root(std::move((*begin++).encoding_view_));
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
    if (!lhs.encoding_view_.keywords.empty() && lhs.encoding_view_.keywords.front() == keyword)
    {
        lhs.encoding_view_.append_to_root(std::move(rhs.encoding_view_));
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

    auto const& link_value = link.encoding_view_.data[0];
    if (link_value.sol_type() != sol::type::string)
    {
        return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Err, {"Symbolic reference shall be a string", BOOST_CURRENT_FUNCTION}));
        // return Err("Symbolic reference shall be a string", BOOST_CURRENT_FUNCTION);
    }
    else if(not link_value.is_link_token())
    {
        return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Err, {"Invalid symbolic reference format", BOOST_CURRENT_FUNCTION}));
        // return Err("Invalid symbolic reference format", BOOST_CURRENT_FUNCTION);
    }
    return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Fn,
        {std::move(link), std::move(referent)}
    ));
}

// ExpressionLua operator~(ExpressionLua expr)
// {
//     return Flip(expr);
// }

}  // namespace haikan
