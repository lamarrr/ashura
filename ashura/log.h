
#pragma once

#include "ashura/time.h"
#include "ashura/types.h"

namespace ash
{

typedef struct Logger_T       *Logger;
typedef struct LoggerInterface LoggerInterface;
typedef struct LoggerImpl      LoggerImpl;

enum class LogLevel : u8
{
  Debug   = 0,
  Trace   = 1,
  Info    = 2,
  Warning = 3,
  Error   = 4,
  Fatal   = 5
};

struct LoggerInterface
{
  void (*log)(Logger self, LogLevel level) = nullptr;
  void (*ref)(Logger self)                                      = nullptr;
  void (*unref)(Logger self)                                    = nullptr;
};

// log destination -> file, network, disk. chaining of multiple log destinations
// to be flushed into trace format style, utc, time encoding, etc.
struct LoggerImpl
{
  Logger                 self      = nullptr;
  LoggerInterface const *interface = nullptr;

  static LoggerImpl create_logger(char const *name);
  template <typename... Args>
  void debug(char const *fmt, Args const &...args);
  template <typename... Args>
  void trace(char const *fmt, Args const &...args);
  template <typename... Args>
  void info(char const *fmt, Args const &...args);
  template <typename... Args>
  void warn(char const *fmt, Args const &...args);
  template <typename... Args>
  void error(char const *fmt, Args const &...args);
  template <typename... Args>
  void fatal(char const *fmt, Args const &...args);
};

}        // namespace ash
