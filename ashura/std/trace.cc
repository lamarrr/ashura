/// SPDX-License-Identifier: MIT
#include "ashura/std/trace.h"
#include "ashura/std/list.h"

namespace ash
{
namespace tracing
{

struct EventSinkHook
{
    EventSinkHook *           next;
    EventSinkHook *           prev;
    EventSink<I64RangeRecord> v;

    static void push(EventSinkHook *);

    static void pop(EventSinkHook *);

    template <typename... Args>
    EventSinkHook(Args &&... args) : v{static_cast<Args &&>(args)...}
    {
        push(this);
    }

    EventSinkHook(EventSinkHook const &)             = delete;
    EventSinkHook(EventSinkHook &&)                  = delete;
    EventSinkHook & operator=(EventSinkHook const &) = delete;
    EventSinkHook & operator=(EventSinkHook &&)      = delete;

    ~EventSinkHook()
    {
        pop(this);
    }
};

struct EventSinkHooks
{
    ISpinLock           lock;
    List<EventSinkHook> sinks{};
};

static EventSinkHooks thread_event_sinks;

void EventSinkHook::push(EventSinkHook * hook)
{
    LockGuard guard{thread_event_sinks.lock};
    thread_event_sinks.sinks.push_front(hook);
}

void EventSinkHook::pop(EventSinkHook * hook)
{
    LockGuard guard{thread_event_sinks.lock};
    thread_event_sinks.sinks.pop_at(hook);
}

EventSink<I64RangeRecord> & get_scope_trace_sink()
{
    static constexpr usize CFG_BUFFER_SIZE = 4'096;

    static thread_local EventSinkHook hook{[] {
        return EventSink<I64RangeRecord>{
          EventData{
                    .label = "ScopeTrace"_s, .type = "I64Range"_s, .unit = "nanoseconds"_s},
          Vec<I64RangeRecord>::make(CFG_BUFFER_SIZE, default_allocator).unwrap()
        };
    }()};

    return hook.v;
}

}    // namespace tracing
}    // namespace ash
