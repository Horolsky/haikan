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

#include "haikan/impl/operator.hpp"
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/lua_json_conversion.hpp"
#include "haikan/impl/encoding_lua.hpp"
#include "haikan/impl/lua_state.hpp"
#include "haikan/impl/lazy_lua_object.hpp"
#include "haikan/impl/eval_context.hpp"
#include "haikan/logger.hpp"
#include "haikan/impl/keyword_attributes.hpp"


namespace haikan {

/// Expression Language implementation class.
/// \details \see <A HREF="/user-guide/expressions/">Expression Language documentation</A>.
class ExpressionLua
{
public:

    using Keyword = impl::Keyword;
    using EncodingLua = impl::EncodingLua;

    static impl::LuaState& lua_state()
    {
        static impl::LuaState state {};
        return state;
    }

    //////////////////
    // CTORS
    //////////////////


    ExpressionLua(ExpressionLua const&) = default;
    ExpressionLua(ExpressionLua&&) = default;
    ExpressionLua& operator=(ExpressionLua const&) = default;
    ExpressionLua& operator=(ExpressionLua&&) = default;

    explicit ExpressionLua(EncodingLua v)
        : encoding_view_{v}
    {
    }

    explicit ExpressionLua(impl::LazyLuaObject x)
        : ExpressionLua(EncodingLua(x))
    {
    }

    explicit ExpressionLua(sol::object x)
        : ExpressionLua(EncodingLua(x))
    {
    }

    template <class T, class = typename std::enable_if<
        !std::is_base_of<ExpressionLua, typename std::decay<T>::type>::value &&
        !std::is_same<typename std::decay<T>::type, impl::EncodingLua>::value &&
        !std::is_same<typename std::decay<T>::type, impl::LazyLuaObject>::value &&
        !std::is_same<typename std::decay<T>::type, sol::object>::value
    >::type>
    ExpressionLua(T&& x)
        : ExpressionLua(impl::LazyLuaObject{std::forward<T>(x)})
    {

    }

    // explicit ExpressionLua(Keyword const keyword);


    virtual ~ExpressionLua() = default;

    EncodingLua encoding_view() const
    {
        return encoding_view_;
    }

    boost::json::value to_json() const
    {
        return encoding_view().to_json(lua_state());
    }


    ///////////////


    // // Deserialize JSON
    // ExpressionLua(boost::json::value const& expr);
    // ExpressionLua(boost::json::value && expr);

    // // construct _Literal from JSON init list
    // ExpressionLua(std::initializer_list<boost::json::value_ref> items);

    // template <class T>
    // ExpressionLua(T&& sample, std::true_type)
    //     : ExpressionLua(boost::json::value_from(std::forward<T>(sample)))
    // {
    // }

    // template <class T>
    // ExpressionLua(T&& sample, std::false_type)
    // {
    //     static_assert(!std::is_same<T, T>::value, "no conversion defined for T");
    // }

    // template <class T, class = std::enable_if_t<!std::is_base_of<ExpressionLua, std::decay_t<T>>::value>>
    // ExpressionLua(T&& sample) : ExpressionLua(
    //     std::forward<T>(sample),
    //     std::integral_constant<bool, boost::json::has_value_from<T>::value>()
    // )
    // {
    // }


    //////////////
    // OPERATORS
    //////////////


    bool operator==(ExpressionLua const& o) const
    {
        return (this == &o) || (encoding_view() == o.encoding_view());
    }

    bool operator!=(ExpressionLua const& o) const
    {
        return !operator==(o);
    }

    /////////////////////////
    // SUGAR SYNTAX OPERATORS
    /////////////////////////

    /// Pipe expressions left-to-right
    /// \details Pipe functional expressions in composition,
    /// s.t. `a | b` is equivalent to `Pipe(a, b)`. \see haikan::Pipe
    friend ExpressionLua operator|(ExpressionLua lhs, ExpressionLua rhs)
    {
        return ExpressionLua::unfold_left_assoc(Keyword::Pipe, std::move(lhs), std::move(rhs));
    }

    /// Pack expression into a tuple without evaluation \see haikan::Tuple.
    friend ExpressionLua operator,(ExpressionLua lhs, ExpressionLua rhs)
    {
        return ExpressionLua::unfold_left_assoc(Keyword::Tuple, std::move(lhs), std::move(rhs));
    }

    /// Pack expression results into an array. \see haikan::Fork.
    friend ExpressionLua operator&(ExpressionLua lhs, ExpressionLua rhs)
    {
        return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Fork, {lhs, rhs}));
    }

    /// Inline named function, equivalent to Fn(link + expr)
    friend ExpressionLua operator<<(ExpressionLua link, ExpressionLua expr);

    friend ExpressionLua operator~(ExpressionLua expr)
    {
        return ExpressionLua(ExpressionLua::encodeNested(impl::Keyword::Flip, {expr}));
    }


    friend inline std::ostream& operator<<(std::ostream& os, ExpressionLua const& expr);
    // {
    //     return operator<<(os, ExpressionLua(expr));
    // }


    /// Flip design-time and eval-time parameters.

    //////////////////////
    // ENCODING OBSERVERS
    //////////////////////



    bool has_subexpr() const
    {
        return encoding_view().size() > 1;
    }

    /// Subexpressions
    std::vector<ExpressionLua> subexpressions_list() const
    {
        auto subtrees = encoding_view().children();
        std::vector<ExpressionLua> result;
        result.reserve(subtrees.size());
        std::transform(subtrees.cbegin(), subtrees.cend(),
                std::back_inserter(result),
                [](EncodingLua const& v) { return ExpressionLua{v}; });
        return result;
    }

    std::vector<ExpressionLua> link_parameters() const
    {
        std::vector<ExpressionLua> result;

        auto tuple = encoding_view();
        if (tuple.head() != impl::Keyword::Fn) return result;

        for (auto const& s: tuple.children())
        {
            result.emplace_back(s);
        }
        return result;
    }


    //////////////////
    // DATA OBSERVERS
    //////////////////

    sol::object data() const
    {
        return data(lua_state());
    }

    sol::object data(sol::state_view sv) const
    {
        auto const child = encoding_view().child(0);
        auto const a = keyword_attributes(child.head());
        if (!child.empty() && ((a & impl::attr::is_literal) || (a & impl::attr::is_preproc)))
        {
            return child.data.front().load(sv);
        }
        return encoding_view().data.front().load(sv);
    }


    std::string serialize() const
    {
        std::stringstream ss;
        impl::lua_to_stream(ss, encoding_view().to_object(lua_state()), 0, false);
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
        auto const& kw = encoding_view().child(0).keywords;
        return kw.empty() ? false : (kw.front() != Keyword::Tuple);
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

    // friend std::ostream& operator<<(std::ostream& os, ExpressionLua const& expr);



    ////////////////////////
    // PREPROCESSING
    ////////////////////////

    /// List of [param, json ptr]
    std::list<std::pair<std::string, std::string>> preprocessing_parameters() const;


    ///////////////////
    // EVALUATORS
    ///////////////////

    /// @brief Evaluate expression
    /// @param x run-time argument
    /// @param ctx evaluation context
    /// @return
    ExpressionLua eval_e(ExpressionLua const& x, impl::EvalContext ctx) const;


    /// Eval const expressions as Eq(expr), except for Noop,
    /// otherwise eval expr.
    bool eval_as_predicate(ExpressionLua const& x, ExpressionLua& err_sts, impl::EvalContext ctx) const;

    /// Eval const expressions as Eq(expr), except for Noop,
    /// otherwise eval expr. Store result in arg reference and return error status.
    bool eval_as_predicate(sol::object const& x, ExpressionLua& err_sts, impl::EvalContext ctx) const;

    ExpressionLua eval_maybe_predicate(ExpressionLua const& x, impl::EvalContext ctx) const;

    /// @brief Evaluate expression
    /// @param x run-time argument
    /// @param ctx evaluation context
    /// @return
    sol::object eval(sol::object const& x = sol::nil, impl::EvalContext ctx = {}) const;

    /// Eval and cast to boolean, return false on error
    bool match(sol::object const& x, impl::Operator const& op = {}) const;

    // /// \brief Evaluate x to lhs expression.
    // /// \details Equivalent to expr.eval(x).
    // friend V operator*(ExpressionLua expr, ExpressionLua const& x);

    // /// \brief Evaluate expression.
    // /// \details Equivalent to expr.eval().
    // friend V operator*(ExpressionLua expr);


    // Terminal expression
    static EncodingLua encodeLiteral(boost::json::value const& params);
    // Terminal expression
    static EncodingLua encodePreProc(boost::json::value const& params);

    // Non-terminal expression
    static EncodingLua encodeNested(Keyword const& keyword, std::initializer_list<ExpressionLua> subexpressions);
    static EncodingLua encodeNested(Keyword const& keyword, std::vector<ExpressionLua>&& subexpressions);

    static bool to_predicate_if_const(ExpressionLua& e);


  protected:
    EncodingLua encoding_view_;
    // mutable std::shared_ptr<ExpressionLua> const_predicate_cache_;
  private:

    static ExpressionLua unfold_left_assoc(Keyword const keyword, ExpressionLua&& lhs, ExpressionLua&& rhs);

    template <class T>
    static EncodingLua encodeNested(Keyword const& keyword, std::move_iterator<T> begin, std::move_iterator<T> const end);

};



// // Boost JSON conversion from ExpressionLua
// void tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, ExpressionLua const& ev);


// // Keep templates: without indirection of zmbt::reflection the tag_invoke ADL resolves to ExpressionLua template ctor for containers

// // Boost JSON conversion from ExpressionLua
// template <class T>
// auto tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, T const& expr)
// -> std::enable_if_t<std::is_base_of<ExpressionLua, std::decay_t<T>>::value>
// {
//     v = expr.to_json();
// }

// // Boost JSON conversion to ExpressionLua
// template <class T>
// auto tag_invoke(boost::json::value_to_tag<T> const&, boost::json::value const& v)
// -> std::enable_if_t<std::is_base_of<ExpressionLua, std::decay_t<T>>::value, T>
// {
//     return T(v);
// }

}  // namespace haikan
