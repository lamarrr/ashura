#include "ashura/std/async.h"
#include "ashura/std/alias_count.h"
#include "ashura/std/arena_allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/list.h"
#include "ashura/std/spinlock.h"
#include "ashura/std/stop_token.h"
#include "ashura/std/vec.h"
#include <thread>

namespace ash
{

/// memory is returned back to the scheduler once ac reaches 0.
///
/// arenas are individually allocated from heap and typically span a page
/// boundary.
///
struct TaskArena
{
  alias_count ac    = {};
  Arena       arena = {};
};

/// once task is executed, the arena holding the memory associated with the task
/// is returned back to the source.
///
///
/// tasks are always owned exclusively. the arena holds the memory for this Task
/// struct, and the memory for its related data. this has the advantage that
/// accessing the struct is cache-local.
///
/// @arena: always non-null. arena used to allocate the memory belonging to this
/// task.
///
///
struct Task
{
  uid                               thread     = UID_MAX;
  Semaphore *ASH_RESTRICT           await_sem  = nullptr;
  u64 *ASH_RESTRICT                 awaits     = nullptr;
  usize                             num_awaits = 0;
  Fn<bool(void *)>                  task = to_fn([](void *) { return false; });
  void *ASH_RESTRICT                data = nullptr;
  usize                             num_increments = 0;
  usize                             num_signals    = 0;
  Semaphore *ASH_RESTRICT           increment_sems = nullptr;
  u64 *ASH_RESTRICT                 increments     = nullptr;
  Semaphore *ASH_RESTRICT           signal_sems    = nullptr;
  u64 *ASH_RESTRICT                 signals        = nullptr;
  ListNode<TaskArena> *ASH_RESTRICT arena          = nullptr;
};

/// left to decide: how to store tasks? shared global queue.
/// check from head until find task with a matching thread id, pop from front,
/// try to execute by checking availability, if not ready re-insert to the back
/// of the queue.
///
/// dedicated thread will do same but have a dedicated queue.
///
/// the task queue is a linked list with O(1) insert and remove.
///
/// - Fair, mostly
/// - Will handle degenerate dependencies well
///
/// arenas are not shared across queues.
///
/// @allocator: must be thread-safe.
///
///
struct TaskQueue
{
  SpinLock                          lock           = {};
  ListNode<Task> *ASH_RESTRICT      tasks          = nullptr;
  ListNode<TaskArena> *ASH_RESTRICT current_arena  = nullptr;
  ListNode<TaskArena> *ASH_RESTRICT arena_freelist = nullptr;
  // AllocatorImpl                     allocator      = default_allocator;
};

/// @brief return memory from the arena free list back to the allocator.
static void purge_memory(TaskQueue &q);

/// @dedicated_queue: only used when the thread is a dedicated thread.
///
struct alignas(CACHELINE_ALIGNMENT) WorkerThread
{
  uid         id              = UID_MAX;
  TaskQueue   dedicated_queue = {};
  StopToken   stop_token      = {};
  std::thread thread          = {};
  nanoseconds max_sleep       = {};
};

static bool poll_task(Task const &t)
{
  return await_semaphores({t.await_sem, t.num_awaits}, {t.awaits, t.num_awaits},
                          0);
}

static void complete_task(Task const &t)
{
  for (usize i = 0; i < t.num_increments; i++)
  {
    increment_semaphore(t.increment_sems[i], t.increments[i]);
  }

  for (usize i = 0; i < t.num_signals; i++)
  {
    signal_semaphore(t.signal_sems[i], t.signals[i]);
  }
}

static void worker_task(WorkerThread &t, TaskQueue &gq)
{
  u64 poll = 0;
  do
  {
    lock->lock();
    if (true)
    {
      poll = 0;
      // exec task
    }
    lock->unlock();
    sleepy_backoff(poll, responsiveness);
  } while (!token->stop_requested());
}

static void dedicated_task(WorkerThread &t)
{
}

static void main_thread_task(WorkerThread &t);

/// TODO(lamarrr): task fairness whilst acknowledging dependencies
struct SchedulerImpl final : Scheduler
{
  AllocatorImpl allocator    = default_allocator;
  WorkerThread *threads      = nullptr;
  u32           n_threads    = 0;
  TaskQueue     global_queue = {};

  virtual void init(u32 num_dedicated) override
  {
    u32 n = std::thread::hardware_concurrency();
    CHECK(n > 0);
    n         = min(1U, n - 1);
    n_threads = n;
    tasks     = {allocator};
    CHECK(allocator.nalloc(n, &stop_tokens));
    CHECK(allocator.nalloc(n, &threads));
    CHECK(allocator.nalloc(n, &reponsiveness));
    for (u32 i = 0; i < n; i++)
    {
      new (stop_tokens + i) StopToken{};
      new (reponsiveness + i) nanoseconds{500us};
      new (threads + i) std::thread{worker_task, stop_tokens + i, &lock, &tasks,
                                    reponsiveness[i]};
    }
  }

  virtual void uninit() override
  {
    for (u32 i = 0; i < n_threads; i++)
    {
      threads[i].stop_token.request_stop();
    }

    for (u32 i = 0; i < n_threads; i++)
    {
      threads[i].thread.join();
      // TODO(lamarrr): free all queue memory
    }

    for (u32 i = 0; i < n_threads; i++)
    {
      threads[i].~WorkerThread();
    }

    allocator.ndealloc(threads, n_threads);
    n_threads = 0;

    // TODO (lamarrr): release global queue
  }

  virtual u32 num_worker_threads()
  {
  }

  virtual u32 num_dedicated_threads()
  {
  }

  virtual void schedule(ScheduleInfo const &info) override
  {
    // allocate
    lock.lock();
    // push onto stack.
    lock.unlock();
  }

  virtual void execute_main_thread_work(u64 timeout_ns) override
  {
  }
};

static SchedulerImpl task_scheduler_impl;
Scheduler           *scheduler = &task_scheduler_impl;
}        // namespace ash