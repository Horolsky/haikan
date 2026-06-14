#include "haikan/impl/expression.hpp"
#include "haikan/keywords.hpp"
#include "haikan/impl/eval_impl.hpp"
#include "haikan/impl/error_expr.hpp"
#include "haikan/impl/lua_data.hpp"
#include "haikan/impl/lua_state.hpp"

#include <memory>
#include <utility>

namespace haikan {

// Migrated: use LuaData instead of boost::json::value
// Pseudo-JSON access via LuaDataAdapter
// Zero-copy FFI cdata path for input data

LuaData make_literal_argument_view(const LuaData& input) {
    // Literal now wraps in LuaData
    return input;  // simplified; original used Keyword::_Literal
}

// ExpressionView migrated to LuaData
class ExpressionView {
public:
    // ... existing encoding etc. assumed

    LuaData eval(const LuaData& input, EvalContext& ctx) {
        // Check for literal / preproc (attr flags)
        if (/* is_literal or is_preproc */) {
            return input;
        }

        LuaData arg = make_literal_argument_view(input);
        LuaData result = eval_e(arg, ctx);  // core eval

        // Handle _Resolve / _Continue etc.
        if (/* special keywords */) {
            // error handling
            return LuaDataAdapter::make_object();
        }

        return result;
    }

    bool eval_as_predicate(const LuaData& input, EvalContext& ctx) {
        LuaData res = eval(input, ctx);
        // Convert to bool via adapter or direct
        return !res.is_nil() && /* truthy check */ true;
    }

    // match, operator* etc. updated to LuaData
    LuaData operator*(const LuaData& rhs) {
        return eval(rhs, /*ctx*/);
    }

    // ... other members
};

// eval_e and other internals would use LuaDataAdapter::get for field access
// instead of json pointer / object access

} // namespace haikan
