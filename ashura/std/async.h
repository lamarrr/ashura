/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/semaphore.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"

namespace ash
{

struct TaskInfo
{
  Fn<bool()>               task                 = fn([] { return false; });
  Span<SemaphoreRef const> await_semaphores     = {};
  Span<u64 const>          awaits               = {};
  Span<SemaphoreRef const> signal_semaphores    = {};
  Span<u64 const>          signals              = {};
  Span<SemaphoreRef const> increment_semaphores = {};
  Span<u64 const>          increments           = {};
};

/// @brief Static Thread Pool Scheduler.
///
/// all tasks execute out-of-order and have dependencies enforced by
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
///
/// @note work submitted to the main thread MUST be extremely light-weight and
/// non-blocking.
///
/// Requirements:
/// [x] result collection
/// [x] inter-task communication
/// [x] inter-task sharing
/// [x] inter-task data flow, reporting cancelation
/// [x] external polling contexts
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

ASH_C_LINKAGE ASH_DLL_EXPORT Scheduler *scheduler;

}        // namespace ash
