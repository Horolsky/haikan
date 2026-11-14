/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <ostream>
#include <type_traits>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/keyword.hpp"
#include "haikan/impl/lua_stream.hpp"
#include "haikan/impl/encoding.hpp"
#include "haikan/impl/lua_state.hpp"
#include "haikan/impl/expression_parameter.hpp"
// #include "haikan/impl/eval_context.hpp"
#include "haikan/logger.hpp"
#include "haikan/impl/keyword_attributes.hpp"


namespace haikan {
namespace impl {


/// Expression Language implementation class.
/// \details \see <A HREF="/user-guide/expressions/">Expression Language documentation</A>.
class Expression
{

public:

    using Keyword = impl::Keyword;
    using Encoding = impl::Encoding;
    using EncodingView = impl::EncodingView;

    //////////////////
    // CTORS
    //////////////////


    Expression(Expression const&) = default;
    Expression(Expression&&) = default;
    Expression& operator=(Expression const&) = default;
    Expression& operator=(Expression&&) = default;

    explicit Expression(Encoding v)
        : encoding_{v}
    {
    }


    explicit Expression(sol::object x)
        : Expression(Encoding(x))
    {
    }

    template <class T, class U  = typename std::decay<T>::type
        , class = typename std::enable_if<
            !std::is_base_of<Expression, U>::value &&
            !std::is_same<U, impl::Encoding>::value &&
            !std::is_same<U, sol::object>::value &&
            true
    >::type>
    Expression(T&& x)
        : Expression(lua::make_object(std::forward<T>(x)))
    {

    }

    Expression()
        : Expression(encodeNested(impl::Keyword::_Void, {}))
    {
    }

    // explicit Expression(Keyword const keyword);


    virtual ~Expression()
    {
    }

    EncodingView encoding_view() const
    {
        return encoding_;
    }

    boost::json::value to_json() const
    {
        return encoding_view().to_json();
    }

    operator sol::object() const
    {
        return encoding_view().to_object();
    }

    ///////////////


    // // Deserialize JSON
    // Expression(boost::json::value const& expr);
    // Expression(boost::json::value && expr);
    // Expression(std::initializer_list<boost::json::value_ref> items);

    //////////////
    // OPERATORS
    //////////////


    bool operator==(Expression const& o) const
    {
        return (this == &o) || (encoding_view() == o.encoding_view());
    }

    bool operator!=(Expression const& o) const
    {
        return !operator==(o);
    }

    /////////////////////////
    // SUGAR SYNTAX OPERATORS
    /////////////////////////

    /// Pipe expressions left-to-right
    /// \details Pipe functional expressions in composition,
    /// s.t. `a | b` is equivalent to `Pipe(a, b)`. \see haikan::Pipe
    friend Expression operator|(Expression lhs, Expression rhs)
    {
        return Expression::unfold_left_assoc(Keyword::Pipe, std::move(lhs), std::move(rhs));
    }

    /// Pack expression into a tuple without evaluation \see haikan::Tuple.
    friend Expression operator,(Expression lhs, Expression rhs)
    {
        return Expression::unfold_left_assoc(Keyword::Tuple, std::move(lhs), std::move(rhs));
    }

    /// Pack expression results into an array. \see haikan::Juxt.
    friend Expression operator&(Expression lhs, Expression rhs)
    {
        return Expression(encodeNested(impl::Keyword::Juxt, {lhs, rhs}));
    }

    /// Inline named function, equivalent to Fn(link + expr)
    friend Expression operator<<(Expression link, Expression expr);

    /// Flip design-time and eval-time parameters.
    friend Expression operator~(Expression expr)
    {
        return Expression(encodeNested(impl::Keyword::Flip, {expr}));
    }


    friend inline std::ostream& operator<<(std::ostream& os, Expression const& expr);

    std::string prettify() const;
    std::ostream& prettify_to(std::ostream& os) const;
    void prettify_to(char* buff, std::size_t n) const;
    template <std::size_t N>
    void prettify_to(char (&buff)[N]) const
    {
        return prettify_to(buff, N);
    }


    Keyword keyword() const
    {
        return encoding_view().opcode();
    }

    boost::json::string_view keyword_to_str() const;


    // /// \brief Evaluate x to lhs expression.
    // /// \details Equivalent to expr.eval(x).
    // friend V operator*(Expression expr, Expression const& x);

    // /// \brief Evaluate expression.
    // /// \details Equivalent to expr.eval().
    // friend V operator*(Expression expr);


    // Terminal expression
    // static Encoding encodeLiteral(boost::json::value const& params);
    // Terminal expression
    static Expression encodePreProc(std::string && params);

    // Non-terminal expression
    static Encoding encodeNested(Keyword const& keyword, std::initializer_list<Expression> subexpressions);
    static Encoding encodeNested(Keyword const& keyword, std::vector<Expression>&& subexpressions);

    static boost::optional<Expression> to_predicate_if_const(Expression const& e);

    class Args;

  protected:
    Encoding encoding_;

    // mutable std::shared_ptr<Expression> const_predicate_cache_;
  private:

    static Expression unfold_left_assoc(Keyword const keyword, Expression&& lhs, Expression&& rhs);

    template <class T>
    static Encoding encodeNested(Keyword const& keyword, std::move_iterator<T> begin, std::move_iterator<T> const end);

};



// // Boost JSON conversion from Expression
// void tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, Expression const& ev);


// // Keep templates: without indirection of zmbt::reflection the tag_invoke ADL resolves to Expression template ctor for containers

// Boost JSON conversion from Expression
template <class T>
auto tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, T const& expr)
-> std::enable_if_t<std::is_base_of<Expression, std::decay_t<T>>::value>
{
    v = expr.to_json();
}

// Boost JSON conversion to Expression
template <class T>
auto tag_invoke(boost::json::value_to_tag<T> const&, boost::json::value const& v)
-> std::enable_if_t<std::is_base_of<Expression, std::decay_t<T>>::value, T>
{
    return T(v);
}


}  // namespace impl
}  // namespace haikan
