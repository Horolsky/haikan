/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/current_function.hpp>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <boost/optional.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/error.hpp"
#include "haikan/logger.hpp"
#include "haikan/reflection_context.hpp"
#include "haikan/serialization.hpp"
#include "haikan/type_tag.hpp"
#include "haikan/impl/default_next_fn.hpp"
#include "haikan/impl/operator_table.hpp"
#include "haikan/impl/reflect_serialize_fallback.hpp"
#include "haikan/impl/reflect_traits.hpp"
#include "haikan/impl/user_data_enum.hpp"

namespace haikan {
namespace impl {

class TypeRegistry;
template <class T, class W> class TypeRegistrar;

template <class> struct registry_bases_for_sol;
template <template <class...> class L, class... D>
struct registry_bases_for_sol<L<D...>> : sol::bases<typename D::type...> {};

template <class T, bool = boost::describe::has_describe_members<T>::value>
struct registered_members { using type = boost::mp11::mp_list<>; };

template <class T>
struct registered_members<T, true>
{
    using type = boost::describe::describe_members<
        T, boost::describe::mod_public | boost::describe::mod_inherited>;
};

template <class T>
struct registration_config
{
    using context_type = typename ReflectionContextFactory::template RegistrationContext<T>;
    std::vector<std::function<void(context_type&)>> actions;
    boost::optional<T> init_value;

    static registration_config& get()
    {
        static registration_config<T> result;
        return result;
    }
};


template <class T>
boost::optional<T> default_init(std::true_type)
{
    return T{};
}

template <class T>
boost::optional<T> default_init(std::false_type)
{
    return boost::none;
}

template <class T>
boost::optional<T> type_init()
{
    auto const& configured = registration_config<T>::get().init_value;
    return configured ? configured : default_init<T>(std::is_default_constructible<T>{});
}

template <class T, class = void> struct type_codec;

class TypeRegistry
{
    using init_function = void (*)(ReflectionContextFactory&);
    using registry_type = std::unordered_map<std::size_t, init_function>;

    static registry_type& registrations()
    {
        static registry_type result;
        return result;
    }

    template <class T, class W> friend class TypeRegistrar;

    template <class T>
    static void insert(void (*function)(ReflectionContextFactory&))
    {
        registrations().emplace(typeid(T).hash_code(), function);
    }

public:
    template <class T>
    TypeRegistrar<T, T> Add(type_tag<T>);

    template <class T>
    static boost::optional<T> Init(type_tag<T> = {})
    {
        return type_init<T>();
    }

    template <class T>
    static sol::object Serialize(T const& value, sol::state_view state)
    {
        return type_codec<T>::serialize(value, state);
    }

    template <class T>
    static boost::optional<T> Deserialize(sol::object object, type_tag<T> = {})
    {
        return type_codec<T>::deserialize(object);
    }

    void init(sol::state_view state)
    {
        ReflectionContextFactory factory(state);
        for (auto const& registration: registrations())
        {
            registration.second(factory);
        }
        for (auto& record: factory.records)
        {
            record.on_init(state);
        }

        auto const& operator_types = OperatorRegistry::types_seen();
        auto log = HAIKAN_LOG_CERR(DEBUG);
        log << "Missing runtime reflection: {";
        for (auto const& entry: operator_types)
        {
            if (registrations().find(entry.first) == registrations().end())
            {
                log << entry.second << ',';
            }
        }
        log << '}';
    }
};

template <class T, class Enable>
struct type_codec
{
    static sol::object serialize(T const& value, sol::state_view state)
    {
        auto meta = get_rmeta(state, type<T>);
        if (meta && meta->serialize)
        {
            sol::protected_function_result result = meta->serialize(
                sol::make_object(state.lua_state(), std::ref(const_cast<T&>(value))));
            if (result.valid())
            {
                return result.template get<sol::object>();
            }
        }
        return reflect_serialize_fallback<T>::serialize(value, state);
    }

    static boost::optional<T> deserialize(sol::object object)
    {
        auto meta = get_rmeta(sol::state_view(object.lua_state()), type<T>);
        if (meta && meta->deserialize)
        {
            sol::protected_function_result result = meta->deserialize(object);
            if (result.valid())
            {
                auto value = reflect_serialize_fallback<T>::deserialize(
                    result.template get<sol::object>());
                if (value)
                {
                    return value;
                }
            }
        }
        return reflect_serialize_fallback<T>::deserialize(object);
    }
};

template <class T>
struct type_codec<T const, void> : type_codec<T> {};

template <class T>
struct type_codec<std::reference_wrapper<T>, void>
{
    static sol::object serialize(std::reference_wrapper<T> value, sol::state_view state)
    {
        return TypeRegistry::Serialize(value.get(), state);
    }

    static boost::optional<std::reference_wrapper<T>> deserialize(sol::object)
    {
        return boost::none;
    }
};

template <class T>
struct type_codec<std::shared_ptr<T>, void>
{
    static sol::object serialize(std::shared_ptr<T> const& value, sol::state_view state)
    {
        return value ? TypeRegistry::Serialize(*value, state) : sol::make_object(state, sol::nil);
    }

    static boost::optional<std::shared_ptr<T>> deserialize(sol::object object)
    {
        auto value = TypeRegistry::Deserialize(object, type<T>);
        return value
            ? boost::optional<std::shared_ptr<T>>(std::make_shared<T>(std::move(value.value())))
            : boost::none;
    }
};

template <class T>
struct array_codec
{
    static sol::object serialize(T const& container, sol::state_view state)
    {
        sol::table result = state.create_table(container.size(), 0);
        for (auto const& item: container)
        {
            result.add(TypeRegistry::Serialize(item, state));
        }
        return result;
    }

    static boost::optional<T> deserialize(sol::object object)
    {
        if (object.get_type() != sol::type::table)
        {
            return boost::none;
        }
        return deserialize_impl(sol::table(object), has_member_push_back<T>{}, has_member_insert_value<T>{});
    }

private:
    template <class U>
    static void reserve(U& value, std::size_t size, std::true_type) { value.reserve(size); }
    template <class U>
    static void reserve(U&, std::size_t, std::false_type) {}

    static boost::optional<T> deserialize_impl(sol::table table, std::true_type, std::false_type)
    {
        T result{};
        reserve(result, table.size(), has_member_reserve<T>{});
        for (std::size_t i = 1; i <= table.size(); ++i)
        {
            auto item = TypeRegistry::Deserialize(table[i], type<typename T::value_type>);
            if (!item)
            {
                return boost::none;
            }
            result.push_back(std::move(item.value()));
        }
        return result;
    }

    static boost::optional<T> deserialize_impl(sol::table table, std::false_type, std::true_type)
    {
        T result{};
        reserve(result, table.size(), has_member_reserve<T>{});
        for (std::size_t i = 1; i <= table.size(); ++i)
        {
            auto item = TypeRegistry::Deserialize(table[i], type<typename T::value_type>);
            if (!item)
            {
                return boost::none;
            }
            result.insert(std::move(item.value()));
        }
        return result;
    }

    static boost::optional<T> deserialize_impl(sol::table table, std::false_type, std::false_type)
    {
        T result{};
        if (table.size() != result.size())
        {
            return boost::none;
        }
        for (std::size_t i = 1; i <= table.size(); ++i)
        {
            auto item = TypeRegistry::Deserialize(table[i], type<typename T::value_type>);
            if (!item)
            {
                return boost::none;
            }
            result[i - 1] = std::move(item.value());
        }
        return result;
    }
};

template <class T>
struct type_codec<T, typename std::enable_if<is_array_like_container<T>::value>::type>
    : array_codec<T>
{};

template <class T1, class T2>
struct type_codec<std::pair<T1, T2>, void>
{
    static sol::object serialize(std::pair<T1, T2> const& value, sol::state_view state)
    {
        sol::table result = state.create_table(2, 0);
        result[1] = TypeRegistry::Serialize(value.first, state);
        result[2] = TypeRegistry::Serialize(value.second, state);
        return result;
    }

    static boost::optional<std::pair<T1, T2>> deserialize(sol::object object)
    {
        if (object.get_type() != sol::type::table)
        {
            return boost::none;
        }
        sol::table table = object;
        auto first = TypeRegistry::Deserialize(table[1], type<T1>);
        auto second = TypeRegistry::Deserialize(table[2], type<T2>);
        if (!first || !second)
        {
            return boost::none;
        }
        return std::make_pair(std::move(first.value()), std::move(second.value()));
    }
};

template <class T, class W = T>
class TypeRegistrar
{
    using ThisType = TypeRegistrar<T, W>;
    using context_type = typename ReflectionContextFactory::template RegistrationContext<T>;
    using Descr = typename registered_members<T>::type;
    bool committed_ = false;

    template <class Key, class Value>
    void usertype_set(Key&& key, Value&& value)
    {
        using key_type = typename std::decay<Key>::type;
        using value_type = typename std::decay<Value>::type;
        key_type saved_key(std::forward<Key>(key));
        value_type saved_value(std::forward<Value>(value));
        registration_config<T>::get().actions.emplace_back(
            [key=std::move(saved_key), value=std::move(saved_value)](context_type& ctx) mutable {
                ctx.usertype_set(key, value);
            });
    }

    template <class M, class Owner>
    static M Owner::* make_field(M Owner::* pointer, std::false_type) { return pointer; }

    template <class M, class Owner>
    static decltype(auto) make_field(M Owner::* pointer, std::true_type)
    {
        using enum_type = user_data_enum<M>;
        return sol::property(
            [pointer](T const& object) { return enum_type(object.*pointer); },
            [pointer](T& object, sol::object value) -> M {
                M result{};
                if (value.is<enum_type>())
                {
                    result = static_cast<M>(value.as<enum_type>());
                }
                else if (value.get_type() == sol::type::number)
                {
                    result = static_cast<M>(value.as<typename enum_type::underlying_type>());
                }
                else if (value.get_type() == sol::type::string)
                {
                    result = static_cast<M>(enum_type(value.as<std::string>()));
                }
                object.*pointer = result;
                return result;
            });
    }

    template <class M, class Owner>
    static decltype(auto) make_field(M Owner::* pointer)
    {
        return make_field(pointer, std::is_enum<M>{});
    }

    static sol::object construct_impl(sol::variadic_args args)
    {
        sol::state_view state(args.lua_state());
        auto value = TypeRegistry::Init(type<T>);
        if (!value)
        {
            return sol::make_object(state, error("type is not default constructible", BOOST_CURRENT_FUNCTION));
        }
        auto arg = args.begin();
        if (arg != args.end() && sol::object(*arg) == sol::object(state[sol::usertype_traits<T>::name()]))
        {
            ++arg;
        }
        bool failed = false;
        boost::mp11::mp_for_each<Descr>([&](auto descriptor) {
            if (failed || arg == args.end())
            {
                return;
            }

            using member_type = remove_qualifiers_t<decltype(value.value().*descriptor.pointer)>;
            auto member = TypeRegistry::Deserialize(sol::object(*arg++), type<member_type>);
            if (!member) { failed = true; return; }
            value.value().*descriptor.pointer = std::move(member.value());
        });
        return failed
            ? sol::make_object(state, error("invalid ctor argument", BOOST_CURRENT_FUNCTION))
            : sol::make_object(state, std::move(value.value()));
    }

    static sol::object serialize_impl(T const& value, sol::this_state lua_state)
    {
        sol::state_view state(lua_state);
        sol::table result = state.create_table(0, static_cast<int>(boost::mp11::mp_size<Descr>::value));
        boost::mp11::mp_for_each<Descr>([&](auto descriptor) {
            using member_type = remove_qualifiers_t<decltype(value.*descriptor.pointer)>;
            result.set(descriptor.name, serialize_member(
                value.*descriptor.pointer, state, std::is_enum<member_type>{}));
        });
        return result;
    }

    template <class M>
    static sol::object serialize_member(M const& value, sol::state_view state, std::false_type)
    {
        return TypeRegistry::Serialize(value, state);
    }

    template <class M>
    static sol::object serialize_member(M value, sol::state_view state, std::true_type)
    {
        return sol::make_object(state, user_data_enum<M>(value).to_string());
    }

    static boost::optional<T> deserialize_impl(sol::object object)
    {
        if (object.get_type() != sol::type::table)
        {
            return boost::none;
        }
        auto value = TypeRegistry::Init(type<T>);
        if (!value)
        {
            return boost::none;
        }
        sol::table table = object;
        bool failed = false;
        boost::mp11::mp_for_each<Descr>([&](auto descriptor) {
            if (failed)
            {
                return;
            }
            using member_type = remove_qualifiers_t<decltype(value.value().*descriptor.pointer)>;
            auto member = TypeRegistry::Deserialize(table[descriptor.name], type<member_type>);
            if (!member) { failed = true; return; }
            value.value().*descriptor.pointer = std::move(member.value());
        });
        return failed ? boost::optional<T>{} : value;
    }

    static void initialize_struct(ReflectionContextFactory& factory)
    {
        using Bases = boost::describe::describe_bases<T, boost::describe::mod_public>;
        auto ctx = factory.make_registration_context(type<T>);
        auto& meta = ctx.reflection_meta();
        ctx.usertype_set(sol::meta_function::construct, sol::factories(&ThisType::construct_impl));
        ctx.usertype_set(sol::call_constructor, sol::factories(&ThisType::construct_impl));
        ctx.usertype_set(sol::base_classes, registry_bases_for_sol<Bases>());
        meta.members.reserve(boost::mp11::mp_size<Descr>::value);
        boost::mp11::mp_for_each<Descr>([&](auto descriptor) {
            using member_type = remove_qualifiers_t<decltype(std::declval<T>().*descriptor.pointer)>;
            ctx.usertype_set(descriptor.name, make_field(descriptor.pointer));
            meta.members.emplace_back(descriptor.name);
            TypeRegistrar<member_type>{}.commit_into(factory);
        });
        sol::function next = make_reflect_default_next_fn(meta.members, ctx.state_view());
        ctx.usertype_set(sol::meta_function::pairs, make_reflect_default_pairs_fn(next));
        ctx.usertype_set(sol::meta_function::next, next);
        meta.serialize = ctx.make_function(&ThisType::serialize_impl);
        meta.deserialize = ctx.make_function([](sol::object object) -> sol::object {
            auto value = ThisType::deserialize_impl(object);
            return value ? sol::make_object(object.lua_state(), std::move(value.value()))
                         : sol::make_object(object.lua_state(), sol::nil);
        });
        for (auto& action: registration_config<T>::get().actions)
        {
            action(ctx);
        }
    }

    static void initialize_enum(ReflectionContextFactory& factory)
    {
        using enum_type = user_data_enum<T>;
        auto ctx = factory.make_registration_context(type<T>, type<enum_type>);
        auto constructor = sol::factories([](sol::variadic_args args) -> sol::object {
            sol::state_view state(args.lua_state());
            if (args.leftover_count() == 0)
            {
                auto value = TypeRegistry::Init(type<T>);
                return value ? sol::make_object(state, enum_type(value.value()))
                             : sol::make_object(state, error("enum is not default constructible", BOOST_CURRENT_FUNCTION));
            }
            if (args.leftover_count() != 1)
            {
                return sol::make_object(state, error("invalid argument count", BOOST_CURRENT_FUNCTION));
            }
            sol::object value = *args.begin();
            if (value.is<enum_type>())
            {
                return sol::make_object(state, value.as<enum_type>());
            }
            if (value.is<typename enum_type::underlying_type>())
            {
                return sol::make_object(state, enum_type(value.as<typename enum_type::underlying_type>()));
            }
            if (value.is<std::string>())
            {
                return sol::make_object(state, enum_type(value.as<std::string>()));
            }
            return sol::make_object(state, error("invalid argument", BOOST_CURRENT_FUNCTION));
        });
        ctx.usertype_set(sol::meta_function::construct, constructor);
        ctx.usertype_set(sol::call_constructor, constructor);
        ctx.usertype_set("str", &enum_type::to_string);
        ctx.usertype_set("num", &enum_type::value);
        ctx.usertype_set(sol::meta_function::to_string, &enum_type::to_string);
        auto& meta = ctx.reflection_meta();
        meta.serialize = ctx.make_function([](enum_type const& value, sol::this_state state) {
            return sol::make_object(state.L, value.to_string());
        });
        meta.deserialize = ctx.make_function([](sol::object object) -> sol::object {
            boost::optional<enum_type> value;
            if (object.is<enum_type>())
            {
                value = object.as<enum_type>();
            }
            else if (object.is<typename enum_type::underlying_type>())
            {
                value = enum_type(object.as<typename enum_type::underlying_type>());
            }
            else if (object.is<std::string>())
            {
                value = enum_type(object.as<std::string>());
            }
            return value ? sol::make_object(object.lua_state(), value.value())
                         : sol::make_object(object.lua_state(), sol::nil);
        });
    }

    static void initialize_container(ReflectionContextFactory& factory)
    {
        auto ctx = factory.make_registration_context(type<T>);
        auto& meta = ctx.reflection_meta();
        meta.serialize = ctx.make_function([](T const& value, sol::this_state state) {
            return array_codec<T>::serialize(value, sol::state_view(state));
        });
        meta.deserialize = ctx.make_function([](sol::object object) -> sol::object {
            auto value = array_codec<T>::deserialize(object);
            return value ? sol::make_object(object.lua_state(), std::move(value.value()))
                         : sol::make_object(object.lua_state(), sol::nil);
        });
    }

    static void initialize(ReflectionContextFactory& factory)
    {
        initialize_dispatch(factory, boost::describe::has_describe_members<T>{},
            boost::describe::has_describe_enumerators<T>{}, is_array_like_container<T>{});
    }

    static void initialize_dispatch(ReflectionContextFactory& f, std::true_type, std::false_type, std::false_type) { initialize_struct(f); }
    template <class A, class C> static void initialize_dispatch(ReflectionContextFactory& f, A, std::true_type, C) { initialize_enum(f); }
    template <class A, class E> static void initialize_dispatch(ReflectionContextFactory& f, A, E, std::true_type) { initialize_container(f); }
    static void initialize_dispatch(ReflectionContextFactory& factory, std::false_type, std::false_type, std::false_type)
    {
        initialize_indirect(factory, type<T>);
    }

    template <class U>
    static void initialize_indirect(ReflectionContextFactory& factory, type_tag<std::reference_wrapper<U>>)
    {
        TypeRegistrar<U>{}.commit_into(factory);
    }

    template <class U>
    static void initialize_indirect(ReflectionContextFactory& factory, type_tag<std::shared_ptr<U>>)
    {
        TypeRegistrar<U>{}.commit_into(factory);
    }

    template <class U>
    static void initialize_indirect(ReflectionContextFactory&, type_tag<U>)
    {
    }

    void commit_into(ReflectionContextFactory& factory)
    {
        initialize(factory);
        committed_ = true;
    }

    template <class U, class X> friend class TypeRegistrar;

public:
    TypeRegistrar() { static_assert(!std::is_union<T>::value, "union types are not supported"); }
    TypeRegistrar(TypeRegistrar const&) = delete;
    TypeRegistrar& operator=(TypeRegistrar const&) = delete;
    TypeRegistrar(TypeRegistrar&& other) noexcept: committed_(other.committed_) { other.committed_ = true; }
    ~TypeRegistrar() { commit(); }

    TypeRegistrar& InitValue(T const& value) { registration_config<T>::get().init_value = value; return *this; }

    template <class M, class Owner>
    TypeRegistrar& Member(std::string const& name, M Owner::* pointer)
    {
        static_assert(std::is_base_of<Owner, T>::value || std::is_same<Owner, T>::value, "invalid member owner");
        usertype_set(name, make_field(pointer));
        return *this;
    }

    template <class M>
    TypeRegistrar& MetaField(sol::meta_function function, M&& value)
    {
        usertype_set(function, std::forward<M>(value));
        return *this;
    }

    template <class U, template <class...> class... C>
    typename OperatorRegistry::template SignatureBuilder<U, C...>
    OperatorsWith(type_tag<U> tag, template_list_t<C...> containers = {})
    {
        return OperatorRegistry::AddSignature(tag, containers);
    }

    void commit()
    {
        if (!committed_)
        {
            TypeRegistry::insert<T>(&ThisType::initialize);
            committed_ = true;
        }
    }

    template <class U>
    TypeRegistrar<U> Add(type_tag<U>) { commit(); return TypeRegistrar<U>{}; }
};

template <class T>
TypeRegistrar<T, T> TypeRegistry::Add(type_tag<T>)
{
    return TypeRegistrar<T, T>{};
}

} // namespace impl
} // namespace haikan
