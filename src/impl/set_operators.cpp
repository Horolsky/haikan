/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/set_operators.hpp"
#include "haikan/impl/operators.hpp"
#include "haikan/impl/generic_operand.hpp"





namespace haikan {

namespace impl {

    namespace
{
// Check if table is mapping based on first element lookup
// It is assumed the user will not put hybrid lua tables in set operations
static bool lua_table_is_mapping(sol::table const& table)
{
    return table[1] == sol::nil;
}
}

bool set_equal(sol::table const& lhs, sol::table const& rhs)
{
    auto log = HAIKAN_LOG(DEBUG)
        << "op::set_equal()(" << lhs.as<sol::object>() << ", " << rhs.as<sol::object>() << ") -> ";

    bool result {true};
    for (auto const& item: lhs)
    {

        bool const ne = (GenericOperand(rhs[item.first].get<sol::object>()) !=  GenericOperand(item.second)).as<bool>();
        if (ne)
        {
            result = false;
            break;
        }
    }
    bool const lhs_is_arr = not lua_table_is_mapping(lhs);
    bool const rhs_is_arr = not lua_table_is_mapping(rhs);
    if (lhs_is_arr and rhs_is_arr and lhs.size() == rhs.size())
    {
        return true;
    }
    for (auto const& item: rhs)
    {
        bool const ne = (GenericOperand(lhs[item.first].get<sol::object>()) != GenericOperand(item.second)).as<bool>();
        if (ne)
        {
            result = false;
            break;
        }
    }
    log << result;
    // log << (result ? "true" : "false");
    return result;
}


namespace
{

static bool table_array_contains_value(sol::table const& table, sol::object const& value)
{
    for (auto const& item : table)
    {
        if (item.second == value)
        {
            return true;
        }
    }
    return false;
}



static sol::table set_union_table_mapping(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = sol::state_view(lhs.lua_state()).create_table();
    for (auto const& item : rhs)
    {
        result[item.first] = item.second;
    }
    for (auto const& item : lhs)
    {
        result[item.first] = item.second;
    }
    return result;
}

static sol::table set_union_table_array(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = sol::state_view(lhs.lua_state()).create_table();
    for (auto const& item : lhs)
    {
        if (!table_array_contains_value(result, item.second))
        {
            result.add(item.second);
        }
    }
    for (auto const& item : rhs)
    {
        if (!table_array_contains_value(result, item.second))
        {
            result.add(item.second);
        }
    }
    return result;
}


static sol::table set_intersection_table_array(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = sol::state_view(lhs.lua_state()).create_table();
    for (auto const& item : lhs)
    {
        if (table_array_contains_value(rhs, item.second) &&
            !table_array_contains_value(result, item.second))
        {
            result.add(item.second);
        }
    }
    return result;
}

static sol::table set_intersection_table_mapping(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = sol::state_view(lhs.lua_state()).create_table();
    for (auto const& item : lhs)
    {
        if (rhs[item.first] != sol::nil)
        {
            result[item.first] = item.second;
        }
    }
    return result;
}

static sol::table set_difference_table_array(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = sol::state_view(lhs.lua_state()).create_table();
    for (auto const& item : lhs)
    {
        if (!table_array_contains_value(rhs, item.second) &&
            !table_array_contains_value(result, item.second))
        {
            result.add(item.second);
        }
    }
    return result;
}

static sol::table set_difference_table_mapping(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = sol::state_view(lhs.lua_state()).create_table();
    for (auto const& item : lhs)
    {
        if (rhs[item.first] == sol::nil)
        {
            result[item.first] = item.second;
        }
    }
    return result;
}


static sol::table set_symmetric_difference_table_array(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = set_difference_table_array(lhs, rhs);
    for (auto const& item : rhs)
    {
        if (!table_array_contains_value(lhs, item.second) &&
            !table_array_contains_value(result, item.second))
        {
            result.add(item.second);
        }
    }
    return result;
}
static sol::table set_symmetric_difference_table_mapping(sol::table const& lhs, sol::table const& rhs)
{
    sol::table result = set_difference_table_mapping(lhs, rhs);
    for (auto const& item : rhs)
    {
        if (lhs[item.first] == sol::nil)
        {
            result[item.first] = item.second;
        }
    }
    return result;
}


} // namespace

sol::table set_union(sol::table const& lhs, sol::table const& rhs)
{
    return lua_table_is_mapping(lhs)
        ? set_union_table_mapping(lhs, rhs)
        : set_union_table_array(lhs, rhs);
}


sol::table set_intersection(sol::table const& lhs, sol::table const& rhs)
{
    return lua_table_is_mapping(lhs)
        ? set_intersection_table_mapping(lhs, rhs)
        : set_intersection_table_array(lhs, rhs);
}


sol::table set_difference(sol::table const& lhs, sol::table const& rhs)
{
    return lua_table_is_mapping(lhs)
        ? set_difference_table_mapping(lhs, rhs)
        : set_difference_table_array(lhs, rhs);
}

sol::table set_symmetric_difference(sol::table const& lhs, sol::table const& rhs)
{
    return lua_table_is_mapping(lhs)
        ? set_symmetric_difference_table_mapping(lhs, rhs)
        : set_symmetric_difference_table_array(lhs, rhs);
}


} // namespace impl
}
