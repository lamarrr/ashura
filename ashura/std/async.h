#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/semaphore.h"
#include "ashura/std/types.h"

namespace ash
{

// requirements:
// [v] result collection
// [v] semaphore deletion
// [v] inter-task communication
// [v] inter-task sharing
// [v] inter-task data flow, reporting cancelation
// [v] SPSC-buffer ring buffer?
// [v] external polling contexts
// [v] task tracers

/// @thread: thread to schedule task on. main thread is always thread 0. U32_MAX
/// means any thread.
// TODO(lamarrr): Fn<bool(void *)>; - external poll, i.e. on gpu work ready>????
struct ScheduleInfo
{
  Fn<bool(void *&)>     task             = to_fn([](void *&) { return false; });
  u64                   instances        = 1;
  void                 *data             = nullptr;
  uid                   thread           = UID_MAX;
  Span<Semaphore const> await_semaphores = {};
  Span<u64 const>       awaits           = {};
  Span<Semaphore const> signal           = {};
  Span<u64 const>       signals          = {};
  Span<Semaphore const> increment_semaphores = {};
  Span<u64 const>       increments           = {};
};

/// called after work is drained from a queue and awaiting tasks.
/// can point to another async context that provides tasks to the scheduler.
//
/// latency hints, i.e. async io, gpu io, etc. or more specific requirements
/// that are set by named constants
///
/// store polling information. is thread_safe. actually, constraints on how many
/// callees it can have? multi-producer or what?
///
/// how to poll work from GPU or audio and video provider. i.e. network
///
///
/// TODO(lamarrr): remove
struct PollInfo
{
  Span<char const> label                  = {};
  Fn<bool(void *)> task                   = {};
  bool             is_context_thread_safe = false;
  void            *context                = nullptr;
  uid              thread                 = UID_MAX;
};

/// all tasks execute out-of-order. but have dependencies enforced by
/// semaphores.
///
/// it has 2 types of threads: worker threads and dedicated threads.
///
/// dedicated threads are for processing latency-sensitive tasks that need to
/// happen within a deadline, i.e. audio, video. they can spin, sleep, preempt
/// and/or wait for tasks.
///
/// worker threads process any type of tasks, although might not be as
/// responsive as dedicated threads.
///
/// work submitted to the main thread MUST be extremely light-weight and
/// non-blocking.
///
///
/// if only 1 thread is supported, all tasks are executed on the main thread.
///
struct Scheduler
{
  virtual void init(u32 num_workers, u32 num_dedicated = 0) = 0;
  virtual void uninit()                                     = 0;
  virtual u32  num_worker_threads()                         = 0;
  virtual u32  num_dedicated_threads()                      = 0;
  virtual void schedule(ScheduleInfo const &info)           = 0;
  virtual void execute_main_thread_work(u64 timeout_ns)     = 0;
  virtual uid  add_poll(PollInfo const &)                   = 0;
  virtual void remove_poll(uid poll)                        = 0;
};

extern Scheduler *scheduler;

}        // namespace ash
