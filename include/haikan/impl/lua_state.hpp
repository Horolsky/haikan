#pragma once

// Lua state management for data representation (one pool per context)
// Supports Sol2 and LuaJIT FFI cdata for zero-copy C++ struct access

#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace haikan {

class LuaState {
public:
    LuaState();
    ~LuaState();

    sol::state& get() { return state_; }
    const sol::state& get() const { return state_; }

    // Load FFI cdef for C++ structs (for zero-copy reflection)
    void load_ffi_cdef(const std::string& cdef);

    // Create cdata from existing C++ pointer (true zero-copy)
    template <typename T>
    sol::object make_cdata_from_ptr(T* ptr, const std::string& ctype_name);

    // Create Lua table for transformation results
    sol::table make_table();

private:
    sol::state state_;
};

} // namespace haikan
