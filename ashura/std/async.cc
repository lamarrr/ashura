/// SPDX-License-Identifier: MIT
#include "ashura/std/async.h"
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/list.h"
#include "ashura/std/thread.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include <thread>

#if ASH_OS_WINDOWS
#    include <Windows.h>
#else
#    if ASH_OS_LINUX
#        include <linux/futex.h>
#    else
#        if ASH_OS_APPLE
#            include <sys/ulock.h>
#        endif
#    endif
#endif

namespace ash
{

#if ASH_OS_WINDOWS

struct SyncLib
{
    typedef BOOL
      WINAPI (*PFn_WaitOnAddress)(_In_reads_bytes_(AddressSize) volatile VOID * Address,
                                  _In_reads_bytes_(AddressSize) PVOID CompareAddress,
                                  _In_ SIZE_T                         AddressSize,
                                  _In_opt_ DWORD                      dwMilliseconds);

    typedef VOID WINAPI (*PFn_WakeByAddressAll)(_In_ PVOID Address);

    HMODULE              lib              = nullptr;
    PFn_WaitOnAddress    WaitOnAddress    = nullptr;
    PFn_WakeByAddressAll WakeByAddressAll = nullptr;

    void load()
    {
        lib = LoadLibraryW(L"API-MS-Win-Core-Synch-l1-2-0.dll");
        ASH_CHECK(lib != nullptr, "Failed to load API-MS-Win-Core-Synch-l1-2-0.dll");

        WaitOnAddress = (PFn_WaitOnAddress) GetProcAddress(lib, "WaitOnAddress");
        ASH_CHECK(WaitOnAddress != nullptr,
                  "Failed to get address of WaitOnAddress from "
                  "API-MS-Win-Core-Synch-l1-2-0.dll");

        WakeByAddressAll =
          (PFn_WakeByAddressAll) GetProcAddress(lib, "WakeByAddressAll");
        ASH_CHECK(WakeByAddressAll != nullptr,
                  "Failed to get address of WakeByAddressAll from "
                  "API-MS-Win-Core-Synch-l1-2-0.dll");
    }
};

#else

struct SyncLib
{
    void load()
    {
    }
};

#endif

static SyncLib   sync_lib;
static SyncLib * sink_lib_ptr;

SyncLib & get_sync_lib()
{
    ASH_CHECK(sink_lib_ptr != nullptr, "Sync lib not initialized");
    return *sink_lib_ptr;
}

void init_sync_runtime()
{
    sync_lib.load();
    sink_lib_ptr = &sync_lib;
}

void IWaitToken::os_notify()
{
#if ASH_OS_WINDOWS

    get_sync_lib().WakeByAddressAll(&state_);

#else

#    if ASH_OS_APPLE

    __ulock_wake(UL_COMPARE_AND_WAIT | ULF_WAKE_ALL, &state_, 0);

#    else

#        if ASH_OS_LINUX

    futex(&state_, FUTEX_WAKE_PRIVATE, I32_MAX, nullptr, nullptr);

#        endif

#    endif

#endif
}

void IWaitToken::os_wait(u32 old_state, std::memory_order order)
{
    // TODO: handle errors

#if ASH_OS_WINDOWS

    std::atomic_ref state{state_};

    do
    {
        // might wake up spuriously
        get_sync_lib().WaitOnAddress((u32 *) &state_, (PVOID) &old_state,
                                     sizeof(old_state), U32_MAX);
    } while (state.load(order) == old_state);

#else

#    if ASH_OS_APPLE
    std::atomic_ref state{state_};

    do
    {
        __ulock_wait(UL_COMPARE_AND_WAIT, (u32 *) &state_, (u64) old_state, U32_MAX);
    } while (state.load(order) == old_state);

#    else

#        if ASH_OS_LINUX
    std::atomic_ref state{state_};

    do
    {
        futex((i32 *) &state_, FUTEX_WAIT_PRIVATE, (i32) old_state, nullptr, nullptr);
    } while (state.load(order) == old_state);

#        else

    std::atomic_ref state{state_};
    usize           poll = 0;
    do
    {
        backoff_yield(poll);
        poll++;
    } while (state.load(order) == old_state);

#        endif

#    endif

#endif
}

/// @brief Memory is returned back to the scheduler once ac reaches 0.
///
/// arenas are individually allocated from heap and span a page boundary.
///
struct TaskArena : Pin<>
{
    TaskArena *      next = nullptr;
    TaskArena *      prev = nullptr;
    AtomicAliasCount ac{};
    IArena           arena{};
    Layout           arena_layout{};

    static constexpr auto flex(Layout arena_layout)
    {
        return Flex<TaskArena, u8>{
          {layout_of<TaskArena>, arena_layout}
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

    typedef TaskState (*Tick)(void *);

    Task * next = nullptr;

    Task * prev = nullptr;

    Layout frame_layout{};

    Tick tick = [](void *) { return TaskState::Finished; };

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

inline constexpr usize TASK_ARENA_SIZE     = 32_KB;
inline constexpr usize MAX_TASK_FRAME_SIZE = 8_KB;

static_assert(TASK_ARENA_SIZE != 0, "Task arena size must be a non-zero power of 2");

static_assert(is_pow2((u64) TASK_ARENA_SIZE),
              "Task arena size must be a non-zero power of 2");

static_assert(TASK_ARENA_SIZE >= MAX_TASK_FRAME_SIZE << 1,
              "Task arena size is too small");

static_assert(MAX_TASK_FRAME_SIZE >= MAX_STANDARD_ALIGNMENT,
              "Maximum task frame size is too small");

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
        ISpinLock       lock{};
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
        ISpinLock   lock{};
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
        auto const flex = TaskArena::flex(
          Layout{.alignment = MAX_STANDARD_ALIGNMENT, .size = TASK_ARENA_SIZE});
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
        source->dealloc(arena->flex(arena->arena_layout).layout(), (u8 *) arena);
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
                                     .tick         = info.tick,
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
    IWaitToken    task_wait_token{0U};
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
        task_wait_token.store(1U, std::memory_order_release);
        task_wait_token.os_notify();
    }

    void push_task(TaskInfo const & info)
    {
        Task * t;
        ASH_CHECK(allocator.create_task(info, t), "");
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
    IWaitToken  shutdown_token;
    u64         stop_token;
    std::thread thread;

    TaskThread(Allocator allocator, ThreadType type) :
      type{type},
      queue{allocator},
      shutdown_token{0U},
      stop_token{0}
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
        ASH_CHECK(joined_, "Scheduler not joined yet");
    }

    virtual void shutdown() override
    {
        ASH_CHECK(main_thread_id_ == std::this_thread::get_id(),
                  "Scheduler can only be joined on the main thread");

        for (auto & t : worker_threads_)
        {
            std::atomic_ref stop_token{t->stop_token};
            stop_token.store(1U, std::memory_order_release);
        }

        for (auto & t : dedicated_threads_)
        {
            std::atomic_ref stop_token{t->stop_token};
            stop_token.store(1U, std::memory_order_release);
        }

        for (auto & t : worker_threads_)
        {
            t->shutdown_token.os_wait(0, std::memory_order_acquire);
        }

        for (auto & t : dedicated_threads_)
        {
            t->shutdown_token.os_wait(0, std::memory_order_acquire);
        }

        for (auto & t : worker_threads_)
        {
            if (t->thread.joinable())
            {
                t->thread.join();
            }
        }

        for (auto & t : dedicated_threads_)
        {
            if (t->thread.joinable())
            {
                t->thread.join();
            }
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

    static void thread_loop(TaskAllocator & a, TaskQueue & q,
                            WaitToken drain_wait_token, u64 & stop_token_)
    {
        u64             poll = 0;
        std::atomic_ref stop_token{stop_token_};

        // stop execution once drain token is signaled
        while (!stop_token.load(std::memory_order_relaxed)) [[likely]]
        {
            Task * task = q.pop_task();

            if (task == nullptr) [[unlikely]]
            {
                backoff_yield(poll);
                poll++;
                continue;
            }

            auto [_, frame] = Task::flex(task->frame_layout).unpack(task);

            auto state = task->tick(frame.data());

            switch (state)
            {
                case TaskState::NotReady:
                {
                    // add to the back of the queue
                    q.push_task(task);
                }
                break;

                case TaskState::Again:
                {
                    // finally gotten a ready task, reset poll counter
                    poll = 0;

                    // add to the back of the queue, giving pending tasks the opportunity to
                    // run
                    q.push_task(task);
                }
                break;

                case TaskState::Finished:
                {
                    // finally gotten a ready task, reset poll counter
                    poll = 0;

                    a.release_task(task);
                }
                break;

                default:
                    ASH_UNREACHABLE;
            }
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

        drain_wait_token->store(1U, std::memory_order_release);
        drain_wait_token->os_notify();
    }

    static void main_thread_loop(TaskAllocator & a, TaskQueue & q, nanoseconds duration,
                                 nanoseconds poll_max)
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

            auto state = task->tick(frame.data());

            switch (state)
            {
                case TaskState::NotReady:
                {
                    // add to the back of the queue
                    q.push_task(task);
                }
                break;

                case TaskState::Again:
                {
                    // advance poll timer, since we've gotten a ready task
                    poll_start = now;

                    // add to the back of the queue, giving pending tasks the opportunity to
                    // run

                    q.push_task(task);
                }
                break;

                case TaskState::Finished:
                {
                    // advance poll timer, since we've gotten a ready task
                    poll_start = now;

                    a.release_task(task);
                }
                break;

                default:
                    ASH_UNREACHABLE;
            }
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

    virtual void schedule_raw(Thread thread, TaskInfo const & info) override
    {
        thread.match(
          [&](WorkerThread t) {
              ASH_CHECK(t == WorkerThread::Any, "Invalid worker thread id");
              worker_queue_.push_task(info);
          },
          [&](DedicatedThread t) {
              ASH_CHECK((u32) t < num_dedicated(), "Invalid dedicated thread id");
              dedicated_threads_[(u32) t]->queue.push_task(info);
          },
          [&](MainThread) { main_queue_.push_task(info); });
    }

    virtual void run_main_loop(nanoseconds duration, nanoseconds poll_max) override
    {
        main_thread_loop(main_queue_.allocator, main_queue_, duration, poll_max);
    }

    // TODO: incorrect if threads are shutdown individually
    virtual void request_thread_shutdown(Thread thread) override
    {
        thread.match(
          [&](WorkerThread) {
              ASH_CHECK(false, "Worker threads cannot be shutdown individually");
          },
          [&](DedicatedThread t) {
              ASH_CHECK((u32) t < num_dedicated(), "Invalid dedicated thread id");
              auto &          token = dedicated_threads_[(u32) t]->stop_token;
              std::atomic_ref token_ref{token};
              token_ref.store(true, std::memory_order_relaxed);
          },
          [&](MainThread) { ASH_CHECK(false, "Main thread cannot be shutdown"); });
    }

    virtual void await_thread_shutdown(Thread thread) override
    {
        thread.match(
          [&](WorkerThread) {
              ASH_CHECK(false, "Worker threads cannot be shutdown individually");
          },
          [&](DedicatedThread t) {
              ASH_CHECK((u32) t < num_dedicated(), "Invalid dedicated thread id");
              auto & tok = dedicated_threads_[(u32) t]->shutdown_token;
              tok.os_wait(0U, std::memory_order_acquire);
          },
          [&](MainThread) { ASH_CHECK(false, "Main thread cannot be shutdown"); });
    }
};

Dyn<Scheduler> IScheduler::create(SchedulerInfo const & info)
{
    auto impl =
      dyn<SchedulerImpl>(inplace, info.allocator, info.allocator, info.main_thread_id)
        .unwrap();

    for (auto thread_info : info.dedicated_threads)
    {
        auto thread = dyn<TaskThread>(inplace, info.allocator, info.allocator,
                                      ThreadType::Dedicated)
                        .unwrap();
        auto thread_name = vec::copy(info.allocator, thread_info.name).unwrap();
        thread->thread =
          std::thread{[t = thread.get(), thread_name = std::move(thread_name)] mutable {
              set_thread_name(thread_name);
              thread_name.reset();
              SchedulerImpl::thread_loop(t->queue.allocator, t->queue,
                                         &t->shutdown_token, t->stop_token);
          }};
        impl->dedicated_threads_.push(std::move(thread)).unwrap();
    }

    for (auto thread_info : info.worker_threads)
    {
        auto thread =
          dyn<TaskThread>(inplace, info.allocator, info.allocator, ThreadType::Worker)
            .unwrap();
        auto thread_name = vec::copy(info.allocator, thread_info.name).unwrap();
        thread->thread   = std::thread{[t = thread.get(), q = &impl->worker_queue_,
                                      thread_name = std::move(thread_name)] mutable {
            set_thread_name(thread_name);
            thread_name.reset();
            SchedulerImpl::thread_loop(q->allocator, *q, &t->shutdown_token,
                                         t->stop_token);
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
