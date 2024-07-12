/// SPDX-License-Identifier: MIT
#include "ashura/std/log.h"
#include <stdio.h>
#include <time.h>

namespace ash
{

Logger *create_logger(Span<LogSink *const> sinks, AllocatorImpl allocator)
{
  u32 const num_sinks = sinks.size32();
  LogSink **log_sinks;

  if (!allocator.nalloc(num_sinks, &log_sinks))
  {
    abort();
  }

  mem::copy(sinks, log_sinks);
  Logger *logger;

  if (!allocator.nalloc(1, &logger))
  {
    abort();
  }

  return new (logger) Logger{
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
                      [](void *data, Span<char const> buffer) {
                        Logger     *logger = (Logger *) data;
                        usize const required_size =
                            logger->buffer_size + buffer.size_bytes();
                        if (required_size > logger->buffer_capacity)
                        {
                          if (!logger->allocator.nrealloc(
                                  logger->buffer_capacity, required_size,
                                  &logger->buffer))
                          {
                            return false;
                          }

                          logger->buffer_capacity = required_size;
                        }
                        mem::copy(buffer, logger->buffer + logger->buffer_size);
                        logger->buffer_size += buffer.size_bytes();
                        return true;
                      },
                          .data = logger,
              },
                  .scratch_buffer = to_span(logger->scratch_buffer)}};
}

void destroy_logger(Logger *logger)
{
  logger->allocator.ndealloc(logger->sinks, logger->num_sinks);
  logger->allocator.ndealloc(logger->buffer, logger->buffer_capacity);
  logger->~Logger();
  logger->allocator.ndealloc(logger, 1);
}

char const *get_level_str(LogLevels level)
{
  switch (level)
  {
    case LogLevels::Debug:
      return "\x1b[94;20m"
             "DEBUG"
             "\x1b[0m";
    case LogLevels::Trace:
      return "\x1b[35;20m"
             "TRACE"
             "\x1b[0m";
    case LogLevels::Info:
      return "\x1b[32;20m"
             "INFO"
             "\x1b[0m";
    case LogLevels::Warning:
      return "\x1b[33;20m"
             "WARNING"
             "\x1b[0m";
    case LogLevels::Error:
      return "\x1b[31;20m"
             "ERROR"
             "\x1b[0m";
    case LogLevels::Fatal:
      return "\x1b[31;1m"
             "FATAL"
             "\x1b[0m";
    default:
      return "";
  }
}

void StdioSink::log(LogLevels level, Span<char const> log_message)
{
  char const *level_str = get_level_str(level);
  FILE       *file      = stdout;

  switch (level)
  {
    case LogLevels::Debug:
    case LogLevels::Trace:
    case LogLevels::Info:
    case LogLevels::Warning:
      file = stdout;
      break;
    case LogLevels::Error:
    case LogLevels::Fatal:
      file = stderr;
      break;
    default:
      break;
  }

  constexpr char const time_format[] = "%d/%m/%Y, %H:%M:%S";
  char                 time_string[256];
  usize                time_string_length = 0;

  time_t current_time = time(nullptr);
  if (current_time != (time_t) -1)
  {
    tm *current_local_time = localtime(&current_time);
    if (current_local_time != nullptr)
    {
      time_string_length = strftime(time_string, sizeof(time_string),
                                    time_format, current_local_time);
    }
  }

  std::unique_lock lock{mutex};

  (void) fputs("[", file);
  (void) fputs(level_str, file);
  (void) fputs(": ", file);
  (void) fwrite(time_string, 1, time_string_length, file);
  (void) fputs("] ", file);
  (void) fwrite(log_message.data(), 1, log_message.size(), file);
}

void StdioSink::flush()
{
  std::unique_lock lock{mutex};
  (void) fflush(stdout);
  (void) fflush(stderr);
}

void FileSink::log(LogLevels level, Span<char const> log_message)
{
  char const          *level_str     = get_level_str(level);
  constexpr char const time_format[] = "%d/%m/%Y, %H:%M:%S";
  char                 time_string[256];
  usize                time_string_length = 0;

  time_t current_time = time(nullptr);
  if (current_time != (time_t) -1)
  {
    tm *current_local_time = localtime(&current_time);
    if (current_local_time != nullptr)
    {
      time_string_length = strftime(time_string, sizeof(time_string),
                                    time_format, current_local_time);
    }
  }

  std::unique_lock lock{mutex};
  (void) fputs("[", file);
  (void) fputs(level_str, file);
  (void) fputs(": ", file);
  (void) fwrite(time_string, 1, time_string_length, file);
  (void) fputs("] ", file);
  (void) fwrite(log_message.data(), 1, log_message.size(), file);
}

void FileSink::flush()
{
  std::unique_lock lock{mutex};
  (void) fflush(file);
}

}        // namespace ash
