#include "haikan/impl/expression_lua.hpp"
#include "haikan/error.hpp"
#include "haikan/impl/keyword_to_str.hpp"
#include "haikan/impl/pp.hpp"

namespace haikan {
namespace impl {


ExpressionLua dispatch_eval(keyword_tag_t<Keyword::Pipe>, ExpressionLua::Args const args, EvalContextLua ctx)
// {
//     return ExpressionLua(error("not implemented", keyword_to_str(Keyword::Pipe)));
// }
{
    auto const subexpressions = args.self().subexpressions_vec();
    if (HAIKAN_UNLIKELY(subexpressions.empty()))
    {
        return {error("invalid parameter", keyword_to_str(Keyword::Pipe))};
    }
    // ASSERT(subexpressions.size() > 0, "invalid parameter (empty array)");

    auto fn = subexpressions.cbegin();
    // first el eval as is
    ExpressionLua ret = (*fn++).eval(args.lhs(), ctx);
    while (fn != subexpressions.cend())
    {
        // any consequent literals eval as eq
        ret = (*fn++).eval(ret,ctx);
        // ret = (*fn++).eval_maybe_predicate(ret,curr_ctx());
    }
    return ret;
}
} // namespace impl
} // namespace haikan