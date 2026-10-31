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
        : Expression(encodeNested(impl::Keyword::_Void, {sol::nil}))
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

    ///////////////


    // // Deserialize JSON
    // Expression(boost::json::value const& expr);
    // Expression(boost::json::value && expr);

    // // construct _Literal from JSON init list
    // Expression(std::initializer_list<boost::json::value_ref> items);

    // template <class T>
    // Expression(T&& sample, std::true_type)
    //     : Expression(boost::json::value_from(std::forward<T>(sample)))
    // {
    // }

    // template <class T>
    // Expression(T&& sample, std::false_type)
    // {
    //     static_assert(!std::is_same<T, T>::value, "no conversion defined for T");
    // }

    // template <class T, class = std::enable_if_t<!std::is_base_of<Expression, std::decay_t<T>>::value>>
    // Expression(T&& sample) : Expression(
    //     std::forward<T>(sample),
    //     std::integral_constant<bool, boost::json::has_value_from<T>::value>()
    // )
    // {
    // }


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

    /// Pack expression results into an array. \see haikan::Fork.
    friend Expression operator&(Expression lhs, Expression rhs)
    {
        return Expression(encodeNested(impl::Keyword::Fork, {lhs, rhs}));
    }

    /// Inline named function, equivalent to Fn(link + expr)
    friend Expression operator<<(Expression link, Expression expr);

    /// Flip design-time and eval-time parameters.
    friend Expression operator~(Expression expr)
    {
        return Expression(encodeNested(impl::Keyword::Flip, {expr}));
    }


    friend inline std::ostream& operator<<(std::ostream& os, Expression const& expr);
    // {
    //     return operator<<(os, Expression(expr));
    // }



    //////////////////////
    // ENCODING OBSERVERS
    //////////////////////



    bool has_subexpr() const
    {
        return encoding_view().size() > 1;
    }

    /// Subexpressions
    std::vector<Expression> subexpressions_vec() const
    {
        auto subtrees = encoding_view().children();
        std::vector<Expression> result;
        result.reserve(subtrees.size());
        std::transform(subtrees.cbegin(), subtrees.cend(),
                std::back_inserter(result),
                [](EncodingView const v) { return Expression{v.freeze()}; });
        return result;
    }

    std::vector<Expression> link_parameters() const
    {
        std::vector<Expression> result;

        auto tuple = encoding_view();
        if (tuple.head() != impl::Keyword::Fn) return result;

        for (auto const& s: tuple.children())
        {
            result.emplace_back(s.freeze());
        }
        return result;
    }


    //////////////////
    // DATA OBSERVERS
    //////////////////

    sol::object data() const
    {
        auto const child = encoding_view().child(0);
        auto const a = keyword_attributes(child.head());
        sol::object res;
        if (!child.empty() && ((a & impl::attr::is_literal) || (a & impl::attr::is_preproc)))
        {
            res = *child.if_data();
        }
        else
        {
            res = *encoding_view().if_data();
        }
        return res;
    }


    std::string serialize() const
    {
        std::stringstream ss;
        impl::lua_to_stream(ss, encoding_view().to_object(), 0, false);
        return ss.str();
    }

    std::string prettify() const;
    std::ostream& prettify_to(std::ostream& os) const;
    void prettify_to(char* buff, std::size_t n) const;


    ////////////////////////
    // KEYWORD OBSERVERS
    ////////////////////////
    Keyword keyword() const
    {
        return encoding_view().head();
    }

    boost::json::string_view keyword_to_str() const;

    bool is(Keyword const kwrd) const
    {
        return kwrd == keyword();
    }

    bool is_void() const
    {
        return is(Keyword::_Void);
    }

    bool is_identity() const
    {
        return is(Keyword::Id);
    }

    bool is_compose() const
    {
        return is(Keyword::Pipe);
    }

    bool is_fork() const
    {
        return is(Keyword::Fork);
    }

    bool is_tuple() const
    {
        return is(Keyword::Tuple);
    }

    bool is_literal() const
    {
        return is(Keyword::_Literal);
    }

    bool is_preproc() const
    {
        return is(Keyword::PreProc);
    }

    bool is_link() const
    {
        return is(Keyword::Link);
    }

    bool is_noop() const
    {
        return is(Keyword::Noop);
    }

    bool is_quote() const
    {
        return is(Keyword::Q);
    }

    bool is_error() const
    {
        return is(Keyword::Err);
    }

    /// Internal error identifier.
    /// Returns empty string when is_error() equals false.
    std::string error_id() const;

    bool is_complete_flip() const
    {
        return is(Keyword::Flip) && encoding_view().size() > 1;
    }


    ///////////////////////////
    // ATTR-BASED OBSERVERS
    ///////////////////////////

    bool is_const() const
    {
        return encoding_view().is_const();
    }

    bool is_boolean() const
    {
        return encoding_view().is_boolean();
    }

    bool is_valid_link() const
    {
        auto const self = encoding_view();
        return is(Keyword::Fn) && (self.arity() == 2);
    }

    bool is_infix_pipe() const
    {
        return is_compose() && (encoding_view().arity() > 1);
    }

    bool is_infix_tuple() const
    {
        if (not (is_tuple() && (encoding_view().arity() > 1)))
        {
            return false;
        }
        auto const child = encoding_view().child(0);
        return child.empty() ? false : (child.head() != Keyword::Tuple);
    }

    bool is_infix_fork() const
    {
        return is_fork() && encoding_view().arity() == 2;
    }



    // explicit operator boost::json::value() const
    // {
    //     return to_json();
    // }

    ///////////////////////////
    // Boost.Spirit Karma generator
    ///////////////////////////

    // std::string prettify() const;
    // std::ostream& prettify_to(std::ostream& os) const;

    template <std::size_t N>
    void prettify_to(char (&buff)[N]) const
    {
        return prettify_to(buff, N);
    }

    // void prettify_to(char* buff, std::size_t n) const;

    // friend std::ostream& operator<<(std::ostream& os, Expression const& expr);



    ////////////////////////
    // PREPROCESSING
    ////////////////////////

    /// List of [param, json ptr]
    std::list<std::pair<std::string, std::string>> preprocessing_parameters() const;


    Expression param() const
    {
        auto const child = encoding_view().child(0);
        if (!child.empty())
        {
            return Expression(child.freeze());
        }
        else
        {
            return Expression(encodeNested(Keyword::_Void, {}));
        }
    }


    /// Eval const expressions as Eq(expr), except for Noop,
    // /// otherwise eval expr.
    // bool eval_as_predicate(Expression const& x, Expression& err_sts, EvalContext ctx) const;

    // /// Eval const expressions as Eq(expr), except for Noop,
    // /// otherwise eval expr. Store result in arg reference and return error status.
    // bool eval_as_predicate(sol::object const& x, Expression& err_sts, EvalContext ctx) const;

    // Expression eval_maybe_predicate(Expression const& x, EvalContext ctx) const;

    /// Eval and cast to boolean, return false on error
    bool match(sol::object const& x) const;

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

    static bool to_predicate_if_const(Expression& e);

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
