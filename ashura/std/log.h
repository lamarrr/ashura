
/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/format.h"
#include "ashura/std/mem.h"
#include "ashura/std/runtime.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include <atomic>
#include <mutex>
#include <stdlib.h>

namespace ash
{

enum class LogLevels : u8
{
  None    = 0x00,
  Debug   = 0x01,
  Trace   = 0x02,
  Info    = 0x04,
  Warning = 0x08,
  Error   = 0x10,
  Fatal   = 0x20
};

ASH_DEFINE_ENUM_BIT_OPS(LogLevels)

struct LogSink
{
  virtual void log(LogLevels level, Span<char const> log_message) = 0;
  virtual void flush()                                            = 0;
};

struct Logger
{
  static constexpr u32 SCRATCH_SIZE = 256;

  Vec<LogSink *> sinks                 = {};
  Vec<char>      buffer                = {};
  char           scratch[SCRATCH_SIZE] = {};
  fmt::Context   fmt_ctx               = {};
  std::mutex     mutex                 = {};

  template <typename... Args>
  [[maybe_unused]] Result<Void, Void> debug(Args const &...args)
  {
    return log(LogLevels::Debug, args...);
  }

  template <typename... Args>
  [[maybe_unused]] Result<Void, Void> trace(Args const &...args)
  {
    return log(LogLevels::Trace, args...);
  }

  template <typename... Args>
  [[maybe_unused]] Result<Void, Void> info(Args const &...args)
  {
    return log(LogLevels::Info, args...);
  }

  template <typename... Args>
  [[maybe_unused]] Result<Void, Void> warn(Args const &...args)
  {
    return log(LogLevels::Warning, args...);
  }

  template <typename... Args>
  [[maybe_unused]] Result<Void, Void> error(Args const &...args)
  {
    return log(LogLevels::Error, args...);
  }

  template <typename... Args>
  [[maybe_unused]] Result<Void, Void> fatal(Args const &...args)
  {
    return log(LogLevels::Fatal, args...);
  }

  void flush()
  {
    std::lock_guard lock{mutex};
    for (LogSink *sink : sinks)
    {
      sink->flush();
    }
  }

  template <typename... Args>
  [[maybe_unused]] Result<Void, Void> log(LogLevels level, Args const &...args)
  {
    std::lock_guard lock{mutex};
    if (fmt::format(fmt_ctx, args..., "\n"))
    {
      for (LogSink *sink : sinks)
      {
        sink->log(level, span(buffer));
      }
      buffer.clear();
      return Ok{};
    }

    buffer.clear();
    return Err{};
  }

  template <typename... Args>
  [[noreturn]] void panic(Args const &...args)
  {
    if (panic_count.fetch_add(1, std::memory_order::relaxed))
    {
      (void) fputs("panicked while processing a panic. aborting...", stderr);
      (void) fflush(stderr);
      abort();
    }
    fatal(args...).expect(
        "formatting failed while processing a panic. aborting...");
    flush();
    if (panic_handler != nullptr)
    {
      panic_handler();
    }
    abort();
  }

  void reset()
  {
    std::lock_guard lock{mutex};
    sinks.reset();
    buffer.reset();
  }
};

Logger *create_logger(Span<LogSink *const> sinks);

void destroy_logger(Logger *logger);

struct StdioSink final : LogSink
{
  std::mutex mutex;

  void log(LogLevels level, Span<char const> log_message) override;
  void flush() override;
};

struct FileSink final : LogSink
{
  FILE      *file = nullptr;
  std::mutex mutex;

  void log(LogLevels level, Span<char const> log_message) override;
  void flush() override;
};

extern ash::Logger *default_logger;

}        // namespace ash
