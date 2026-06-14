#include "haikan/impl/lua_state.hpp"

#include <sol/sol.hpp>

namespace haikan {

LuaState::LuaState() {
    // Open basic libs; FFI via Sol2 or direct require("ffi") if LuaJIT
    state_.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
    // TODO: load FFI if LuaJIT available
}

LuaState::~LuaState() = default;

void LuaState::load_ffi_cdef(const std::string& cdef) {
    // Example: state_.script("ffi = require('ffi'); ffi.cdef[[" + cdef + "]] ");
    // For now stub
}

template <typename T>
sol::object LuaState::make_cdata_from_ptr(T* ptr, const std::string& ctype_name) {
    // ffi.cast(ctype_name, ptr) for zero-copy
    // Return as sol::object
    return sol::make_object(state_, ptr); // placeholder
}

sol::table LuaState::make_table() {
    return state_.create_table();
}

} // namespace haikan
