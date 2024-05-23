#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/semaphore.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"


namespace ash
{

// requirements:
// [v] result collection
// [v] inter-task communication
// [v] inter-task sharing
// [v] inter-task data flow, reporting cancelation
// [v] SPSC-buffer ring buffer?
// [v] external polling contexts

/// @thread: thread to schedule task on. main thread is always thread 0. U32_MAX
/// means any thread.
struct TaskInfo
{
  Fn<bool(void *&)>     task             = to_fn([](void *&) { return false; });
  void                 *data             = nullptr;
  Span<Semaphore const> await_semaphores = {};
  Span<u64 const>       awaits           = {};
  Span<Semaphore const> signal           = {};
  Span<u64 const>       signals          = {};
  Span<Semaphore const> increment_semaphores = {};
  Span<u64 const>       increments           = {};
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
struct Scheduler
{
  virtual void init(Span<nanoseconds const> dedicated_thread_sleep,
                    Span<nanoseconds const> worker_thread_sleep)    = 0;
  virtual void uninit()                                             = 0;
  virtual void schedule_dedicated(u32 thread, TaskInfo const &info) = 0;
  virtual void schedule_worker(TaskInfo const &info)                = 0;
  virtual void schedule_main(TaskInfo const &info)                  = 0;
  virtual void execute_main_thread_work(nanoseconds timeout)        = 0;
};

extern Scheduler *scheduler;

}        // namespace ash
