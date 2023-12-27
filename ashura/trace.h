
#pragma once

#include "ashura/time.h"
#include "ashura/types.h"
#include "stx/source_location.h"

#define DECLARE_TRACE_CATEGORY()
#define DEFINE_TRACE_CATEGORY()
#define TRACE_EVENT()

namespace ash
{

typedef struct TraceSink_T       *TraceSink;
typedef struct TraceSinkInterface TraceSinkInterface;
typedef struct TraceSinkImpl      TraceSinkImpl;

typedef struct Tracer_T       *Tracer;
typedef struct TracerInterface TracerInterface;
typedef struct TracerImpl      TracerImpl;

// trace_u64(slot, value)
// trace_i64(slot, value)
// trace_f64(slot, value)

// log to console, log to json file, log to whatever
// use for regression testing
// use sink only?
// trace memory allocations
// trace times
struct TracerInterface
{
  /// @category: category of event
  /// @source: id of source of this event
  /// @event: event name
  u64 (*begin_event)(Tracer self, Span<char const> scope,
                     Span<char const> source, Span<char const> event) = nullptr;
  void (*end_event)(Tracer self, u64 event_id)                        = nullptr;
};

struct TracerImpl
{
  Tracer                 self      = nullptr;
  TracerInterface const *interface = nullptr;
};

}        // namespace ash
