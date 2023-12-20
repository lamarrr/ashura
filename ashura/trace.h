
#pragma once

#define ASH_DECLARE_TRACE_CATEGORY()
#define ASH_DEFINE_TRACE_CATEGORY()
#define ASH_TRACE()
#define ASH_TRACE_FUNC()
#define ASH_TRACE_BEGIN()
#define ASH_TRACE_END()

#define DEFINE_TRACE
#define TRACE_PARAM

#include "ashura/time.h"
#include "ashura/types.h"
#include "stx/source_location.h"

namespace ash
{
typedef struct Tracer_T       *Tracer;
typedef struct TracerInterface TracerInterface;
typedef struct TracerImpl      TracerImpl;

// log to console, log to json file, log to whatever
// use for regression testing
struct TracerInterface
{
  u64  (*begin_event)(Tracer self, char const *category,
                     char const *event)       = nullptr;
  void (*end_event)(Tracer self, u64 event_id) = nullptr;
};

struct TracerImpl
{
  Tracer                 self      = nullptr;
  TracerInterface const *interface = nullptr;
};

}        // namespace ash
