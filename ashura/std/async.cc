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
  Fn<bool(void *&)>                 task = to_fn([](void *&) { return false; });
  void                             *data = nullptr;
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
/// @head: circular linked list
///
struct TaskQueue
{
  SpinLock   lock  = {};
  List<Task> tasks = {};
  // pop from front if matches cond
  // TODO(lamarrr): if task requires a specific thread, it will starve!, we
  // can't be checking each and every task and lead to O(n) worst case, or
  // hold the list for too long.
  // only dedicated threads should handle specific tasks.
  ListNode<Task> *pop_task()
  {
    lock.lock();
    ListNode<Task> *t = tasks.pop_front();
    lock.unlock();
    return t;
  }

  void insert_tasks(List<Task> t)
  {
    lock.lock();
    tasks.extend_back(t);
    lock.unlock();
  }
};

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

static void worker_task(WorkerThread &t, TaskQueue &q)
{
  u64 poll = 0;
  do
  {
    ListNode<Task> *task = q.pop_task();

    if (task == nullptr)
    {
      sleepy_backoff(poll, t.max_sleep);
      poll++;
      continue;
    }

    if (!await_semaphores({task->v.await_sem, task->v.num_awaits},
                          {task->v.awaits, task->v.num_awaits}, 0))
    {
      q.insert_tasks(List{task});
      continue;
    }

    // finally gotten a ready task, resetting here will prevent spinning when
    // we've gotten multiple tasks but none have been ready.
    poll = 0;

    bool const should_requeue = task->v.task(task->v.data);

    for (usize i = 0; i < task->v.num_signals; i++)
    {
      signal_semaphore(task->v.signal_sems[i], task->v.signals[i]);
    }

    for (usize i = 0; i < task->v.num_increments; i++)
    {
      signal_semaphore(task->v.increment_sems[i], task->v.increments[i]);
    }

    if (should_requeue)
    {
      // add back to end of queue
      q.insert_tasks(List{task});
      continue;
    }

    // release arena memory

  } while (!t.stop_token.is_stop_requested());
}

static void dedicated_task(WorkerThread &t)
{
}

static void main_thread_task(WorkerThread &t);

/// TODO(lamarrr): task fairness whilst acknowledging dependencies
struct SchedulerImpl final : Scheduler
{
  SpinLock                          lock           = {};
  AllocatorImpl                     allocator      = default_allocator;
  WorkerThread                     *threads        = nullptr;
  u32                               n_threads      = 0;
  TaskQueue                         global_queue   = {};
  ListNode<TaskArena> *ASH_RESTRICT current_arena  = nullptr;
  ListNode<TaskArena> *ASH_RESTRICT arena_freelist = nullptr;

  /// @brief return memory from the arena free list back to the allocator.
  void purge_memory(AllocatorImpl const &src);
  void uninit();

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