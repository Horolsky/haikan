/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */


#include "haikan/impl/expression_lua.hpp"
#include "haikan/error.hpp"
#include "haikan/impl/keyword_to_str.hpp"

namespace haikan {
namespace impl {

ExpressionLua dispatch_eval(keyword_tag_t<Keyword::_Literal>, ExpressionLua::Args const args, EvalContextLua)
{
    return args.self();
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::_Void>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::_Void)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::_Continue>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::_Continue)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::_Resolve>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::_Resolve)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Near>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Near)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Union>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Union)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Intersect>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Intersect)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Diff>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Diff)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::If>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::If)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Elif>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Elif)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Else>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Else)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Transp>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Transp)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Cartesian>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Cartesian)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Reverse>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Reverse)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Uniques>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Uniques)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Items>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Items)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Keys>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Keys)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Values>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Values)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Enumerate>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Enumerate)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Flatten>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Flatten)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::ToList>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::ToList)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::First>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::First)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Last>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Last)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Rand>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Rand)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::RandInt>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::RandInt)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Sequence>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Sequence)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Arange>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Arange)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Parse>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Parse)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Re>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Re)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Fmt>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Fmt)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Capitalize>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Capitalize)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::LowerCase>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::LowerCase)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::UpperCase>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::UpperCase)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Card>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Card)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Size>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Size)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Sum>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Sum)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Prod>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Prod)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Avg>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Avg)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Slide>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Slide)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Stride>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Stride)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Chunks>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Chunks)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Repeat>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Repeat)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Cat>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Cat)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Push>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Push)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::At>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::At)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Del>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Del)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Upd>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Upd)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Lookup>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Lookup)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Cast>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Cast)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Fold>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Fold)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Map>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Map)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Filter>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Filter)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Count>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Count)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Each>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Each)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Sort>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Sort)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Find>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Find)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::FindPtr>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::FindPtr)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::FindIdx>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::FindIdx)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Min>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Min)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Max>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Max)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Argmin>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Argmin)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Argmax>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Argmax)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Recur>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Recur)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Unfold>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Unfold)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Op>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Op)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Bind>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Bind)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Fn>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Fn)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Link>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Link)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Get>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Get)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::EnvLoad>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::EnvLoad)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::EnvStore>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::EnvStore)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Any>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Any)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::All>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::All)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Saturate>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Saturate)));
}
// ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Pipe>, ExpressionLua::Args const, EvalContextLua)
// {
//     return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Pipe)));
// }
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Tuple>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Tuple)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Fork>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Fork)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Flip>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Flip)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Dbg>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Dbg)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Eval>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Eval)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Try>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Try)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Kwrd>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Kwrd)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Prms>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Prms)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::D>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::D)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Assert>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Assert)));
}
ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Trace>, ExpressionLua::Args const, EvalContextLua)
{
    return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Trace)));
}


} // namespace impl
} // namespace haikan
