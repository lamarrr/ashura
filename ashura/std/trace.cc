/// SPDX-License-Identifier: MIT
#include "ashura/std/trace.h"

namespace ash
{
namespace tracing
{

static constexpr usize BUFFER_SIZE     = 4'096;
static constexpr usize MAX_NUM_THREADS = 256;

static InplaceVec<Tuple<EventSink<I64RangeRecord> *, std::thread::id>,
                  MAX_NUM_THREADS>
                  thread_scope_sinks;
static std::mutex thread_scope_sinks_lock;

struct EventSinkHook
{
  EventSink<I64RangeRecord> sink;
  std::thread::id           thread_id;

  template <typename... Args>
  EventSinkHook(Args &&... args) :
    sink{static_cast<Args &&>(args)...},
    thread_id{std::this_thread::get_id()}
  {
    LockGuard guard{thread_scope_sinks_lock};
    thread_scope_sinks
      .push(
        Tuple<EventSink<I64RangeRecord> *, std::thread::id>{&sink, thread_id})
      .unwrap();
  }

  EventSinkHook(EventSinkHook const &)             = delete;
  EventSinkHook(EventSinkHook &&)                  = delete;
  EventSinkHook & operator=(EventSinkHook const &) = delete;
  EventSinkHook & operator=(EventSinkHook &&)      = delete;

  ~EventSinkHook()
  {
    LockGuard guard{thread_scope_sinks_lock};
    for (usize i = 0; i < thread_scope_sinks.size(); i++)
    {
      if (thread_scope_sinks[i].v1 == thread_id)
      {
        thread_scope_sinks.erase(i, 1);
        break;
      }
    }
  }
};

EventSink<I64RangeRecord> & scope_trace_sink()
{
  static thread_local EventSinkHook hook{
    EventData{.label = "ScopeTrace"_str,
              .type  = "I64Range"_str,
              .unit  = "nanoseconds"_str},
    Vec<I64RangeRecord>::make(BUFFER_SIZE, default_allocator).unwrap()
  };

  return hook.sink;
}

}    // namespace tracing
}    // namespace ash
