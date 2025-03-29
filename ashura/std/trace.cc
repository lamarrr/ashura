/// SPDX-License-Identifier: MIT
#include "ashura/std/trace.h"

namespace ash
{

TraceSink * trace_sink = &noop_trace_sink;

ASH_DLL_EXPORT ASH_C_LINKAGE void hook_trace_sink(TraceSink * instance)
{
  trace_sink = instance;
}

void FileTraceSink::trace(TraceEvent event, Span<TraceRecord const> records)
{
  // [ ] output as json,csv
  // [ ] what frequency? per-thread buffer? when to flush? etc.
  // [ ] file utils, truncate, flush
}

void MemoryTraceSink::trace(TraceEvent event, Span<TraceRecord const> records)
{
  LockGuard guard{mutex_};

  auto [_, current_records] =
    traces_.insert(event, Vec<TraceRecord>{allocator_}, nullptr, false)
      .unwrap();

  if (current_records.size() + records.size() > buffer_size_)
  {
    upstream_->trace(event, current_records);
    current_records.clear();
  }

  current_records.extend(records).unwrap();
}

void MemoryTraceSink::flush()
{
  LockGuard guard{mutex_};
  for (auto & [event, records] : traces_)
  {
    upstream_->trace(event, records);
    records.clear();
  }
}

}    // namespace ash
