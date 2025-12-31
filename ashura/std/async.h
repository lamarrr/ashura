/// SPDX-License-Identifier: MIT
///
/// Stage-based Asynchrony
///
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/dyn.h"
#include "ashura/std/enum.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/option.h"
#include "ashura/std/rc.h"
#include "ashura/std/result.h"
#include "ashura/std/time.h"
#include "ashura/std/tuple.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

#include <atomic>
#include <thread>

#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
#  include <emmintrin.h>
#endif

namespace ash
{

// [ ] check runtime status in worker threads; to allow proper shutdown

typedef struct IWaitToken * WaitToken;

/// @brief A wait token for sleeping and waking up threads.
struct [[nodiscard]] IWaitToken
{
  u32 state__;

  constexpr IWaitToken(u32 state) : state__{state}
  {
  }

  [[nodiscard]] bool cmpxchg_weak(u32 & expected, u32 desired,
                                  std::memory_order success,
                                  std::memory_order failure)
  {
    std::atomic_ref state{state__};
    return state.compare_exchange_weak(expected, desired, success, failure);
  }

  [[nodiscard]] bool cmpxchg_strong(u32 & expected, u32 desired,
                                    std::memory_order success,
                                    std::memory_order failure)
  {
    std::atomic_ref state{state__};
    return state.compare_exchange_strong(expected, desired, success, failure);
  }

  void store(u32 state, std::memory_order order)
  {
    std::atomic_ref state_ref{state__};
    state_ref.store(state, order);
  }

  [[nodiscard]] u32 load(std::memory_order order)
  {
    std::atomic_ref state{state__};
    return state.load(order);
  }

  void os_notify();

  void os_wait(u32 old_state, std::memory_order order);
};

typedef struct IFutex * Futex;

struct [[nodiscard]] IFutex
{
  IWaitToken token_;

  IFutex() : token_{0U}
  {
  }

  void lock()
  {
    u32 expected = 0;
    u32 target   = 1;
    while (!token_.cmpxchg_weak(expected, target, std::memory_order_acquire,
                                std::memory_order_relaxed))
    {
      token_.os_wait(1U, std::memory_order_acquire);
      expected = 0;
    }
  }

  void unlock()
  {
    token_.store(0U, std::memory_order_release);
    token_.os_notify();
  }
};

inline void backoff_pause(u64 poll)
{
  if (poll < 8)
  {
    return;
  }

#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
  _mm_pause();
#else
#  if ASH_CFG(ARCH, ARM32) || ASH_CFG(ARCH, ARM64)
  __asm("yield");
#  endif
#endif

  return;
}

inline void backoff_yield(u64 poll)
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
  }

  std::this_thread::yield();
  return;
}

typedef struct ISpinLock * SpinLock;

/// @brief Fast user-space spinlock suitable for deterministic and short critical sections.
/// The spinlock is unpaced and can cause cache invalidation and inefficient CPU usage, use with caution.
struct [[nodiscard]] ISpinLock
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
      backoff_pause(poll);
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

typedef struct ITicketSpinLock * TicketSpinLock;

struct [[nodiscard]] ITicketSpinLock
{
  usize front_ = 0;
  usize back_  = 0;

  void lock()
  {
    std::atomic_ref front{front_};
    std::atomic_ref back{back_};
    u64             poll = 0;

    auto ticket = back.fetch_add(1, std::memory_order_relaxed) - 1;

    while (true)
    {
      auto front_val = front.load(std::memory_order_acquire);

      if (front_val == ticket)
      {
        return;
      }

      backoff_pause(poll);
      poll++;
    }
  }

  [[nodiscard]] bool try_lock()
  {
    std::atomic_ref front{front_};
    std::atomic_ref back{back_};

    auto ticket    = back.fetch_add(1, std::memory_order_relaxed) - 1;
    auto front_val = front.load(std::memory_order_acquire);

    return front_val == ticket;
  }

  void unlock()
  {
    std::atomic_ref front{front_};
    front.fetch_add(1, std::memory_order_release);
  }
};

// [ ] https://en.wikipedia.org/wiki/Seqlock
struct [[nodiscard]] ISequenceLock
{
  usize version_ = 0;

  void begin_read();
  void end_read();

  void begin_write();
  void end_write();
};

// [ ] https://lwn.net/Articles/262464/
struct Rcu;

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

typedef struct IRWSpinLock * RWSpinLock;

struct [[nodiscard]] IRWSpinLock
{
  usize state_ = 0;

  void lock_read()
  {
    std::atomic_ref state{state_};
    usize           expected = 0;
    usize           target   = 1;
    usize           poll     = 0;

    while (true)
    {
      if (state.compare_exchange_weak(expected, target,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed))
      {
        return;
      }

      if (expected != USIZE_MAX)
      {
        target = expected + 1;
      }

      backoff_pause(poll);
      poll++;
    }
  }

  void lock_write()
  {
    std::atomic_ref state{state_};
    usize           expected = 0;
    usize           target   = USIZE_MAX;
    usize           poll     = 0;

    while (true)
    {
      if (state.compare_exchange_weak(expected, target,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed))
      {
        return;
      }

      expected = 0;
      target   = USIZE_MAX;

      backoff_pause(poll);
      poll++;
    }
  }

  void unlock_read()
  {
    std::atomic_ref state{state_};
    state.fetch_sub(1, std::memory_order_relaxed);
  }

  void unlock_write()
  {
    std::atomic_ref state{state_};
    state.store(0, std::memory_order_release);
  }
};

template <typename RWLockType>
struct ReadGuard
{
  RWLockType * lock_;

  explicit ReadGuard(RWLockType & lock) : lock_{&lock}
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

template <typename RWSpinLockType>
struct WriteGuard
{
  RWSpinLockType * lock_;

  explicit WriteGuard(RWSpinLockType & lock) : lock_{&lock}
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
struct [[nodiscard]] AtomicInit
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

  /// @brief Attempt to initialize the value
  /// @return true if the object has not been initialized yet
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

template <typename T, typename RWSpinLock>
struct [[nodiscard]] ISync
{
  IRWSpinLock lock_;

  T data_;

  template <typename... Args>
  constexpr ISync(Args &&... args) :
    lock_{},
    data_{static_cast<Args &&>(args)...}
  {
  }

  constexpr ISync(ISync const &) = delete;

  constexpr ISync(ISync &&) = delete;

  constexpr ISync & operator=(ISync const &) = delete;

  constexpr ISync & operator=(ISync &&) = delete;

  constexpr ~ISync() = default;

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

typedef struct ITimelineSemaphore * TimelineSemaphore;

/// @brief A CPU Timeline TimelineSemaphore used for synchronization in multi-stage cooperative multitasking jobs.
/// Unlike typical Binary/Counting Semaphores, A timeline semaphore is a monotonic counter
/// representing the stages of an operation.
/// - Guarantees Forward Progress
/// - Scatter-gather operations only require one primitive
/// - Primitive can encode state of multiple operations and also be awaited by
/// multiple operations at once.
/// - Task ordering is established by the `stage` which describes the number of
/// current active stage being worked on, and can be awaited by other tasks.
/// - No deadlocks can occur when synchronization is done using this. This also enables cooperative synchronization between systems processing different
/// stages of an operation without explicit sync between them.
///
/// TimelineSemaphore can only move from state `i` to state `i+n` where n > 1.
///
/// TimelineSemaphore should ideally not be destroyed before completion as there could
/// possibly be other tasks awaiting it.
///
/// Semaphores never overflow.
/// It can have a maximum of U64_MAX stages.
/// U64_MAX is often used to denote that all operations are completed.
struct [[nodiscard]] ITimelineSemaphore
{
  u64 stage_;

  constexpr ITimelineSemaphore() : stage_{0}
  {
  }

  constexpr ITimelineSemaphore(u64 initial_stage) : stage_{initial_stage}
  {
  }

  /// @brief Get the current semaphore stage. This represents the current stage
  /// being worked on.
  /// @return the current active stage
  [[nodiscard]] u64 stage()
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire);
  }

  /// @brief Returns true if the semaphore has been completed. i.e. reached its
  /// last declared stage.
  [[nodiscard]] bool is_completed()
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire) == U64_MAX;
  }

  /// @brief Returns true if the polled stage is completed.
  [[nodiscard]] bool is_completed(u64 poll_stage)
  {
    std::atomic_ref stage{stage_};
    auto            current = stage.load(std::memory_order_acquire);
    return current == U64_MAX || current > poll_stage;
  }

  /// @brief force completion of all stages on the semaphore
  [[nodiscard]] bool complete()
  {
    std::atomic_ref stage{stage_};
    return stage.exchange(U64_MAX, std::memory_order_release) != U64_MAX;
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
  [[nodiscard]] bool complete(u64 complete_stage)
  {
    auto            next    = sat_add(complete_stage, 1ULL);
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

  /// @brief Increment the semaphore by `inc` number of stages
  /// @param inc stage increment of semaphore. increment of >= num_stages is
  /// equivalent to driving it to completion.
  [[nodiscard]] u64 increment(u64 inc)
  {
    u64             current = 0;
    u64             target  = inc;
    std::atomic_ref stage{stage_};
    while (!stage.compare_exchange_weak(
      current, target, std::memory_order_release, std::memory_order_relaxed))
      [[unlikely]]
    {
      target = sat_add(current, inc);
    }

    return current;
  }

  /// @brief Poll completion of this semaphore at stage `stage` for `timeout` duration
  /// @param stage stage to wait for
  /// @param timeout duration to wait for
  [[nodiscard]] bool poll(u64 stage);
};

typedef Rc<TimelineSemaphore> RcTimelineSemaphore;

typedef Dyn<TimelineSemaphore> DynTimelineSemaphore;

/// @brief Create an independently allocated semaphore object
inline Result<RcTimelineSemaphore> semaphore(Allocator allocator,
                                             u64       initial_stage = 0)
{
  return rc<ITimelineSemaphore>(inplace, allocator, initial_stage);
}

namespace impl
{

/// @brief Poll semaphores at the specified stages.
/// @param sems semaphores to wait for
/// @param stages stages of the semaphores to wait for completion of. must be <
/// semaphore.num_stages or == U64_MAX. U64_MAX meaning waiting for all stages'
/// completion.
/// @param any if to wait for all semaphores or atleast 1 semaphore.
/// @returns returns true if the semaphore poll operation completed successfully
template <typename Sem, typename Stage, typename SemaphoreKey,
          typename StageKey>
[[nodiscard]] bool poll_semaphores(Span<Sem> semaphores, Span<Stage> stages,
                                   SemaphoreKey && semaphore_key = {},
                                   StageKey &&     stage_key     = {})
{
  CHECK(semaphores.size() == stages.size(), "");
  usize const n = semaphores.size();

  for (auto i = 0uz; i < n; i++)
  {
    ITimelineSemaphore & semaphore = semaphore_key(semaphores[i]);
    u64 const            stage     = stage_key(stages[i]);
    bool const           is_ready  = semaphore.is_completed(stage);

    if (!is_ready)
    {
      return false;
    }
  }

  return true;
}

template <typename Future, typename FutureStageKey>
[[nodiscard]] bool poll_futures(Span<Future>      futures,
                                FutureStageKey && stage_key = {})
{
  usize const n = futures.size();

  for (auto i = 0uz; i < n; i++)
  {
    FutureStage &   stage = stage_key(futures[i]);
    std::atomic_ref stage_ref{stage};
    bool const      is_ready =
      stage_ref.load(std::memory_order_acquire) == FutureStage::Yielded;

    if (!is_ready)
    {
      return false;
    }
  }

  return true;
}

[[nodiscard]] inline bool poll_futures(Span<FutureStage * const> futures)
{
  return poll_futures(futures,
                      [](auto const & f) -> FutureStage & { return *f; });
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

  RcTimelineSemaphore semaphore_;

  Stream(Rc<T *> data, RcTimelineSemaphore semaphore) :
    data_{static_cast<Rc<T *> &&>(data)},
    semaphore_{static_cast<RcTimelineSemaphore &&>(semaphore)}
  {
  }

  Stream alias() const
  {
    return Stream{data_.alias(), semaphore_.alias()};
  }

  [[nodiscard]] bool is_completed(u64 stage) const
  {
    return semaphore_->is_completed(stage);
  }

  [[nodiscard]] bool is_completed() const
  {
    return semaphore_->is_completed();
  }

  template <Callable<T &> F>
  void yield_unsequenced(F && op, u64 increment) const
  {
    static_cast<F &&>(op)(*data_.get());
    (void) semaphore_->increment(increment);
  }

  template <Callable<T &> F>
  void yield_sequenced(F && op, u64 stage) const
  {
    static_cast<F &&>(op)(*data_.get());
    CHECK(semaphore_->complete(stage),
          "`Stream` yielded with invalid sequencing");
  }
};

template <typename T, typename... Args>
Result<Stream<T>> stream(Inplace, Allocator allocator, u64 num_stages,
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
    Stream<T>{static_cast<Rc<T *> &&>(data.v()),
              static_cast<RcTimelineSemaphore &&>(sem.v())}
  };
}

template <typename T>
Result<Stream<T>> stream(Allocator allocator, u64 num_stages, T value)
{
  return stream<T>(inplace, allocator, num_stages, static_cast<T &&>(value));
}

struct [[nodiscard]] AnyStream
{
  RcTimelineSemaphore semaphore_;

  template <typename T>
  AnyStream(Stream<T> stream) :
    semaphore_{static_cast<RcTimelineSemaphore &&>(stream.semaphore_)}
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
Result<Future<T>> future(Allocator allocator)
{
  Result s = rc<AtomicInit<T>>(inplace, allocator);

  if (!s)
  {
    return Err{};
  }

  return Ok{Future<T>{static_cast<Rc<AtomicInit<T> *> &&>(s.v())}};
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

template <typename P>
concept Poll = requires (P p) {
  { p() && true };
};

template <typename R>
concept Runner = requires (R r) {
  { r() && true };
};

template <typename F>
concept TaskFrame = requires (F f) {
  { !f.poll() };
  { !f.run() };
};

enum class TaskState : u8
{
  /// @brief The task is not yet ready to execute
  NotReady = 0,
  /// @brief The task needs to be re-scheduled for execution
  Again    = 1,
  /// @brief The task has completed execution
  Finished = 2
};

struct [[nodiscard]] TaskInfo
{
  typedef Fn<void(void *)> Init;

  typedef void (*Uninit)(void *);

  typedef TaskState (*Tick)(void *);

  /// @brief Nemory layout of the task's frame object.
  Layout frame_layout{};

  /// @brief Function to initialize the task to a memory placement.
  Init init = noop;

  /// @brief Function to uninitialize the task from its memory placement
  Uninit uninit = noop;

  /// @brief The task's main body and ticker. It handles task scheduling state flow.
  /// It also determines task readiness, repetition and continuation.
  Tick tick = [](void *) { return TaskState::Finished; };
};

/// @brief Wrap a Task frame
/// @return TaskInfo struct to be passed to the scheduler for execution
template <TaskFrame F>
constexpr TaskInfo to_task_info(F & frame)
{
  Fn init{&frame, +[](F * frame, void * mem) {
            new (mem) F{static_cast<F &&>(*frame)};
          }};

  TaskInfo::Uninit uninit = [](void * f) {
    F * frame = reinterpret_cast<F *>(f);
    frame->~F();
  };

  TaskInfo::Tick tick = [](void * f) -> TaskState {
    F *  frame = reinterpret_cast<F *>(f);
    bool ready = frame->poll();

    if (!ready)
    {
      return TaskState::NotReady;
    }

    bool repeat = frame->run();

    if (repeat)
    {
      return TaskState::Again;
    }
    else
    {
      return TaskState::Finished;
    }
  };

  return TaskInfo{
    .frame_layout = layout_of<F>, .init = init, .uninit = uninit, .tick = tick};
}

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
  u64 dim = 1;
  u64 idx = 0;
};

[[nodiscard]] inline bool
  poll_semaphores(Span<TimelineSemaphore const> semaphores,
                  Span<u64 const>               stages)
{
  return impl::poll_semaphores(
    semaphores, stages,
    [](TimelineSemaphore s) -> ITimelineSemaphore & { return *s; },
    [](u64 stage) -> u64 { return stage; });
}

[[nodiscard]] inline bool ITimelineSemaphore::poll(u64 stage)
{
  return poll_semaphores(span({this}), span({stage}));
}

[[nodiscard]] inline bool poll_streams(Span<AnyStream const> streams,
                                       Span<u64 const>       stages)
{
  return impl::poll_semaphores(
    streams, stages,
    [](AnyStream const & s) -> ITimelineSemaphore & { return *s.semaphore_; },
    [](u64 stage) -> u64 { return stage; });
}

[[nodiscard]] inline bool poll_futures(Span<AnyFuture const> futures)
{
  return impl::poll_futures(
    futures, [](AnyFuture const & f) -> FutureStage & { return *f.state_; });
}

template <typename T>
[[nodiscard]] inline bool poll_futures(Span<Future<T> const> futures)
{
  return impl::poll_futures(
    futures, [](Future<T> const & f) -> FutureStage & { return *f.state_; });
}

template <typename T>
[[nodiscard]] inline bool poll_futures(Span<Future<T>> futures)
{
  return impl::poll_futures(
    futures, [](Future<T> const & f) -> FutureStage & { return *f.state_; });
}

template <usize N>
struct [[nodiscard]] AwaitStreams
{
  AnyStream streams[N];
  u64       stages[N] = {};

  bool operator()() const
  {
    return poll_streams(streams, stages);
  }
};

template <usize N>
struct [[nodiscard]] AwaitFutures
{
  AnyFuture futures[N];

  [[nodiscard]] bool operator()() const
  {
    return poll_futures(futures);
  }
};

template <typename... T>
AwaitFutures(T...) -> AwaitFutures<sizeof...(T)>;

struct [[nodiscard]] AwaitFuturesVec
{
  Vec<AnyFuture> futures;

  [[nodiscard]] bool operator()() const
  {
    return poll_futures(futures.view());
  }
};

struct [[nodiscard]] Delay
{
  steady_clock::time_point from{};

  nanoseconds delay = 0ns;

  [[nodiscard]] constexpr bool operator()() const
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
  [[nodiscard]] static constexpr bool operator()()
  {
    return true;
  }
};

inline constexpr Ready ready;

enum class WorkerThread : u32
{
  Any = U32_MAX
};

enum class DedicatedThread : u32
{
  First = 0
};

enum class MainThread : u32
{
  Main = 0
};

inline constexpr MainThread main_thread = MainThread::Main;

using Thread = Enum<WorkerThread, DedicatedThread, MainThread>;

typedef struct IScheduler * Scheduler;

struct SchedulerThreadInfo
{
  Str name = "SchedulerThread"_str;
};

struct SchedulerInfo
{
  /// @brief thread-safe allocator to allocate tasks from, must be able to allocate page-sized allocations
  Allocator allocator;

  Span<SchedulerThreadInfo const> dedicated_threads = {};

  Span<SchedulerThreadInfo const> worker_threads = {};

  std::thread::id main_thread_id = {};
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
struct IScheduler
{
  /// @brief Create a Scheduler
  static Dyn<Scheduler> create(SchedulerInfo const & info);

  /// @brief Destroys the scheduler. The scheduler must have been joined.
  virtual ~IScheduler() = default;

  /// @brief Request that the threads stop executing and purges the tasks on the
  /// task queue.
  virtual void shutdown() = 0;

  [[nodiscard]] virtual u32 num_dedicated() = 0;

  [[nodiscard]] virtual u32 num_workers() = 0;

  /// @brief Schedule task to a specific thread
  /// @param info Task frame information
  /// @param thread the index of the thread to schedule to. If none is specified,
  /// the task is scheduled to the main thread.
  virtual void schedule_raw(Thread thread, TaskInfo const & info) = 0;

  /// @brief Execute work on the main thread queue
  /// @param duration maximum timeout to spend executing tasks
  /// @param poll_max minimum time (within duration) to wait for tasks when
  /// the task queue is empty
  virtual void run_main_loop(nanoseconds duration, nanoseconds poll_max) = 0;

  virtual void request_thread_shutdown(Thread thread) = 0;

  virtual void await_thread_shutdown(Thread thread) = 0;

  /// @brief Schedule a task to run
  /// @param thread the thread to run the task on
  template <TaskFrame F>
  void schedule(Thread thread, F && task)
  {
    schedule_raw(thread, to_task_info(task));
  }

  /// @brief Launch a one-shot task
  /// @tparam F Task Functor type
  /// @tparam P Poller Functor type
  /// @param fn Task functor
  /// @param poll Poller functor that returns true when ready
  /// @param schedule How to schedule the task
  template <Callable F, Poll P = Ready>
  void once(Thread thread, F fn, P poll)
  {
    this->schedule(thread,
                   TaskBody{static_cast<P &&>(poll),
                            [fn = static_cast<F &&>(fn)]() mutable -> bool {
                              fn();
                              return false;
                            }});
  }

  /// @brief Launch a one-shot task with multiple functions to be called in sequence
  template <Callable F, Callable... F1, Poll P = Ready>
  void once(Thread thread, Tuple<F, F1...> fns, P poll)
  {
    this->schedule(
      thread,
      TaskBody{static_cast<P &&>(poll),
               [fns = static_cast<Tuple<F, F1...> &&>(fns)]() mutable -> bool {
                 ash::fold(fns);
                 return false;
               }});
  }

  /// @brief Launch a task that produces a future value
  template <typename Func>
  auto run(Allocator future_allocator, Thread thread, Func func)
  {
    using Ret = decltype(func());
    using Fut = Future<Ret>;

    auto fut_r = future<Ret>(future_allocator);

    if (!fut_r)
    {
      return Result<Fut>{Err{}};
    }

    auto fut = std::move(fut_r.v());

    this->once(
      thread,
      [fut = fut.alias(), func = static_cast<Func &&>(func)]() mutable {
        fut.yield(func()).unwrap();
      },
      ready);

    return Result<Fut>{Ok{std::move(fut)}};
  }

  /// @brief Launch a task that produces a future value after awaiting other
  /// futures
  template <typename Func, typename... FutureTypes>
  auto then(Allocator future_allocator, Thread thread, Func func,
            Future<FutureTypes>... awaits)
  {
    using Ret = decltype(func(awaits.get()...));
    using Fut = Future<Ret>;

    auto fut_r = future<Ret>(future_allocator);

    if (!fut_r)
    {
      return Result<Fut>{Err{}};
    }

    auto fut = std::move(fut_r.v());

    struct Frame
    {
      Func                          func;
      Tuple<Future<FutureTypes>...> awaits;
      Fut                           yield_fut;

      bool poll()
      {
        return apply(
          [](auto &... f) {
            FutureStage * stages[] = {&f.state_.get()->stage_...};
            return impl::poll_futures(stages);
          },
          awaits);
      }

      bool run()
      {
        yield_fut
          .yield(
            apply([this](auto &... fs) { return func(fs.get()...); }, awaits))
          .unwrap();
        return false;
      }
    };

    this->schedule(thread, Frame{.func{static_cast<Func &&>(func)},
                                 .awaits{std::move(awaits)...},
                                 .yield_fut{fut.alias()}});

    return Result<Fut>{Ok{std::move(fut)}};
  }

  /// @brief Flattens a nested future result into a single future result
  template <typename FlattenFunc, typename T, typename E>
  auto flatten(Allocator future_allocator, Thread thread,
               FlattenFunc                             flatten_func,
               Future<Result<Future<Result<T, E>>, E>> await)
  {
    using U   = decltype(flatten_func(declval<Result<T, E>>()));
    using Fut = Future<U>;

    struct Frame
    {
      FlattenFunc                             flatten_func;
      Future<Result<Future<Result<T, E>>, E>> await;
      Fut                                     yield_fut;
      u8                                      nesting_level = 0;

      bool poll()
      {
        switch (nesting_level)
        {
          case 0:
          {
            FutureStage * stages[] = {&await.state_.get()->stage_};
            return impl::poll_futures(stages);
          }

          case 1:
          {
            auto &        r0       = await.get();
            FutureStage * stages[] = {&r0.v().state_.get()->stage_};
            return impl::poll_futures(stages);
          }

          default:
            ASH_UNREACHABLE;
        }
      }

      bool run()
      {
        switch (nesting_level)
        {
          case 0:
          {
            auto & r0 = await.get();
            if (r0.is_err())
            {
              yield_fut
                .yield(flatten_func(Result<T, E>{Err{std::move(r0.err())}}))
                .unwrap();
              return false;
            }

            nesting_level++;

            // if next level is not ready, reschedule
            if (!poll())
            {
              return true;
            }

            [[fallthrough]];
          }

          case 1:
          {
            auto r0     = std::move(await.get());
            auto r1_fut = r0.unwrap();
            auto r1     = std::move(r1_fut.get());
            yield_fut.yield(flatten_func(std::move(r1))).unwrap();
            return false;
          }

          default:
            ASH_UNREACHABLE;
        }
      }
    };

    auto fut_r = future<U>(future_allocator);

    if (!fut_r)
    {
      return Result<Fut>{Err{}};
    }

    auto fut = std::move(fut_r.v());

    this->schedule(
      thread, Frame{.flatten_func{static_cast<FlattenFunc &&>(flatten_func)},
                    .await{std::move(await)},
                    .yield_fut{fut.alias()}});

    return Result<Fut>{Ok{std::move(fut)}};
  }

  /// @brief Flattens a nested future result into a single future result
  template <typename T, typename E>
  auto flatten(Allocator future_allocator, Thread thread,
               Future<Result<Future<Result<T, E>>, E>> await)
  {
    return flatten(
      future_allocator, thread, [](auto r) { return r; }, std::move(await));
  }

  /// @brief Launch a task that is repeatedly called until it is done
  /// @tparam F Task Functor type
  /// @tparam P Poller Functor type
  /// @param fn Task functor that returns false when it is done
  /// @param poll Poller functor that returns true when ready
  /// @param schedule How to schedule the task
  template <Callable F, Poll P = Ready>
  requires (Convertible<CallResult<F>, bool>)
  void loop(Thread thread, F fn, P poll)
  {
    this->schedule(thread,
                   TaskBody{static_cast<P &&>(poll),
                            [fn = static_cast<F &&>(fn)]() mutable -> bool {
                              return fn();
                            }});
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
  void repeat(Thread thread, F fn, u64 n, P poll)
  {
    if (n == 0)
    {
      return;
    }

    this->schedule(
      thread,
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
               }});
  }

  /// @brief Launch shards of tasks, All shards share the same state and task
  /// frame and run concurrently. Typically used for SPMD
  /// (https://en.wikipedia.org/wiki/Single_program,_multiple_data)
  /// @tparam F Shard functor type
  /// @tparam P Poller Functor type
  /// @param fn Shard body
  /// @param dim Number of shard instances of the task to launch
  /// @param poll Poller functor that returns true when ready
  /// @param schedule How to schedule the shards
  template <typename State, Poll P = Ready>
  void shard(Thread thread, Rc<State> state, Fn<void(TaskInstance, State)> fn,
             u64 dim, P poll)
  {
    if (dim == 0)
    {
      return;
    }

    // we need to first dispatch a task that will poll for readiness, and once the
    // shard is ready for dispatch we dispatch the shards. we can avoid this
    // intermediate process if we know the task is immediately available (Ready
    // type) but that's really not a good idea for a generic type. we also need
    // the dispatch as we don't expect the polling function to be thread-safe when
    // called across all instances.
    //
    //
    // [ ] use metadata to optimize Ready pollers
    //
    //
    this->schedule(
      thread,
      TaskBody{
        static_cast<P &&>(poll),
        [fn, state = std::move(state), thread, dim, this]() mutable -> bool {
          for (u64 i = 0; i < dim; i++)
          {
            this->schedule(
              thread,
              TaskBody{ready,
                       [fn, i, dim, state = state.alias()]() mutable -> bool {
                         fn(TaskInstance{.dim = dim, .idx = i}, state.get());
                         return false;
                       }});
          }
          return false;
        }});
  }
};

extern Scheduler scheduler;

/// @brief Global scheduler object. Designed for hooking across DLLs. Must be
/// initialized at program startup.
ASH_C_LINKAGE ASH_DLL_EXPORT void hook_scheduler(Scheduler);

}    // namespace ash
