/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */


#include "haikan/impl/expression.hpp"
#include "haikan/error.hpp"
#include "haikan/impl/keyword_to_str.hpp"

namespace haikan {
namespace impl {

Expression dispatch_eval(keyword_tag_t<Keyword::_Literal>, Expression::Args const args, EvalContextLua)
{
    return args.self();
}
Expression dispatch_eval(keyword_tag_t<Keyword::_Void>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::_Void)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::_Continue>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::_Continue)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::_Resolve>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::_Resolve)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Near>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Near)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Union>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Union)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Intersect>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Intersect)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Diff>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Diff)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::If>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::If)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Elif>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Elif)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Else>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Else)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Transp>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Transp)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Cartesian>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Cartesian)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Reverse>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Reverse)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Uniques>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Uniques)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Items>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Items)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Keys>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Keys)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Values>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Values)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Enumerate>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Enumerate)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Flatten>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Flatten)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::ToList>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::ToList)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::First>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::First)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Last>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Last)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Rand>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Rand)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::RandInt>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::RandInt)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Sequence>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Sequence)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Arange>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Arange)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Parse>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Parse)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Re>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Re)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Fmt>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Fmt)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Capitalize>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Capitalize)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::LowerCase>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::LowerCase)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::UpperCase>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::UpperCase)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Card>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Card)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Size>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Size)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Sum>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Sum)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Prod>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Prod)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Avg>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Avg)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Slide>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Slide)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Stride>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Stride)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Chunks>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Chunks)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Repeat>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Repeat)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Cat>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Cat)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Push>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Push)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::At>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::At)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Del>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Del)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Upd>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Upd)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Lookup>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Lookup)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Cast>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Cast)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Fold>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Fold)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Map>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Map)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Filter>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Filter)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Count>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Count)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Each>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Each)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Sort>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Sort)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Find>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Find)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::FindPtr>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::FindPtr)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::FindIdx>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::FindIdx)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Min>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Min)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Max>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Max)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Argmin>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Argmin)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Argmax>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Argmax)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Recur>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Recur)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Unfold>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Unfold)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Op>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Op)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Bind>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Bind)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Fn>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Fn)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Link>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Link)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Get>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Get)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::EnvLoad>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::EnvLoad)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::EnvStore>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::EnvStore)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Any>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Any)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::All>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::All)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Saturate>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Saturate)));
}
// Expression dispatch_eval(keyword_tag_t<Keyword::Pipe>, Expression::Args const, EvalContextLua)
// {
//     return Expression(error("not implemented", keyword_to_str(Keyword::Pipe)));
// }
Expression dispatch_eval(keyword_tag_t<Keyword::Tuple>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Tuple)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Fork>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Fork)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Flip>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Flip)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Dbg>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Dbg)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Eval>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Eval)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Try>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Try)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Kwrd>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Kwrd)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Prms>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Prms)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::D>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::D)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Assert>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Assert)));
}
Expression dispatch_eval(keyword_tag_t<Keyword::Trace>, Expression::Args const, EvalContextLua)
{
    return Expression(error("not implemented", keyword_to_str(Keyword::Trace)));
}


} // namespace impl
} // namespace haikan
