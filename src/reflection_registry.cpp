/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include <typeindex>
#include <unordered_map>

#include "haikan/reflection_registry.hpp"
#include "haikan/impl/operator_table.hpp"


namespace
{
using InitReflectionFn = void (*)(haikan::ReflectionContextFactory&);
using Registry = std::unordered_map<std::size_t, InitReflectionFn>;

Registry& instance()
{
    static Registry r;
    return r;
}




} // namespace


namespace haikan {

void ReflectionRegistry::init(sol::state_view L)
{
    ReflectionContextFactory ctx{L};
    for(auto const& entry: instance())
    {
        entry.second(ctx);
    }

    for(auto& record: ctx.records)
    {
        record.on_init(L);
    }

    auto const& op_types = impl::OperatorRegistry::types_seen();
    auto log = HAIKAN_LOG_CERR(DEBUG);
    log << "Missing runtime reflection: {";
    for(auto const& entry: op_types)
    {
        if (instance().find(entry.first) == instance().cend())
        {
            log << entry.second << ",";
        }
    }
    log << "}";
}

void ReflectionRegistry::insert(
    std::type_index type_index,
    void (*init_reflection_fn)(ReflectionContextFactory&))
{
    instance().emplace(type_index.hash_code(), init_reflection_fn);
}

} // namespace haikan
