/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/format.h"
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/panic.h"
#include "ashura/std/types.h"
#include <atomic>
#include <mutex>
#include <stdlib.h>

#define ASH_DUMP(x) ::ash::logger->trace(#x, " = ", x);

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

ASH_BIT_ENUM_OPS(LogLevels)

struct LogSink
{
  virtual void log(LogLevels level, Span<char const> log_message) = 0;
  virtual void flush()                                            = 0;
};

/// @brief Logger needs to use fixed-size memory as malloc can fail and make
/// logging unreliable. This means each log statement's content/payload is
/// limited to `BUFFER_CAPACITY`.
struct Logger : Pin<>
{
  static constexpr usize BUFFER_CAPACITY = 16_KB;
  static constexpr usize SCRATCH_SIZE    = 256;
  static constexpr u32   MAX_SINKS       = 8;

  LogSink *  sinks[MAX_SINKS]        = {};
  u32        num_sinks               = 0;
  char       buffer[BUFFER_CAPACITY] = {};
  std::mutex mutex                   = {};

  template <typename... Args>
  bool debug(Args const &... args)
  {
    return log(LogLevels::Debug, args...);
  }

  template <typename... Args>
  bool trace(Args const &... args)
  {
    return log(LogLevels::Trace, args...);
  }

  template <typename... Args>
  bool info(Args const &... args)
  {
    return log(LogLevels::Info, args...);
  }

  template <typename... Args>
  bool warn(Args const &... args)
  {
    return log(LogLevels::Warning, args...);
  }

  template <typename... Args>
  bool error(Args const &... args)
  {
    return log(LogLevels::Error, args...);
  }

  template <typename... Args>
  bool fatal(Args const &... args)
  {
    return log(LogLevels::Fatal, args...);
  }

  void flush()
  {
    std::lock_guard lock{mutex};
    for (LogSink * sink : Span{sinks, num_sinks})
    {
      sink->flush();
    }
  }

  template <typename... Args>
  bool log(LogLevels level, Args const &... args)
  {
    std::lock_guard lock{mutex};
    char            scratch[SCRATCH_SIZE];
    Buffer<char>    msg = ash::buffer<char>(buffer);
    fmt::Context    ctx = fmt::buffer(msg, scratch);
    if (!fmt::format(ctx, args..., "\n"))
    {
      return false;
    }

    for (LogSink * sink : Span{sinks, num_sinks})
    {
      sink->log(level, msg);
    }
    return true;
  }

  template <typename... Args>
  [[noreturn]] void panic(Args const &... args)
  {
    std::atomic_ref panic_count{*ash::panic_count};
    if (panic_count.fetch_add(1, std::memory_order::relaxed))
    {
      (void) fputs("panicked while processing a panic. aborting...", stderr);
      (void) fflush(stderr);
      abort();
    }
    if (!fatal(args...))
    {
      (void) fputs("ran out of log buffer memory while panicking.", stderr);
    }
    flush();
    if (panic_handler != nullptr)
    {
      panic_handler();
    }
    abort();
  }

  static void init();

  static void uninit();

  [[nodiscard]] bool add_sink(LogSink * s)
  {
    std::lock_guard lock{mutex};
    if ((num_sinks + 1) > MAX_SINKS)
    {
      return false;
    }

    sinks[num_sinks++] = s;
    return true;
  }

  void reset()
  {
    std::lock_guard lock{mutex};
    num_sinks = 0;
  }
};

struct StdioSink : LogSink
{
  std::mutex mutex;

  void log(LogLevels level, Span<char const> log_message) override;
  void flush() override;
};

struct FileSink : LogSink
{
  std::FILE * file = nullptr;
  std::mutex  mutex;

  void log(LogLevels level, Span<char const> log_message) override;
  void flush() override;
};

ASH_C_LINKAGE ASH_DLL_EXPORT ash::Logger * logger;

extern StdioSink stdio_sink;

}    // namespace ash
