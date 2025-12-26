/// SPDX-License-Identifier: MIT
#include "ashura/std/log.h"
#include "ashura/std/async.h"
#include "ashura/std/list.h"
#include "ashura/std/log_sinks.h"
#include <stdio.h>
#include <time.h>

namespace ash
{

StdioSink stdio_sink{};

ILogger::ILogger(Span<LogSink const> sinks_span) : head_{}
{
  head_.next = &head_;
  head_.prev = &head_;

  ListView sinks{&head_};

  for (auto * sink : sinks_span)
  {
    sinks.push_back(sink);
  }
}

void ILogger::flush()
{
  ListView sinks{&head_};
  for (auto & sink : sinks)
  {
    sink.flush();
  }
}

void ILogger::write_to_sinks(LogLevel level, Str str, Buffer<char> & buffer)
{
  ListView sinks{&head_};

  // try to format into the buffer first
  if (!buffer.append(str))
  {
    // buffer is exhausted, log pending message to all sinks
    flush_buffer(level, buffer);

    // try to push into the buffer again
    if (!buffer.append(str))
    {
      // still doesn't fit, log directly to all sinks without buffering
      // since the message is too large
      for (auto & sink : sinks)
      {
        sink.log(level, str);
      }
    }
  }
}

void ILogger::flush_buffer(LogLevel level, Buffer<char> & buffer)
{
  ListView sinks{&head_};

  for (auto & sink : sinks)
  {
    sink.log(level, buffer);
  }

  buffer.clear();
}

Logger logger = nullptr;

void hook_logger(Logger instance)
{
  logger = instance;
}

char const * get_level_str(LogLevel level)
{
  switch (level)
  {
    case LogLevel::Debug:
      return "\x1b[94;20m"
             "DEBUG"
             "\x1b[0m";
    case LogLevel::Trace:
      return "\x1b[35;20m"
             "TRACE"
             "\x1b[0m";
    case LogLevel::Info:
      return "\x1b[32;20m"
             "INFO"
             "\x1b[0m";
    case LogLevel::Warning:
      return "\x1b[33;20m"
             "WARNING"
             "\x1b[0m";
    case LogLevel::Error:
      return "\x1b[31;20m"
             "ERROR"
             "\x1b[0m";
    case LogLevel::Fatal:
      return "\x1b[31;1m"
             "FATAL"
             "\x1b[0m";
    default:
      return "";
  }
}

void StdioSink::log(LogLevel level, Str log_message)
{
  char const * level_str = get_level_str(level);
  std::FILE *  file      = stdout;

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

  static constexpr char const time_format[] = "%d/%m/%Y, %H:%M:%S";
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
  (void) std::fputs("[", file);
  (void) std::fputs(level_str, file);
  (void) std::fputs(": ", file);
  (void) std::fwrite(time_string, 1, time_string_length, file);
  (void) std::fputs("] ", file);
  (void) std::fwrite(log_message.data(), 1, log_message.size(), file);
  (void) std::fputs("\n", file);
}

void StdioSink::flush()
{
  LockGuard guard{futex};
  (void) std::fflush(stdout);
  (void) std::fflush(stderr);
}

void FileSink::log(LogLevel level, Str log_message)
{
  char const *                level_str     = get_level_str(level);
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
  (void) std::fputs("[", file);
  (void) std::fputs(level_str, file);
  (void) std::fputs(": ", file);
  (void) std::fwrite(time_string, 1, time_string_length, file);
  (void) std::fputs("] ", file);
  (void) std::fwrite(log_message.data(), 1, log_message.size(), file);
  (void) std::fputs("\n", file);
}

void FileSink::flush()
{
  LockGuard guard{futex};
  (void) std::fflush(file);
}

}    // namespace ash
