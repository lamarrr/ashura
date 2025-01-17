/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/time.h"
#include "ashura/std/types.h"

namespace ash
{

enum class TraceUnit
{
  None,
  Other,
  Items,
  Bytes
};

enum class TraceSource
{
};

struct TraceSink
{
  // [ ] static buffer
  // [ ] hooke
  virtual void trace_f64(Span<char const> event, TraceUnit unit, f64 number,
                         nanoseconds begin, nanoseconds end);
  virtual void trace_i64(Span<char const> event, TraceUnit unit, i64 number,
                         nanoseconds begin, nanoseconds end);
};

extern TraceSink * trace_sink;

ASH_DLL_EXPORT ASH_C_LINKAGE void hook_trace_sink(TraceSink * instance);

struct ScopeTrace
{
  Span<char const> name;
  time_point       begin;
};

}    // namespace ash
