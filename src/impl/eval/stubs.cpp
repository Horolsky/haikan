#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> sol::object Interpreter::Evaluate<Keyword::Near>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Transp>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Cartesian>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Reverse>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Uniques>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Items>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Keys>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Values>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Enumerate>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Flatten>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::ToList>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::First>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Last>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Rand>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::RandInt>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Sequence>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Arange>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Parse>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Str>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Re>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Fmt>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Capitalize>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::LowerCase>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::UpperCase>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Card>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Sum>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Prod>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Avg>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Slide>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Stride>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Chunks>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Repeat>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Cat>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Push>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::At>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Del>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Upd>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Lookup>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Cast>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Assert>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Trace>(sol::object const&, sol::object const&) const { return sol::nil; }


} // namespace impl
} // namespace haikan
