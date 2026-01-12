/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/format.h"
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/panic.h"
#include "ashura/std/types.h"
#include <atomic>
#include <cstdlib>
#include <mutex>

#define ASH_DUMP(x) ::ash::trace(#x, " = ", x);

namespace ash
{

enum class LogLevel : u32
{
    Debug   = 0,
    Trace   = 1,
    Info    = 2,
    Warning = 3,
    Error   = 4,
    Fatal   = 5
};

enum class LogLevels : u32
{
    None    = 0x00,
    Debug   = 0x01,
    Trace   = 0x02,
    Info    = 0x04,
    Warning = 0x08,
    Error   = 0x10,
    Fatal   = 0x20
};

ASH_BIT_ENUM_OPS(LogLevels)

typedef struct ILogSink * LogSink;
typedef struct ILogger *  Logger;

struct ILogSink
{
    ILogSink * next = nullptr;
    ILogSink * prev = nullptr;

    virtual void log(LogLevel level, Str log_message)
    {
        (void) level;
        (void) log_message;
    }

    virtual void flush()
    {
    }
};

/// @brief Logger needs to use fixed-size memory as malloc can fail and make
/// logging unreliable. This means each log statement's content/payload is
/// limited to `BUFFER_CAPACITY`.
struct ILogger
{
    static constexpr usize BUFFER_CAPACITY = 16_KB;

    ILogSink head_;

    ILogger(Span<LogSink const> sinks);
    constexpr ILogger(ILogger const &)             = delete;
    constexpr ILogger(ILogger &&)                  = default;
    constexpr ILogger & operator=(ILogger &&)      = default;
    constexpr ILogger & operator=(ILogger const &) = delete;
    constexpr ~ILogger()                           = default;

    template <typename... Args>
    bool debug(Str fstr, Args const &... args)
    {
        return log(LogLevel::Debug, fstr, args...);
    }

    template <typename... Args>
    bool trace(Str fstr, Args const &... args)
    {
        return log(LogLevel::Trace, fstr, args...);
    }

    template <typename... Args>
    bool info(Str fstr, Args const &... args)
    {
        return log(LogLevel::Info, fstr, args...);
    }

    template <typename... Args>
    bool warn(Str fstr, Args const &... args)
    {
        return log(LogLevel::Warning, fstr, args...);
    }

    template <typename... Args>
    bool error(Str fstr, Args const &... args)
    {
        return log(LogLevel::Error, fstr, args...);
    }

    template <typename... Args>
    bool fatal(Str fstr, Args const &... args)
    {
        return log(LogLevel::Fatal, fstr, args...);
    }

    void flush();

    void write_to_sinks(LogLevel level, Str str, Buffer<char> & buffer);

    void flush_buffer(LogLevel level, Buffer<char> & buffer);

    template <typename... Args>
    bool log(LogLevel level, Str fstr, Args const &... args)
    {
        static_assert(sizeof...(args) <= fmt::MAX_ARGS);

        char            ops_scratch[sizeof(fmt::Op) * (fmt::MAX_ARGS * 2)];
        Buffer<fmt::Op> ops{span(ops_scratch).reinterpret<fmt::Op>()};
        char            buffer_scratch[BUFFER_CAPACITY];
        Buffer<char>    buffer{buffer_scratch};

        auto sink_writer = [&](Str str) { write_to_sinks(level, str, buffer); };

        fmt::Context ctx{&sink_writer, std::move(ops)};

        if (fmt::Result result = ctx.format(fstr, args...);
            result.error != fmt::Error::None)
        {
            switch (result.error)
            {
                case fmt::Error::ItemsMismatch:
                case fmt::Error::UnexpectedToken:
                case fmt::Error::UnmatchedToken:
                {
                    (void) std::fprintf(stderr, "Format Error: %s\n",
                                        to_str(result.error).data());
                    (void) std::fflush(stderr);
                    std::abort();
                }
                case fmt::Error::OutOfMemory:
                default:
                {
                    return false;
                }
            }
        }

        // flush remaining buffer content to sinks
        flush_buffer(level, buffer);

        return true;
    }

    template <typename... Args>
    [[noreturn]] void panic(Str fstr, Args const &... args)
    {
        std::atomic_ref panic_count{*ash::panic_count};
        if (panic_count.fetch_add(1, std::memory_order::relaxed))
        {
            (void) std::fputs("panicked while processing a panic. aborting...",
                              stderr);
            (void) std::fflush(stderr);
            std::abort();
        }
        if (!fatal(fstr, args...))
        {
            (void) std::fputs("ran out of log buffer memory while panicking.",
                              stderr);
        }
        flush();
        handle_panic();
        std::abort();
    }
};

extern Logger logger;

ASH_C_LINKAGE ASH_DLL_EXPORT void hook_logger(Logger);

template <typename... Args>
void debug(Str fstr, Args const &... args)
{
    logger->debug(fstr, args...);
}

template <typename... Args>
void trace(Str fstr, Args const &... args)
{
    logger->trace(fstr, args...);
}

template <typename... Args>
void info(Str fstr, Args const &... args)
{
    logger->info(fstr, args...);
}

template <typename... Args>
void warn(Str fstr, Args const &... args)
{
    logger->warn(fstr, args...);
}

template <typename... Args>
void error(Str fstr, Args const &... args)
{
    logger->error(fstr, args...);
}

template <typename... Args>
void fatal(Str fstr, Args const &... args)
{
    logger->fatal(fstr, args...);
}

}    // namespace ash
