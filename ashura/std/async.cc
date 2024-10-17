/// SPDX-License-Identifier: MIT
#include "ashura/std/async.h"
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/backoff.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/list.h"
#include "ashura/std/log.h"
#include "ashura/std/time.h"
#include <chrono>
#include <thread>

namespace ash
{

Rc<Semaphore *> create_semaphore(u64 num_stages, AllocatorImpl const &allocator)
{
  Semaphore *s;
  CHECK(allocator.nalloc(1, s));
  new (s) Semaphore{};
  s->init(num_stages);
  Rc<Semaphore *> sem;
  sem.init(s, s->inner.alias_count, allocator,
           fn([](Semaphore *s, AliasCount &, AllocatorImpl const &allocator) {
             s->uninit();
             allocator.ndealloc(s, 1);
           }));
  return sem;
}

bool await_semaphores(Span<Rc<Semaphore *> const> semaphores,
                      Span<u64 const> stages, nanoseconds timeout)
{
  CHECK(semaphores.size() == stages.size());
  usize const n = semaphores.size();
  for (usize i = 0; i < n; i++)
  {
    CHECK((stages[i] == U64_MAX) ||
          (stages[i] <= semaphores[i]->inner.num_stages));
  }

  steady_clock::time_point begin = steady_clock::time_point{};
  for (usize i = 0; i < n; i++)
  {
    Rc<Semaphore *> const &s     = semaphores[i];
    u64 const              stage = min(stages[i], s->inner.num_stages - 1);
    u64                    poll  = 0;
    while (true)
    {
      // always monotonically increasing
      if (stage <= s->get_stage())
      {
        break;
      }

      /// we want to avoid syscalls if timeout is 0
      if (timeout == 0ns)
      {
        return false;
      }

      if (begin == steady_clock::time_point{}) [[unlikely]]
      {
        begin = steady_clock::now();
      }

      steady_clock::time_point const curr = steady_clock::now();
      nanoseconds const dur = duration_cast<nanoseconds>(curr - begin);
      if (dur > timeout) [[unlikely]]
      {
        return false;
      }

      yielding_backoff(poll);
      poll++;
    }
  }

  return true;
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

constexpr mem::Flex<2> task_arena_layout()
{
  return {mem::layout<ListNode<TaskArena>>,
          mem::Layout{.alignment = MAX_STANDARD_ALIGNMENT,
                      .size      = TASK_ARENA_SIZE}};
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
struct Task
{
  struct Sizing
  {
    usize       num_awaits     = 0;
    usize       num_increments = 0;
    usize       num_signals    = 0;
    mem::Layout ctx_layout     = {};
  };

  Fn<bool(void *)>     task       = fn([](void *) { return false; });
  Fn<void(void *)>     ctx_uninit = fn([](void *) {});
  ListNode<TaskArena> *arena      = nullptr;
  Sizing               sizing     = {};
};

constexpr mem::Flex<8> task_layout(Task::Sizing const &sizing)
{
  return {mem::layout<ListNode<Task>>,
          mem::layout<Rc<Semaphore *>>.array(sizing.num_awaits),
          mem::layout<u64>.array(sizing.num_awaits),
          mem::layout<Rc<Semaphore *>>.array(sizing.num_increments),
          mem::layout<u64>.array(sizing.num_increments),
          mem::layout<Rc<Semaphore *>>.array(sizing.num_signals),
          mem::layout<u64>.array(sizing.num_signals),
          sizing.ctx_layout};
}

void uninit_task(ListNode<Task> *task)
{
  Span<Rc<Semaphore *>> await_sems;
  Span<u64>             awaits;
  Span<Rc<Semaphore *>> increment_sems;
  Span<u64>             increments;
  Span<Rc<Semaphore *>> signal_sems;
  Span<u64>             signals;
  u8                   *ctx;

  mem::Flex flex = task_layout(task->data.sizing);

  flex.unpack(task, task, await_sems, awaits, increment_sems, increments,
              signal_sems, signals, ctx);

  for (auto &sem : await_sems)
  {
    sem.uninit();
  }

  for (auto &sem : increment_sems)
  {
    sem.uninit();
  }

  for (auto &sem : signal_sems)
  {
    sem.uninit();
  }
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
    mem::Flex   flex   = task_arena_layout();
    mem::Layout layout = flex.layout();

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
    mem::Flex         flex   = task_arena_layout();
    mem::Layout const layout = flex.layout();
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
    CHECK(info.awaits.size() == info.await_semaphores.size());
    CHECK(info.signals.size() == info.signal_semaphores.size());
    CHECK(info.increments.size() == info.increment_semaphores.size());

    Task::Sizing sizing{.num_awaits     = info.awaits.size(),
                        .num_increments = info.signals.size(),
                        .num_signals    = info.increments.size(),
                        .ctx_layout     = info.ctx_layout};

    mem::Flex   flex   = task_layout(sizing);
    mem::Layout layout = flex.layout();

    CHECK(layout.size <= TASK_ARENA_SIZE);

    u8 *head;

    if (!arena.data.arena.alloc(layout.alignment, layout.size, head))
    {
      return false;
    }

    arena.data.ac.alias();

    task = (ListNode<Task> *) head;

    for (auto const &sem : info.await_semaphores)
    {
      sem.alias();
    }

    for (auto const &sem : info.increment_semaphores)
    {
      sem.alias();
    }

    for (auto const &sem : info.signal_semaphores)
    {
      sem.alias();
    }

    Span<Rc<Semaphore *>> await_sems;
    Span<u64>             awaits;
    Span<Rc<Semaphore *>> increment_sems;
    Span<u64>             increments;
    Span<Rc<Semaphore *>> signal_sems;
    Span<u64>             signals;
    u8                   *ctx;

    flex.unpack(task, task, await_sems, awaits, increment_sems, increments,
                signal_sems, signals, ctx);

    mem::copy(info.await_semaphores, await_sems);
    mem::copy(info.awaits, awaits);
    mem::copy(info.increment_semaphores, increment_sems);
    mem::copy(info.increments, increments);
    mem::copy(info.signal_semaphores, signal_sems);
    mem::copy(info.signals, signals);

    info.ctx_init(ctx);

    new (task) ListNode<Task>{.data = Task{.task       = info.task,
                                           .ctx_uninit = info.ctx_uninit,
                                           .arena      = &arena,
                                           .sizing     = sizing}};

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

      mem::Flex flex = task_layout(task->data.sizing);

      Span<Rc<Semaphore *>> await_sems;
      Span<u64>             awaits;
      Span<Rc<Semaphore *>> increment_sems;
      Span<u64>             increments;
      Span<Rc<Semaphore *>> signal_sems;
      Span<u64>             signals;
      u8                   *ctx;

      flex.unpack(task, task, await_sems, awaits, increment_sems, increments,
                  signal_sems, signals, ctx);

      if (!await_semaphores(await_sems, awaits, 0ns))
      {
        q.push_task(task);
        continue;
      }

      // finally gotten a ready task, resetting here will prevent  when
      // we've gotten multiple tasks but none have been ready.
      poll = 0;

      bool const should_requeue = task->data.task(ctx);

      for (usize i = 0; i < increments.size(); i++)
      {
        increment_sems[i]->increment(increments[i]);
      }

      for (usize i = 0; i < signals.size(); i++)
      {
        signal_sems[i]->signal(signals[i]);
      }

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

      mem::Flex flex = task_layout(task->data.sizing);

      Span<Rc<Semaphore *>> await_sems;
      Span<u64>             awaits;
      Span<Rc<Semaphore *>> increment_sems;
      Span<u64>             increments;
      Span<Rc<Semaphore *>> signal_sems;
      Span<u64>             signals;
      u8                   *ctx;

      flex.unpack(task, task, await_sems, awaits, increment_sems, increments,
                  signal_sems, signals, ctx);

      if (!await_semaphores(await_sems, awaits, 0ns))
      {
        q.push_task(task);
        continue;
      }

      bool const should_requeue = task->data.task(ctx);

      for (usize i = 0; i < increments.size(); i++)
      {
        increment_sems[i]->increment(increments[i]);
      }

      for (usize i = 0; i < signals.size(); i++)
      {
        signal_sems[i]->increment(signals[i]);
      }

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