/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <atomic>
#include <fstream>
#include <iostream>
#include <iostream>
#include <mutex>
#include <chrono>

#include "haikan/logger.hpp"
#include "haikan/reflect.hpp"
#include "haikan/impl/lua_state.hpp"
#include "haikan/impl/lua_stream.hpp"
#include "haikan/impl/expression_lua.hpp"
#include "haikan/impl/vm.hpp"


namespace{
using Level = haikan::Logger::Level;

static Level g_max_log_level = Level::TRACE;
static bool g_notrim = true;
static bool g_pretty_print = false;


char const* level_to_str(Level const& l)
{
    switch(l)
    {
        case Level::FATAL: return "FATAL";
        case Level::ERROR: return "ERROR";
        case Level::WARNING: return "WARNING";
        case Level::INFO: return "INFO";
        case Level::DEBUG: return "DEBUG";
        case Level::TRACE: return "TRACE";
        case Level::DEVMODE: return "DEVMODE";
        default: return "UNKNOWN";
    }
}

} // namespace

namespace haikan {


void Logger::set_max_level(Level const max_level)
{
    g_max_log_level = max_level;
}

void Logger::set_notrim(bool const notrim)
{
    g_notrim = notrim;
}

bool Logger::is_notrim_enabled()
{
    return g_notrim;
}

Level Logger::max_level()
{
    return g_max_log_level;
}


void Logger::set_pretty_print(bool const pretty_print)
{
    g_pretty_print = pretty_print;
}

bool Logger::is_pretty_print_enabled()
{
    return g_pretty_print;
}

Logger::Logger()
{
   reset_timestamp();
}

Logger& Logger::WithOutput(Logger::Output const output) &
{
    output_ = output;
    return *this;
}

Logger&& Logger::WithOutput(Logger::Output const output) &&
{
    output_ = output;
    return std::move(*this);
}

Logger& Logger::WithLevel(Level const level) &
{
    level_ = level;
    return *this;
}

Logger&& Logger::WithLevel(Level const level) &&
{
    level_ = level;
    return std::move(*this);
}

Logger& Logger::WithSrcLoc(boost::string_view const src_loc) &
{
    src_loc_ = src_loc;
    return *this;
}

Logger&& Logger::WithSrcLoc(boost::string_view const src_loc) &&
{
    src_loc_ = src_loc;
    return std::move(*this);
}




Logger::~Logger()
{
    if (payload_cache_.empty()) return;
    if (level_ > g_max_log_level) return;

    auto const level = level_to_str(level_);
    boost::string_view const timestamp{timestamp_};

    // if (JSON & output_)
    // {
    //     boost::json::value const& payload = payload_cache_.size() == 1 ? payload_cache_.front() : payload_cache_;
    //     AsyncJsonLogger::instance().push({
    //         {"timestamp", timestamp },
    //         {"level" , level  },
    //         {"src_loc"  , src_loc_  },
    //         {"payload"  , payload   },
    //     });
    // }
    // if (STDERR & output_)
    {
        std::stringstream ss;
        ss << timestamp
        << ' ' << level
        << ' ' << src_loc_
        << ' ';
        for (auto const& record: payload_cache_)
        {
            if (not record.lua_state())
            {
                ss << "NULL";
            }
            else if (record.is<impl::ExpressionLua>())
            {
                record.as<impl::ExpressionLua>().prettify_to(ss);
            }
            else if (record.is<error>())
            {
                ss << record.as<error const&>();
            }
            else if (record.is<impl::Args>())
            {
                // TODO: fix unescaped null characters terminating whole log message here
                ss << "{";
                impl::Args const& args = record.as<impl::Args const&>();
                impl::lua_to_stream(ss, args.lhs, 0, false);
                ss << ", ";
                impl::lua_to_stream(ss, args.rhs, 0, false);
                ss << "}";

                // ss << "{" << args.lhs << args.rhs 
            }
            else if (record.get_type() == sol::type::userdata)
            {
                sol::object plain = to_plain_lua(record);
                if (plain.is<error>())
                {
                    ss << plain.as<error const&>();
                }
                else
                {
                    // std::cerr << " -- " << ss.str() << '\n';
                    impl::lua_to_stream(ss, plain, 0, false);
                    // std::cerr << " ++ " << ss.str() << '\n';
                }
            }
            else if (record.get_type() == sol::type::string)
            {
                ss << record.as<char const*>();
            }
            else
            {
                impl::lua_to_stream(ss, record, 0, false);
            }
            // if (auto const as_string =record.if_string())
            // {
            //     ss << as_string->c_str();
            // }
            // else
            // {
            //     ss << record;
            // }
        }
        std::cerr << ss.str() << '\n';
    }
}

// ISO8601 + nanoseconds
void Logger::reset_timestamp()
{

    using namespace std::chrono;
    std::memset(timestamp_, 0, sizeof(timestamp_));
    auto now = system_clock::now();
    auto time = system_clock::to_time_t(now);
    auto duration = now.time_since_epoch();

    auto nanos = duration_cast<nanoseconds>(duration).count() % 1'000'000'000;

    // Convert to UTC time
    std::tm* tm_ptr = std::gmtime(&time);  // UTC version

    if (tm_ptr) {
        // Format with milliseconds (e.g. 2025-07-08T18:12:45.123Z)
        std::snprintf(timestamp_, sizeof(timestamp_),
                    "%04d-%02d-%02dT%02d:%02d:%02d.%09lldZ",
                    tm_ptr->tm_year + 1900,
                    tm_ptr->tm_mon + 1,
                    tm_ptr->tm_mday,
                    tm_ptr->tm_hour,
                    tm_ptr->tm_min,
                    tm_ptr->tm_sec,
                    static_cast<long long>(nanos));  // or use nanos for .%09lld
    }
}

} // namespace haikan

