#pragma once

// Core data representation replacing boost::json::value
// Hybrid: FFI cdata/userdata for zero-copy input, Lua tables for transforms
// Pseudo-JSON access via adapter (string keys, implicit types via Operator)

#include <sol/sol.hpp>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>

namespace haikan {

// LuaData: wraps sol::object with helpers for expression eval
struct LuaData {
    sol::object obj;

    LuaData() = default;
    explicit LuaData(sol::object o) : obj(std::move(o)) {}

    bool is_nil() const { return !obj.valid() || obj == sol::nil; }
    bool is_table() const { return obj.is<sol::table>(); }
    bool is_userdata() const { return obj.is<sol::userdata>(); }
    bool is_number() const { return obj.is<double>() || obj.is<int64_t>(); }
    bool is_string() const { return obj.is<std::string>(); }

    sol::table as_table() const { return obj.as<sol::table>(); }
    // For FFI cdata zero-copy access
};

// LuaDataAdapter: provides pseudo-JSON interface (string key / index access)
// Delegates type handling to Operator; supports both userdata and tables
class LuaDataAdapter {
public:
    static LuaData get(const LuaData& data, const std::string& key);
    static LuaData get(const LuaData& data, std::size_t index);
    static void set(LuaData& data, const std::string& key, const LuaData& value);
    static LuaData make_object();           // for new objects
    static LuaData make_array();            // for new arrays
    static LuaData from_json_like(const std::string& json_str); // transition helper

private:
    static sol::object get_field_impl(const sol::object& obj, const std::string& key);
};

} // namespace haikan
