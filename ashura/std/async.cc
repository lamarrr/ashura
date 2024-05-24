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
/// arenas are individually allocated from heap and span a page boundary.
///
struct TaskArena
{
  alias_count ac    = {};
  Arena       arena = {};
};

/// once task is executed, the arena holding the memory associated with the task
/// is returned back to the source.
///
/// tasks are always owned exclusively. the arena holds the memory for this Task
/// struct, and the memory for its related data. this has the advantage that
/// accessing the struct is cache-local.
///
/// @arena: always non-null. arena used to allocate the memory belonging to this
/// task.
///
struct Task
{
  uid                               thread     = UID_MAX;
  usize                             num_awaits = 0;
  Semaphore *ASH_RESTRICT           await_sems = nullptr;
  u64 *ASH_RESTRICT                 awaits     = nullptr;
  Fn<bool(void *&)>                 task = to_fn([](void *&) { return false; });
  void                             *data = nullptr;
  usize                             num_increments = 0;
  Semaphore *ASH_RESTRICT           increment_sems = nullptr;
  u64 *ASH_RESTRICT                 increments     = nullptr;
  usize                             num_signals    = 0;
  Semaphore *ASH_RESTRICT           signal_sems    = nullptr;
  u64 *ASH_RESTRICT                 signals        = nullptr;
  ListNode<TaskArena> *ASH_RESTRICT arena          = nullptr;
};

struct TaskQueue
{
  SpinLock   lock  = {};
  List<Task> tasks = {};

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
struct alignas(CACHELINE_ALIGNMENT) TaskThread
{
  uid         id              = UID_MAX;
  TaskQueue   dedicated_queue = {};
  StopToken   stop_token      = {};
  std::thread thread          = {};
  nanoseconds max_sleep       = {};
};

/// @allocator: must be thread-safe.
struct SchedulerImpl final : Scheduler
{
  SpinLock        lock                  = {};
  AllocatorImpl   allocator             = default_allocator;
  TaskThread     *dedicated_threads     = nullptr;
  TaskThread     *worker_threads        = nullptr;
  u32             num_dedicated_threads = 0;
  u32             num_worker_threads    = 0;
  TaskQueue       global_queue          = {};
  List<TaskArena> arena_freelist        = {};

  /// @brief return memory from the arena free list back to the allocator.
  void purge_arenas()
  {
    while (true)
    {
      lock.lock();

      ListNode<TaskArena> *arena = arena_freelist.pop_front();
      if (arena == nullptr)
      {
        break;
      }

      lock.unlock();

      CHECK(arena->data.ac.num_aliases() == 0);
      allocator.dealloc(arena->data.arena.alignment, arena->data.arena.begin,
                        arena->data.arena.size());
      allocator.ndealloc(arena, 1);
    }
  }

  /// @brief return unused arena memory to the scheduler
  void return_arena(ListNode<TaskArena> *ASH_RESTRICT arena)
  {
    lock.lock();
    // add to front and make the memory hot
    arena_freelist.extend_front(List{arena});
    lock.unlock();
  }

  static void thread_task(TaskThread &t, TaskQueue &q, SchedulerImpl &s,
                          bool is_dedicated)
  {
    u64 poll = 0;
    do
    {
      ListNode<Task> *const task = q.pop_task();

      if (task == nullptr)
      {
        sleepy_backoff(poll, t.max_sleep);
        poll++;
        continue;
      }

      // if a task requires a specific non-dedicated thread, it could starve.
      // checking each and every task before popping will lead to O(n) worst
      // case, and also cause starvation due to the spinlock on the list for too
      // long. therefore, only dedicated threads can immediately handle tasks
      // with specific threads.
      if (task->data.thread != UID_MAX && task->data.thread != t.id)
      {
        q.insert_tasks(List{task});
        continue;
      }

      if (!await_semaphores({task->data.await_sems, task->data.num_awaits},
                            {task->data.awaits, task->data.num_awaits}, 0))
      {
        q.insert_tasks(List{task});
        continue;
      }

      // finally gotten a ready task, resetting here will prevent  when
      // we've gotten multiple tasks but none have been ready.
      poll = 0;

      bool const should_requeue = task->data.task(task->data.data);

      for (usize i = 0; i < task->data.num_signals; i++)
      {
        signal_semaphore(task->data.signal_sems[i], task->data.signals[i]);
      }

      for (usize i = 0; i < task->data.num_increments; i++)
      {
        increment_semaphore(task->data.increment_sems[i],
                            task->data.increments[i]);
      }

      if (should_requeue)
      {
        // add back to end of queue
        q.insert_tasks(List{task});
        continue;
      }

      // decrease alias count of arena, if only alias left, add to the arena
      // free list.
      if (task->data.arena->data.ac.unalias())
      {
        task->data.arena->data.arena.reset();
        s.return_arena(task->data.arena);
      }
    } while (!t.stop_token.is_stop_requested());
  }

  virtual void init(Span<nanoseconds const> dedicated_thread_sleep,
                    Span<nanoseconds const> worker_thread_sleep) override
  {
    CHECK(dedicated_thread_sleep.size() < U32_MAX);
    CHECK(worker_thread_sleep.size() < U32_MAX);
    num_dedicated_threads = (u32) dedicated_thread_sleep.size();
    num_worker_threads    = (u32) worker_thread_sleep.size();
    CHECK(allocator.nalloc(num_dedicated_threads, &dedicated_threads));
    CHECK(allocator.nalloc(num_worker_threads, &worker_threads));

    for (u32 i = 0; i < num_dedicated_threads; i++)
    {
      new (dedicated_threads + i) TaskThread{
          .id              = 0x0000,
          .dedicated_queue = {},
          .stop_token      = {},
          .thread          = std::thread{[i, this] {
            thread_task(dedicated_threads[i],
                                 dedicated_threads[i].dedicated_queue, *this, true);
          }},
          .max_sleep       = dedicated_thread_sleep[i]};
    }

    for (u32 i = 0; i < num_worker_threads; i++)
    {
      new (worker_threads + i) TaskThread{
          .id              = 0x0000,
          .dedicated_queue = {},
          .stop_token      = {},
          .thread          = std::thread{[i, this] {
            thread_task(worker_threads[i], global_queue, *this, false);
          }},
          .max_sleep       = worker_thread_sleep[i]};
    }
  }

  virtual void uninit() override
  {
    /*
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
      threads[i].~TaskThread();
    }

    allocator.ndealloc(threads, n_threads);
    n_threads = 0;

    // TODO (lamarrr): release global queue
    */
  }

  virtual void schedule_dedicated(u32 thread, TaskInfo const &info) override
  {
    // allocate
    lock.lock();
    // push onto stack.
    lock.unlock();
  }

  virtual void schedule_worker(TaskInfo const &info) override
  {
  }

  virtual void schedule_main(TaskInfo const &info) override
  {
  }

  virtual void execute_main_thread_work(nanoseconds timeout_ns) override
  {
  }
};

static SchedulerImpl task_scheduler_impl;
Scheduler           *scheduler = &task_scheduler_impl;
}        // namespace ash