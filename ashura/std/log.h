
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
typedef struct LogSink_T         *LogSink;
typedef struct LogSinkInterface   LogSinkInterface;
typedef struct LogSinkImpl        LogSinkImpl;
typedef struct StdioSink          StdioSink;
typedef struct StdioSinkInterface StdioSinkInterface;
typedef struct FileSink           FileSink;
typedef struct FileSinkInterface  FileSinkInterface;

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

struct LogSinkInterface
{
  void (*log)(LogSink self, LogLevel level, char const *log_message,
              usize len)      = nullptr;
  void (*flush)(LogSink self) = nullptr;
};

struct LogSinkImpl
{
  LogSink                 self      = nullptr;
  LogSinkInterface const *interface = nullptr;
};

// to be flushed into trace format style, utc, time encoding, etc.
// format context should append to the buffer
// TODO(lamarrr): Level filter and filter for sinks?
struct Logger
{
  static constexpr u32 SCRATCH_BUFFER_SIZE                 = 256;
  LogSinkImpl         *sinks                               = nullptr;
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
      LogSinkImpl impl = sinks[i];
      impl.interface->flush(impl.self);
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
        LogSinkImpl impl = sinks[i];
        impl.interface->log(impl.self, level, buffer, buffer_size);
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

inline bool create_logger(Logger *memory, Span<LogSinkImpl const> sinks,
                          AllocatorImpl allocator)
{
  u32 const    num_sinks = (u32) sinks.size;
  LogSinkImpl *log_sinks = allocator.allocate_typed<LogSinkImpl>(num_sinks);
  if (log_sinks == nullptr)
  {
    return false;
  }

  mem::copy(sinks, log_sinks);

  new (memory) Logger{
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
                        memcpy(logger->buffer + logger->buffer_size, buffer,
                                       size);
                        logger->buffer_size += size;
                        return true;
                      },
                          .data = memory,
              },
                  .scratch_buffer      = memory->scratch_buffer,
                  .scratch_buffer_size = Logger::SCRATCH_BUFFER_SIZE}};

  return true;
};

inline void destroy_logger(Logger *logger)
{
  logger->allocator.deallocate_typed(logger->sinks, logger->num_sinks);
  logger->allocator.deallocate_typed(logger->buffer, logger->buffer_capacity);
}

struct StdioSink
{
  std::mutex mutex;
};

struct StdioSinkInterface
{
  static void log(LogSink self, LogLevel level, char const *log_message,
                  usize len);
  static void flush(LogSink self);
};

static LogSinkInterface stdio_sink_interface{
    .log = StdioSinkInterface::log, .flush = StdioSinkInterface::flush};

struct FileSink
{
  FILE      *file = nullptr;
  std::mutex mutex;
};

struct FileSinkInterface
{
  static void log(LogSink self, LogLevel level, char const *log_message,
                  usize len);
  static void flush(LogSink self);
};

static LogSinkInterface file_sink_interface{.log   = FileSinkInterface::log,
                                            .flush = FileSinkInterface::flush};

}        // namespace ash
