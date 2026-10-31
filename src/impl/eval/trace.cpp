#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {



// HAIKAN_DEFINE_EVALUATE_IMPL(Trace)
// {
//     auto const trace_id = rhs().eval_e({}, curr_ctx()).prettify();
//     HAIKAN_LOG(INFO).WithSrcLoc("haikan::Trace") << trace_id << " " << lhs().data();
//     if( auto traces = curr_ctx().traces)
//     {
//         traces->push_back(boost::json::object{{trace_id, lhs().data()}});
//     }
//     return lhs();
// }

} // namespace impl
} // namespace haikan
