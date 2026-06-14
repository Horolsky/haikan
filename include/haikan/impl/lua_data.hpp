#pragma once

// Initial stub for Lua + Sol2 / FFI cdata data representation
// Replaces boost::json::value as core data type for expressions.

#include <sol/sol.hpp>
#include <string>
#include <variant>

namespace haikan {

// Placeholder for hybrid data: either Lua table, FFI cdata userdata, or primitive
// In full impl: use sol::object or custom wrapper with Lua state
struct LuaData {
    sol::object value;  // sol::object can hold table, userdata, number, string, etc.
    // Future: FFI cdata specific handling for zero-copy
};

// Stub adapter for pseudo-JSON access
// To be expanded with metatable or FFI for dynamic string key access on userdata
class LuaDataAdapter {
public:
    static LuaData get_field(const LuaData& data, const std::string& key);
    static LuaData make_table();  // for transformation results
    // ... more
};

} // namespace haikan
