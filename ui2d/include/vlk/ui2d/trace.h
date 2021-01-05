#pragma once

#include <chrono>
#include <string_view>
#include <vector>
#include "stx/source_location.h"

namespace vlk {
namespace ui2d {

struct EventTraceEntry {
  std::string_view event_name;
  std::chrono::steady_clock::time_point timepoint;
  bool begin;
};

struct ScalarTraceEntry {
  std::string_view scalar_name;
  std::chrono::steady_clock::time_point timepoint;
  double scalar;
};

// trace sink not thread-safe. not required since rendering is always on a
// single thread and it won't be used across multiple threads anyway
struct TraceSink {
  std::vector<EventTraceEntry> events;
  std::vector<ScalarTraceEntry> scalars;
  std::string_view name;
};

struct ScopeEventTrace {
  ScopeEventTrace(TraceSink &sink,
                  stx::SourceLocation location = stx::SourceLocation::current())
      : sink_{&sink}, event_name_{location.function_name()} {
    sink_->events.push_back(
        EventTraceEntry{event_name_, std::chrono::steady_clock::now(), true});
  }

  ~ScopeEventTrace() {
    sink_->events.push_back(
        EventTraceEntry{event_name_, std::chrono::steady_clock::now(), false});
  }

 private:
  TraceSink *sink_;
  std::string_view event_name_;
};

#define VLK_TRACE_SINK_FUNC_NAME(sink_name) \
  VLK_TRACE_API_get_tracer_trace_sink__##sink_name
#define VLK_DECLARE_TRACE_SINK(sink_name) \
  extern ::vlk::ui2d::TraceSink &VLK_TRACE_SINK_FUNC_NAME(sink_name)()
#define VLK_DEFINE_TRACE_SINK(sink_name)                                 \
  extern ::vlk::ui2d::TraceSink &VLK_TRACE_SINK_FUNC_NAME(sink_name)() { \
    static ::vlk::ui2d::TraceSink sink{{}, {}, #sink_name};              \
    return sink;                                                         \
  }

#define VLK_SCOPE_EVENT_TRACE_TO_SINK(sink_name)                                                                       \
  ::vlk::ui2d::ScopeEventTrace                                                                                         \
      VLK_ScopedEventTrace_must_be_unique_for_sink__##sink_name##__sink_source_per_scope_else_you_are_doing_it_wrong { \
    VLK_TRACE_SINK_FUNC_NAME(sink_name)()                                                                              \
  }

#define VLK_SCALAR_TRACE_TO_SINK(scalar, sink_name)           \
  do {                                                        \
    double value = static_cast<double>(scalar);               \
    VLK_TRACE_SINK_FUNC_NAME(sink_name)                       \
    ().scalars.push_back(::vlk::ui2d::ScalarTraceEntry{       \
        #scalar, ::std::chrono::steady_clock::now(), value}); \
  } while (false)



}  // namespace ui2d
}  // namespace vlk

