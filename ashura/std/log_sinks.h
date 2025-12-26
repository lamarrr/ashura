/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/async.h"
#include "ashura/std/log.h"

namespace ash
{

struct StdioSink : ILogSink
{
  alignas(CACHELINE_ALIGNMENT) IFutex futex{};

  void log(LogLevel level, Str log_message) override;
  void flush() override;
};

extern StdioSink stdio_sink;

struct FileSink : ILogSink
{
  std::FILE * file = nullptr;
  alignas(CACHELINE_ALIGNMENT) IFutex futex{};

  void log(LogLevel level, Str log_message) override;
  void flush() override;
};

}    // namespace ash
