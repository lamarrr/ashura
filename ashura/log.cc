#include "ashura/log.h"
#include <stdio.h>
#include <time.h>

namespace ash
{

char const *get_level_str(LogLevel level)
{
  switch (level)
  {
    case LogLevel::Debug:
      return "\x1b[38;20m"
             "DEBUG"
             "\x1b[0m";
    case LogLevel::Trace:
      return "\x1b[38;20m"
             "TRACE"
             "\x1b[0m";
    case LogLevel::Info:
      return "\x1b[32;1m"
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

void StdioSinkInterface::log(LogSink self_, LogLevel level,
                             char const *log_message, usize len)
{
  StdioSink  *self      = (StdioSink *) self_;
  char const *level_str = get_level_str(level);
  FILE       *file      = stdout;

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

  std::unique_lock lock{self->mutex};

  (void) fputs("[", file);
  (void) fputs(level_str, file);
  (void) fputs(": ", file);
  (void) fwrite(time_string, 1, time_string_length, file);
  (void) fputs("] ", file);
  (void) fwrite(log_message, 1, len, file);
}

void StdioSinkInterface::flush(LogSink self_)
{
  StdioSink       *self = (StdioSink *) self_;
  std::unique_lock lock{self->mutex};
  (void) fflush(stdout);
  (void) fflush(stderr);
}

void FileSinkInterface::log(LogSink self_, LogLevel level,
                            char const *log_message, usize len)
{
  FileSink *self = (FileSink *) self_;

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

  std::unique_lock lock{self->mutex};
  (void) fputs("[", self->file);
  (void) fputs(level_str, self->file);
  (void) fputs(": ", self->file);
  (void) fwrite(time_string, 1, time_string_length, self->file);
  (void) fputs("] ", self->file);
  (void) fwrite(log_message, 1, len, self->file);
}

void FileSinkInterface::flush(LogSink self_)
{
  FileSink        *self = (FileSink *) self_;
  std::unique_lock lock{self->mutex};
  (void) fflush(self->file);
}

}        // namespace ash
