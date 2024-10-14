/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/backoff.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/rc.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{

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

template <typename R>
struct LockGuard
{
  explicit constexpr LockGuard(R &resource) : r{&resource}
  {
    r->lock();
  }

  LockGuard(LockGuard const &)            = delete;
  LockGuard &operator=(LockGuard const &) = delete;
  LockGuard(LockGuard &&)                 = delete;
  LockGuard &operator=(LockGuard &&)      = delete;

  constexpr ~LockGuard()
  {
    r->unlock();
  }

  R *r;
};

struct StopToken
{
  bool stop_ = false;

  /// @brief synchronizes with the scope
  /// @return
  bool is_stop_requested() const
  {
    std::atomic_ref stop{stop_};
    return stop.load(std::memory_order_acquire);
  }

  /// @brief synchronizes with the scope
  void request_stop()
  {
    std::atomic_ref stop{stop_};
    stop.store(true, std::memory_order_release);
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
/// Semaphore can only move from state `i` to state `j` where `j` >= `i`.
///
/// Semaphore should ideally not be destroyed before completion as there could
/// possibly be other tasks awaiting it.
///
/// Semaphores never overflows. so it can have a maximum of U64_MAX stages.
struct Semaphore : Pin<>
{
  struct Inner
  {
    u64        num_stages  = 1;
    u64        stage       = 0;
    AliasCount alias_count = {};
  } inner = {};

  /// @brief initialize the semaphore
  void init(u64 num_stages)
  {
    CHECK(num_stages > 0);
    new (&inner) Inner{.num_stages = num_stages, .stage = 0, .alias_count = {}};
  }

  void uninit()
  {
    // no-op
  }

  void reset()
  {
    new (&inner) Inner{};
  }

  /// @brief Get the current semaphore stage
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
  void signal(u64 next)
  {
    next                    = min(next, inner.num_stages);
    u64             current = 0;
    std::atomic_ref stage{inner.stage};
    while (!stage.compare_exchange_strong(
        current, next, std::memory_order_release, std::memory_order_relaxed))
    {
      CHECK(current <= next);
    }
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
    {
      target = min(sat_add(current, inc), inner.num_stages);
    }
  }
};

///
/// @brief Create an independently allocated semaphore object
///
/// @param num_stages: number of stages represented by this semaphore. must be
///  non-zero.
/// @return Semaphore
///
[[nodiscard]] Rc<Semaphore *> create_semaphore(u64                  num_stages,
                                               AllocatorImpl const &allocator);

///
/// @brief no syscalls are made unless timeout_ns is non-zero.
/// @param await semaphores to wait for
/// @param stages stages of the semaphores to wait for completion of. must be <
/// semaphore.num_stages or == U64_MAX. U64_MAX meaning waiting for all stages'
/// completion.
/// @param timeout_ns timeout in nanoseconds to stop attempting to wait for the
/// semaphore. U64_MAX for an infinite timeout.
/// @return: true if all semaphores completed the expected stages before the
/// timeout.
[[nodiscard]] bool await_semaphores(Span<Rc<Semaphore *> const> await,
                                    Span<u64 const>             stages,
                                    nanoseconds                 timeout);

struct TaskInfo
{
  Fn<bool(void *)>            task       = fn([](void *) { return false; });
  mem::Layout                 ctx_layout = mem::Layout{};
  Fn<void(void *)>            ctx_init   = fn([](void *) {});
  Fn<void(void *)>            ctx_uninit = fn([](void *) {});
  Span<Rc<Semaphore *> const> await_semaphores     = {};
  Span<u64 const>             awaits               = {};
  Span<Rc<Semaphore *> const> signal_semaphores    = {};
  Span<u64 const>             signals              = {};
  Span<Rc<Semaphore *> const> increment_semaphores = {};
  Span<u64 const>             increments           = {};
};

/// @brief Static Thread Pool Scheduler.
///
/// all tasks execute out-of-order and have dependencies enforced by
/// semaphores.
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
/// Requirements:
/// [x] result collection
/// [x] inter-task communication
/// [x] inter-task sharing
/// [x] inter-task data flow, reporting cancelation
/// [x] external polling contexts
/// [ ] helper functions to correctly dispatch to required types.
/// [ ] shutdown is performed immediately as we can't guarantee when tasks will
/// complete.
struct Scheduler
{
  virtual void init(Span<nanoseconds const> dedicated_thread_sleep,
                    Span<nanoseconds const> worker_thread_sleep)    = 0;
  virtual void uninit()                                             = 0;
  virtual void schedule_dedicated(u32 thread, TaskInfo const &info) = 0;
  virtual void schedule_worker(TaskInfo const &info)                = 0;
  virtual void schedule_main(TaskInfo const &info)                  = 0;
  virtual void execute_main_thread_work(nanoseconds timeout)        = 0;
};

ASH_C_LINKAGE ASH_DLL_EXPORT Scheduler *scheduler;

}        // namespace ash
