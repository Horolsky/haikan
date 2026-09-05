/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <initializer_list>
#include <boost/json.hpp>
#include <boost/format.hpp>

#include "haikan/impl/keyword.hpp"
#include "haikan/expression_lua.hpp"
#include "haikan/impl/type_tag.hpp"
#include "haikan/impl/type_info.hpp"



namespace haikan {
namespace impl {

template <Keyword K>
struct LuaSignatureBase : public ExpressionLua
{
    LuaSignatureBase() : ExpressionLua(encodeNested(K, {}))
    {}
};

/// @brief Const expression
/// @tparam K keyword
/// \anchor const-syntactic-forms
/// \paragraph par-const-syntactic-forms Syntactic forms
/// Syntatic form over constant C: \f$E \mapsto (x \mapsto C)\f$
template <Keyword K>
struct LuaSignatureConst : public LuaSignatureBase<K>
{
    using LuaSignatureBase<K>::LuaSignatureBase;

};

/// @brief Unary expression
/// @tparam K keyword
/// \anchor unary-syntactic-forms
/// \paragraph par-unary-syntactic-forms Syntactic forms
/// Syntatic form over function f: \f$E \mapsto (x \mapsto f(x))\f$
template <Keyword K>
struct LuaSignatureUnary : public LuaSignatureBase<K>
{
    using LuaSignatureBase<K>::LuaSignatureBase;
};

/// @brief Binary expression
/// @tparam K keyword
/// \anchor binary-syntactic-forms
/// \paragraph par-binary-syntactic-forms Syntactic forms
/// Syntatic forms over operator `*`:
/// 1. \f$E    \mapsto ([x, y] \mapsto x * y      )\f$
/// 2. \f$E    \mapsto (x      \mapsto x * default)\f$
/// 3. \f$E(y) \mapsto (x      \mapsto x * y      )\f$
///
/// The second form is used in place of first for expressions that have default value defined,
/// see documentation for specific expression.
///
/// In the third form the expression parameter is used as the right-hand side operand.
template <Keyword K>
struct LuaSignatureBinary : public LuaSignatureBase<K>
{
    using LuaSignatureBase<K>::LuaSignatureBase;

    /// \brief Make parametrized expression
    ExpressionLua operator()(ExpressionLua const& param) const
    {
        return ExpressionLua(ExpressionLua::encodeNested(K, {param}));
    }

    ExpressionLua operator()(ExpressionLua && param) const
    {
        return ExpressionLua(ExpressionLua::encodeNested(K, {std::move(param)}));
    }

    /// \brief Make parametrized expression with initializer list
    /// \details Interpret {x} as single-element array instead of using default boost::json::value ctor
    ExpressionLua operator()(std::initializer_list<ExpressionLua> param) const
    {
        auto& state = ExpressionLua::lua_state();
        auto values = state.view().create_table(static_cast<int>(param.size()), 0);
        for (auto const& value : param)
        {
            values.add(value.encoding_view().to_object(state));
        }
        return ExpressionLua(ExpressionLua::encodeNested(K, {ExpressionLua(sol::object(values))}));
    }
};

template <>
struct LuaSignatureBinary<Keyword::PreProc> : public LuaSignatureBase<Keyword::PreProc>
{
    using LuaSignatureBase<Keyword::PreProc>::LuaSignatureBase;

    /// \brief Make parametrized expression
    ExpressionLua operator()(boost::json::string_view const param) const
    {
        return ExpressionLua(ExpressionLua::encodePreProc((boost::format("$[%s]") % param).str().c_str()));
    }

    /// \brief Make parametrized expression
    ExpressionLua operator()(std::size_t const param) const
    {
        return ExpressionLua(ExpressionLua::encodePreProc((boost::format("$[%s]") % param).str().c_str()));
    }
};


/// @brief Variadic expression
/// @tparam K keyword
/// \anchor variadic-syntactic-forms
/// \paragraph par-variadic-syntactic-forms Syntactic forms
/// Syntatic form over function f: \f$E(a,b,c,...) \mapsto (x \mapsto f(a,b,c,...)(x))\f$
template <Keyword K>
struct LuaSignatureVariadic : public LuaSignatureBase<K>
{
  private:
    static ExpressionLua encodeVariadic(std::initializer_list<ExpressionLua> params)
    {
        return ExpressionLua(ExpressionLua::encodeNested(K,  params));
    }

  public:
    using LuaSignatureBase<K>::LuaSignatureBase;

    using E = ExpressionLua;
    ExpressionLua operator()() const {
        return encodeVariadic({});
    }
    ExpressionLua operator()(E const& p0) const {
        return encodeVariadic({p0});
    }
    ExpressionLua operator()(E const& p0, E const& p1) const {
        return encodeVariadic({p0, p1});
    }
    ExpressionLua operator()(E const& p0, E const& p1, E const& p2) const {
        return encodeVariadic({p0, p1, p2});
    }
    ExpressionLua operator()(E const& p0, E const& p1, E const& p2, E const& p3) const {
        return encodeVariadic({p0, p1, p2, p3});
    }
    ExpressionLua operator()(E const& p0, E const& p1, E const& p2, E const& p3, E const& p4) const {
        return encodeVariadic({p0, p1, p2, p3, p4});
    }
    ExpressionLua operator()(E const& p0, E const& p1, E const& p2, E const& p3, E const& p4, E const& p5) const {
        return encodeVariadic({p0, p1, p2, p3, p4, p5});
    }
    ExpressionLua operator()(E const& p0, E const& p1, E const& p2, E const& p3, E const& p4, E const& p5, E const& p6) const {
        return encodeVariadic({p0, p1, p2, p3, p4, p5, p6});
    }
    ExpressionLua operator()(E const& p0, E const& p1, E const& p2, E const& p3, E const& p4, E const& p5, E const& p6, E const& p7) const {
        return encodeVariadic({p0, p1, p2, p3, p4, p5, p6, p7});
    }
    template <class... T>
    ExpressionLua operator()(E const& p0, E const& p1, E const& p2, E const& p3, E const& p4, E const& p5, E const& p6, E const& p7, T&&... rest) const {
        return encodeVariadic({p0, p1, p2, p3, p4, p5, p6, p7, boost::json::value_from(rest)...});
    }
};


struct LuaSignatureOp : public LuaSignatureBase<Keyword::Op>
{
    using LuaSignatureBase<Keyword::Op>::LuaSignatureBase;


    ExpressionLua operator()(ExpressionLua const& type, ExpressionLua const& expr) const
    {
        return ExpressionLua(ExpressionLua::encodeNested(Keyword::Op, {type, expr}));
    }

    template <class T>
    ExpressionLua operator()(type_tag<T> tag, ExpressionLua const& expr) const
    {
        Operator const op{tag};
        return ExpressionLua(ExpressionLua::encodeNested(Keyword::Op, {op.annotation(), expr}));
    }
};


struct LuaSignatureCast : public LuaSignatureBinary<Keyword::Cast>
{
    using LuaSignatureBinary<Keyword::Cast>::LuaSignatureBinary;
    using LuaSignatureBinary<Keyword::Cast>::operator();


    template <class T>
    ExpressionLua operator()(type_tag<T> tag) const
    {
        Operator const op{tag};
        return ExpressionLua(ExpressionLua::encodeNested(Keyword::Cast, {op.annotation()}));
    }
};


struct LuaSignatureErr : public LuaSignatureBase<Keyword::Err>
{
    using LuaSignatureBase<Keyword::Err>::LuaSignatureBase;

    ExpressionLua operator()(boost::json::object payload) const
    {
        return ExpressionLua(ExpressionLua::encodeNested(Keyword::Err, {payload}));
    }

    /// \brief Error message and context
    ExpressionLua operator()(boost::json::string_view msg, boost::json::string_view ctx = "") const
    {
        return make_error("", msg, ctx);
    }


    /// \brief Error type, message, and context
    template <class T>
    ExpressionLua operator()(type_tag<T>, boost::json::string msg = "", boost::json::string ctx = "") const
    {
        static_assert(std::is_base_of<std::exception, T>::value, "Error type is not supported");
        ExpressionLua e = make_error(type_name<T>(), msg, ctx);
        make_throw_action(e.error_id(), [msg]{
            throw T(msg.c_str());
        });
        return std::move(e);
    }

    private:
        void make_throw_action(std::string const key, std::function<void()> f) const;
        ExpressionLua make_error(boost::json::string_view type, boost::json::string_view msg, boost::json::string_view ctx) const;
};


} // namespace impl
} // namespace haikan
