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
#include "haikan/impl/type_tag.hpp"


namespace haikan {

class ReflectionContextFactory
{
public:

    struct Record
    {
        ReflectionMeta meta;
        std::function<void(sol::state_view)> on_init;
    };

    template <class T>
    class RegistrationContext
    {
    public:

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
        impl::type_tag<T>, impl::type_tag<W> = {}
    )
    {
        using Action = typename RegistrationContext<W>::Action;
        auto actions = std::make_shared<std::vector<Action>>();

        records.emplace_back();
        auto& record = records.back();
        record.meta.type_name = sol::usertype_traits<T>::name();
        record.meta.type_index_hash = typeid(T).hash_code();
        record.meta.wrapper_type_index_hash = typeid(W).hash_code();

        record.on_init = [meta=std::ref(record.meta), actions=actions](sol::state_view L){
            sol::simple_usertype<W> ut = L.create_simple_usertype<W>();
            for (auto& action: *actions)
            {
                action(ut);
            }
            ut.set("serialize", meta.get().serialize);
            ut.set("deserialize", meta.get().deserialize);

            sol::object lua_meta = sol::make_object(L.lua_state(), meta.get());

            sol::table root = impl::get_or_create_table(L, L.globals(), "haikan");
            sol::table utypes = impl::get_or_create_table(L, root, "utypes");
            utypes[meta.get().type_index_hash] = lua_meta;
            if (meta.get().wrapper_type_index_hash != meta.get().type_index_hash)
            {
                utypes[meta.get().wrapper_type_index_hash] = lua_meta;
            }

            ut.set("meta", lua_meta);
            L.set_usertype(meta.get().type_name, ut);
        };

        return RegistrationContext<W>{std::ref(record.meta), actions, state};
    }

private:

    friend class ReflectionRegistry;

    ReflectionContextFactory(sol::state_view L)
    : state{L}
    , records{}
    {
    }

    sol::state_view state;
    std::deque<Record> records;
};

} // namespace haikan
