/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/async.h"
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

namespace tracing
{

struct I64
{
  i64 value = 0;
};

struct F64
{
  f64 value = 0.0;
};

struct I64Range
{
  i64 begin = 0;
  i64 end   = 0;
};

struct F64Range
{
  f64 begin = 0;
  f64 end   = 0;
};

// [ ] trace spans

template <typename Data>
struct Record
{
  u64            variant  = 0;
  SourceLocation location = {};
  Str            label    = ""_str;
  Data           data     = {};
};

using I64Record      = Record<I64>;
using F64Record      = Record<F64>;
using I64RangeRecord = Record<I64Range>;
using F64RangeRecord = Record<F64Range>;

struct EventData
{
  Str label = ""_str;

  Str type = ""_str;

  Str unit = ""_str;

  /// @brief encoded with "property0=value0;property1=value1;"
  Str attributes = ""_str;
};

template <typename Record>
struct EventSink
{
  EventData          event_;
  Vec<Record>        records_storage_;
  RingBuffer<Record> records_;
  usize              num_written_;
  usize              num_read_;
  alignas(CACHELINE_ALIGNMENT) ISpinLock spin_lock_;

  EventSink(EventData event, Vec<Record> buffer) :
    event_{event},
    records_storage_{std::move(buffer)},
    records_{records_storage_.data(), records_storage_.capacity()},
    num_written_{0},
    num_read_{0},
    spin_lock_{}
  {
    ASH_CHECK(is_pow2(buffer.capacity()), "");
  }

  EventSink(EventSink const &)             = delete;
  EventSink(EventSink &&)                  = default;
  EventSink & operator=(EventSink const &) = delete;
  EventSink & operator=(EventSink &&)      = default;
  ~EventSink()                             = default;

  void trace(Record const & record)
  {
    LockGuard guard{spin_lock_};
    records_.push_overrun(record);
    num_written_++;
  }

  /// @brief drain all available records into `out`
  /// @returns number of elements left in the RingBuffer after the operation
  usize drain(Buffer<Record> & out)
  {
    LockGuard guard{spin_lock_};
    return records_.pop_many(out);
  }
};

extern EventSink<I64RangeRecord> & get_scope_trace_sink();

struct ScopeTrace
{
  I64RangeRecord record_;

  ScopeTrace(Str label = SourceLocation::current().function, u64 variant = 0,
             SourceLocation loc = SourceLocation::current()) :
    record_{
      .variant  = variant,
      .location = loc,
      .label    = label,
      .data     = I64Range{.begin = static_cast<nanoseconds>(
                                  steady_clock::now().time_since_epoch())
                                  .count(),
                           .end = 0}
  }
  {
  }

  ScopeTrace(ScopeTrace const &)             = delete;
  ScopeTrace(ScopeTrace &&)                  = delete;
  ScopeTrace & operator=(ScopeTrace const &) = delete;
  ScopeTrace & operator=(ScopeTrace &&)      = delete;

  ~ScopeTrace()
  {
    record_.data.end =
      static_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
    get_scope_trace_sink().trace(record_);
  }
};

}    // namespace tracing
}    // namespace ash
