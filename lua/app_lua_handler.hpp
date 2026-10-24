/**
 * @file
 * @brief Lua Handler class.
 *
 * @copyright (c) 2026 Oleksandr Khorolskyi
 *
 */


#pragma once

#include <set>
#include <string>
#include <filesystem>
#include <fstream>

#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/mp11.hpp>
#include <boost/program_options.hpp>

#include <sol/sol.hpp>


namespace lauseke {
namespace impl {

class LuaHandler
{
    sol::state lua_{};
    std::string const script_path_;
    sol::load_result script_;

    void load_mbtxx_package()
    {
        sol::function require = lua_["require"];
        auto new_require = [this, require](std::string const& mod_name) -> sol::table {
            if (mod_name == "haikan") {
                auto package = lua_.create_table();
                package.set_function("hello_boost", hello_boost);
                return package;
            }
            return require(mod_name);
        };

        lua_.set_function("require", new_require);
    }

  public:
    LuaHandler(std::string const& script_path, std::string lib_path)
        : script_path_(script_path)
    {

        lua_.open_libraries();
        load_mbtxx_package();
        std::string modules_path = std::filesystem::path(script_path_)
            .parent_path()
            .append("?.lua")
            .string();

        if (!lib_path.empty())
        {
            modules_path += ";" + std::filesystem::path(lib_path).append("?.lua").string();
        }

        std::string path_upd = (format("package.path = \"%s;/\" .. package.path") % modules_path).str();
        lua_.script(path_upd);

        script_ = lua_.load_file(script_path_);
        if (!script_.valid()) {
            sol::error err = script_;
            throw sol::error(err.what());
        }
    }

    ~LuaHandler()
    {
        lua_.stack_clear();
    }

    sol::protected_function_result operator()(std::string const& tags) noexcept
    {
        return script_(tags);
    }
};

} // namespace impl
} // namespace lauseke
