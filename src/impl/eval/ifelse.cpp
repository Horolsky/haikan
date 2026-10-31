#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/interpreter.hpp"

namespace haikan {
namespace impl {

template <> sol::object Interpreter::Evaluate<Keyword::_Continue>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::_Resolve>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::If>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Elif>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object Interpreter::Evaluate<Keyword::Else>(sol::object const&, sol::object const&) const { return sol::nil; }

} // namespace impl
} // namespace haikan
