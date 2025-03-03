/// SPDX-License-Identifier: MIT
#include "ashura/std/trace.h"

namespace ash
{

TraceSink * trace_sink = &noop_trace_sink;

ASH_DLL_EXPORT ASH_C_LINKAGE void hook_trace_sink(TraceSink * instance)
{
  trace_sink = instance;
}

}    // namespace ash
