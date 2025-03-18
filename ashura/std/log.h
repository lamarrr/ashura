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

struct LogSink
{
  virtual void log(LogLevel level, Str log_message) = 0;
  virtual void flush()                              = 0;
};

/// @brief Logger needs to use fixed-size memory as malloc can fail and make
/// logging unreliable. This means each log statement's content/payload is
/// limited to `BUFFER_CAPACITY`.
struct Logger
{
  static constexpr usize MAX_SINKS       = 16;
  static constexpr usize BUFFER_CAPACITY = 8_KB;

  LogSink * sinks_[MAX_SINKS] = {};
  usize     num_sinks_        = 0;

  constexpr Logger(Span<LogSink * const> sinks)
  {
    auto src = sinks.slice(0, MAX_SINKS);
    obj::copy_assign(src, sinks_);
    num_sinks_ = src.size();
  }

  constexpr Logger(std::initializer_list<LogSink * const> sinks) :
    Logger{span(sinks)}
  {
  }

  constexpr Logger(Logger const &)             = delete;
  constexpr Logger(Logger &&)                  = default;
  constexpr Logger & operator=(Logger &&)      = default;
  constexpr Logger & operator=(Logger const &) = delete;
  constexpr ~Logger()                          = default;

  constexpr Span<LogSink * const> sinks() const
  {
    return Span{sinks_, num_sinks_};
  }

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

  void flush()
  {
    for (LogSink * sink : sinks())
    {
      sink->flush();
    }
  }

  template <typename... Args>
  bool log(LogLevel level, Str fstr, Args const &... args)
  {
    static_assert(sizeof...(args) <= fmt::MAX_ARGS);

    char buffer_scratch[BUFFER_CAPACITY];

    fmt::Op ops_scratch[fmt::MAX_ARGS * 2];

    Buffer<char> buffer{buffer_scratch};

    Buffer<fmt::Op> ops{ops_scratch};

    auto format_sink = [&](Str str) {
      if (!buffer.extend(str))
      {
        for (LogSink * sink : sinks())
        {
          sink->log(level, buffer);
        }

        buffer.clear();

        if (!buffer.extend(str))
        {
          for (LogSink * sink : sinks())
          {
            sink->log(level, str);
          }
        }
      }
    };

    fmt::Context ctx{fn(format_sink), std::move(ops)};

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

    for (LogSink * sink : sinks())
    {
      sink->log(level, buffer);
    }

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
    if (panic_handler != nullptr)
    {
      panic_handler();
    }
    std::abort();
  }
};

struct StdioSink : LogSink
{
  std::mutex mutex;

  void log(LogLevel level, Str log_message) override;
  void flush() override;
};

extern StdioSink stdio_sink;

struct FileSink : LogSink
{
  std::FILE * file = nullptr;
  std::mutex  mutex;

  void log(LogLevel level, Str log_message) override;
  void flush() override;
};

extern Logger * logger;

ASH_C_LINKAGE ASH_DLL_EXPORT void hook_logger(Logger *);

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
