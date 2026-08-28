/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/async.hpp"
#include "ashura/std/sformat.hpp"
#include "ashura/std/time.hpp"
#include "ashura/std/types.hpp"
#include "ashura/std/vec.hpp"

namespace ash
{
/// Functional Requirements
///
/// - Record function scopes and time entry points or additional scope-related
/// meta-data
/// - Record component values: strings, floats, integers, booleans, blobs,
/// meta-data
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

// TODO: trace spans

template <typename Data>
struct Record
{
    u64            variant = 0;
    SourceLocation loc     = {};
    Str            label   = ""_s;
    Data           data    = {};
};

using I64Record      = Record<I64>;
using F64Record      = Record<F64>;
using I64RangeRecord = Record<I64Range>;
using F64RangeRecord = Record<F64Range>;

struct EventData
{
    Str label = ""_s;

    Str type = ""_s;

    Str unit = ""_s;

    /// @brief encoded with "property0=value0;property1=value1;"
    Str attributes = ""_s;
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
        .variant = variant,
        .loc     = loc,
        .label   = label,
        .data    = I64Range{
                            .begin =
            static_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count(),
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
        // trace("[PERF TRACE] function: {}, begin: {} ns, end: {} ns"_s, record_.label,
        //       record_.data.begin, record_.data.end);
    }
};

}    // namespace tracing
}    // namespace ash

#define ASH_TRACE_SCOPE                      \
    ::ash::tracing::ScopeTrace scope_trace__ \
    {                                        \
        cstr(__PRETTY_FUNCTION__)            \
    }

#define ASH_TRACE_SUBSCOPE_IMPL(id, label)                                            \
    static ::ash::u8           id##_reserved__[256];                                  \
    static ::ash::IArena       id##_arena__{id##_reserved__};                         \
    static ::ash::ScratchScope id##_scratch__{&id##_arena__,                          \
                                              ::ash::default_allocator};              \
    static auto id##_label__ = ::ash::sformat(id##_scratch__, "{} -- {}"_s,           \
                                              cstr(__PRETTY_FUNCTION__), cstr(label)) \
                                 .unwrap();                                           \
    ::ash::tracing::ScopeTrace id                                                     \
    {                                                                                 \
        id##_label__                                                                  \
    }

#define ASH_TRACE_SUBSCOPE_DISPATCH(id, label) ASH_TRACE_SUBSCOPE_IMPL(id, label)

#define ASH_TRACE_SUBSCOPE(label) \
    ASH_TRACE_SUBSCOPE_DISPATCH(ASH_UNIQUE_NAME(scope_subtrace_), label)
