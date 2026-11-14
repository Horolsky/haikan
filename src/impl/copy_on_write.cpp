#include <memory>
#include <set>

#include <boost/optional.hpp>

#include "haikan/impl/copy_on_write.hpp"

#include <iostream>

namespace
{

int cow_next_fn(lua_State* L) {

    using haikan::impl::Cow;
    sol::userdata self = sol::stack::get<sol::userdata>(L, 1);
    sol::stack_object current_key(L, 2);
    Cow cow = self.as<Cow>();

    bool const has_skeys = !cow.keys().empty();
    bool const has_ikeys = !cow.ikeys().empty();

    if (!has_skeys && !has_ikeys)
    {
        return 0;
    }

    if (current_key.is<sol::lua_nil_t>())
    {
        if (has_ikeys)
        {
            auto first = *cow.ikeys().cbegin();
            sol::stack::push(L, first);
            sol::stack::push(L, self[first]);
            return 2;
        }

        auto first = *cow.keys().cbegin();
        sol::stack::push(L, first);
        sol::stack::push(L, self[first]);
        return 2;
    }

    if (current_key.get_type() == sol::type::string)
    {
        std::string const key_str = current_key.as<std::string>();
        auto item = cow.keys().find(key_str);

        if (item != cow.keys().cend())
        {
            item++;
        }

        if (item == cow.keys().cend())
        {
            return 0;
        }
        std::string const next_key = *item;
        sol::stack::push(L, next_key);
        sol::stack::push(L, self[next_key]);
        return 2;
    }

    if (current_key.get_type() == sol::type::number)
    {
        std::int64_t const key_idx = current_key.as<std::int64_t>();
        auto item = cow.ikeys().find(key_idx);

        if (item != cow.ikeys().cend())
        {
            item++;
        }

        if (item != cow.ikeys().cend())
        {
            std::int64_t const next_key = *item;
            sol::stack::push(L, next_key);
            sol::stack::push(L, self[next_key]);
            return 2;
        }

        if (has_skeys)
        {
            auto first = *cow.keys().cbegin();
            sol::stack::push(L, first);
            sol::stack::push(L, self[first]);
            return 2;
        }
    }

    return 0;
}



} // namespace


namespace haikan
{
namespace impl
{

sol::object Cow::make(sol::object original)
{
    sol::state_view L(original.lua_state());
    register_utype(L);
    return sol::make_object(L, Cow(original));
}



void Cow::register_utype(sol::state_view L)
{
    static const std::string& registered_name = sol::usertype_traits<Cow>::name();
    if (L[registered_name].valid())
    {
        return;
    }

    L.new_usertype<Cow>(
        registered_name, sol::constructors<sol::types<sol::table>>(),
        sol::meta_function::index, &Cow::mf_index,
        sol::meta_function::new_index, &Cow::mf_new_index,
        sol::meta_function::pairs, [](Cow& self) { return std::make_tuple(cow_next_fn, std::ref(self), sol::lua_nil_t{}); },
        sol::meta_function::next, cow_next_fn,
        sol::meta_function::length, &Cow::length
    );
}

Cow::Cow(sol::table original, sol::table tracked)
    : state_{std::make_shared<State>()}
{
    state_->original = original;
    state_->tracked = tracked;

    auto maybe_tracked = state_->tracked[original];
    if (maybe_tracked != sol::nil)
    {
        *this = maybe_tracked.get<Cow>();
        return;
    }

    sol::state_view L(original.lua_state());
    state_->delta = L.create_table();
    state_->deleted = L.create_table();
    state_->proxies = L.create_table();

    state_->tracked[original] = sol::make_user(*this);

    if (original.is<Cow>())
    {
        Cow const orig_cow = original.as<Cow>();
        state_->keys = orig_cow.keys();
        state_->ikeys = orig_cow.ikeys();
        state_->length = orig_cow.length();
    }
    else
    {
        sol::stack::push(L, original);
        lua_Integer const len = luaL_len(L, -1);
        lua_pop(L, 1);
        state_->length = static_cast<std::size_t>(len);

        original.for_each([&](sol::object key, sol::object){

            switch (key.get_type())
            {
            case sol::type::string:
                state_->keys.insert(key.as<std::string>());
                break;
            case sol::type::number:
            {
                std::int64_t const idx = key.as<std::int64_t>();
                if ((idx >= 1) && (static_cast<std::size_t>(idx) <= state_->length))
                {
                    // pass
                }
                else
                {
                    state_->ikeys.insert(idx);
                }

            }

            default:
                break;
            }
        });

    }
}


void Cow::del_at_key(std::string const& key, sol::this_state _s)
{
    (void) _s;
    state_->delta[key] = sol::nil;
    state_->deleted[key] = 1;
    state_->keys.erase(key);
}

void Cow::del_at_idx(std::int64_t const idx, sol::this_state _s)
{
    (void) _s;
    state_->delta[idx] = sol::nil;
    state_->deleted[idx] = 1;
    state_->ikeys.erase(idx);
    if ((idx >= 1) && static_cast<std::size_t>(idx) == state_->length)
    {
        --state_->length;
    }
}

void Cow::insert_at_key(std::string const& key, sol::object const value, sol::this_state _s)
{
    (void) _s;
    state_->delta[key] = value;
    state_->deleted[key] = sol::nil;
    state_->keys.insert(key);
}

void Cow::insert_at_idx(std::int64_t const idx, sol::object const value, sol::this_state _s)
{
    (void) _s;
    if ((idx >= 1) && (static_cast<std::size_t>(idx) <= (state_->length)))
    {
        state_->delta[idx] = value;
        state_->deleted[idx] = sol::nil;
    }
    else if ((idx >= 1) && static_cast<std::size_t>(idx) == (state_->length + 1))
    {
        state_->delta[idx] = value;
        state_->deleted[idx] = sol::nil;
        if (value != sol::nil) { ++state_->length; };
    }
    else
    {
        state_->delta[idx] = value;
        state_->deleted[idx] = sol::nil;
        state_->ikeys.insert(idx);
    }
}



void Cow::mf_new_index(sol::object const key, sol::object value, sol::this_state _s)
{
    if (key.is<Cow>() && (&key.as<Cow>() == this))
    {
        return;
    }

    if (value == sol::nil)
    {
        switch (key.get_type())
        {
        case sol::type::string: return del_at_key(key.as<std::string>(), _s);
        case sol::type::number: return del_at_idx(key.as<std::int64_t>(), _s);
        default: return;
        }
    }

    auto const get_value = [&]() -> sol::object {
        if(value.get_type() == sol::type::table)
        {
            return sol::make_object(_s, Cow(value));
        }
        else
        {
            return value;
        }
    };

    switch (key.get_type())
    {
    case sol::type::string: return insert_at_key(key.as<std::string>(), get_value(), _s);
    case sol::type::number: return insert_at_idx(key.as<std::int64_t>(), get_value(), _s);
    default: return;
    }
}


sol::object Cow::mf_index(sol::object const key, sol::this_state _s)
{
    if (key.is<Cow>() && (&key.as<Cow>() == this))
    {
        return sol::make_object(_s, *this);
    }

    if (state_->deleted[key] != sol::nil)
    {
        return sol::nil;
    }
    else if(state_->delta[key] != sol::nil)
    {
        return state_->delta[key];
    }

    sol::object orig_value = state_->original[key];
    if (orig_value.get_type() == sol::type::table)
    {
        auto proxy = state_->proxies[key];
        if (proxy == sol::nil)
        {
            proxy = Cow(orig_value, state_->tracked);
        }
        return proxy;
    }
    return orig_value;
}


} // namespace impl
} // namespace haikan
