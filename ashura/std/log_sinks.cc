/// SPDX-License-Identifier: MIT
#include "ashura/std/log_sinks.h"
#include "ashura/std/async.h"
#include "ashura/std/log.h"
#include <stdio.h>
#include <time.h>

namespace ash
{

StdioSink stdio_sink{};

constexpr Str get_level_str(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Debug:
            return "\x1b[94;20m"
                   "DEBUG"
                   "\x1b[0m"_s;
        case LogLevel::Trace:
            return "\x1b[35;20m"
                   "TRACE"
                   "\x1b[0m"_s;
        case LogLevel::Info:
            return "\x1b[32;20m"
                   "INFO"
                   "\x1b[0m"_s;
        case LogLevel::Warning:
            return "\x1b[33;20m"
                   "WARNING"
                   "\x1b[0m"_s;
        case LogLevel::Error:
            return "\x1b[31;20m"
                   "ERROR"
                   "\x1b[0m"_s;
        case LogLevel::Fatal:
            return "\x1b[31;1m"
                   "FATAL"
                   "\x1b[0m"_s;
        default:
            return ""_s;
    }
}

void StdioSink::log(LogLevel level, Str log_message)
{
    Str         level_str = get_level_str(level);
    std::FILE * file      = stdout;

    switch (level)
    {
        case LogLevel::Debug:
        case LogLevel::Trace:
        case LogLevel::Info:
        case LogLevel::Warning:
            file = stdout;
            break;
        case LogLevel::Error:
        case LogLevel::Fatal:
            file = stderr;
            break;
        default:
            break;
    }

    static constexpr char const time_format[] = "%d-%m-%Y, %H:%M:%S";
    char                        time_string[256];
    usize                       time_string_length = 0;

    std::time_t current_time = std::time(nullptr);
    if (current_time != (std::time_t) -1)
    {
        tm * current_local_time = std::localtime(&current_time);
        if (current_local_time != nullptr)
        {
            time_string_length = std::strftime(time_string, sizeof(time_string),
                                               time_format, current_local_time);
        }
    }

    LockGuard guard{futex};
    (void) std::fwrite("[", 1, 1, file);
    (void) std::fwrite(level_str.data(), 1, level_str.size(), file);
    (void) std::fwrite(": ", 1, 2, file);
    (void) std::fwrite(time_string, 1, time_string_length, file);
    (void) std::fwrite("] ", 1, 2, file);
    (void) std::fwrite(log_message.data(), 1, log_message.size(), file);
    (void) std::fwrite("\n", 1, 1, file);
}

void StdioSink::flush()
{
    LockGuard guard{futex};
    (void) std::fflush(stdout);
    (void) std::fflush(stderr);
}

void FileSink::log(LogLevel level, Str log_message)
{
    Str                         level_str     = get_level_str(level);
    static constexpr char const time_format[] = "%d/%m/%Y, %H:%M:%S";
    char                        time_string[256];
    usize                       time_string_length = 0;

    std::time_t current_time = std::time(nullptr);
    if (current_time != (std::time_t) -1)
    {
        std::tm * current_local_time = std::localtime(&current_time);
        if (current_local_time != nullptr)
        {
            time_string_length = std::strftime(time_string, sizeof(time_string),
                                               time_format, current_local_time);
        }
    }

    LockGuard guard{futex};
    (void) std::fwrite("[", 1, 1, file);
    (void) std::fwrite(level_str.data(), 1, level_str.size(), file);
    (void) std::fwrite(": ", 1, 2, file);
    (void) std::fwrite(time_string, 1, time_string_length, file);
    (void) std::fwrite("] ", 1, 2, file);
    (void) std::fwrite(log_message.data(), 1, log_message.size(), file);
    (void) std::fwrite("\n", 1, 1, file);
}

void FileSink::flush()
{
    LockGuard guard{futex};
    (void) std::fflush(file);
}

}    // namespace ash
