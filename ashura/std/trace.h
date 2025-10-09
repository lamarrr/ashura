/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/async.h"
#include "ashura/std/dict.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{
/// Functional Requirements
///
/// - Record function scopes and time entry points or additional scope-related meta-data
/// - Record component values: strings, floats, integers, booleans, blobs, meta-data
/// - Re-play scalar values in-memory for visualizers
/// - Save traces to disk
/// - Thread-safe, fast, 0-overhead
/// - Configurable frame size, clear time-interval
///

// [ ] separate: attributes based with each being in a different data frame
// [ ] string arena
struct TraceRecord
{
  Str label = {};

  u64 id = 0;

  SourceLocation loc = {};

  i64 i = 0;

  f64 f = 0;

  nanoseconds begin = {};

  nanoseconds end = {};
};

struct TraceEvent
{
  Str label = {};

  u64 id = 0;
};

struct TraceEventHash
{
  usize operator()(TraceEvent const & event) const
  {
    return hash_combine(span_hash(event.label), bit_hash(event.id));
  }
};

struct TraceEventEq
{
  bool operator()(TraceEvent const & a, TraceEvent const & b) const
  {
    return str_eq(a.label, b.label) && a.id == b.id;
  }
};

struct TraceSink
{
  virtual void trace(TraceEvent event, Span<TraceRecord const> records) = 0;
};

struct NoopTraceSink final : TraceSink
{
  virtual void trace(TraceEvent, Span<TraceRecord const>) override
  {
  }
};

struct FileTraceSink final : TraceSink
{
  std::FILE * file = nullptr;

  FileTraceSink();

  virtual void trace(TraceEvent              event,
                     Span<TraceRecord const> records) override;
};

struct MemoryTraceSink final : TraceSink
{
  typedef Dict<TraceEvent, Vec<TraceRecord>, TraceEventHash, TraceEventEq>
    Records;

  std::mutex mutex_;

  Allocator allocator_;

  FileTraceSink * upstream_ = nullptr;

  /// @brief Number of records for each trace event before a flush happens
  usize buffer_size_ = 2'048;

  Records traces_;

  MemoryTraceSink(Allocator allocator);

  virtual void trace(TraceEvent event, Span<TraceRecord const> records);

  void flush();
};

extern TraceSink * trace_sink;

ASH_DLL_EXPORT ASH_C_LINKAGE void hook_trace_sink(TraceSink * instance);

struct ScopeTrace
{
  TraceEvent event;

  TraceRecord record;

  ScopeTrace(TraceEvent     event = TraceEvent{.label = "[Scope]"_str, .id = 0},
             SourceLocation loc   = SourceLocation::current()) :
    event{event},
    record{.loc = loc, .begin = steady_clock::now().time_since_epoch()}
  {
  }

  ScopeTrace(ScopeTrace const &)             = delete;
  ScopeTrace(ScopeTrace &&)                  = delete;
  ScopeTrace & operator=(ScopeTrace const &) = delete;
  ScopeTrace & operator=(ScopeTrace &&)      = delete;

  ~ScopeTrace()
  {
    record.end = steady_clock::now().time_since_epoch();
    trace_sink->trace(event, Span{&record, 1});
  }
};

inline NoopTraceSink noop_trace_sink;

}    // namespace ash
