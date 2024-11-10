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
  bool flag_ = false;

  void lock()
  {
    bool            expected = false;
    bool            target   = true;
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
    bool            expected = false;
    bool            target   = true;
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
  L *lock_ = nullptr;

  explicit LockGuard(L &lock) : lock_{&lock}
  {
    lock_->lock();
  }

  LockGuard(LockGuard const &)            = delete;
  LockGuard &operator=(LockGuard const &) = delete;
  LockGuard(LockGuard &&)                 = delete;
  LockGuard &operator=(LockGuard &&)      = delete;

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

struct ReadLock
{
  ReadWriteLock *lock_ = nullptr;

  explicit ReadLock(ReadWriteLock &rwlock) : lock_{&rwlock}
  {
  }

  void lock()
  {
    lock_->lock_read();
  }

  void unlock()
  {
    lock_->unlock_read();
  }
};

struct WriteLock
{
  ReadWriteLock *lock_ = nullptr;

  explicit WriteLock(ReadWriteLock &rwlock) : lock_{&rwlock}
  {
  }

  void lock()
  {
    lock_->lock_write();
  }

  void unlock()
  {
    lock_->unlock_write();
  }
};

/// @brief A Stop Sequence Token
struct StopToken
{
  /// @brief stage to stop execution before.
  /// this means the stage represented by `stop_before_` and all proceeding
  /// stages are canceled.
  u64 stop_point_ = U64_MAX;

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

/// @brief A CPU Timeline Semaphore used for synchronization in multi-stage
/// cooperative multitasking jobs. Unlike typical Binary/Counting Semaphores, A
/// timeline semaphores a monotonic counter representing the stages of an
/// operation.
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
struct Semaphore : Pin<>
{
  struct Inner
  {
    u64 num_stages = 1;
    u64 stage      = 0;
  } inner = {};

  explicit constexpr Semaphore(Inner in) : inner{in}
  {
  }

  explicit constexpr Semaphore(u64 num_stages = 1) :
      inner{.num_stages = num_stages}
  {
  }

  void uninit()
  {
    // no-op
  }

  void reset()
  {
    inner = Inner{};
  }

  /// @brief Get the current semaphore stage. This represents the current stage
  /// being worked on.
  /// @param sem non-null
  /// @return
  [[nodiscard]] u64 get_stage() const
  {
    std::atomic_ref stage{inner.stage};
    return stage.load(std::memory_order_acquire);
  }

  /// @brief Get the number of stages in the semaphore
  /// @param sem non-null
  /// @return
  [[nodiscard]] u64 get_num_stages() const
  {
    return inner.num_stages;
  }

  /// @brief
  /// @param sem non-null
  /// @return
  [[nodiscard]] bool is_completed() const
  {
    std::atomic_ref stage{inner.stage};
    return stage.load(std::memory_order_acquire) == inner.num_stages;
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
    next                    = min(next, inner.num_stages);
    u64             current = 0;
    std::atomic_ref stage{inner.stage};
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
    inc                     = min(inc, inner.num_stages);
    u64             current = 0;
    u64             target  = inc;
    std::atomic_ref stage{inner.stage};
    while (!stage.compare_exchange_strong(
        current, target, std::memory_order_release, std::memory_order_relaxed))
        [[unlikely]]
    {
      target = min(sat_add(current, inc), inner.num_stages);
    }
  }
};

///
/// @brief Create an independently allocated semaphore object
///
/// @param num_stages: number of stages represented by this semaphore
/// @return Semaphore
///
[[nodiscard]] Rc<Semaphore *> create_semaphore(u64           num_stages,
                                               AllocatorImpl allocator)
{
  return rc_inplace<Semaphore>(allocator, num_stages).unwrap();
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
[[nodiscard]] bool await_semaphores(Span<Semaphore const *const> sems,
                                    Span<u64 const> stages, nanoseconds timeout)
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
    for (; next < n; next++)
    {
      Semaphore const *const &s = sems[next];
      u64 const  stage          = min(stages[next], s->inner.num_stages - 1);
      bool const is_ready       = stage <= s->get_stage();

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

      nanoseconds const past =
          duration_cast<nanoseconds>(steady_clock::now() - poll_begin);

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

struct TaskInstance
{
  u64 instances = 1;
  u64 idx       = 0;
};

constexpr usize MAX_TASK_FRAME_SIZE = PAGE_SIZE >> 4;

template <typename F>
concept TaskFrame = requires(F f, TaskInstance instance) {
  { !f.poll() };
  { !f.run(instance) };
} && (sizeof(F) <= MAX_TASK_FRAME_SIZE);

/// @brief Task data layout and callbacks
/// @param ctx memory layout of context data associated with the task.
/// @param init function to initialize the context data associated with the task
/// to a task stack.
/// @param uninit function to uninitialize the task and the associated data
/// context upon completion of the task.
/// @param poll function to poll for readiness of the task, must be extremely
/// light-weight and non-blocking.
/// @param run task to be executed on the executor. must return true if it
/// should be re-queued onto the executor (with the same parameters as it got
/// in).
/// @param instances number of instances of the task to spawn. all instances
/// share the same state/stack.
/// @note Cancelation is handled within the task itself, as various tasks have
/// various methods/techniques of reacting to cancelation.
struct TaskInfo
{
  typedef Fn<void(void *)> Init;
  typedef void (*Uninit)(void *);
  typedef bool (*Poll)(void *);
  typedef bool (*Run)(void *, TaskInstance);

  Layout frame_layout{};

  Init init = noop;

  Uninit uninit = noop;

  Poll poll = [](void *) { return true; };

  Run run = [](void *, TaskInstance) { return true; };

  u64 instances = 1;
};

enum class TaskTarget
{
  Worker    = 0,
  Main      = 1,
  Dedicated = 2
};

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
  Fn init = fn(&frame, [](F *f, void *mem) { new (mem) F{(F &&) (*f)}; });

  TaskInfo::Uninit uninit = [](void *frame) {
    F *f = (F *) frame;
    f->~F();
  };

  TaskInfo::Poll poll = [](void *frame) -> bool {
    F *f = (F *) frame;
    return f->poll();
  };

  TaskInfo::Run run = [](void *frame, TaskInstance instance) -> bool {
    F *f = (F *) frame;
    return f->run(instance);
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
/// responsive as dedicated threads.
///
///
/// @note work submitted to the main thread MUST be extremely light-weight and
/// non-blocking.
///
struct Scheduler
{
  // [ ] shutdown is performed immediately as we can't guarantee when tasks will
  // complete. send a purge signal to the threads so they purge their task queue
  // before returning.
  virtual void init(Span<nanoseconds const> dedicated_thread_sleep,
                    Span<nanoseconds const> worker_thread_sleep) = 0;

  virtual void uninit() = 0;

  virtual ~Scheduler() = 0;

  virtual u32 num_dedicated() = 0;

  virtual u32 num_workers() = 0;

  // [ ] U32_MAX on dedicated should mean any dedicated thread
  virtual void schedule_dedicated(u32 thread, TaskInfo const &info) = 0;

  virtual void schedule_worker(TaskInfo const &info) = 0;

  virtual void schedule_main(TaskInfo const &info) = 0;

  virtual void execute_main_thread_work(nanoseconds timeout) = 0;

  void schedule(TaskFrame auto &&task, TaskSchedule schedule, u64 instances)
  {
    TaskInfo info = to_task_info(task, instances);

    switch (schedule.target)
    {
      case TaskTarget::Worker:
        schedule_worker(info);
        return;
      case TaskTarget::Dedicated:
        schedule_dedicated(schedule.thread, info);
        return;
      case TaskTarget::Main:
        schedule_main(info);
        return;
      default:
        UNREACHABLE();
        return;
    }
  }
};

ASH_C_LINKAGE ASH_DLL_EXPORT Scheduler *scheduler;

// create future, add lambda to
// TODO: unreal engine verse async semantics
//
//
// concept cancelable: can take cancel token as arg
//
//
// this functions should be able to take futures as arguments? so we can await
// tasks outside of this scope
//
//
//
// model timeflow instead of logic
//
// cancelation tag, with each stage taking a cancelation flag or number
//
namespace async
{

// not exactly a future! it's just a state the op outputs to
template <typename T>
struct [[nodiscard]] Future
{
  typedef T Type;

  // [ ] we need to be careful with destruction semantics here, do we intialize
  // anyway?
  Rc<T *>         result{};
  Rc<Semaphore *> semaphore{};
  u64             stage = 0;

  Future alias() const
  {
    return Future{.result    = result.alias(),
                  .semaphore = semaphore.alias(),
                  .stage     = stage};
  }

  [[nodiscard]] bool is_ready() const
  {
    return semaphore->get_stage() > stage;
  }

  Result<Void, Void> complete(T &&);
};

template <>
struct [[nodiscard]] Future<void>
{
  typedef void ResultType;

  Rc<Semaphore *> semaphore{};
  u64             stage = 0;

  Future alias() const
  {
    return Future{.semaphore = semaphore.alias(), .stage = stage};
  }

  [[nodiscard]] bool is_ready() const
  {
    return semaphore->get_stage() > stage;
  }

  Result<Void, Void> complete();

  Result<Void, Void> complete(Void)
  {
    return complete();
  }
};

template <typename... T>
bool await_futures(nanoseconds timeout, Future<T> const &...futures)
{
  Rc<Semaphore *> const semaphores[] = {(futures.semaphore)...};

  u64 const stages[] = {(futures.stage)...};

  return await_semaphores(span(semaphores), span(stages), timeout);
}

struct StagedStopToken;

struct StopToken
{
  Rc<::ash::StopToken *> stop_token{};

  bool is_stop_requested(u64 stage) const
  {
    return stop_token->is_stop_requested(stage);
  }

  void request_stop(u64 stage) const
  {
    stop_token->request_stop(stage);
  }

  StagedStopToken stage(u64 stage) const;
};

struct StagedStopToken
{
  StopToken stop_token{};
  u64       stage = 0;

  bool is_stop_requested() const
  {
    return stop_token.is_stop_requested(stage);
  }

  void request_stop() const
  {
    stop_token.request_stop(stage);
  }
};

StagedStopToken StopToken::stage(u64 stage) const
{
  return StagedStopToken{StopToken{stop_token.alias()}, stage};
}

template <typename T>
struct StoppableFuture
{
  Future<T>       future{};
  StagedStopToken stop_token{};
};

template <typename P>
concept Poll = requires(P p) {
  { !p() };
};

template <typename R>
concept Run = requires(R r) {
  { !r() };
};

template <typename R>
concept InstanceRun = requires(R r, TaskInstance instance) {
  { !r(instance) };
};

template <Run R, Poll P>
struct TaskBody
{
  typedef R Runner;
  typedef P Poller;

  R run_{};
  P poll{};

  constexpr bool run(TaskInstance)
  {
    return run_();
  }
};

template <InstanceRun R, Poll P>
struct InstanceTaskBody
{
  typedef R Runner;
  typedef P Poller;

  R run{};
  P poll{};
};

template <typename... Futures>
struct AwaitAny
{
  Tuple<Futures...> futures;

  bool operator()() const
  {
    return apply(
        [](auto const &...f) {
          return (false || ... || await_futures(nanoseconds{0}, f));
        },
        futures);
  }
};

template <typename... Futures>
struct AwaitAll
{
  Tuple<Futures...> futures;

  bool operator()() const
  {
    return apply(
        [](auto const &...f) { return await_futures(nanoseconds{0}, f...); },
        futures);
  }
};

struct Delay
{
  steady_clock::time_point start{};
  nanoseconds              delay = 0ns;

  constexpr bool operator()() const
  {
    return (delay == 0ns) ||
           (duration_cast<nanoseconds>(steady_clock::now() - start) >= delay);
  }
};

struct Ready
{
  constexpr bool operator()() const
  {
    return true;
  }
};

struct StoppableTag
{
};

constexpr StoppableTag stoppable;

template <Callable F, Poll P>
Future<CallResult<F>> once(F &&fn, P &&poll = Ready{},
                           TaskSchedule  schedule  = {},
                           AllocatorImpl allocator = default_allocator);

template <Callable<StagedStopToken const &> F, Poll P>
StoppableFuture<CallResult<F, StagedStopToken const &>>
    once(StoppableTag, F &&fn, P &&poll = Ready{}, TaskSchedule schedule = {},
         AllocatorImpl allocator = default_allocator);

template <Callable<TaskInstance> F, Poll P>
Future<void> loop(F &&fn, P &&poll = Ready{}, TaskSchedule schedule = {},
                  AllocatorImpl allocator = default_allocator);

template <Callable<StagedStopToken const &, TaskInstance> F, Poll P>
StoppableFuture<void> loop(StoppableTag, F &&fn, P &&poll = Ready{},
                           TaskSchedule  schedule  = {},
                           AllocatorImpl allocator = default_allocator);

template <Callable<TaskInstance> F, Poll P>
Future<void> loop(F &&fn, u64 count, P &&poll = Ready{},
                  TaskSchedule  schedule  = {},
                  AllocatorImpl allocator = default_allocator);

template <Callable<StagedStopToken const &, TaskInstance> F, Poll P>
StoppableFuture<void> loop(StoppableTag, F &&fn, u64 count, P &&poll = Ready{},
                           TaskSchedule  schedule  = {},
                           AllocatorImpl allocator = default_allocator);

template <Callable<TaskInstance> F, Poll P>
Future<void> bulk(F &&fn, u64 count, P &&poll = Ready{},
                  TaskSchedule  schedule  = {},
                  AllocatorImpl allocator = default_allocator);

template <Callable<StagedStopToken const &&, TaskInstance> F, Poll P>
StoppableFuture<void> bulk(StoppableTag, F &&fn, u64 count, P &&poll = Ready{},
                           TaskSchedule  schedule  = {},
                           AllocatorImpl allocator = default_allocator);

// block_result; future? Future, Callable? CallResult<F>
//
//
// if one task takes a cancelation token, all tasks must take it,
// and a cancelation token would need to be bundled with the future.

/// @brief execute tasks one by one with only one executing at a time.
/// result of one task will be passed to the other.
/// the last result in the sequence is returned.
///
/// each task can return false to request cancelation of the preceding tasks???
/// set a value.
///
/// BlockContext{ dedicated, worker, main?; cancelable?; interruptable;  };
///
// template <SyncPoint... Func>
// void seq(Context &&context, Func &&...func);

// template <SyncPoint... Func>
// StoppableFuture<void> seq(StoppableTag, Func &&...func)
// {
// }

/// @brief execute all tasks at once but wait until all are done before
/// returning result.
/// returns a tuple of the combined results.
///
///

/// generator function that continuously yields data

// map-reduce, fan-in fan-out: possible with bulk

/// @brief execute all tasks at once but only return result for the first
/// finished task. If a cancelation token is passed as argument the tasks
/// returns whichever result finished first.
///
///
/// struct RaceContext{};
///
// template <SyncPoint... Func>
// Union<> race(Func &&...func);

};        // namespace async

}        // namespace ash
