
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

enum class LogLevel : u8
{
  None    = 0x00,
  Debug   = 0x01,
  Trace   = 0x02,
  Info    = 0x04,
  Warning = 0x08,
  Error   = 0x10,
  Fatal   = 0x20
};

ASH_DEFINE_ENUM_BIT_OPS(LogLevel)

struct LogSink
{
  virtual void log(LogLevel level, Span<char const> log_message) = 0;
  virtual void flush()                                           = 0;
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
    return log(LogLevel::Debug, args...);
  }

  template <typename... Args>
  bool trace(Args const &...args)
  {
    return log(LogLevel::Trace, args...);
  }

  template <typename... Args>
  bool info(Args const &...args)
  {
    return log(LogLevel::Info, args...);
  }

  template <typename... Args>
  bool warn(Args const &...args)
  {
    return log(LogLevel::Warning, args...);
  }

  template <typename... Args>
  bool error(Args const &...args)
  {
    return log(LogLevel::Error, args...);
  }

  template <typename... Args>
  bool fatal(Args const &...args)
  {
    return log(LogLevel::Fatal, args...);
  }

  void flush()
  {
    std::unique_lock lock{mutex};
    for (u32 i = 0; i < num_sinks; i++)
    {
      sinks[i]->flush();
    }
  }

  template <typename... Args>
  bool log(LogLevel level, Args const &...args)
  {
    std::unique_lock lock{mutex};
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

inline Logger *create_logger(Span<LogSink *const> sinks,
                             AllocatorImpl        allocator)
{
  u32 const num_sinks = (u32) sinks.size();
  LogSink **log_sinks = allocator.allocate_typed<LogSink *>(num_sinks);
  if (log_sinks == nullptr)
  {
    return nullptr;
  }

  mem::copy(sinks, log_sinks);
  Logger *logger = allocator.allocate_typed<Logger>(1);
  if (logger == nullptr)
  {
    allocator.deallocate_typed(log_sinks, num_sinks);
    return nullptr;
  }

  new (logger) Logger{
      .sinks           = log_sinks,
      .num_sinks       = num_sinks,
      .buffer          = nullptr,
      .buffer_size     = 0,
      .buffer_capacity = 0,
      .allocator       = allocator,
      .fmt_ctx         = fmt::Context{
                  .push =
                      {
                          .dispatcher =
                      [](void *data, char const *buffer, usize size) {
                        Logger     *logger        = (Logger *) data;
                        usize const required_size = logger->buffer_size + size;
                        if (required_size > logger->buffer_capacity)
                        {
                          char *buffer = logger->allocator.reallocate_typed(
                              logger->buffer, logger->buffer_capacity,
                              required_size);
                          if (buffer == nullptr)
                          {
                            return false;
                          }
                          logger->buffer          = buffer;
                          logger->buffer_capacity = required_size;
                        }
                        mem::copy(buffer, logger->buffer + logger->buffer_size,
                                          size);
                        logger->buffer_size += size;
                        return true;
                      },
                          .data = logger,
              },
                  .scratch_buffer      = logger->scratch_buffer,
                  .scratch_buffer_size = Logger::SCRATCH_BUFFER_SIZE}};

  return logger;
};

inline void destroy_logger(Logger *logger)
{
  logger->allocator.deallocate_typed(logger->sinks, logger->num_sinks);
  logger->allocator.deallocate_typed(logger->buffer, logger->buffer_capacity);
  logger->allocator.deallocate_typed(logger, 1);
}

struct StdioSink final : LogSink
{
  std::mutex mutex;

  void log(LogLevel level, Span<char const> log_message) override;
  void flush() override;
};

struct FileSink final : LogSink
{
  FILE      *file = nullptr;
  std::mutex mutex;

  void log(LogLevel level, Span<char const> log_message) override;
  void flush() override;
};

}        // namespace ash
