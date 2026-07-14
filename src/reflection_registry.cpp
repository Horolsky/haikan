/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <atomic>
#include <unordered_set>

#include "haikan/reflection_registry.hpp"


namespace
{
struct Record
{
    std::type_index type_index;
    const char* type_name;
    void (*init_reflection_fn)(haikan::ReflectionContext&);
};

struct RecordHash
{
    std::size_t operator()(Record const& record) const noexcept
    {
        return record.type_index.hash_code();
    }
};

struct RecordEqual
{
    bool operator()(Record const& lhs, Record const& rhs) const noexcept
    {
        return lhs.type_index == rhs.type_index;
    }
};

using Registry = std::unordered_set<Record, RecordHash, RecordEqual>;

Registry& instance()
{
    static Registry r;
    return r;
}


sol::table get_or_create_table(sol::state_view L, sol::table parent, char const* name)
{
    sol::object obj = parent[name];
    if (obj.get_type() == sol::type::table)
    {
        return obj;
    }
    sol::table tbl = L.create_table();
    parent[name] = tbl;
    return tbl;
}

} // namespace


namespace haikan {

void ReflectionRegistry::init(sol::state_view L)
{
    ReflectionContext ctx{L};
    for(auto const& rec: instance())
    {
        rec.init_reflection_fn(ctx);
    }

    sol::table root = get_or_create_table(L, L.globals(), "haikan");
    sol::table utypes = get_or_create_table(L, root, "utypes");

    for(auto& record: ctx.records)
    {
        utypes[record.type_index_hash] = record.meta;
        record.on_init(L);
    }
}

void ReflectionRegistry::insert(
    std::type_index type_index,
    const char* type_name,
    void (*init_reflection_fn)(ReflectionContext&))
{
    instance().insert(Record{type_index, type_name, init_reflection_fn});
}

sol::object ReflectionRegistry::get_utype(std::type_index type_index, sol::state_view L)
{
    return L["haikan"]["utypes"][type_index.hash_code()];
}


} // namespace haikan
