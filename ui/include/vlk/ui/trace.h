#pragma once
#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace vlk {

namespace trace {
// causal tracing?

struct Frame {
  std::string_view identifier;
  std::chrono::steady_clock::time_point begin;
  std::chrono::steady_clock::time_point end;
  std::chrono::steady_clock::duration duration;
};

struct Context {
  std::mutex mutex;
  std::vector<Frame> frames;
  void add_frame(Frame frame) {
    std::lock_guard lock{mutex};
    frames.push_back(frame);
  }
};

// for tracing events that begin and end on the same thread
struct SyncEntry {
  SyncEntry(Context* context, std::string_view identifier) {
    frame.identifier = identifier;
    context = context;
    frame.begin = std::chrono::steady_clock::now();
  }

  ~SyncEntry() {
    frame.end = std::chrono::steady_clock::now();
    frame.duration = frame.end - frame.begin;
    context->add_frame(frame);
  }

  Frame frame;
  Context* context;
};

}  // namespace trace

}  // namespace vlk

#define VLK_MAKE_TRACE_CONTEXT_FUNCNAME(identifier) VlkTraceContext_##identifier
#define VLK_DECLARE_TRACE_CONTEXT(identifier) \
  vlk::trace::Context* VLK_MAKE_TRACE_CONTEXT_FUNCNAME(identifier)()
#define VLK_DEFINE_TRACE_CONTEXT(identifier)                           \
  vlk::trace::Context* VLK_MAKE_TRACE_CONTEXT_FUNCNAME(identifier)() { \
    static vlk::trace::Context context;                                \
    return &context;                                                   \
  }

#define VLK_TRACEr(context_identifier, trace_identifier)                    \
  vlk::trace::SyncEntry {                                                   \
    VLK_MAKE_TRACE_CONTEXT_FUNCNAME(context_identifier)(), trace_identifier \
  }

#define VLK_TRACE(context_identifier, trace_identifier)

// VLK_DECLARE_TRACE_CONTEXT(Render);
// VLK_DEFINE_TRACE_CONTEXT(Render);
