#include <cstdint>
#include "haikan/impl/keyword.hpp"
#include "haikan/impl/vm.hpp"

namespace haikan {
namespace impl {


template <> void VirtualMachine::Evaluate<Keyword::Map>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Filter>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Count>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Each>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Sort>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Find>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::FindPtr>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::FindIdx>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Min>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Max>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Argmin>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Argmax>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Recur>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Unfold>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Op>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Bind>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Fn>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Link>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Get>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::EnvLoad>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::EnvStore>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Any>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::All>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Saturate>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Tuple>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Dbg>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Eval>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Try>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Kwrd>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Prms>(VirtualMachine::Stack&) {}
template <> void VirtualMachine::Evaluate<Keyword::Err>(VirtualMachine::Stack&) {}

} // namespace impl
} // namespace haikan
