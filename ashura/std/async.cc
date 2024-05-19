#include "ashura/std/async.h"
#include "ashura/std/arena_allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/vec.h"
#include <atomic>
#include <thread>

namespace ash
{

struct Task
{
  Fn<void(void *)> task          = to_fn([](void *) {});
  void            *data          = nullptr;
  u32              thread        = U32_MAX;
  Semaphore       *await         = nullptr;
  Semaphore       *signal        = nullptr;
  u64             *await_stages  = nullptr;
  u64             *signal_stages = nullptr;
  usize            num_await     = 0;
  usize            num_signal    = 0;
};

// TODO(lamarrr): add checking of readiness
///
/// pop task, not ready, re-add, now blocked because queue is full.
///
/// @tasks: tasks that can be executed by any thread
///
/// we create N thread pools, where N = min(2, number of logical cores - 1), and
/// then assign a fixed-capacity ring buffer of tasks to the global task queue.
///
///
/// once a thread becomes idle and needs work, it will pop a task from the
/// global task queue, check if it is ready to run. if it ready to run, it
/// executes it, otherwise, it pushes it back to the end of the queue.
///
/// once a task is added to the scheduler (only from the
/// main thread), if it has a specific thread to run on, it is added to the task
/// ring buffer queue of that thread, otherwise, it is added to the global
///
///
///
// use balloting to decide which thread will take work from the global queue
// and push it to other threads.
///
struct SchedulerImpl final : Scheduler
{
  AllocatorImpl      allocator   = default_allocator;
  std::atomic<bool> *stop_tokens = nullptr;
  u32                nr_threads  = 0;
  Vec<Task>          tasks       = {};

  alignas(CACHELINE_ALIGNMENT) std::atomic<bool> is_locked = false;

  virtual void init(usize task_queue_size) override
  {
  }

  virtual void shutdown() override
  {
  }

  virtual void wait_queue_idle(u32 thread) override
  {
  }

  virtual u32 num_threads() override
  {
  }

  virtual void schedule(ScheduleInfo const &info, bool wait) override
  {
  }

  virtual void execute_main_thread_work(u64 timeout_ns) override
  {
  }
};

static SchedulerImpl task_scheduler_impl;
Scheduler           *scheduler = &task_scheduler_impl;

}        // namespace ash