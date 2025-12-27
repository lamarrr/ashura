/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/async.h"
#include "ashura/std/log.h"

namespace ash
{

struct StdioSink final : ILogSink
{
  alignas(CACHELINE_ALIGNMENT) IFutex futex{};

  virtual void log(LogLevel level, Str log_message) override;
  virtual void flush() override;
};

extern StdioSink stdio_sink;

struct FileSink final : ILogSink
{
  std::FILE * file = nullptr;
  alignas(CACHELINE_ALIGNMENT) IFutex futex{};

  virtual void log(LogLevel level, Str log_message) override;
  virtual void flush() override;
};

}    // namespace ash
