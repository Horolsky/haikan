#include "haikan/impl/lua_migrate.hpp"
#include "haikan/reflection_meta.hpp"

#include <unordered_set>


namespace haikan {
namespace impl {
namespace {

using TablePath = std::unordered_set<void const*>;

boost::optional<sol::object> migrate_to_state_impl(
    sol::state_view target_state, sol::object object, TablePath& path);


boost::optional<sol::object> migrate_utype(sol::state_view target_state, sol::object object)
{
    sol::userdata userdata = object.as<sol::userdata>();

    sol::object metadata = userdata["meta"];

    if (metadata.is<ReflectionMeta>())
    {
        ReflectionMeta const& meta = metadata.as<ReflectionMeta const&>();
        if (meta.migrate)
        {
            return meta.migrate(target_state, object);
        }
    }
    return boost::none;

}


boost::optional<sol::object> migrate_table_to_state(
    sol::state_view target_state, sol::object object, TablePath& path)
{
    sol::table const source = object.as<sol::table>();

    source.push();
    void const* const identity = lua_topointer(source.lua_state(), -1);
    lua_pop(source.lua_state(), 1);

    auto const inserted = path.insert(identity);
    if (!inserted.second)
    {
        return boost::none;
    }

    struct PathErase
    {
        TablePath& path;
        void const* identity;

        ~PathErase()
        {
            path.erase(identity);
        }
    } const erase_from_path{path, identity};

    sol::table target = target_state.create_table(static_cast<int>(source.size()), 0);

    for (auto const& entry : source)
    {
        auto const key = migrate_to_state_impl(target_state, entry.first, path);
        if (!key)
        {
            return boost::none;
        }

        auto const value = migrate_to_state_impl(target_state, entry.second, path);
        if (!value)
        {
            return boost::none;
        }

        target.set(*key, *value);
    }

    return sol::make_object(target_state, target);
}

boost::optional<sol::object> migrate_to_state_impl(
    sol::state_view target_state, sol::object object, TablePath& path)
{
    if (target_state == nullptr)
    {
        return boost::none;
    }

    if (object.lua_state() == target_state)
    {
        return object;
    }
    switch (object.get_type())
    {
    case sol::type::lua_nil  : return sol::make_object(target_state, sol::nil);
    case sol::type::string   : return sol::make_object(target_state, object.as<std::string>());
    case sol::type::number   : return sol::make_object(target_state, object.as<double>());
    case sol::type::boolean  : return sol::make_object(target_state, object.as<bool>());
    case sol::type::table    : return migrate_table_to_state(target_state, object, path);
    case sol::type::userdata    : return migrate_utype(target_state, object);
    default                  : return boost::none;
    }
}

}  // namespace

boost::optional<sol::object> migrate_to_state(sol::state_view target_state, sol::object object)
{
    TablePath path;
    return migrate_to_state_impl(target_state, object, path);
}

} // namespace impl
} // namespace haikan
