#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {


template <> void Interpreter::Evaluate<Keyword::Map>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Filter>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Count>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Each>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Sort>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Find>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::FindPtr>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::FindIdx>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Min>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Max>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Argmin>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Argmax>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Recur>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Unfold>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Op>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Bind>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Fn>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Link>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Get>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::EnvLoad>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::EnvStore>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Any>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::All>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Saturate>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Tuple>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Dbg>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Eval>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Try>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Kwrd>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Prms>(Interpreter::Stack&) {}
template <> void Interpreter::Evaluate<Keyword::Err>(Interpreter::Stack&) {}

} // namespace impl
} // namespace haikan
