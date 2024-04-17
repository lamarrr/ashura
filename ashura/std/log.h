
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/enum.h"
#include "ashura/std/format.h"
#include "ashura/std/mem.h"
#include "ashura/std/runtime.h"
#include "ashura/std/types.h"
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

// to be flushed into trace format style, utc, time encoding, etc.
// format context should append to the buffer
struct Logger
{
  static constexpr u32 SCRATCH_BUFFER_SIZE                 = 256;
  LogSink            **sinks                               = nullptr;
  u32                  num_sinks                           = 0;
  char                *buffer                              = nullptr;
  usize                buffer_size                         = 0;
  usize                buffer_capacity                     = 0;
  char                 scratch_buffer[SCRATCH_BUFFER_SIZE] = {};
  AllocatorImpl        allocator                           = {};
  fmt::Context         fmt_ctx                             = {};
  std::mutex           mutex                               = {};

  template <typename... Args>
  bool debug(Args const &...args)
  {
    return log(LogLevels::Debug, args...);
  }

  template <typename... Args>
  bool trace(Args const &...args)
  {
    return log(LogLevels::Trace, args...);
  }

  template <typename... Args>
  bool info(Args const &...args)
  {
    return log(LogLevels::Info, args...);
  }

  template <typename... Args>
  bool warn(Args const &...args)
  {
    return log(LogLevels::Warning, args...);
  }

  template <typename... Args>
  bool error(Args const &...args)
  {
    return log(LogLevels::Error, args...);
  }

  template <typename... Args>
  bool fatal(Args const &...args)
  {
    return log(LogLevels::Fatal, args...);
  }

  void flush()
  {
    std::lock_guard lock{mutex};
    for (u32 i = 0; i < num_sinks; i++)
    {
      sinks[i]->flush();
    }
  }

  template <typename... Args>
  bool log(LogLevels level, Args const &...args)
  {
    std::lock_guard lock{mutex};
    if (fmt::format(fmt_ctx, args..., "\n"))
    {
      for (u32 i = 0; i < num_sinks; i++)
      {
        sinks[i]->log(level, Span{buffer, buffer_size});
      }
      buffer_size = 0;
      return true;
    }

    buffer_size = 0;
    return false;
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
    fatal(args...);
    flush();
    if (panic_handler != nullptr)
    {
      panic_handler();
    }
    abort();
  }
};

Logger *create_logger(Span<LogSink *const> sinks, AllocatorImpl allocator);

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
