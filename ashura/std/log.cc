/// SPDX-License-Identifier: MIT
#include "ashura/std/log.h"
#include <stdio.h>
#include <time.h>

namespace ash
{

ASH_C_LINKAGE ASH_DLL_EXPORT Logger * logger = nullptr;

void Logger::init()
{
  if (logger != nullptr)
  {
    std::abort();
  }

  alignas(Logger) static u8 storage[sizeof(Logger)] = {};

  logger = new (storage) Logger{};
}

void Logger::uninit()
{
  if (logger == nullptr)
  {
    std::abort();
  }
  logger->~Logger();
  logger = nullptr;
}

StdioSink stdio_sink{};

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

void StdioSink::log(LogLevel level, Span<char const> log_message)
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

  std::unique_lock lock{mutex};

  (void) std::fputs("[", file);
  (void) std::fputs(level_str, file);
  (void) std::fputs(": ", file);
  (void) std::fwrite(time_string, 1, time_string_length, file);
  (void) std::fputs("] ", file);
  (void) std::fwrite(log_message.data(), 1, log_message.size(), file);
}

void StdioSink::flush()
{
  std::unique_lock lock{mutex};
  (void) std::fflush(stdout);
  (void) std::fflush(stderr);
}

void FileSink::log(LogLevel level, Span<char const> log_message)
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

  std::unique_lock lock{mutex};
  (void) std::fputs("[", file);
  (void) std::fputs(level_str, file);
  (void) std::fputs(": ", file);
  (void) std::fwrite(time_string, 1, time_string_length, file);
  (void) std::fputs("] ", file);
  (void) std::fwrite(log_message.data(), 1, log_message.size(), file);
}

void FileSink::flush()
{
  std::unique_lock lock{mutex};
  (void) std::fflush(file);
}

}    // namespace ash
