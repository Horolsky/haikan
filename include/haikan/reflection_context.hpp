/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/reflection_meta.hpp"
#include "haikan/impl/get_or_create_table.hpp"
#include "haikan/type_tag.hpp"
#include "haikan/error.hpp"
#include "haikan/impl/operator_table.hpp"


namespace haikan {

namespace impl { class TypeRegistry; }

class ReflectionContextFactory
{
public:

    sol::state_view state_view() const
    {
        return state;
    }

    struct Record
    {
        ReflectionMeta meta;
        std::function<void(sol::state_view)> on_init;
    };

    template <class T>
    class RegistrationContext
    {
    public:
        using type = T;

        template <class Key, class Value>
        void usertype_set(Key&& key, Value&& value)
        {
            using key_type = typename std::decay<Key>::type;
            using value_type = typename std::decay<Value>::type;

            key_type key_copy{std::forward<Key>(key)};
            value_type value_copy{std::forward<Value>(value)};
            actions->emplace_back(
                [key=std::move(key_copy), value=std::move(value_copy)](sol::simple_usertype<T>& ut) mutable {
                    ut.set(key, value);
                });
        }

        //
        ReflectionMeta& reflection_meta()
        {
            return reflection_meta_.get();
        }

        sol::state_view state_view() const
        {
            return state;
        }

        template <class Function>
        sol::protected_function make_function(Function&& function) const
        {
            return sol::make_object(
                state.lua_state(), sol::as_function(std::forward<Function>(function)))
                .template as<sol::protected_function>();
        }

    private:

        friend class ReflectionContextFactory;

        using Action = std::function<void(sol::simple_usertype<T>&)>;

        RegistrationContext(std::reference_wrapper<ReflectionMeta> rm, std::shared_ptr<std::vector<Action>> actions, sol::state_view state)
        : reflection_meta_{rm}
        , actions{std::move(actions)}
        , state{state}
        {
        }

        std::reference_wrapper<ReflectionMeta> reflection_meta_;
        std::shared_ptr<std::vector<Action>> actions;
        sol::state_view state;
    };


    ~ReflectionContextFactory() = default;

    template <class T, class W = T>
    RegistrationContext<W> make_registration_context(
        type_tag<T>, type_tag<W> = {}
    )
    {
        using Action = typename RegistrationContext<W>::Action;
        auto actions = std::make_shared<std::vector<Action>>();

        records.emplace_back();
        auto& record = records.back();
        record.meta.type_name = sol::usertype_traits<T>::name();
        record.meta.type_index_hash = typeid(T).hash_code();
        record.meta.wrapper_type_index_hash = typeid(W).hash_code();

        record.meta.migrate = [](lua_State* target_state, sol::object src) -> sol::object {
            if (src.valid() and src.is<W>())
            {
                return sol::make_object(target_state, src.as<W>());
            }
            return sol::make_object(target_state, error{"invalid argument", "utype migrate"});
        };

        record.on_init = [meta=std::ref(record.meta), actions=actions](sol::state_view L) {
            initialize_registration<T, W>(
                L,
                meta.get(),
                actions,
                std::integral_constant<bool, sol::is_container<W>::value>{});
        };

        return RegistrationContext<W>{std::ref(record.meta), actions, state};
    }

private:

    static sol::object register_metadata(sol::state_view L, ReflectionMeta& meta)
    {
        sol::object lua_meta = sol::make_object(L.lua_state(), meta);
        sol::table root = impl::get_or_create_table(L, L.globals(), "haikan");
        sol::table utypes = impl::get_or_create_table(L, root, "utypes");
        utypes[meta.type_index_hash] = lua_meta;
        if (meta.wrapper_type_index_hash != meta.type_index_hash)
        {
            utypes[meta.wrapper_type_index_hash] = lua_meta;
        }
        return lua_meta;
    }

    template <class T, class W>
    static void initialize_registration(
        sol::state_view L,
        ReflectionMeta& meta,
        std::shared_ptr<std::vector<typename RegistrationContext<W>::Action>> const& actions,
        std::false_type)
    {
        sol::simple_usertype<W> ut = L.create_simple_usertype<W>();
        impl::OperatorRegistry::AddSignature(type<T>); // TODO: correct?

        for (auto& action: *actions)
        {
            action(ut);
        }
        ut.set("serialize", meta.serialize);
        ut.set("deserialize", meta.deserialize);
        ut.set("meta", register_metadata(L, meta));

        HAIKAN_LOG(DEBUG) << "reflection initializing for " << meta.type_name;
        L.set_usertype(meta.type_name, ut);

        std::stringstream ss;
        ss << L.lua_state();
        HAIKAN_LOG(DEBUG) << "reflection initialized for " << meta.type_name << " at state " << ss.str();
    }

    /// Container variant - extending the metatable introduced by Sol
    /// Using sol::simple_usertype will overwrite it.
    template <class T, class W>
    static void initialize_registration(
        sol::state_view L,
        ReflectionMeta& meta,
        std::shared_ptr<std::vector<typename RegistrationContext<W>::Action>> const&,
        std::true_type)
    {
        sol::object container = sol::make_object(L.lua_state(), W{});
        sol::object lua_meta = register_metadata(L, meta);

        container.push();
        if (lua_getmetatable(L.lua_state(), -1) != 0)
        {
            lua_pushliteral(L.lua_state(), "serialize");
            sol::stack::push(L.lua_state(), meta.serialize);
            lua_rawset(L.lua_state(), -3);
            lua_pushliteral(L.lua_state(), "deserialize");
            sol::stack::push(L.lua_state(), meta.deserialize);
            lua_rawset(L.lua_state(), -3);
            lua_pushliteral(L.lua_state(), "meta");
            sol::stack::push(L.lua_state(), lua_meta);
            lua_rawset(L.lua_state(), -3);
            lua_pop(L.lua_state(), 1);
        }
        lua_pop(L.lua_state(), 1);

        impl::OperatorRegistry::AddSignature(type<T>);

        std::stringstream ss;
        ss << L.lua_state();
        HAIKAN_LOG(DEBUG) << "reflection initialized for " << meta.type_name << " at state " << ss.str();
    }

    friend class impl::TypeRegistry;

    ReflectionContextFactory(sol::state_view L)
    : state{L}
    , records{}
    {
    }

    sol::state_view state;
    std::deque<Record> records;
};

} // namespace haikan
