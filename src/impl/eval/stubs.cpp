#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {

template <> sol::object VirtualMachine::Evaluate<Keyword::Near>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Transp>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Cartesian>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Reverse>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Uniques>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Items>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Keys>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Values>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Enumerate>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Flatten>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::ToList>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::First>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Last>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Rand>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::RandInt>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Sequence>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Arange>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Parse>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Str>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Re>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Fmt>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Capitalize>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::LowerCase>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::UpperCase>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Card>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Sum>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Prod>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Avg>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Slide>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Stride>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Chunks>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Repeat>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Cat>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Push>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::At>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Del>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Upd>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Lookup>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Cast>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Assert>(sol::object const&, sol::object const&) const { return sol::nil; }
template <> sol::object VirtualMachine::Evaluate<Keyword::Trace>(sol::object const&, sol::object const&) const { return sol::nil; }


} // namespace impl
} // namespace haikan
