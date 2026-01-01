/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <chrono>
#include <iomanip>
#include <string>

#include <boost/describe.hpp>
#include <boost/json.hpp>


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
    StubLogger& operator<<(boost::json::value const&)
    {
        return *this;
    }
};
#endif

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

    /// Attach logger to file sink ("" means stdout)
    static void open_json(const std::string& filename = "");

    static void set_max_level(Level const max_level);
    static Level max_level();

    static void set_notrim(bool const trim_line);
    static bool is_notrim_enabled();

    static void set_pretty_print(bool const trim_line);
    static bool is_pretty_print_enabled();


    Logger();

    Logger& WithLevel(Level const level);
    Logger& WithOutput(Output const output);
    Logger& WithSrcLoc(boost::json::string_view const src_loc);

    Logger& operator<<(boost::json::value const& value);
    Logger& operator<<(boost::json::value && value);

    template <class T>
    Logger& operator<<(T&& value)
    {
        return this->operator<<(boost::json::value_from(std::forward<T>(value)));
    }

    ~Logger();

  private:

    Level level_{INFO};
    int output_{STDERR | JSON};
    boost::json::string_view src_loc_{"unknown"};
    char timestamp_[64];
    boost::json::array payload_cache_;

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
