#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include <boost/optional.hpp>


#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

#include "haikan/impl/traits.hpp"

namespace haikan
{
namespace impl
{

inline bool is_preproc_token(boost::string_view str)
{
    return str.starts_with("$[") && str.ends_with("]");
}

inline bool is_link_token(boost::string_view str)
{
    if (str.size() <= 1 || !str.starts_with("$")) return false;
    auto const second = str.at(1);
    auto const end = str.back();

    bool const is_bracketed // TODO: regex
        =   (second == '[' && end == ']')
        ||  (second == '{' && end == '}')
        ||  (second == '(' && end == ')');
    return not is_bracketed && (second != '$');
}


class ExpressionParameter
{
    struct Meta
    {
        sol::type sol_type{sol::type::nil};
        std::size_t type_index_hash;
        bool is_preproc_token;
        bool is_link_token;
    };


    static Meta get_meta(sol::object obj)
    {
        Meta m{};
        m.sol_type = obj.get_type();
        m.type_index_hash = typeid(sol::object).hash_code();

        if (obj.get_type() == sol::type::string)
        {
            boost::string_view str = obj.as<char const*>(); // TODO: check me
            m.is_preproc_token = ::haikan::impl::is_preproc_token(str);
            m.is_link_token = ::haikan::impl::is_link_token(str);
        }
        return m;
    }


    // TODO: handle SFINAE for T string vs non-string to select
    // is_preproc_token/is_link_token. Do not integrate or test, make implementation only.
    template <class T>
    static auto get_meta(T const& value) -> mp_if<has_boost_string_view<T>, Meta>
    {
        Meta m{};
        m.sol_type = sol::type::string;
        m.type_index_hash = typeid(typename std::decay<T>::type).hash_code();

        boost::string_view str = value; // TODO: check me
        m.is_preproc_token = ::haikan::impl::is_preproc_token(str);
        m.is_link_token = ::haikan::impl::is_link_token(str);
        return m;
    }

    template <class T>
    static auto get_meta(T const&) -> mp_if<mp_not<has_boost_string_view<T>>, Meta>
    {
        Meta m{};
        using Value = typename std::decay<T>::type;
        m.sol_type = std::is_same<Value, bool>::value ? sol::type::boolean
            : std::is_arithmetic<Value>::value ? sol::type::number
            : std::is_same<Value, sol::nil_t>::value ? sol::type::nil
            : sol::type::userdata;
        m.type_index_hash = typeid(Value).hash_code();

        m.is_preproc_token = false;
        m.is_link_token = false;
        return m;
    }

public:
    ExpressionParameter() : ExpressionParameter(sol::nil)
    {
    }

    ExpressionParameter(std::function<sol::object(sol::state_view)> getter)
        : getter_{std::move(getter)}
        , meta_{}
        , cache_{}
        , cache_state_{nullptr}
        , source_state_{nullptr}
    {
    }


    ExpressionParameter(sol::object obj)
        : getter_{[obj](sol::state_view sv) -> sol::object
            {
                if (sv == nullptr || sv == obj.lua_state())
                {
                    return obj;
                }
                // TODO: handle state exchange (maybe)
                return obj;
            }}
        , meta_{get_meta(obj)}
        , cache_{obj}
        , cache_state_{obj.lua_state()}
        , source_state_{obj.lua_state()}
    {
    }

    template <class T>
    ExpressionParameter(T v)
        : getter_{[v](sol::state_view sv) -> sol::object
            {
                return sol::make_object(sv.lua_state(), v);
            }}
        , meta_{get_meta(v)}
        , cache_{}
        , cache_state_{nullptr}
        , source_state_{nullptr}
    {
    }

    ~ExpressionParameter() = default;

    ExpressionParameter(ExpressionParameter const &) = default;
    ExpressionParameter(ExpressionParameter &&) = default;
    ExpressionParameter &operator=(ExpressionParameter const &) = default;
    ExpressionParameter &operator=(ExpressionParameter &&) = default;

    // Load the lazy parameter into Lua state.
    // Return last valid state cached if null state given,
    // otherwise, update the cache.
    sol::object load() const
    {
        evaluate_cache(sol::state_view{nullptr});
        return cache_.value();
    }

    sol::object load(sol::state_view sv) const
    {
        cache_ = getter_(sv);
        cache_state_ = sv.lua_state();
        meta_ = get_meta(cache_.value());
        return cache_.value();
    }

    bool valid() const
    {
        return cache_state_ != nullptr && cache_ && cache_->valid();
    }

    lua_State* cached_state() const
    {
        return cache_state_;
    }

    lua_State* source_state() const
    {
        return source_state_;
    }

    friend bool operator==(ExpressionParameter const &l, ExpressionParameter const &r)
    {
        return l.cache_ && l.cache_->valid() && l.cache_ == r.cache_;
    }

    sol::type sol_type() const
    {
        evaluate_meta();
        return meta_->sol_type;
    };
    std::size_t type_index_hash() const
    {
        evaluate_meta();
        return meta_->type_index_hash;
    };
    bool is_preproc_token() const
    {
        evaluate_meta();
        return meta_->is_preproc_token;
    };
    bool is_link_token() const
    {
        evaluate_meta();
        return meta_->is_link_token;
    };

private:
    void evaluate_cache(sol::state_view sv) const
    {
        if (cache_)
        {
            return;
        }
        cache_ = getter_(sv);
        cache_state_ = cache_.value().lua_state();
    }

    void evaluate_meta() const
    {
        if (meta_)
        {
            return;
        }
        evaluate_cache(sol::state_view{nullptr});
        meta_ = get_meta(cache_.value());
    }

    std::function<sol::object(sol::state_view)> getter_;
    mutable boost::optional<Meta> meta_;
    mutable boost::optional<sol::object> cache_;
    mutable lua_State* cache_state_;
    lua_State* source_state_;
};

} // namespace impl
} // namespace haikan
