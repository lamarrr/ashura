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
//

/// @thread: thread to schedule task on. main thread is always thread 0. U32_MAX
/// means any thread.
/// @await_state: expected states of the await semaphores before the task begins
/// execution.
/// @signal_state: state to change the signal semaphores to after task
/// execution.
struct ScheduleInfo
{
  Fn<void(void *)>      task          = to_fn([](void *) {});
  void                 *data          = nullptr;
  u32                   thread        = U32_MAX;
  Span<Semaphore const> await         = {};
  Span<Semaphore const> signal        = {};
  Span<u64 const>       await_stages  = {};
  Span<u64 const>       signal_stages = {};
};

/// all tasks execute out-of-order. but have dependencies enforced by
/// semaphores.
struct Scheduler
{
  virtual void init(usize task_queue_size) = 0;
  virtual void shutdown()                  = 0;
  virtual void wait_queue_idle(u32 thread) = 0;
  virtual u32  num_threads()               = 0;

  ///
  /// @param info
  /// @return false if the scheduler's queue is full
  virtual void schedule(ScheduleInfo const &info, bool wait) = 0;

  virtual void execute_main_thread_work(u64 timeout_ns) = 0;
};

extern Scheduler *scheduler;

}        // namespace ash
