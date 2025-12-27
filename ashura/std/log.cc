/// SPDX-License-Identifier: MIT
#include "ashura/std/log.h"
#include "ashura/std/list.h"

namespace ash
{

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

}    // namespace ash
