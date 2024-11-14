/// SPDX-License-Identifier: MIT
///
/// Stage-based Asynchrony
///
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/functional.h"
#include "ashura/std/mem.h"
#include "ashura/std/rc.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"

#include <atomic>
#include <thread>

#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
#  include <emmintrin.h>
#endif

namespace ash
{

inline void yielding_backoff(u64 poll)
{
  if (poll < 8)
  {
    return;
  }

  if (poll < 16)
  {
#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
    _mm_pause();
#else
#  if ASH_CFG(ARCH, ARM32) || ASH_CFG(ARCH, ARM64)
    __asm("yield");
#  endif
#endif

    return;
  }

  std::this_thread::yield();
  return;
}

inline void sleepy_backoff(u64 poll, nanoseconds sleep)
{
  if (poll < 8)
  {
    return;
  }

  if (poll < 16)
  {
#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
    _mm_pause();
#else
#  if ASH_CFG(ARCH, ARM32) || ASH_CFG(ARCH, ARM64)
    __asm("yield");
#  endif
#endif

    return;
  }

  if (poll < 64)
  {
    std::this_thread::yield();
    return;
  }

  std::this_thread::sleep_for(sleep);
  return;
}

struct SpinLock
{
  usize flag_ = false;

  void lock()
  {
    usize           expected = false;
    usize           target   = true;
    u64             poll     = 0;
    std::atomic_ref flag{flag_};
    while (!flag.compare_exchange_strong(
        expected, target, std::memory_order_acquire, std::memory_order_relaxed))
    {
      expected = false;
      yielding_backoff(poll);
      poll++;
    }
  }

  [[nodiscard]] bool try_lock()
  {
    usize           expected = false;
    usize           target   = true;
    std::atomic_ref flag{flag_};
    flag.compare_exchange_strong(expected, target, std::memory_order_acquire,
                                 std::memory_order_relaxed);
    return expected;
  }

  void unlock()
  {
    std::atomic_ref flag{flag_};
    flag.store(false, std::memory_order_release);
  }
};

template <typename L>
struct LockGuard
{
  L *lock_;

  explicit LockGuard(L &lock) : lock_{&lock}
  {
    lock_->lock();
  }

  LockGuard(LockGuard const &) = delete;

  LockGuard &operator=(LockGuard const &) = delete;

  LockGuard(LockGuard &&) = delete;

  LockGuard &operator=(LockGuard &&) = delete;

  ~LockGuard()
  {
    lock_->unlock();
  }
};

struct ReadWriteLock
{
  SpinLock lock_{};
  usize    num_writers_ = 0;
  usize    num_readers_ = 0;

  void lock_read()
  {
    u64 poll = 0;
    while (true)
    {
      LockGuard guard{lock_};
      if (num_writers_ == 0)
      {
        num_readers_++;
        return;
      }
      yielding_backoff(poll);
      poll++;
    }
  }

  void lock_write()
  {
    u64 poll = 0;

    while (true)
    {
      LockGuard guard{lock_};
      if (num_writers_ == 0 && num_readers_ == 0)
      {
        num_writers_++;
        return;
      }
      yielding_backoff(poll);
      poll++;
    }
  }

  void unlock_read()
  {
    LockGuard guard{lock_};
    num_readers_--;
  }

  void unlock_write()
  {
    LockGuard guard{lock_};
    num_writers_--;
  }
};

struct ReadGuard
{
  ReadWriteLock *lock_;

  explicit ReadGuard(ReadWriteLock &lock) : lock_{&lock}
  {
    lock_->lock_read();
  }

  ReadGuard(ReadGuard const &) = delete;

  ReadGuard &operator=(ReadGuard const &) = delete;

  ReadGuard(ReadGuard &&) = delete;

  ReadGuard &operator=(ReadGuard &&) = delete;

  ~ReadGuard()
  {
    lock_->unlock_read();
  }
};

struct WriteGuard
{
  ReadWriteLock *lock_;

  explicit WriteGuard(ReadWriteLock &lock) : lock_{&lock}
  {
    lock_->lock_write();
  }

  WriteGuard(WriteGuard const &) = delete;

  WriteGuard &operator=(WriteGuard const &) = delete;

  WriteGuard(WriteGuard &&) = delete;

  WriteGuard &operator=(WriteGuard &&) = delete;

  ~WriteGuard()
  {
    lock_->unlock_write();
  }
};

template <typename T>
struct [[nodiscard]] Sync
{
  T data_;

  ReadWriteLock lock_;

  template <typename... Args>
  constexpr Sync(Args &&...args) : data_{((Args &&) args)...}, lock_{}
  {
  }

  constexpr Sync(Sync const &) = delete;

  constexpr Sync(Sync &&) = delete;

  constexpr Sync &operator=(Sync const &) = delete;

  constexpr Sync &operator=(Sync &&) = delete;

  constexpr ~Sync() = default;

  template <Callable<T &> Op>
  void read(Op &&op)
  {
    ReadGuard guard{lock_};
    ((Op &&) op)(data_);
  }

  template <Callable<T &> Op>
  void write(Op &&op)
  {
    WriteGuard guard{lock_};
    ((Op &&) op)(data_);
  }
};

template <typename T>
Sync(T &&) -> Sync<T>;

template <typename T>
Sync(T const &) -> Sync<T>;

/// @brief A CPU Timeline Semaphore (a.k.a. Sequence Barrier) used for
/// synchronization in multi-stage cooperative multitasking jobs. Unlike typical
/// Binary/Counting Semaphores, A timeline semaphores a monotonic counter
/// representing the stages of an operation.
/// - Guarantees Forward Progress
/// - Scatter-gather operations only require one primitive
/// - Primitive can encode state of multiple operations and also be awaited by
/// multiple operations at once.
/// - Task ordering is established by the `state` which describes the number of
/// steps needed to complete a task, and can be awaited by other tasks.
/// - It is use and increment once, hence no deadlocks can occur. This also
/// enables cooperative synchronization between systems processing different
/// stages of an operation without explicit sync between them.
///
/// Semaphore can only move from state `i` to state `i+n` where n > 1.
///
/// Semaphore should ideally not be destroyed before completion as there could
/// possibly be other tasks awaiting it.
///
/// Semaphores never overflow. so it can have a maximum of U64_MAX stages.
struct SemaphoreState
{
  u64 num_stages_;

  u64 stage_;

  explicit constexpr SemaphoreState(u64 num_stages) :
      num_stages_{num_stages}, stage_{0}
  {
  }

  /// @brief Get the current semaphore stage. This represents the current stage
  /// being worked on.
  /// @param sem non-null
  /// @return
  [[nodiscard]] u64 get_stage() const
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire);
  }

  /// @brief Get the number of stages in the semaphore
  /// @param sem non-null
  /// @return
  [[nodiscard]] constexpr u64 get_num_stages() const
  {
    return num_stages_;
  }

  /// @brief
  /// @param sem non-null
  /// @return
  [[nodiscard]] bool is_completed() const
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire) == num_stages_;
  }

  /// @brief
  ///
  /// @param stage: stage of the semaphore currently executing. stage >=
  /// num_stages or U64_MAX means completion of the last stage of the operation.
  /// must be monotonically increasing for each call to signal_semaphore.
  ///
  /// @returns returns true if the signaled stage has not been passed yet.
  /// otherwise returns false.
  ///
  bool signal(u64 next)
  {
    next                    = min(next, num_stages_);
    u64             current = 0;
    std::atomic_ref stage{stage_};
    while (!stage.compare_exchange_strong(
        current, next, std::memory_order_release, std::memory_order_relaxed))
        [[unlikely]]
    {
      if (current >= next)
      {
        return false;
      }
    }
    return true;
  }

  /// @brief
  /// @param inc stage increment of semaphore. increment of >= num_stages is
  /// equivalent to driving it to completion.
  void increment(u64 inc)
  {
    inc                     = min(inc, num_stages_);
    u64             current = 0;
    u64             target  = inc;
    std::atomic_ref stage{stage_};
    while (!stage.compare_exchange_strong(
        current, target, std::memory_order_release, std::memory_order_relaxed))
        [[unlikely]]
    {
      target = min(sat_add(current, inc), num_stages_);
    }
  }
};

typedef Rc<SemaphoreState *> Semaphore;

/// @brief A Stop Sequence Token
struct StopTokenState
{
  /// @brief stage to stop execution before.
  /// this means the stage represented by `stop_before_` and all proceeding
  /// stages are canceled.
  u64 stop_point_ = U64_MAX;

  constexpr StopTokenState() = default;

  /// @brief check whether the specified stage has been canceled. synchronizes
  /// with the scope
  /// @return
  bool is_stop_requested(u64 stage = 0) const
  {
    std::atomic_ref stop_point{stop_point_};
    return stop_point.load(std::memory_order_acquire) <= stage;
  }

  /// @brief stop the execution at the specified stage, all tasks beyond that
  /// stage are also stopped. synchronizes with the scope
  void request_stop(u64 stage = 0)
  {
    std::atomic_ref stop_point{stop_point_};
    u64             expected = U64_MAX;
    u64             target   = min(expected, stage);
    while (!stop_point.compare_exchange_strong(
        expected, target, std::memory_order_release, std::memory_order_relaxed))
        [[unlikely]]
    {
      target = min(expected, stage);
    }
  }
};

typedef Rc<StopTokenState *> StopToken;

///
/// @brief Create an independently allocated semaphore object
///
/// @param num_stages: number of stages represented by this semaphore
/// @return Semaphore
///
inline Semaphore create_semaphore(u64 num_stages, AllocatorImpl allocator)
{
  return rc_inplace<SemaphoreState>(allocator, num_stages).unwrap();
}

inline StopToken create_stop_token(AllocatorImpl allocator)
{
  return rc_inplace<StopTokenState>(allocator).unwrap();
}

/// @brief await semaphores at the specified stages.
/// @param sems semaphores to wait for
/// @param stages stages of the semaphores to wait for completion of. must be <
/// semaphore.num_stages or == U64_MAX. U64_MAX meaning waiting for all stages'
/// completion.
/// @param timeout timeout to stop attempting to wait for the semaphores. when
/// timeout is 0, there's an immediate result, and when timeout is
/// nanoseconds::max() it waits for the semaphores forever untilt they are
/// ready.
/// @param any if to wait for all semaphores or atleast 1 semaphore.
/// @returns returns if the semaphore await operation completed successfully
/// based on the `any` criteria.
[[nodiscard]] inline bool
    await_semaphores(Span<SemaphoreState const *const> sems,
                     Span<u64 const> stages, nanoseconds timeout)
{
  CHECK(sems.size() == stages.size());
  usize const n = sems.size();
  for (usize i = 0; i < n; i++)
  {
    CHECK((stages[i] == U64_MAX) || (stages[i] <= sems[i]->get_num_stages()));
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
    for (; next < n; next++)
    {
      SemaphoreState const *const &s = sems[next];
      u64 const  stage               = min(stages[next], s->num_stages_ - 1);
      bool const is_ready            = stage <= s->get_stage();

      if (!is_ready)
      {
        break;
      }
    }

    if (next == n)
    {
      return true;
    }

    // fast-path to avoid syscalls
    if (timeout <= nanoseconds{0}) [[likely]]
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

      nanoseconds const past = steady_clock::now() - poll_begin;

      if (past > timeout) [[unlikely]]
      {
        return false;
      }
    }

    yielding_backoff(poll);
    poll++;
  }

  return false;
}

constexpr usize MAX_TASK_FRAME_SIZE = PAGE_SIZE >> 4;

template <typename F>
concept TaskFrame = requires(F f) {
  { !f.poll() };
  { !f.run() };
} && (sizeof(F) <= MAX_TASK_FRAME_SIZE);

/// @brief Task Frame layout and dynamic dispatch thunks
/// @param frame_layout memory layout of the task's frame.
/// @param init function to initialize the context data associated with the task
/// to a task stack.
/// @param uninit function to uninitialize the task and the associated data
/// context upon completion of the task.
/// @param poll function to poll for readiness of the task, must be extremely
/// light-weight and non-blocking. it is never called again once it returns
/// true.
/// @param run task to be executed on the executor. must return true if it
/// should be re-queued onto the executor.
/// @param instances number of instances of the task to spawn. all instances
/// share the same state/stack. multi-instanced tasks can not be re-queued for
/// execution. and must return false in their body (unchecked).
/// @note Cancelation is handled within the task itself, as various tasks have
/// various methods/techniques of reacting to cancelation.
struct [[nodiscard]] TaskInfo
{
  typedef Fn<void(void *)> Init;

  typedef void (*Uninit)(void *);

  typedef bool (*Poll)(void *);

  typedef bool (*Run)(void *);

  Layout frame_layout{};

  Init init = noop;

  Uninit uninit = noop;

  Poll poll = [](void *) { return true; };

  Run run = [](void *) { return false; };

  u64 instances = 1;
};

enum class TaskTarget
{
  Worker    = 0,
  Main      = 1,
  Dedicated = 2
};

/// @brief Describes how to schedule the task onto the executor
/// @param target the target execution unit
/// @param thread the thread on the execution unit. U32_MAX means any available
/// thread. This is ignored when target is main thread.
struct TaskSchedule
{
  TaskTarget target = TaskTarget::Worker;
  u32        thread = U32_MAX;
};

/// @brief Wrap a Task frame
/// @return TaskInfo struct to be passed to the scheduler for execution
template <TaskFrame F>
TaskInfo to_task_info(F &frame, u64 instances)
{
  Fn init =
      fn(&frame, [](F *frame, void *mem) { new (mem) F{(F &&) (*frame)}; });

  TaskInfo::Uninit uninit = [](void *f) {
    F *frame = (F *) f;
    frame->~F();
  };

  TaskInfo::Poll poll = [](void *f) -> bool {
    F *frame = (F *) f;
    return frame->poll();
  };

  TaskInfo::Run run = [](void *f) -> bool {
    F *frame = (F *) f;
    return frame->run();
  };

  return TaskInfo{.frame_layout = layout<F>,
                  .init         = init,
                  .uninit       = uninit,
                  .poll         = poll,
                  .run          = run,
                  .instances    = instances};
}

/// @brief Static Thread Pool Scheduler.
///
/// all tasks execute out-of-order.
///
/// it has 2 types of threads: worker threads and dedicated threads.
///
/// dedicated threads are for processing latency-sensitive tasks that need to
/// happen within a deadline, i.e. audio, video. they can spin, sleep, preempt
/// and/or wait for tasks.
///
/// worker threads process any type of tasks, although might not be as
/// responsive as dedicated threads due to their over-susbscription model.
///
///
/// @note work submitted to the main thread MUST be extremely light-weight and
/// non-blocking.
///
struct Scheduler
{
  /// @brief Initialize the scheduler.
  /// @note not thread-safe, typically called at program startup or DLL
  /// loading-time.
  /// @param allocator thread-safe allocator to allocate tasks from, must be
  /// able to allocate page-sized allocations
  /// @param dedicated_thread_sleep max sleep time for the dedicated threads.
  /// enables responsiveness. `.size()` represents the number of dedicated
  /// threads to create.
  /// @param worker_thread_sleep maximum sleep time for the worker threads.
  /// enables responsiveness. `.size()` represents the number of worker threads
  /// to create.
  static void init(AllocatorImpl allocator, std::thread::id main_thread_id,
                   Span<nanoseconds const> dedicated_thread_sleep,
                   Span<nanoseconds const> worker_thread_sleep);

  static void uninit();

  /// @brief Destroys the scheduler. The scheduler must have been joined.
  virtual ~Scheduler() = default;

  /// @brief Request that the threads stop executing and purges the tasks on the
  /// task queue.
  virtual void join() = 0;

  virtual u32 num_dedicated() = 0;

  virtual u32 num_workers() = 0;

  /// @brief Schedule task to a specific dedicated thread
  /// @param info Task frame information
  /// @param thread the index of the dedicated thread to schedule to
  virtual void schedule_dedicated(TaskInfo const &info, u32 thread) = 0;

  /// @brief Schedule task to a worker thread
  /// @param info Task frame information
  virtual void schedule_worker(TaskInfo const &info) = 0;

  /// @brief Schedule task to the main thread. The tasks are executed once the
  /// main thread loop runs.
  /// @param info Task frame information
  virtual void schedule_main(TaskInfo const &info) = 0;

  /// @brief Execute work on the main thread queue
  /// @param grace_period minimum time (within duration) to wait for tasks when
  /// the task queue is empty
  /// @param duration maximum timeout to spend executing tasks
  virtual void execute_main_thread_loop(nanoseconds grace_period,
                                        nanoseconds duration) = 0;

  template <TaskFrame F>
  void schedule(F &&task, TaskSchedule schedule, u64 instances)
  {
    TaskInfo info = to_task_info(task, instances);

    switch (schedule.target)
    {
      case TaskTarget::Worker:
        schedule_worker(info);
        return;
      case TaskTarget::Dedicated:
        schedule_dedicated(info, schedule.thread);
        return;
      case TaskTarget::Main:
        schedule_main(info);
        return;
      default:
        UNREACHABLE();
    }
  }
};

/// @brief Global scheduler object. Designed for hooking across DLLs. Must be
/// initialized with `Scheduler::init()` and uninitialized with
/// `Scheduler::uninit()`.
ASH_C_LINKAGE ASH_DLL_EXPORT Scheduler *scheduler;

template <typename T>
struct [[nodiscard]] Stream
{
  typedef T Type;

  Rc<T *> data_;

  Semaphore semaphore_;

  Stream(Rc<T *> data, Semaphore semaphore) :
      data_{std::move(data)}, semaphore_{std::move(semaphore)}
  {
  }

  Stream alias() const
  {
    return Stream{data_.alias(), semaphore_.alias()};
  }

  [[nodiscard]] bool is_ready(u64 stage) const
  {
    return semaphore_->get_stage() > stage;
  }

  [[nodiscard]] bool is_complete() const
  {
    return semaphore_->is_completed();
  }

  template <Callable<T &> Fn>
  void yield(Fn &&operation, u64 increment = 1) const
  {
    operation(*data_.get());
    semaphore_->increment(increment);
  }
};

template <>
struct [[nodiscard]] Stream<void>
{
  typedef void Type;

  Semaphore semaphore_;

  Stream(Semaphore semaphore) : semaphore_{std::move(semaphore)}
  {
  }

  Stream alias() const
  {
    return Stream{semaphore_.alias()};
  }

  [[nodiscard]] bool is_ready(u64 stage) const
  {
    return semaphore_->get_stage() > stage;
  }

  [[nodiscard]] bool is_complete() const
  {
    return semaphore_->is_completed();
  }

  template <Callable Fn>
  void yield(Fn &&operation, u64 increment = 1) const
  {
    ((Fn &&) operation)();
    semaphore_->increment(increment);
  }
};

template <typename T>
using SyncStream = Stream<Sync<T>>;

template <typename... T>
[[nodiscard]] bool await_streams(nanoseconds timeout,
                                 Stream<T> const &...streams,
                                 Span<u64 const> stages)
{
  SemaphoreState const *semaphores[] = {(streams.semaphore_.get())...};

  return await_semaphores(span(semaphores), stages, timeout);
}

namespace async
{

template <typename P>
concept Poll = requires(P p) {
  { p() && true };
};

template <typename R>
concept Run = requires(R r) {
  { r() && true };
};

template <Run R, Poll P>
struct TaskBody
{
  typedef P Poller;
  typedef R Runner;

  R run{};
  P poll{};
};

struct TaskInstance
{
  u64 instances = 1;
  u64 idx       = 0;
};

template <typename... T>
struct [[nodiscard]] AwaitStreams
{
  Tuple<Stream<T>...> streams{};

  Array<u64, sizeof...(T)> stages{};

  explicit AwaitStreams(Tuple<Stream<T>...>             streams,
                        Array<u64, sizeof...(T)> const &stages = {}) :
      streams{std::move(streams)}, stages{stages}
  {
  }

  bool operator()() const
  {
    return apply(
        [this](auto const &...s) {
          return await_streams(nanoseconds{0}, s..., span(this->stages));
        },
        streams);
  }
};

struct [[nodiscard]] Delay
{
  steady_clock::time_point from{};

  nanoseconds delay = 0ns;

  constexpr bool operator()() const
  {
    if (delay == 0ns)
    {
      return true;
    }
    auto const past = steady_clock::now() - from;
    return past >= delay;
  }
};

struct [[nodiscard]] Ready
{
  constexpr bool operator()() const
  {
    return true;
  }
};

/// @brief Launch a one-shot task
/// @tparam F Task Functor type
/// @tparam P Poller Functor type
/// @param fn Task functor
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the task
template <Callable F, Poll P = Ready>
void once(F fn, P poll = {}, TaskSchedule schedule = {})
{
  TaskBody body{[fn = std::move(fn)]() mutable -> bool {
                  fn();
                  return false;
                },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule, 1);
}

template <Callable F, Callable... F1, Poll P = Ready>
void once(Tuple<F, F1...> fns, P poll = {}, TaskSchedule schedule = {})
{
  TaskBody body{[fns = std::move(fns)]() mutable -> bool {
                  ::ash::fold(fns);
                  return false;
                },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule, 1);
}

/// @brief Launch a task that is repeatedly called until it is done
/// @tparam F Task Functor type
/// @tparam P Poller Functor type
/// @param fn Task functor that returns false when it is done
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the task
template <Callable F, Poll P = Ready>
  requires(Convertible<CallResult<F>, bool>)
void loop(F fn, P poll = {}, TaskSchedule schedule = {})
{
  TaskBody body{[fn = std::move(fn)]() mutable -> bool { return fn(); },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule, 1);
}

/// @brief Launch a task that is repeatedly called n times
/// @tparam F Functor type
/// @tparam P Poller Functor type
/// @param fn Task functor to be called, can terminate early by returning a
/// boolean
/// @param n Number of times to execute the task
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the task
template <Callable<u64> F, Poll P = Ready>
  requires(Same<CallResult<F, u64>, void> ||
           Convertible<CallResult<F, u64>, bool>)
void repeat(F fn, u64 n, P poll = {}, TaskSchedule schedule = {})
{
  if (n == 0)
  {
    return;
  }

  TaskBody body{[fn = std::move(fn), n, i = (u64) 0]() mutable -> bool {
                  if constexpr (Same<CallResult<F, u64>, void>)
                  {
                    fn(i);
                    i++;
                    return n == i;
                  }
                  else
                  {
                    bool const done = fn(i);
                    i++;
                    return done || (n == i);
                  }
                },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule, 1);
}

/// @brief Launch shards of tasks, All shards share the same state and task
/// frame and run concurrently. Typically used for SPMD
/// (https://en.wikipedia.org/wiki/Single_program,_multiple_data)
/// @tparam F Shard functor type
/// @tparam P Poller Functor type
/// @param fn Shard body
/// @param n Number of shard instances of the task to launch
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the shards
template <Callable<TaskInstance> F, Poll P = Ready>
  requires(Same<CallResult<F, TaskInstance>, void> ||
           Convertible<CallResult<F, TaskInstance>, bool>)
void shard(F fn, u64 n, P poll = {}, TaskSchedule schedule = {})
{
  if (n == 0)
  {
    return;
  }

  TaskBody shard_body{
      [fn = std::move(fn), next_id = (u64) 0, n]() mutable -> bool {
        std::atomic_ref next_id_ref{next_id};
        u64 const id = next_id_ref.fetch_add(1, std::memory_order_relaxed);

        if constexpr (Same<CallResult<F, TaskInstance>, void>)
        {
          fn(TaskInstance{.instances = n, .idx = id});
          return false;
        }
        else
        {
          return fn(TaskInstance{.instances = n, .idx = id});
        }
      },
      Ready{}};

  // we need to first dispatch a task that will poll for readiness, and once the
  // shard is ready for dispatch we dispatch the shards. we can avoid this
  // intermediate process if we know the task is immediately available (Ready
  // type) but that's really not a good idea for a generic type. we also need
  // the dispatch as we don't expect the polling function to be thread-safe when
  // called across all instances.

  TaskBody body{
      [shard_body = std::move(shard_body), schedule, n]() mutable -> bool {
        logger->info("scheduled shard");
        scheduler->schedule(std::move(shard_body), schedule, n);
        return false;
      },
      std::move(poll)};

  scheduler->schedule(std::move(body), schedule, 1);
}

};        // namespace async

}        // namespace ash
