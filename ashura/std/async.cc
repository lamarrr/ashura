/// SPDX-License-Identifier: MIT
#include "ashura/std/async.h"
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/list.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include <thread>

namespace ash
{

inline constexpr usize TASK_ARENA_SIZE = PAGE_SIZE;

/// @brief Memory is returned back to the scheduler once ac reaches 0.
///
/// arenas are individually allocated from heap and span a page boundary.
///
struct TaskArena : Pin<>
{
  TaskArena *      next = nullptr;
  TaskArena *      prev = nullptr;
  AtomicAliasCount ac{};
  Arena            arena{};

  static constexpr auto flex()
  {
    return Flex<TaskArena, u8>{
      {layout_of<TaskArena>,
       Layout{.alignment = MAX_STANDARD_ALIGNMENT, .size = TASK_ARENA_SIZE}}
    };
  }
};

/// @brief Once the task is executed, the arena holding the memory associated
/// with the task is returned back to the source.
///
/// the arena holds the memory for this Task struct, and the memory for its
/// related data. this has the advantage that accessing the struct is
/// cache-local.
///
///
/// @note this struct is a flexible struct with the task frame appended to the
/// end of its allocation. The struct is carefully ordered based on the access
/// pattern by the executors.
///
struct Task
{
  typedef Fn<void(void *)> Init;

  typedef void (*Uninit)(void *);

  typedef bool (*Poll)(void *);

  typedef bool (*Runner)(void *);

  Task * next = nullptr;

  Task * prev = nullptr;

  Layout frame_layout{};

  Poll poll = [](void *) { return true; };

  Runner runner = [](void *) { return false; };

  Uninit uninit = noop;

  /// @brief Arena this task was allocated from. always non-null.
  TaskArena * arena = nullptr;

  static constexpr auto flex(Layout frame_layout)
  {
    return Flex<Task, u8>{
      {layout_of<Task>, frame_layout}
    };
  }
};

static_assert(TASK_ARENA_SIZE != 0,
              "Task arena size must be a non-zero power of 2");

static_assert(is_pow2((u64) TASK_ARENA_SIZE),
              "Task arena size must be a non-zero power of 2");

static_assert(TASK_ARENA_SIZE >= (MAX_TASK_FRAME_SIZE << 2),
              "Task arena size is too small");

static_assert(MAX_TASK_FRAME_SIZE >= MAX_STANDARD_ALIGNMENT,
              "Maximum task frame size is too small");

// assuming it has maximum alignment as well, although this would typically be
// maximum of MAX_STANDARD_ALIGNMENT
inline constexpr Layout MAX_TASK_FRAME_LAYOUT{.alignment = MAX_TASK_FRAME_SIZE,
                                              .size      = MAX_TASK_FRAME_SIZE};

inline constexpr Layout MAX_TASK_FLEX_LAYOUT =
  Task::flex(MAX_TASK_FRAME_LAYOUT).layout();

static_assert(TASK_ARENA_SIZE >= MAX_TASK_FLEX_LAYOUT.size,
              "Task arena size is too small to fit the maximum task frame and "
              "task context");

struct TaskAllocator
{
  /// @brief The source allocator the arenas are allocated from
  Allocator source;

  /// @brief Arena free list. all arenas on the free list a fully reclaimed and
  /// can be immediately used.
  /// This is appended to by the task executors once they are done with the
  /// arenas.
  struct alignas(CACHELINE_ALIGNMENT)
  {
    IFutex          lock{};
    List<TaskArena> list{};

    TaskArena * pop()
    {
      LockGuard   guard{lock};
      // return the most recently used arena
      TaskArena * arena = list.pop_back();
      return arena;
    }

  } free_list{};

  /// @brief Current arena being used for allocating new tasks. once this arena
  /// is exhausted, we query from the freelist, and if that is empty, we
  /// allocate a new arena and make it the current arena.
  struct alignas(CACHELINE_ALIGNMENT)
  {
    IFutex      lock{};
    TaskArena * node = nullptr;

  } current_arena{};

  explicit TaskAllocator(Allocator src) : source{src}
  {
  }

  TaskAllocator(TaskAllocator const &) = delete;

  TaskAllocator(TaskAllocator &&) = delete;

  TaskAllocator & operator=(TaskAllocator const &) = delete;

  TaskAllocator & operator=(TaskAllocator &&) = delete;

  ~TaskAllocator()
  {
    if (current_arena.node != nullptr)
    {
      dealloc_arena(current_arena.node);
    }

    while (!free_list.list.is_empty())
    {
      dealloc_arena(free_list.list.pop_front());
    }
  }

  void release_arena(TaskArena * arena)
  {
    // decrease alias count of arena, if only alias left, add to the arena
    // free list.
    if (arena->ac.unalias() == 0)
    {
      arena->arena.reclaim();
      LockGuard guard{free_list.lock};
      free_list.list.push_back(arena);
    }
  }

  bool alloc_arena(TaskArena *& out)
  {
    auto const   flex   = TaskArena::flex();
    Layout const layout = flex.layout();

    u8 * stack;

    if (!source->alloc(layout, stack))
    {
      return false;
    }

    auto [arena, memory] = flex.unpack(stack);

    out = new (arena.data()) TaskArena{.arena{memory}};

    return true;
  }

  void dealloc_arena(TaskArena * arena)
  {
    source->dealloc(TaskArena::flex().layout(), (u8 *) arena);
  }

  bool request_arena(TaskArena *& out)
  {
    /// get from free list, otherwise allocate a new arena
    TaskArena * a = free_list.pop();
    if (a != nullptr)
    {
      out = a;
      return true;
    }
    return alloc_arena(out);
  }

  static bool alloc_task(TaskArena & arena, TaskInfo const & info, Task *& out)
  {
    auto const   flex   = Task::flex(info.frame_layout);
    Layout const layout = flex.layout();

    u8 * stack;

    if (!arena.arena.alloc(layout, stack))
    {
      return false;
    }

    arena.ac.alias();

    auto [task, ctx] = flex.unpack(stack);

    out = new (task.data()) Task{.frame_layout = info.frame_layout,
                                 .poll         = info.poll,
                                 .runner       = info.runner,
                                 .uninit       = info.uninit,
                                 .arena        = &arena};

    info.init(ctx.data());

    return true;
  }

  static void uninit_task(Task * task)
  {
    auto [_, ctx] = Task::flex(task->frame_layout).unpack(task);
    task->uninit(ctx.data());
  }

  void release_task(Task * task)
  {
    TaskArena * arena = task->arena;
    uninit_task(task);
    release_arena(arena);
  }

  bool create_task(TaskInfo const & info, Task *& task)
  {
    LockGuard guard{current_arena.lock};

    // no arena is set as current, request a new arena
    if (current_arena.node == nullptr && !request_arena(current_arena.node))
      [[unlikely]]
    {
      return false;
    }

    // try to allocate on the current arena
    if (alloc_task(*current_arena.node, info, task)) [[likely]]
    {
      return true;
    }

    // decrease alias count of current arena, if last alias, reclaim the memory
    // instead
    if (current_arena.node->ac.unalias() == 0) [[unlikely]]
    {
      current_arena.node->arena.reclaim();
    }
    else
    {
      current_arena.node = nullptr;
      // request new arena from free-list or source allocator
      if (!request_arena(current_arena.node)) [[unlikely]]
      {
        return false;
      }
    }

    return alloc_task(*current_arena.node, info, task);
  }
};

/// @brief FIFO task queue backed by a linked list
struct TaskQueue
{
  ISpinLock     lock{};
  List<Task>    tasks{};
  TaskAllocator allocator;

  explicit TaskQueue(Allocator src) : allocator{src}
  {
  }

  TaskQueue(TaskQueue const &) = delete;

  TaskQueue(TaskQueue &&) = delete;

  TaskQueue & operator=(TaskQueue const &) = delete;

  TaskQueue & operator=(TaskQueue &&) = delete;

  ~TaskQueue() = default;

  bool is_empty()
  {
    LockGuard guard{lock};
    return tasks.is_empty();
  }

  Task * pop_task()
  {
    LockGuard guard{lock};
    Task *    t = tasks.pop_front();
    return t;
  }

  /// @brief Push task on the queue
  /// @param t non-null task node
  void push_task(Task * t)
  {
    LockGuard guard{lock};
    tasks.push_back(t);
  }

  void push_task(TaskInfo const & info)
  {
    Task * t;
    CHECK(allocator.create_task(info, t), "");
    push_task(t);
  }
};

enum class ThreadType : u32
{
  Worker    = 0,
  Dedicated = 1
};

/// @param queue dedicated queue only used when the thread is a dedicated
/// thread.
struct alignas(CACHELINE_ALIGNMENT) TaskThread
{
  ThreadType  type;
  TaskQueue   queue;
  ISemaphore  drain_semaphore;
  nanoseconds max_sleep;
  std::thread thread;

  TaskThread(Allocator allocator, ThreadType type, nanoseconds max_sleep) :
    type{type},
    queue{allocator},
    drain_semaphore{},
    max_sleep{max_sleep}
  {
  }

  TaskThread(TaskThread const &)             = delete;
  TaskThread & operator=(TaskThread const &) = delete;
  TaskThread(TaskThread &&)                  = delete;
  TaskThread & operator=(TaskThread &&)      = delete;
  ~TaskThread()                              = default;
};

/// @param allocator must be thread-safe.
/// @param free_list arena free list. arenas not in use by any tasks are
/// inserted here
/// @param current_arena current arena being allocated from
struct ASH_DLL_EXPORT SchedulerImpl final : IScheduler
{
  Allocator allocator_;

  Vec<Dyn<TaskThread *>> dedicated_threads_;

  Vec<Dyn<TaskThread *>> worker_threads_;

  alignas(CACHELINE_ALIGNMENT) TaskQueue main_queue_;

  alignas(CACHELINE_ALIGNMENT) TaskQueue worker_queue_;

  bool joined_;

  std::thread::id main_thread_id_;

  explicit SchedulerImpl(Allocator allocator, std::thread::id main_thread_id) :
    allocator_{allocator},
    dedicated_threads_{allocator},
    worker_threads_{allocator},
    main_queue_{allocator},
    worker_queue_{allocator},
    joined_{false},
    main_thread_id_{main_thread_id}
  {
  }

  SchedulerImpl(SchedulerImpl const &) = delete;

  SchedulerImpl(SchedulerImpl &&) = delete;

  SchedulerImpl & operator=(SchedulerImpl const &) = delete;

  SchedulerImpl & operator=(SchedulerImpl &&) = delete;

  virtual ~SchedulerImpl() override
  {
    CHECK(joined_, "Scheduler not joined yet");
  }

  virtual void shutdown() override
  {
    CHECK(main_thread_id_ == std::this_thread::get_id(),
          "Scheduler can only be joined on the main thread");

    for (auto & t : worker_threads_)
    {
      (void) t->drain_semaphore.complete(0);
    }

    for (auto & t : dedicated_threads_)
    {
      (void) t->drain_semaphore.complete(0);
    }

    for (auto & t : worker_threads_)
    {
      t->thread.join();
    }

    for (auto & t : dedicated_threads_)
    {
      t->thread.join();
    }

    while (true)
    {
      Task * task = main_queue_.pop_task();

      if (task == nullptr)
      {
        break;
      }

      main_queue_.allocator.release_task(task);
    }

    joined_ = true;
  }

  static void thread_loop(TaskAllocator & a, TaskQueue & q, Semaphore s,
                          nanoseconds max_sleep)
  {
    u64 poll = 0;

    while (true)
    {
      Task * task = q.pop_task();

      if (task == nullptr) [[unlikely]]
      {
        // stop execution once all tasks are done and drain semaphore is signaled
        if (s->is_completed(0)) [[unlikely]]
        {
          (void) s->complete();
          break;
        }

        sleepy_backoff(poll, max_sleep);
        poll++;
        continue;
      }

      auto [_, frame] = Task::flex(task->frame_layout).unpack(task);

      bool const ready = task->poll(frame.data());

      if (!ready) [[unlikely]]
      {
        q.push_task(task);
        continue;
      }

      // finally gotten a ready task, reset poll counter
      poll = 0;

      bool const repeat = task->runner(frame.data());

      if (repeat) [[unlikely]]
      {
        // add to the back of the queue, giving pending tasks the opportunity to
        // run
        q.push_task(task);
        continue;
      }

      a.release_task(task);
    }

    // run loop done. purge pending tasks
    while (true)
    {
      Task * task = q.pop_task();

      if (task == nullptr)
      {
        break;
      }

      a.release_task(task);
    }
  }

  static void main_thread_loop(TaskAllocator & a, TaskQueue & q,
                               nanoseconds duration, nanoseconds poll_max)
  {
    time_point const begin      = steady_clock::now();
    time_point       poll_start = begin;

    while (true)
    {
      // avoid syscalls when duration is .max
      time_point now = steady_clock::now();

      if ((now - begin) > duration)
      {
        break;
      }

      Task * task = q.pop_task();

      if (task == nullptr) [[unlikely]]
      {
        // if maximum poll duration has passed, exit loop
        if ((now - poll_start) > poll_max)
        {
          break;
        }
        else
        {
          continue;
        }
      }

      auto [_, frame] = Task::flex(task->frame_layout).unpack(task);

      bool const ready = task->poll(frame.data());

      if (!ready) [[unlikely]]
      {
        q.push_task(task);
        continue;
      }

      // advance poll timer, since we've gotten a ready task
      poll_start = now;

      bool const repeat = task->runner(frame.data());

      if (repeat) [[unlikely]]
      {
        // add to the back of the queue, giving pending tasks the opportunity to
        // run
        q.push_task(task);
        continue;
      }

      a.release_task(task);
    }
  }

  virtual u32 num_dedicated() override
  {
    return size32(dedicated_threads_);
  }

  virtual u32 num_workers() override
  {
    return size32(worker_threads_);
  }

  virtual void schedule(TaskInfo const & info, Thread thread) override
  {
    thread.match(
      [&](WorkerThread t) {
        CHECK(t == WorkerThread::Any, "Invalid worker thread id");
        worker_queue_.push_task(info);
      },
      [&](DedicatedThread t) {
        CHECK((u32) t < num_dedicated(), "Invalid dedicated thread id");
        dedicated_threads_[(u32) t]->queue.push_task(info);
      },
      [&](MainThread) { main_queue_.push_task(info); });
  }

  virtual void run_main_loop(nanoseconds duration,
                             nanoseconds poll_max) override
  {
    main_thread_loop(main_queue_.allocator, main_queue_, duration, poll_max);
  }

  virtual void request_drain() override
  {
    for (auto & t : dedicated_threads_)
    {
      (void) t->drain_semaphore.complete(0);
    }
    for (auto & t : worker_threads_)
    {
      (void) t->drain_semaphore.complete(0);
    }
  }

  virtual bool await_drain(nanoseconds timeout) override
  {
    u8                buffer_[512];
    FallbackAllocator scratch{buffer_, allocator_};
    Vec<Semaphore>    semaphores{scratch};
    Vec<u64>          stages{scratch};

    semaphores.reserve(dedicated_threads_.size() + worker_threads_.size())
      .unwrap();
    stages.reserve(dedicated_threads_.size() + worker_threads_.size()).unwrap();

    for (auto & t : dedicated_threads_)
    {
      semaphores.push(&t->drain_semaphore).unwrap();
      stages.push(1ULL).unwrap();
    }

    for (auto & t : worker_threads_)
    {
      semaphores.push(&t->drain_semaphore).unwrap();
      stages.push(1ULL).unwrap();
    }

    return await_semaphores(semaphores, stages, timeout);
  }

  virtual Semaphore get_drain_semaphore(Thread thread) override
  {
    return thread.match(
      [&](WorkerThread) -> Semaphore {
        CHECK(false, "Worker threads do not have individual drain semaphores");
      },
      [&](DedicatedThread t) -> Semaphore {
        CHECK((u32) t < num_dedicated(), "Invalid dedicated thread id");
        return &dedicated_threads_[(u32) t]->drain_semaphore;
      },
      [&](MainThread) -> Semaphore {
        CHECK(false, "Main thread does not have a drain semaphore");
      });
  }
};

Dyn<Scheduler> IScheduler::create(SchedulerInfo const & info)
{
  auto impl = dyn<SchedulerImpl>(inplace, info.allocator, info.allocator,
                                 info.main_thread_id)
                .unwrap();

  for (auto sleep : info.dedicated_thread_sleep)
  {
    auto thread = dyn<TaskThread>(inplace, info.allocator, info.allocator,
                                  ThreadType::Dedicated, sleep)
                    .unwrap();
    thread->thread = std::thread{[t = thread.get()] {
      SchedulerImpl::thread_loop(t->queue.allocator, t->queue,
                                 &t->drain_semaphore, t->max_sleep);
    }};
    impl->dedicated_threads_.push(std::move(thread)).unwrap();
  }

  for (auto sleep : info.worker_thread_sleep)
  {
    auto thread = dyn<TaskThread>(inplace, info.allocator, info.allocator,
                                  ThreadType::Worker, sleep)
                    .unwrap();
    thread->thread = std::thread{[t = thread.get(), q = &impl->worker_queue_] {
      SchedulerImpl::thread_loop(q->allocator, *q, &t->drain_semaphore,
                                 t->max_sleep);
    }};
    impl->worker_threads_.push(std::move(thread)).unwrap();
  }

  return cast<Scheduler>(std::move(impl));
}

Scheduler scheduler = nullptr;

void hook_scheduler(Scheduler instance)
{
  scheduler = instance;
}

}    // namespace ash
