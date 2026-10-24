#include <iostream>
#include <sol/sol.hpp>

#include "cow.hpp"


int main(int argc, char* argv[])
{
    sol::state state{};
    state.open_libraries();
    lauseke::impl::Cow cow{state.lua_state()};

    sol::table o{state, sol::create};
    o["lol"] = 42;
    o["del"] = "del";

    sol::table kek{state.lua_state(), sol::create};
    o["kek"] = kek;
    kek["foo"] = 15;

    auto c = cow.make(o);

    std::cout << "c.lol    : " << c["lol"].get<int>() << '\n';
    std::cout << "c.kek.foo: " << c["kek"]["foo"].get<int>() << '\n';
    std::cout << "c.del    : " << c["del"].get<std::string>() << '\n';

    c["lol"] = "hoho";
    c["kek"]["foo"] = "hehe";
    c["del"] = sol::lua_nil;

    std::cout << "c.lol    : " << c["lol"].get<std::string>() << '\n';
    std::cout << "c.kek.foo: " << c["kek"]["foo"].get<std::string>() << '\n';
    std::cout << "c.del ?  : " << c["del"].get_or<std::string>("nil") << '\n';


    std::cout << "o.lol    : " << o["lol"].get<int>() << '\n';
    std::cout << "o.kek.foo: " << o["kek"]["foo"].get<int>() << '\n';
    std::cout << "o.del    : " << o["del"].get<std::string>() << '\n';

    sol::type t = c["del"].get<sol::reference>().get_type();

//     enum class type : int {
// 		none = LUA_TNONE,
// 		lua_nil = LUA_TNIL,
// #ifndef __OBJC__
// 		nil = lua_nil,
// #endif // Objective C++ Keywordfalse_type
// 		string = LUA_TSTRING,
// 		number = LUA_TNUMBER,
// 		thread = LUA_TTHREAD,
// 		boolean = LUA_TBOOLEAN,
// 		function = LUA_TFUNCTION,
// 		userdata = LUA_TUSERDATA,
// 		lightuserdata = LUA_TLIGHTUSERDATA,
// 		table = LUA_TTABLE,
// 		poly = none | lua_nil | string | number | thread |
// 		table | boolean | function | userdata | lightuserdata
// 	};

    std::string st;

    switch (t)
    {
    case sol::type::none         : st = "none"; break;
    case sol::type::lua_nil      : st = "nil"; break;
    case sol::type::string       : st = "string"; break;
    case sol::type::number       : st = "number"; break;
    case sol::type::thread       : st = "thread"; break;
    case sol::type::boolean      : st = "boolean"; break;
    case sol::type::function     : st = "function"; break;
    case sol::type::userdata     : st = "userdata"; break;
    case sol::type::lightuserdata: st = "lightuserdata"; break;
    case sol::type::table        : st = "table"; break;
    // case sol::type::poly         : st = "poly"; break;
    default:
        st = "";
    }

    std::cout << "c[\"del\"].tbl.get_type(): " << st << '\n';
}
