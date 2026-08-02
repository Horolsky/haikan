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
#include <tuple>
#include <utility>
#include <vector>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/reflection_meta.hpp"
#include "haikan/impl/type_tag.hpp"


namespace haikan {

class ReflectionContext
{
public:

    struct Record
    {
        std::size_t type_index_hash;
        sol::table meta; // haikan meta
        std::function<void(sol::state_view)> on_init;
    };

    template <class T>
    class RegistrationTable
    {
    public:

        template <class Key, class Value>
        void set(Key&& key, Value&& value)
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
        sol::table& hmeta()
        {
            return table;
        }

    private:

        friend class ReflectionContext;

        using Action = std::function<void(sol::simple_usertype<T>&)>;

        RegistrationTable(sol::table table, std::shared_ptr<std::vector<Action>> actions)
        : table{std::move(table)}
        , actions{std::move(actions)}
        {
        }

        sol::table table;
        std::shared_ptr<std::vector<Action>> actions;
    };


    ~ReflectionContext() = default;

    template <class T>
    RegistrationTable<T> get_registration_table(impl::type_tag<T>)
    {
        sol::table hmeta = state.create_table();
        hmeta["type_name"] = sol::usertype_traits<T>::name();
        hmeta["type_index_hash"] = typeid(T).hash_code();

        using Action = typename RegistrationTable<T>::Action;
        auto actions = std::make_shared<std::vector<Action>>();

        records.emplace_back();
        auto& record = records.back();
        record.on_init = [hmeta=hmeta, actions=actions](sol::state_view state){
            sol::simple_usertype<T> ut = state.create_simple_usertype<T>();
            for (auto& action: *actions)
            {
                action(ut);
            }
            state.set_usertype(hmeta["type_name"].get<std::string>(), ut);
            // ut.set("type_index", typeid(T).hash_code());
        };

        record.type_index_hash = typeid(T).hash_code();
        record.meta = hmeta;

        return RegistrationTable<T>{hmeta, actions};
    }

private:

    friend class ReflectionRegistry;

    ReflectionContext(sol::state_view L)
    : state{L}
    , records{}
    {
    }

    sol::state_view state;
    std::deque<Record> records;
};

} // namespace haikan
