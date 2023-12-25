#include "ashura/log.h"
#include <stdio.h>
#include <time.h>

namespace ash
{

void StdioSinkInterface::log(LogSink self_, LogLevel level,
                             char const *log_message, usize len)
{
  StdioSink  *self      = (StdioSink *) self_;
  char const *level_str = "";
  FILE       *file      = stdout;

  switch (level)
  {
    case LogLevel::Debug:
      level_str = "DEBUG";
      file      = stdout;
      break;
    case LogLevel::Trace:
      level_str = "TRACE";
      file      = stdout;
      break;
    case LogLevel::Info:
      level_str = "INFO";
      file      = stdout;
      break;
    case LogLevel::Warning:
      level_str = "WARNING";
      file      = stdout;
      break;
    case LogLevel::Error:
      level_str = "ERROR";
      file      = stderr;
      break;
    case LogLevel::Fatal:
      level_str = "FATAL";
      file      = stderr;
      break;
    default:
      break;
  }

  constexpr char const time_format[] = "%d/%m/%Y %H:%M:%S";
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
  (void) fputs(", ", file);
  (void) fwrite(time_string, 1, time_string_length, file);
  (void) fputs("]", file);
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

  char const *level_str = "";

  switch (level)
  {
    case LogLevel::Debug:
      level_str = "DEBUG";
      break;
    case LogLevel::Trace:
      level_str = "TRACE";
      break;
    case LogLevel::Info:
      level_str = "INFO";
      break;
    case LogLevel::Warning:
      level_str = "WARNING";
      break;
    case LogLevel::Error:
      level_str = "ERROR";
      break;
    case LogLevel::Fatal:
      level_str = "FATAL";
      break;
    default:
      break;
  }

  constexpr char const time_format[] = "%d/%m/%Y %H:%M:%S";
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
  (void) fputs(", ", self->file);
  (void) fwrite(time_string, 1, time_string_length, self->file);
  (void) fputs("]", self->file);
  (void) fwrite(log_message, 1, len, self->file);
}

void FileSinkInterface::flush(LogSink self_)
{
  FileSink        *self = (FileSink *) self_;
  std::unique_lock lock{self->mutex};
  (void) fflush(self->file);
}

}        // namespace ash
