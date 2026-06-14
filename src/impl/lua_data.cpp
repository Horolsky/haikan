#include "haikan/impl/lua_data.hpp"
#include <sol/sol.hpp>

namespace haikan {

LuaData LuaDataAdapter::get(const LuaData& data, const std::string& key) {
    if (!data.obj.valid()) return LuaData();
    auto field = get_field_impl(data.obj, key);
    return LuaData(field);
}

LuaData LuaDataAdapter::get(const LuaData& data, std::size_t index) {
    if (!data.obj.valid() || !data.is_table()) return LuaData();
    auto tbl = data.as_table();
    return LuaData(tbl[index + 1]); // Lua 1-based
}

void LuaDataAdapter::set(LuaData& data, const std::string& key, const LuaData& value) {
    if (data.is_table()) {
        data.as_table()[key] = value.obj;
    } else if (data.is_userdata()) {
        // For userdata: usually immutable or via metatable setter
        // Stub: ignore or error in full impl
    }
}

LuaData LuaDataAdapter::make_object() {
    return LuaData(sol::make_object(sol::state_view(sol::main_thread), sol::table::create(sol::state_view(sol::main_thread))));
}

LuaData LuaDataAdapter::make_array() {
    auto tbl = sol::table::create(sol::state_view(sol::main_thread));
    return LuaData(tbl);
}

sol::object LuaDataAdapter::get_field_impl(const sol::object& obj, const std::string& key) {
    if (obj.is<sol::table>()) {
        return obj.as<sol::table>()[key];
    } else if (obj.is<sol::userdata>()) {
        // Pseudo-JSON: try metatable __index or Sol2 usertype property
        // For FFI cdata: direct if possible, else adapter
        auto mt = obj.get<sol::table>("__index");
        if (mt.valid() && mt.is<sol::function>()) {
            // call metatable function if present
        }
        // Fallback: return nil or attempt property access
        return obj.as<sol::table>()[key]; // may work via Sol2
    }
    return sol::nil;
}

LuaData LuaDataAdapter::from_json_like(const std::string& json_str) {
    // TODO: parse JSON string to LuaData (for transition)
    return LuaData();
}

} // namespace haikan
