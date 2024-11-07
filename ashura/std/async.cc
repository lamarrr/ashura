/// SPDX-License-Identifier: MIT
#include "ashura/std/async.h"
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/list.h"
#include "ashura/std/log.h"
#include "ashura/std/rc.h"
#include "ashura/std/time.h"
#include <chrono>
#include <thread>

namespace ash
{

Rc<Semaphore *> create_semaphore(u64 num_stages, AllocatorImpl allocator)
{
  CHECK(num_stages > 0);
  return rc_inplace<Semaphore>(allocator,
                               Semaphore::Inner{.num_stages = num_stages})
      .unwrap();
}

bool await_semaphores(Span<Rc<Semaphore *> const> sems, Span<u64 const> stages,
                      nanoseconds timeout, bool any)
{
  CHECK(sems.size() == stages.size());
  usize const n = sems.size();
  for (usize i = 0; i < n; i++)
  {
    CHECK((stages[i] == U64_MAX) || (stages[i] <= sems[i]->inner.num_stages));
  }

  // number of times we've polled so far, counting begins from 0
  u64 poll = 0;

  // avoid sys-calls unless absolutely needed
  steady_clock::time_point poll_begin{};

  // speeds up checks for the 'all' case. points to the next semaphore to be
  // checked
  usize next = 0;

  while (true)
  {
    if (any)
    {
      bool any_ready = false;

      for (usize i = 0; i < n; i++)
      {
        Rc<Semaphore *> const &s     = sems[i];
        u64 const              stage = min(stages[i], s->inner.num_stages - 1);
        bool const             is_ready = stage <= s->get_stage();
        any_ready                       = any_ready || is_ready;

        if (is_ready)
        {
          break;
        }
      }

      if (any_ready)
      {
        return true;
      }
    }
    else
    {
      for (; next < n; next++)
      {
        Rc<Semaphore *> const &s = sems[next];
        u64 const  stage         = min(stages[next], s->inner.num_stages - 1);
        bool const is_ready      = stage <= s->get_stage();

        if (!is_ready)
        {
          break;
        }
      }

      if (next == n)
      {
        return true;
      }
    }

    // fast-path to avoid syscalls
    if (timeout == nanoseconds{0}) [[likely]]
    {
      return false;
    }

    // fast-path to avoid syscalls
    if (timeout == nanoseconds::max()) [[likely]]
    {
      // infinite timeout
    }
    else
    {
      if (poll_begin == steady_clock::time_point{}) [[unlikely]]
      {
        poll_begin = steady_clock::now();
      }

      nanoseconds const time_past =
          duration_cast<nanoseconds>(steady_clock::now() - poll_begin);

      if (time_past > timeout) [[unlikely]]
      {
        return false;
      }
    }

    yielding_backoff(poll);
    poll++;
  }

  return false;
}

constexpr usize TASK_ARENA_SIZE = PAGE_ALIGNMENT;

/// memory is returned back to the scheduler once ac reaches 0.
///
/// arenas are individually allocated from heap and span a page boundary.
///
struct TaskArena
{
  AliasCount ac    = {};
  Arena      arena = {};
};

constexpr Flex<2> task_arena_layout()
{
  return {layout<ListNode<TaskArena>>,
          Layout{.alignment = MAX_STANDARD_ALIGNMENT, .size = TASK_ARENA_SIZE}};
}

/// once task is executed, the arena holding the memory associated with the task
/// is returned back to the source.
///
/// tasks are always owned exclusively. the arena holds the memory for this Task
/// struct, and the memory for its related data. this has the advantage that
/// accessing the struct is cache-local.
///
/// @param arena always non-null. arena used to allocate the memory belonging
/// to this task.
///
/// @param instance once this atomic counter reaches 0, the task is
/// released.
///
struct Task
{
  TaskInfo             info      = {};
  u64                  instances = 1;        // be careful when purging
  ListNode<TaskArena> *arena     = nullptr;
};

constexpr Flex<2> task_layout(Layout ctx_layout)
{
  return {layout<ListNode<Task>>, ctx_layout};
}

void uninit_task(ListNode<Task> *task)
{
  Flex flex = task_layout(task->data.info.ctx);
  u8  *ctx;
  flex.unpack(task, task, ctx);
  task->data.info.uninit(ctx);
}

struct TaskAllocator
{
  AllocatorImpl allocator                                  = default_allocator;
  alignas(CACHELINE_ALIGNMENT) SpinLock free_list_lock     = {};
  List<TaskArena> free_list                                = {};
  alignas(CACHELINE_ALIGNMENT) SpinLock current_arena_lock = {};
  ListNode<TaskArena> *current_arena                       = nullptr;

  void uninit()
  {
    if (current_arena != nullptr)
    {
      dealloc_arena(current_arena);
      current_arena = nullptr;
    }

    while (!free_list.is_empty())
    {
      dealloc_arena(free_list.pop_front());
    }
  }

  void release_arena(ListNode<TaskArena> *arena)
  {
    // decrease alias count of arena, if only alias left, add to the arena
    // free list.
    if (arena->data.ac.unalias())
    {
      arena->data.arena.reclaim();
      free_list_lock.lock();
      free_list.push_back(arena);
      free_list_lock.unlock();
    }
  }

  ListNode<TaskArena> *pop_free_list()
  {
    free_list_lock.lock();
    ListNode<TaskArena> *arena = free_list.pop_front();
    free_list_lock.unlock();
    return arena;
  }

  bool alloc_arena(ListNode<TaskArena> *&arena)
  {
    Flex   flex   = task_arena_layout();
    Layout layout = flex.layout();

    u8 *head;

    if (!allocator.alloc(layout.alignment, layout.size, head))
    {
      return false;
    }

    arena = (ListNode<TaskArena> *) head;

    u8 *arena_memory;
    flex.unpack(head, arena, arena_memory);

    new (arena) ListNode<TaskArena>{
        .data = TaskArena{.arena = Arena{.begin = arena_memory,
                                         .end = arena_memory + TASK_ARENA_SIZE,
                                         .offset    = arena_memory,
                                         .alignment = MAX_STANDARD_ALIGNMENT}}};
    return true;
  }

  void dealloc_arena(ListNode<TaskArena> *arena)
  {
    Flex         flex   = task_arena_layout();
    Layout const layout = flex.layout();
    allocator.dealloc(layout.alignment, (u8 *) arena, layout.size);
  }

  bool request_arena(ListNode<TaskArena> *&arena)
  {
    ListNode<TaskArena> *a = pop_free_list();
    if (a != nullptr)
    {
      arena = a;
      return true;
    }
    return alloc_arena(arena);
  }

  static bool alloc_task(ListNode<TaskArena> &arena, TaskInfo const &info,
                         ListNode<Task> *&task)
  {
    Flex   flex   = task_layout(info.ctx);
    Layout layout = flex.layout();

    CHECK(layout.size <= TASK_ARENA_SIZE);

    u8 *head;

    if (!arena.data.arena.alloc(layout.alignment, layout.size, head))
    {
      return false;
    }

    arena.data.ac.alias();

    u8 *ctx;

    flex.unpack(head, task, ctx);

    info.init(ctx);

    new (task) ListNode<Task>{.data = Task{.info = info, .arena = &arena}};

    return true;
  }

  bool create_task(TaskInfo const &info, ListNode<Task> *&task)
  {
    current_arena_lock.lock();
    defer unlock_{[&] { current_arena_lock.unlock(); }};

    if (current_arena == nullptr)
    {
      if (!request_arena(current_arena))
      {
        return false;
      }
      CHECK(alloc_task(*current_arena, info, task));
    }
    else
    {
      if (!alloc_task(*current_arena, info, task))
      {
        if (current_arena->data.ac.unalias())
        {
          current_arena->data.arena.reclaim();
        }
        else
        {
          current_arena = nullptr;
          if (!request_arena(current_arena))
          {
            return false;
          }
        }
        CHECK(alloc_task(*current_arena, info, task));
      }
    }

    return true;
  }
};

struct TaskQueue
{
  SpinLock      lock      = {};
  List<Task>    tasks     = {};
  TaskAllocator allocator = {};

  void uninit()
  {
    CHECK(tasks.is_empty());
    allocator.uninit();
  }

  ListNode<Task> *pop_task()
  {
    lock.lock();
    ListNode<Task> *t = tasks.pop_front();
    lock.unlock();
    return t;
  }

  void push_task(ListNode<Task> *t)
  {
    CHECK(t != nullptr);
    CHECK(t->is_isolated());
    CHECK(t->is_linked());
    lock.lock();
    tasks.push_back(t);
    lock.unlock();
  }

  void push_task(TaskInfo const &info)
  {
    ListNode<Task> *t;
    CHECK(allocator.create_task(info, t));
    push_task(t);
  }
};

/// @param queue dedicated queue only used when the thread is a dedicated
/// thread.
///
struct alignas(CACHELINE_ALIGNMENT) TaskThread
{
  TaskQueue   queue      = {};
  StopToken   stop_token = {};
  std::thread thread     = {};
  nanoseconds max_sleep  = {};
};

/// @param allocator must be thread-safe.
/// @param free_list arena free list. arenas not in use by any tasks are
/// inserted here
/// @param current_arena current arena being allocated from
struct SchedulerImpl : Scheduler
{
  AllocatorImpl allocator                             = default_allocator;
  TaskThread   *dedicated_threads                     = nullptr;
  TaskThread   *worker_threads                        = nullptr;
  u32           num_dedicated_threads                 = 0;
  u32           num_worker_threads                    = 0;
  alignas(CACHELINE_ALIGNMENT) TaskQueue main_queue   = {};
  alignas(CACHELINE_ALIGNMENT) TaskQueue worker_queue = {};

  static void thread_task(TaskAllocator &a, TaskQueue &q, StopToken &stop_token,
                          nanoseconds max_sleep)
  {
    u64 poll = 0;
    do
    {
      ListNode<Task> *task = q.pop_task();

      if (task == nullptr)
      {
        sleepy_backoff(poll, max_sleep);
        poll++;
        continue;
      }

      Flex flex = task_layout(task->data.info.ctx);

      u8 *ctx;

      flex.unpack(task, task, ctx);

      if (!task->data.info.poll(ctx))
      {
        q.push_task(task);
        continue;
      }

      // finally gotten a ready task, resetting here will prevent  when
      // we've gotten multiple tasks but none have been ready.
      poll = 0;

      bool const should_requeue = task->data.info.task(ctx);

      if (should_requeue)
      {
        // add back to end of queue
        q.push_task(task);
        continue;
      }

      ListNode<TaskArena> *arena = task->data.arena;
      uninit_task(task);
      a.release_arena(arena);
    }
    // stop execution even if there are pending tasks
    while (!stop_token.is_stop_requested());
  }

  static void main_thread_task(TaskAllocator &a, TaskQueue &q,
                               nanoseconds timeout)
  {
    auto begin = steady_clock::now();

    do
    {
      ListNode<Task> *task = q.pop_task();

      if (task == nullptr)
      {
        break;
      }

      Flex flex = task_layout(task->data.info.ctx);
      u8  *ctx;

      flex.unpack(task, task, ctx);

      if (!task->data.info.poll(ctx))
      {
        q.push_task(task);
        continue;
      }

      bool const should_requeue = task->data.info.task(ctx);

      if (should_requeue)
      {
        q.push_task(task);
        continue;
      }

      ListNode<TaskArena> *arena = task->data.arena;
      uninit_task(task);
      a.release_arena(arena);
    } while ((steady_clock::now() - begin) < timeout);
  }

  virtual void init(Span<nanoseconds const> dedicated_thread_sleep,
                    Span<nanoseconds const> worker_thread_sleep) override
  {
    CHECK(dedicated_thread_sleep.size() <= U32_MAX);
    CHECK(worker_thread_sleep.size() <= U32_MAX);
    num_dedicated_threads = dedicated_thread_sleep.size32();
    num_worker_threads    = worker_thread_sleep.size32();
    CHECK(allocator.nalloc(num_dedicated_threads, dedicated_threads));
    CHECK(allocator.nalloc(num_worker_threads, worker_threads));

    for (u32 i = 0; i < num_dedicated_threads; i++)
    {
      TaskThread *t = dedicated_threads + i;
      new (t) TaskThread{.max_sleep = dedicated_thread_sleep[i]};

      t->thread = std::thread{[t] {
        thread_task(t->queue.allocator, t->queue, t->stop_token, t->max_sleep);
      }};
    }

    for (u32 i = 0; i < num_worker_threads; i++)
    {
      TaskThread *t = worker_threads + i;
      new (t) TaskThread{.max_sleep = worker_thread_sleep[i]};

      t->thread = std::thread{[t, this] {
        thread_task(worker_queue.allocator, worker_queue, t->stop_token,
                    t->max_sleep);
      }};
    }
  }

  static void shutdown_thread(TaskThread *t)
  {
    t->stop_token.request_stop();
    t->thread.join();
    t->queue.uninit();
    t->~TaskThread();
  }

  virtual void uninit() override
  {
    for (u32 i = 0; i < num_worker_threads; i++)
    {
      shutdown_thread(worker_threads + i);
    }

    for (u32 i = 0; i < num_dedicated_threads; i++)
    {
      shutdown_thread(dedicated_threads + i);
    }

    worker_queue.uninit();
    main_queue.uninit();

    allocator.ndealloc(worker_threads, num_worker_threads);
    allocator.ndealloc(dedicated_threads, num_dedicated_threads);
    worker_threads        = nullptr;
    dedicated_threads     = nullptr;
    num_worker_threads    = 0;
    num_dedicated_threads = 0;
  }

  virtual u32 num_dedicated() override
  {
    return num_dedicated_threads;
  }

  virtual u32 num_workers() override
  {
    return num_worker_threads;
  }

  virtual void schedule_dedicated(u32 thread, TaskInfo const &info) override
  {
    CHECK(thread < num_dedicated_threads);
    TaskThread &t = dedicated_threads[thread];
    t.queue.push_task(info);
  }

  virtual void schedule_worker(TaskInfo const &info) override
  {
    worker_queue.push_task(info);
  }

  virtual void schedule_main(TaskInfo const &info) override
  {
    main_queue.push_task(info);
  }

  virtual void execute_main_thread_work(nanoseconds timeout) override
  {
    main_thread_task(main_queue.allocator, main_queue, timeout);
  }
};

static SchedulerImpl scheduler_impl;

ASH_C_LINKAGE ASH_DLL_EXPORT Scheduler *scheduler = &scheduler_impl;
}        // namespace ash