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
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include <chrono>
#include <thread>

namespace ash
{

constexpr usize TASK_ARENA_SIZE = PAGE_SIZE;

/// memory is returned back to the scheduler once ac reaches 0.
///
/// arenas are individually allocated from heap and span a page boundary.
///
struct TaskArena
{
  AliasCount ac{};
  Arena      arena{};

  static constexpr auto node_flex()
  {
    return Flex{
        {layout<ListNode<TaskArena>>,
         Layout{.alignment = MAX_STANDARD_ALIGNMENT, .size = TASK_ARENA_SIZE}}};
  }
};

/// once task is executed, the arena holding the memory associated with the task
/// is returned back to the source.
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

  typedef bool (*Run)(void *);

  Layout frame_layout{};

  Poll poll = [](void *) { return true; };

  Run run = [](void *) { return false; };

  Uninit uninit = noop;

  /// @brief arena this task was allocated from. always non-null.
  ListNode<TaskArena> *arena = nullptr;

  static constexpr auto node_flex(Layout frame_layout)
  {
    return Flex{{layout<ListNode<Task>>, frame_layout}};
  }
};

static_assert(TASK_ARENA_SIZE != 0,
              "Task arena size must be a non-zero power of 2");

static_assert(is_pow2(TASK_ARENA_SIZE),
              "Task arena size must be a non-zero power of 2");

static_assert(TASK_ARENA_SIZE >= (MAX_TASK_FRAME_SIZE << 2),
              "Task arena size is too small");

static_assert(MAX_TASK_FRAME_SIZE >= MAX_STANDARD_ALIGNMENT,
              "Task frame size is too small");

// assuming it has maximum alignment as well, although this would typically be
// maximum of MAX_STANDARD_ALIGNMENT
constexpr Layout MAX_TASK_FRAME_LAYOUT{.alignment = MAX_TASK_FRAME_SIZE,
                                       .size      = MAX_TASK_FRAME_SIZE};

constexpr Layout MAX_TASK_NODE_FLEX_LAYOUT =
    Task::node_flex(MAX_TASK_FRAME_LAYOUT).layout();

static_assert(TASK_ARENA_SIZE >= MAX_TASK_NODE_FLEX_LAYOUT.size,
              "Task arena size is too small to fit the maximum task frame and "
              "task metadata");

struct TaskAllocator
{
  /// @brief The source allocator the arenas are allocated from
  AllocatorImpl source;

  /// @brief Arena free list. all arenas on the free list a fully reclaimed and
  /// can be immediately used.
  /// This is appended to by the task executors once they are done with the
  /// arenas.
  struct alignas(CACHELINE_ALIGNMENT)
  {
    SpinLock        lock{};
    List<TaskArena> list{};

    ListNode<TaskArena> *pop()
    {
      LockGuard guard{lock};
      // return the most recently used arena
      ListNode<TaskArena> *arena = list.pop_back();
      return arena;
    }

  } free_list{};

  /// @brief current arena being used for allocating new tasks. once this arena
  /// is exhausted, we query from the freelist, and if that is empty, we
  /// allocate a new arena and make it the current arena.
  struct alignas(CACHELINE_ALIGNMENT)
  {
    SpinLock             lock{};
    ListNode<TaskArena> *node = nullptr;

  } current_arena{};

  explicit TaskAllocator(AllocatorImpl src) : source{src}
  {
  }

  TaskAllocator(TaskAllocator const &) = delete;

  TaskAllocator(TaskAllocator &&) = default;

  TaskAllocator &operator=(TaskAllocator const &) = delete;

  TaskAllocator &operator=(TaskAllocator &&) = default;

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

  void release_arena(ListNode<TaskArena> *arena)
  {
    // decrease alias count of arena, if only alias left, add to the arena
    // free list.
    if (arena->v.ac.unalias())
    {
      arena->v.arena.reclaim();
      LockGuard guard{free_list.lock};
      free_list.list.push_back(arena);
    }
  }

  bool alloc_arena(ListNode<TaskArena> *&arena)
  {
    Flex const   flex   = TaskArena::node_flex();
    Layout const layout = flex.layout();

    u8 *head;

    if (!source.alloc(layout.alignment, layout.size, head))
    {
      return false;
    }

    u8 *memory;
    flex.unpack(head, arena, memory);

    new (arena) ListNode<TaskArena>{
        .v{.arena = to_arena(Span{memory, TASK_ARENA_SIZE})}};

    return true;
  }

  void dealloc_arena(ListNode<TaskArena> *arena)
  {
    Flex const   flex   = TaskArena::node_flex();
    Layout const layout = flex.layout();
    source.dealloc(layout.alignment, (u8 *) arena, layout.size);
  }

  bool request_arena(ListNode<TaskArena> *&arena)
  {
    /// get from free list, otherwise allocate a new arena
    ListNode<TaskArena> *a = free_list.pop();
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
    Flex const   flex   = Task::node_flex(info.frame_layout);
    Layout const layout = flex.layout();

    u8 *head;

    if (!arena.v.arena.alloc(layout.alignment, layout.size, head))
    {
      return false;
    }

    arena.v.ac.alias();

    u8 *ctx;

    flex.unpack(head, task, ctx);

    new (task) ListNode<Task>{.v{.frame_layout = info.frame_layout,
                                 .poll         = info.poll,
                                 .run          = info.run,
                                 .uninit       = info.uninit,
                                 .arena        = &arena}};

    info.init(ctx);

    return true;
  }

  static void uninit_task(ListNode<Task> *task)
  {
    Flex const flex = Task::node_flex(task->v.frame_layout);
    u8        *ctx;
    flex.unpack(task, task, ctx);
    task->v.uninit(ctx);
  }

  void release_task(ListNode<Task> *task)
  {
    ListNode<TaskArena> *arena = task->v.arena;
    uninit_task(task);
    release_arena(arena);
  }

  bool create_task(TaskInfo const &info, ListNode<Task> *&task)
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
    if (current_arena.node->v.ac.unalias()) [[unlikely]]
    {
      current_arena.node->v.arena.reclaim();
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
  SpinLock      lock{};
  List<Task>    tasks{};
  TaskAllocator allocator;

  explicit TaskQueue(AllocatorImpl src) : allocator{src}
  {
  }

  TaskQueue(TaskQueue const &) = delete;

  TaskQueue(TaskQueue &&) = default;

  TaskQueue &operator=(TaskQueue const &) = delete;

  TaskQueue &operator=(TaskQueue &&) = default;

  ~TaskQueue() = default;

  bool is_empty() const
  {
    return tasks.is_empty();
  }

  ListNode<Task> *pop_task()
  {
    LockGuard       guard{lock};
    ListNode<Task> *t = tasks.pop_front();
    return t;
  }

  /// @brief push task on the queue
  /// @param t non-null task node
  void push_task(ListNode<Task> *t)
  {
    LockGuard guard{lock};
    tasks.push_back(t);
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
struct alignas(CACHELINE_ALIGNMENT) TaskThread
{
  TaskQueue      queue;
  StopTokenState stop_token;
  nanoseconds    max_sleep;
  std::thread    thread;

  TaskThread(AllocatorImpl allocator, nanoseconds max_sleep) :
      queue{allocator}, stop_token{}, max_sleep{max_sleep}, thread{}
  {
  }
};

/// @param allocator must be thread-safe.
/// @param free_list arena free list. arenas not in use by any tasks are
/// inserted here
/// @param current_arena current arena being allocated from
struct ASH_DLL_EXPORT SchedulerImpl : Scheduler, Pin<>
{
  PinVec<TaskThread> dedicated_threads;

  PinVec<TaskThread> worker_threads;

  alignas(CACHELINE_ALIGNMENT) TaskQueue main_queue;

  alignas(CACHELINE_ALIGNMENT) TaskQueue worker_queue;

  std::thread::id main_thread_id;

  bool joined;

  explicit SchedulerImpl(AllocatorImpl   allocator,
                         std::thread::id main_thread_id) :
      dedicated_threads{},
      worker_threads{},
      main_queue{allocator},
      worker_queue{allocator},
      main_thread_id{main_thread_id},
      joined{false}
  {
  }

  virtual ~SchedulerImpl() override
  {
    CHECK_DESC(joined, "Scheduler not joined yet");
  }

  virtual void join() override
  {
    CHECK_DESC(main_thread_id == std::this_thread::get_id(),
               "Scheduler can only be joined on the main thread");

    for (TaskThread &t : dedicated_threads)
    {
      t.stop_token.request_stop();
    }

    for (TaskThread &t : worker_threads)
    {
      t.stop_token.request_stop();
    }

    for (TaskThread &t : dedicated_threads)
    {
      t.thread.join();
    }

    for (TaskThread &t : worker_threads)
    {
      t.thread.join();
    }

    while (true)
    {
      ListNode<Task> *task = main_queue.pop_task();

      if (task == nullptr)
      {
        break;
      }

      main_queue.allocator.release_task(task);
    }

    joined = true;
  }

  static void thread_loop(TaskAllocator &a, TaskQueue &q, StopTokenState &s,
                          nanoseconds max_sleep)
  {
    u64 poll = 0;

    while (true)
    {
      // stop execution even if there are pending tasks
      if (s.is_stop_requested()) [[unlikely]]
      {
        break;
      }

      ListNode<Task> *task = q.pop_task();

      if (task == nullptr) [[unlikely]]
      {
        sleepy_backoff(poll, max_sleep);
        poll++;
        continue;
      }

      Flex const flex = Task::node_flex(task->v.frame_layout);

      u8 *frame;

      flex.unpack(task, task, frame);

      if (!task->v.poll(frame)) [[unlikely]]
      {
        q.push_task(task);
        continue;
      }

      // finally gotten a ready task, reset poll counter
      poll = 0;

      bool const repeat = task->v.run(frame);

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
      ListNode<Task> *task = q.pop_task();

      if (task == nullptr)
      {
        break;
      }

      a.release_task(task);
    }
  }

  static void main_thread_loop(TaskAllocator &a, TaskQueue &q,
                               nanoseconds grace_period, nanoseconds duration)
  {
    steady_clock::time_point const begin = steady_clock::now();

    while (true)
    {
      // avoid syscalls when duration is .max
      if (duration != nanoseconds::max() &&
          (steady_clock::now() - begin) > duration)
      {
        break;
      }

      ListNode<Task> *task = q.pop_task();

      if (task == nullptr) [[unlikely]]
      {
        // if within grace period, continue polling.
        // avoid syscalls when timeout is 0
        if (grace_period == nanoseconds{} ||
            (steady_clock::now() - begin) > grace_period)
        {
          break;
        }
        else
        {
          continue;
        }
      }

      Flex const flex = Task::node_flex(task->v.frame_layout);

      u8 *frame;

      flex.unpack(task, task, frame);

      if (!task->v.poll(frame)) [[unlikely]]
      {
        q.push_task(task);
        continue;
      }

      bool const repeat = task->v.run(frame);

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
    return dedicated_threads.size32();
  }

  virtual u32 num_workers() override
  {
    return worker_threads.size32();
  }

  virtual void schedule_dedicated(TaskInfo const &info, u32 thread) override
  {
    CHECK_DESC(thread < dedicated_threads.size32(),
               "Invalid dedicated thread ID");
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

  virtual void execute_main_thread_loop(nanoseconds grace_period,
                                        nanoseconds duration) override
  {
    main_thread_loop(main_queue.allocator, main_queue, grace_period, duration);
  }
};

ASH_C_LINKAGE ASH_DLL_EXPORT Scheduler *scheduler = nullptr;

void Scheduler::init(AllocatorImpl allocator, std::thread::id main_thread_id,
                     Span<nanoseconds const> dedicated_thread_sleep,
                     Span<nanoseconds const> worker_thread_sleep)
{
  if (logger == nullptr)
  {
    abort();
  }
  CHECK(scheduler == nullptr);
  CHECK(dedicated_thread_sleep.size() <= U32_MAX);
  CHECK(worker_thread_sleep.size() <= U32_MAX);

  alignas(SchedulerImpl) static u8 storage[sizeof(SchedulerImpl)];

  SchedulerImpl *impl = new (storage) SchedulerImpl{allocator, main_thread_id};

  u32 const num_dedicated_threads = dedicated_thread_sleep.size32();
  u32 const num_worker_threads    = worker_thread_sleep.size32();

  impl->dedicated_threads =
      pin_vec<TaskThread>(allocator, num_dedicated_threads).unwrap();

  impl->worker_threads =
      pin_vec<TaskThread>(allocator, num_worker_threads).unwrap();

  for (u32 i = 0; i < num_dedicated_threads; i++)
  {
    impl->dedicated_threads.push(allocator, dedicated_thread_sleep[i]).unwrap();
    TaskThread &t = impl->dedicated_threads[i];
    t.thread      = std::thread{[&t] {
      SchedulerImpl::thread_loop(t.queue.allocator, t.queue, t.stop_token,
                                      t.max_sleep);
    }};
  }

  for (u32 i = 0; i < num_worker_threads; i++)
  {
    impl->worker_threads.push(allocator, worker_thread_sleep[i]).unwrap();
    TaskThread &t = impl->worker_threads[i];
    t.thread      = std::thread{[&t, impl] {
      SchedulerImpl::thread_loop(impl->worker_queue.allocator,
                                      impl->worker_queue, t.stop_token, t.max_sleep);
    }};
  }

  scheduler = impl;
}

void Scheduler::uninit()
{
  CHECK(scheduler != nullptr);
  scheduler->~Scheduler();
  scheduler = nullptr;
}

}        // namespace ash