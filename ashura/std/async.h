/// SPDX-License-Identifier: MIT
///
/// Stage-based Asynchrony
///
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/dyn.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/option.h"
#include "ashura/std/rc.h"
#include "ashura/std/result.h"
#include "ashura/std/time.h"
#include "ashura/std/tuple.h"
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
    while (!flag.compare_exchange_weak(
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
    flag.compare_exchange_weak(expected, target, std::memory_order_acquire,
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
  L * lock_;

  explicit LockGuard(L & lock) : lock_{&lock}
  {
    lock_->lock();
  }

  LockGuard(LockGuard const &) = delete;

  LockGuard & operator=(LockGuard const &) = delete;

  LockGuard(LockGuard &&) = delete;

  LockGuard & operator=(LockGuard &&) = delete;

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
  ReadWriteLock * lock_;

  explicit ReadGuard(ReadWriteLock & lock) : lock_{&lock}
  {
    lock_->lock_read();
  }

  ReadGuard(ReadGuard const &) = delete;

  ReadGuard & operator=(ReadGuard const &) = delete;

  ReadGuard(ReadGuard &&) = delete;

  ReadGuard & operator=(ReadGuard &&) = delete;

  ~ReadGuard()
  {
    lock_->unlock_read();
  }
};

struct WriteGuard
{
  ReadWriteLock * lock_;

  explicit WriteGuard(ReadWriteLock & lock) : lock_{&lock}
  {
    lock_->lock_write();
  }

  WriteGuard(WriteGuard const &) = delete;

  WriteGuard & operator=(WriteGuard const &) = delete;

  WriteGuard(WriteGuard &&) = delete;

  WriteGuard & operator=(WriteGuard &&) = delete;

  ~WriteGuard()
  {
    lock_->unlock_write();
  }
};

enum class FutureStage : u64
{
  Pending  = 0,
  Yielding = U64_MAX - 1,
  Yielded  = U64_MAX
};

/// @brief An atomically initialized value, can only be initialized once.
/// Multiple threads can attempt to initialize the value but only one thread
/// will be successful. This means we don't need to use locks to guard the
/// object.
template <typename T>
struct AtomicInit
{
  FutureStage stage_;

  union
  {
    T v_;
  };

  constexpr AtomicInit() : stage_{FutureStage::Pending}
  {
  }

  template <typename... Args>
  constexpr AtomicInit(V<0>, Args &&... args) :
    stage_{FutureStage::Yielded},
    v_{static_cast<Args &&>(args)...}
  {
  }

  constexpr AtomicInit(AtomicInit const &)             = delete;
  constexpr AtomicInit(AtomicInit &&)                  = delete;
  constexpr AtomicInit & operator=(AtomicInit const &) = delete;
  constexpr AtomicInit & operator=(AtomicInit &&)      = delete;

  ~AtomicInit()
  {
    // this is the last reference to the object at this point, but we still need
    // to acquire write side effects from other writers (if any)
    std::atomic_ref stage{stage_};

    if (stage.load(std::memory_order_acquire) == FutureStage::Yielded)
      [[likely]]
    {
      v_.~T();
    }
  }

  template <typename... Args>
  [[nodiscard]] bool init(Args &&... args)
  {
    std::atomic_ref stage{stage_};
    FutureStage     expected = FutureStage::Pending;
    FutureStage     target   = FutureStage::Yielding;

    /// no side-effects need to be observed
    if (!stage.compare_exchange_strong(expected, target,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed))
    {
      return false;
    }

    new (&v_) T{static_cast<Args &&>(args)...};

    stage.store(FutureStage::Yielded, std::memory_order_release);
    return true;
  }

  /// @brief Get the wrapped value
  /// @return none if value is not initialized yet
  Option<T &> ref()
  {
    std::atomic_ref stage{stage_};
    if (stage.load(std::memory_order_acquire) != FutureStage::Yielded)
    {
      return none;
    }

    return v_;
  }
};

template <typename T>
struct [[nodiscard]] Sync
{
  T data_;

  ReadWriteLock lock_;

  template <typename... Args>
  constexpr Sync(Args &&... args) :
    data_{static_cast<Args &&>(args)...},
    lock_{}
  {
  }

  constexpr Sync(Sync const &) = delete;

  constexpr Sync(Sync &&) = delete;

  constexpr Sync & operator=(Sync const &) = delete;

  constexpr Sync & operator=(Sync &&) = delete;

  constexpr ~Sync() = default;

  template <Callable<T &> Op>
  void read(Op && op)
  {
    ReadGuard guard{lock_};
    static_cast<Op &&>(op)(data_);
  }

  template <Callable<T &> Op>
  void write(Op && op)
  {
    WriteGuard guard{lock_};
    static_cast<Op &&>(op)(data_);
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
    num_stages_{num_stages},
    stage_{0}
  {
  }

  /// @brief Get the current semaphore stage. This represents the current stage
  /// being worked on.
  /// @param sem non-null
  /// @return
  [[nodiscard]] u64 stage()
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire);
  }

  /// @brief Get the number of stages in the semaphore
  /// @param sem non-null
  /// @return
  [[nodiscard]] constexpr u64 num_stages() const
  {
    return num_stages_;
  }

  /// @brief returns true if the semaphore has been completed. i.e. reached its
  /// last declared stage.
  /// @param sem non-null
  /// @return
  [[nodiscard]] bool is_completed()
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire) == num_stages_;
  }

  /// @brief returns true if the polled stage is completed.
  [[nodiscard]] bool is_ready(u64 poll_stage)
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire) > poll_stage;
  }

  /// @brief Signal the semaphore to move to stage `next`. This implies a
  /// sequence ordering of the semaphore stages.
  ///
  /// @param next stage of the semaphore to move to. stage >=
  /// num_stages or U64_MAX means completion of the last stage of the operation.
  /// must be monotonically increasing for each call.
  ///
  /// @returns returns true if the signaled stage has not been passed yet
  /// (successful). otherwise returns false (failed).
  ///
  [[nodiscard]] bool signal(u64 next)
  {
    next                    = min(next, num_stages_);
    u64             current = 0;
    std::atomic_ref stage{stage_};
    while (!stage.compare_exchange_weak(
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
    while (!stage.compare_exchange_weak(
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
  bool is_stop_requested(u64 stage = 0)
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
    while (!stop_point.compare_exchange_weak(
      expected, target, std::memory_order_release, std::memory_order_relaxed))
      [[unlikely]]
    {
      target = min(expected, stage);
    }
  }
};

///
/// @brief Create an independently allocated semaphore object
///
/// @param num_stages: number of stages represented by this semaphore
/// @return Semaphore
///
inline Result<Semaphore> semaphore(AllocatorRef allocator, u64 num_stages)
{
  return rc<SemaphoreState>(inplace, allocator, num_stages);
}

typedef Rc<StopTokenState *> StopToken;

inline Result<StopToken> stop_token(AllocatorRef allocator)
{
  return rc<StopTokenState>(inplace, allocator);
}

namespace impl
{

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
template <typename Semaphore, typename Stage, typename SemaphoreKey,
          typename StageKey>
[[nodiscard]] bool await_semaphores(Span<Semaphore> semaphores,
                                    Span<Stage> stages, nanoseconds timeout,
                                    SemaphoreKey && semaphore_key = {},
                                    StageKey &&     stage_key     = {})
{
  CHECK(semaphores.size() == stages.size(), "");
  usize const n = semaphores.size();
  for (usize i = 0; i < n; i++)
  {
    SemaphoreState & semaphore = semaphore_key(semaphores[i]);
    u64 const        stage     = stage_key(stages[i]);
    CHECK((stage == U64_MAX) || (stage <= semaphore.num_stages()), "");
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
      SemaphoreState & semaphore = semaphore_key(semaphores[next]);
      u64 const        stage =
        min(stage_key(stages[next]), semaphore.num_stages() - 1);
      bool const is_ready = stage < semaphore.stage();

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

template <typename Future, typename FutureStageKey>
[[nodiscard]] bool await_futures(Span<Future> futures, nanoseconds timeout,
                                 FutureStageKey && stage_key = {})
{
  usize const n = futures.size();

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
      FutureStage &   stage = stage_key(futures[next]);
      std::atomic_ref stage_ref{stage};
      bool const      is_ready =
        stage_ref.load(std::memory_order_acquire) == FutureStage::Yielded;

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

}    // namespace impl

/// @brief A Stream is a continously mutated value yielding side-effects to
/// consumers. The side-effects are sequenced by the timeline semaphore. There
/// is no locking on the `Stream`, if you need locking consider using
/// `Stream<Sync<T>>`.
///
/// A Stream can yield results in a sequenced or unsequenced manner.
///
template <typename T>
struct [[nodiscard]] Stream
{
  typedef T Type;

  Rc<T *> data_;

  Semaphore semaphore_;

  Stream(Rc<T *> data, Semaphore semaphore) :
    data_{static_cast<Rc<T *> &&>(data)},
    semaphore_{static_cast<Semaphore &&>(semaphore)}
  {
  }

  Stream alias() const
  {
    return Stream{data_.alias(), semaphore_.alias()};
  }

  [[nodiscard]] bool is_ready(u64 stage) const
  {
    return semaphore_->is_ready(stage);
  }

  [[nodiscard]] bool is_completed() const
  {
    return semaphore_->is_completed();
  }

  template <Callable<T &> F>
  void yield_unsequenced(F && op, u64 increment) const
  {
    static_cast<F &&>(op)(*data_.get());
    semaphore_->increment(increment);
  }

  template <Callable<T &> F>
  void yield_sequenced(F && op, u64 stage) const
  {
    static_cast<F &&>(op)(*data_.get());
    CHECK(semaphore_->signal(stage + 1),
          "`Stream` yielded with invalid sequencing");
  }
};

template <typename T, typename... Args>
Result<Stream<T>> stream(Inplace, AllocatorRef allocator, u64 num_stages,
                         Args &&... args)
{
  Result data = rc<T>(inplace, allocator, static_cast<Args &&>(args)...);
  if (!data)
  {
    return Err{};
  }

  Result sem = semaphore(allocator, num_stages);
  if (!sem)
  {
    return Err{};
  }

  return Ok{
    Stream<T>{static_cast<Rc<T *> &&>(data.value()),
              static_cast<Semaphore &&>(sem.v())}
  };
}

template <typename T>
Result<Stream<T>> stream(AllocatorRef allocator, u64 num_stages, T value)
{
  return stream<T>(inplace, allocator, num_stages, static_cast<T &&>(value));
}

struct [[nodiscard]] AnyStream
{
  Semaphore semaphore_;

  template <typename T>
  AnyStream(Stream<T> stream) :
    semaphore_{static_cast<Semaphore &&>(stream.semaphore_)}
  {
  }

  AnyStream(AnyStream const &)             = delete;
  AnyStream & operator=(AnyStream const &) = delete;
  AnyStream(AnyStream &&)                  = default;
  AnyStream & operator=(AnyStream &&)      = default;
  ~AnyStream()                             = default;
};

/// @brief A future is 1-stage Stream that produces a single value. The value is
/// left uninitialized until the future is completed.
template <typename T>
struct [[nodiscard]] Future
{
  typedef T Type;
  using State = Rc<AtomicInit<T> *>;

  State state_;

  Future(State state) : state_{static_cast<State &&>(state)}
  {
  }

  Future alias() const
  {
    return Future{state_.alias()};
  }

  T & get(SourceLocation loc = SourceLocation::current()) const
  {
    return state_->ref().unwrap(
      "Called `Future::get()` on a pending Future"_str, loc);
  }

  Result<ref<T>> poll() const
  {
    return state_->ref().match(
      [](T & v) -> Result<ref<T>> { return Ok{ref{v}}; },
      []() -> Result<ref<T>> { return Err{}; });
  }

  template <typename... Args>
  Result<> yield(Args &&... args) const
  {
    bool const yielded = state_->init(static_cast<Args &&>(args)...);

    if (!yielded)
    {
      return Err{};
    }

    return Ok{};
  }
};

template <typename T>
Result<Future<T>> future(AllocatorRef allocator)
{
  Result s = rc<AtomicInit<T>>(inplace, allocator);

  if (!s)
  {
    return Err{};
  }

  return Ok{Future<T>{static_cast<Rc<AtomicInit<T> *> &&>(s.value())}};
}

struct [[nodiscard]] AnyFuture
{
  using State = Rc<FutureStage *>;

  template <typename T>
  static State transmute(Future<T> future)
  {
    FutureStage * state = &future.state_->stage_;
    return ash::transmute(std::move(future.state_), state);
  }

  State state_;

  AnyFuture(State state) : state_{static_cast<State &&>(state)}
  {
  }

  template <typename T>
  AnyFuture(Future<T> future) :
    state_{transmute(static_cast<Future<T> &&>(future))}
  {
  }

  AnyFuture(AnyFuture const &)             = delete;
  AnyFuture & operator=(AnyFuture const &) = delete;
  AnyFuture(AnyFuture &&)                  = default;
  AnyFuture & operator=(AnyFuture &&)      = default;
  ~AnyFuture()                             = default;

  AnyFuture alias() const
  {
    return AnyFuture{state_.alias()};
  }

  Result<> poll() const
  {
    std::atomic_ref<FutureStage> state{*state_};
    if (state.load(std::memory_order_relaxed) != FutureStage::Yielded)
    {
      return Err{};
    }

    return Ok{};
  }
};

inline constexpr usize MAX_TASK_FRAME_SIZE = 2_KB;

template <typename F>
concept TaskFrame = requires (F f) {
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

  typedef bool (*Runner)(void *);

  Layout frame_layout{};

  Init init = noop;

  Uninit uninit = noop;

  Poll poll = [](void *) { return true; };

  Runner runner = [](void *) { return false; };
};

enum class TaskTarget
{
  Worker    = 0,
  Main      = 1,
  Dedicated = 2
};

/// @brief Describes how to schedule the task onto the executor
/// @param target the target execution unit
/// @param thread the thread on the execution unit. none means any available
/// thread. This is ignored when target is main thread.
struct TaskSchedule
{
  TaskTarget  target = TaskTarget::Worker;
  Option<u32> thread = none;
};

/// @brief Wrap a Task frame
/// @return TaskInfo struct to be passed to the scheduler for execution
template <TaskFrame F>
TaskInfo to_task_info(F & frame)
{
  Fn init{&frame, +[](F * frame, void * mem) {
            new (mem) F{static_cast<F &&>(*frame)};
          }};

  TaskInfo::Uninit uninit = [](void * f) {
    F * frame = reinterpret_cast<F *>(f);
    frame->~F();
  };

  TaskInfo::Poll poll = [](void * f) -> bool {
    F * frame = reinterpret_cast<F *>(f);
    return frame->poll();
  };

  TaskInfo::Runner runner = [](void * f) -> bool {
    F * frame = reinterpret_cast<F *>(f);
    return frame->run();
  };

  return TaskInfo{.frame_layout = layout_of<F>,
                  .init         = init,
                  .uninit       = uninit,
                  .poll         = poll,
                  .runner       = runner};
}

template <typename P>
concept Poll = requires (P p) {
  { p() && true };
};

template <typename R>
concept Runner = requires (R r) {
  { r() && true };
};

template <Poll P, Runner R>
struct TaskBody
{
  typedef P Poll;
  typedef R Runner;

  P poll{};
  R run{};
};

struct TaskInstance
{
  u64 n   = 1;
  u64 idx = 0;
};

inline bool await_streams(Span<AnyStream const> streams, Span<u64 const> stages,
                          nanoseconds timeout)
{
  return impl::await_semaphores(
    streams, stages, timeout,
    [](AnyStream const & s) -> SemaphoreState & { return *s.semaphore_; },
    [](u64 stage) -> u64 { return stage; });
}

inline bool await_futures(Span<AnyFuture const> futures, nanoseconds timeout)
{
  return impl::await_futures(
    futures, timeout,
    [](AnyFuture const & f) -> FutureStage & { return *f.state_; });
}

template <usize N>
struct [[nodiscard]] AwaitStreams
{
  AnyStream streams[N];
  u64       stages[N] = {};

  bool operator()() const
  {
    return await_streams(streams, stages, 0ns);
  }
};

template <usize N>
struct [[nodiscard]] AwaitFutures
{
  AnyFuture futures[N];

  bool operator()() const
  {
    return await_futures(futures, 0ns);
  }
};

template <typename... T>
AwaitFutures(T...) -> AwaitFutures<sizeof...(T)>;

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
  /// @brief Create a Scheduler
  /// @param allocator thread-safe allocator to allocate tasks from, must be
  /// able to allocate page-sized allocations
  /// @param dedicated_thread_sleep max sleep time for the dedicated threads.
  /// enables responsiveness. `.size()` represents the number of dedicated
  /// threads to create.
  /// @param worker_thread_sleep maximum sleep time for the worker threads.
  /// enables responsiveness. `.size()` represents the number of worker threads
  /// to create.
  static Dyn<Scheduler *> create(AllocatorRef            allocator,
                                 std::thread::id         main_thread_id,
                                 Span<nanoseconds const> dedicated_thread_sleep,
                                 Span<nanoseconds const> worker_thread_sleep);

  /// @brief Destroys the scheduler. The scheduler must have been joined.
  virtual ~Scheduler() = default;

  /// @brief Request that the threads stop executing and purges the tasks on the
  /// task queue.
  virtual void shutdown() = 0;

  virtual u32 num_dedicated() = 0;

  virtual u32 num_workers() = 0;

  /// @brief Schedule task to a specific dedicated thread
  /// @param info Task frame information
  /// @param thread the index of the dedicated thread to schedule to
  virtual void schedule_dedicated(TaskInfo const & info, u32 thread) = 0;

  /// @brief Schedule task to a worker thread
  /// @param info Task frame information
  virtual void schedule_worker(TaskInfo const & info) = 0;

  /// @brief Schedule task to the main thread. The tasks are executed once the
  /// main thread loop runs.
  /// @param info Task frame information
  virtual void schedule_main(TaskInfo const & info) = 0;

  /// @brief Execute work on the main thread queue
  /// @param grace_period minimum time (within duration) to wait for tasks when
  /// the task queue is empty
  /// @param duration maximum timeout to spend executing tasks
  virtual void run_main_loop(nanoseconds duration, nanoseconds poll_max) = 0;

  template <TaskFrame F>
  void schedule(F && task, TaskSchedule schedule)
  {
    TaskInfo info = to_task_info(task);

    switch (schedule.target)
    {
      case TaskTarget::Worker:
        schedule_worker(info);
        return;
      case TaskTarget::Dedicated:
        schedule_dedicated(
          info, schedule.thread.unwrap(
                  "Dedicated Thread ID not set when scheduling task"_str));
        return;
      case TaskTarget::Main:
        schedule_main(info);
        return;
      default:
        CHECK_UNREACHABLE();
    }
  }

  /// @brief Launch a one-shot task
  /// @tparam F Task Functor type
  /// @tparam P Poller Functor type
  /// @param fn Task functor
  /// @param poll Poller functor that returns true when ready
  /// @param schedule How to schedule the task
  template <Callable F, Poll P = Ready>
  void once(F fn, P poll = {}, TaskSchedule schedule = {})
  {
    this->schedule(TaskBody{static_cast<P &&>(poll),
                            [fn = static_cast<F &&>(fn)]() mutable -> bool {
                              fn();
                              return false;
                            }},
                   schedule);
  }

  template <Callable F, Callable... F1, Poll P = Ready>
  void once(Tuple<F, F1...> fns, P poll = {}, TaskSchedule schedule = {})
  {
    this->schedule(
      TaskBody{static_cast<P &&>(poll),
               [fns = static_cast<Tuple<F, F1...> &&>(fns)]() mutable -> bool {
                 ash::fold(fns);
                 return false;
               }},
      schedule);
  }

  /// @brief Launch a task that is repeatedly called until it is done
  /// @tparam F Task Functor type
  /// @tparam P Poller Functor type
  /// @param fn Task functor that returns false when it is done
  /// @param poll Poller functor that returns true when ready
  /// @param schedule How to schedule the task
  template <Callable F, Poll P = Ready>
  requires (Convertible<CallResult<F>, bool>)
  void loop(F fn, P poll = {}, TaskSchedule schedule = {})
  {
    this->schedule(
      TaskBody{static_cast<P &&>(poll),
               [fn = static_cast<F &&>(fn)]() mutable -> bool { return fn(); }},
      schedule);
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
  requires (Same<CallResult<F, u64>, void> ||
            Convertible<CallResult<F, u64>, bool>)
  void repeat(F fn, u64 n, P poll = {}, TaskSchedule schedule = {})
  {
    if (n == 0)
    {
      return;
    }

    this->schedule(
      TaskBody{static_cast<P &&>(poll),
               [fn = static_cast<F &&>(fn), n, i = (u64) 0]() mutable -> bool {
                 if constexpr (Same<CallResult<F, u64>, void>)
                 {
                   fn(i);
                   i++;
                   return n == i;
                 }
                 else
                 {
                   // early exit
                   bool const done = fn(i);
                   i++;
                   return done || (n == i);
                 }
               }},
      schedule);
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
  template <typename State, Poll P = Ready>
  void shard(Rc<State> state, Fn<void(TaskInstance, State)> fn, u64 n,
             P poll = {}, TaskSchedule schedule = {})
  {
    if (n == 0)
    {
      return;
    }

    // we need to first dispatch a task that will poll for readiness, and once the
    // shard is ready for dispatch we dispatch the shards. we can avoid this
    // intermediate process if we know the task is immediately available (Ready
    // type) but that's really not a good idea for a generic type. we also need
    // the dispatch as we don't expect the polling function to be thread-safe when
    // called across all instances.
    this->schedule(
      TaskBody{
        static_cast<P &&>(poll),
        [fn, state = std::move(state), schedule, n, this]() mutable -> bool {
          for (u64 i = 0; i < n; i++)
          {
            this->schedule(
              TaskBody{Ready{},
                       [fn, i, n, state = state.alias()]() mutable -> bool {
                         fn(TaskInstance{.n = n, .idx = i}, state.get());
                         return false;
                       }},
              schedule);
          }
          return false;
        }},
      schedule);
  }
};

extern Scheduler * scheduler;

/// @brief Global scheduler object. Designed for hooking across DLLs. Must be
/// initialized at program startup.
ASH_C_LINKAGE ASH_DLL_EXPORT void hook_scheduler(Scheduler *);

}    // namespace ash
