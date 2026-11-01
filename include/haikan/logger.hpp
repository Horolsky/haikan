/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <iomanip>
#include <string>

#include <boost/describe.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/json/string_view.hpp>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>


#include "haikan/impl/lua_state.hpp"


#define HAIKAN_STRINGIFY_IMPL(x) #x
#define HAIKAN_STRINGIFY(x) HAIKAN_STRINGIFY_IMPL(x)
#define HAIKAN_CUR_LOC __FILE__ "#" HAIKAN_STRINGIFY(__LINE__)

#ifdef NDEBUG
    #define HAIKAN_DEBUG (std::cerr << HAIKAN_CUR_LOC << ": ")
#else
    #define HAIKAN_DEBUG ::haikan::StubLogger()
#endif

#define HAIKAN_LOG(lvl) ::haikan::Logger().WithSrcLoc(HAIKAN_CUR_LOC).WithLevel(::haikan::Logger::lvl)
#define HAIKAN_LOG_CERR(lvl) HAIKAN_LOG(lvl).WithOutput(::haikan::Logger::STDERR)
#define HAIKAN_LOG_JSON(lvl) HAIKAN_LOG(lvl).WithOutput(::haikan::Logger::JSON)

namespace haikan {


#ifndef NDEBUG
struct StubLogger
{
    template <class T>
    auto operator<<(T&&)
    -> std::enable_if_t<not std::is_base_of<sol::object, std::decay_t<T>>::value, StubLogger&>
    {
        return *this;
    }
};
#endif


// Logger is flushed on dtor, so no multiple instances are allowed
class Logger {
  public:

    enum Output
    {
        NIL = 0,
        STDERR = 1 << 0,
        JSON   = 1 << 1,
    };

    enum Level
    {
        FATAL,
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        TRACE,
        DEVMODE, // development troubleshooting and perf stats
    };


    static void set_max_level(Level const max_level);
    static Level max_level();

    static void set_notrim(bool const trim_line);
    static bool is_notrim_enabled();

    static void set_pretty_print(bool const trim_line);
    static bool is_pretty_print_enabled();


    Logger();

    Logger(Logger&&) noexcept = default;
    Logger(Logger const&) = delete;
    Logger& operator=(Logger&&) noexcept = default;
    Logger& operator=(Logger const&) = delete;

    Logger& WithLevel(Level const level) &;
    Logger&& WithLevel(Level const level) &&;
    Logger& WithOutput(Output const output) &;
    Logger&& WithOutput(Output const output) &&;
    Logger& WithSrcLoc(boost::string_view const src_loc) &;
    Logger&& WithSrcLoc(boost::string_view const src_loc) &&;

    void drain(std::ostream& sink, bool const skip_meta = false);

    Logger& operator<<(sol::object && value) &
    {
        payload_cache_.emplace_back(std::move(value));
        return *this;
    }
    Logger&& operator<<(sol::object && value) &&
    {
        payload_cache_.emplace_back(std::move(value));
        return std::move(*this);
    }
    Logger& operator<<(sol::object const& value) &
    {
        payload_cache_.emplace_back(value);
        return *this;
    }
    Logger&& operator<<(sol::object const& value) &&
    {
        payload_cache_.emplace_back(value);
        return std::move(*this);
    }

    Logger& operator<<(boost::string_view value) &
    {
        payload_cache_.emplace_back(impl::lua::make_object(value.data()));
        return *this;
    }
    Logger&& operator<<(boost::string_view value) &&
    {
        payload_cache_.emplace_back(impl::lua::make_object(value.data()));
        return std::move(*this);
    }

    Logger& operator<<(boost::json::string_view value) &
    {
        payload_cache_.emplace_back(impl::lua::make_object(value.data()));
        return *this;
    }
    Logger&& operator<<(boost::json::string_view value) &&
    {
        payload_cache_.emplace_back(impl::lua::make_object(value.data()));
        return std::move(*this);
    }

    Logger& operator<<(std::string const& value) &
    {
        payload_cache_.emplace_back(impl::lua::make_object(value));
        return *this;
    }
    Logger&& operator<<(std::string const& value) &&
    {
        payload_cache_.emplace_back(impl::lua::make_object(value));
        return std::move(*this);
    }

    // TODO: handle common lua 
    template <class T>
    auto operator<<(T&& value) &
    -> std::enable_if_t<
            not std::is_base_of<sol::object, std::decay_t<T>>::value
        and not std::is_same<boost::string_view, T>::value
        and not std::is_same<boost::json::string_view, T>::value
    , Logger&>
    {
        return this->operator<<(impl::lua::make_object(std::forward<T>(value)));
    }
    template <class T>
    auto operator<<(T&& value) &&
    -> std::enable_if_t<
            not std::is_base_of<sol::object, std::decay_t<T>>::value
        and not std::is_same<boost::string_view, T>::value
        and not std::is_same<boost::json::string_view, T>::value
    , Logger&&>
    {
        this->operator<<(impl::lua::make_object(std::forward<T>(value)));
        return std::move(*this);
    }

    ~Logger();

  private:

    Level level_{INFO};
    int output_{STDERR | JSON};
    boost::string_view src_loc_{"unknown"};
    char timestamp_[64];
    std::vector<sol::object> payload_cache_;

    void reset_timestamp();

    BOOST_DESCRIBE_NESTED_ENUM(Level,
        FATAL,
        ERROR,
        WARNING,
        INFO,
        DEBUG,
        TRACE,
        DEVMODE
    )
};

}  // namespace haikan
