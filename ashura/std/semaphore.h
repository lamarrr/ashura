/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"

namespace ash
{

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
struct Semaphore
{
  struct Inner
  {
    u64              num_stages = 1;
    std::atomic<u64> stage      = 0;
  } inner = {};

  /// @brief initialize the semaphore
  void init(u64 num_stages)
  {
    CHECK(num_stages > 0);
    inner.stage      = 0;
    inner.num_stages = num_stages;
  }

  void reset()
  {
    // no-op
  }

  /// @brief Get the current semaphore stage
  /// @param sem non-null
  /// @return
  [[nodiscard]] u64 get_stage()
  {
    return inner.stage.load(std::memory_order_acquire);
  }

  /// @brief Get the number of stages in the semaphore
  /// @param sem non-null
  /// @return
  [[nodiscard]] u64 get_num_stages()
  {
    return inner.num_stages;
  }

  /// @brief
  /// @param sem non-null
  /// @return
  [[nodiscard]] bool is_completed()
  {
    return inner.stage.load(std::memory_order_acquire) == inner.num_stages;
  }

  /// @brief
  ///
  /// @param stage: stage of the semaphore currently executing. stage >=
  /// num_stages or U64_MAX means completion of the last stage of the operation.
  /// must be monotonically increasing for each call to signal_semaphore.
  ///
  void signal(u64 stage)
  {
    stage       = min(stage, inner.num_stages);
    u64 current = 0;
    while (!inner.stage.compare_exchange_strong(
        current, stage, std::memory_order_release, std::memory_order_relaxed))
    {
      CHECK(current <= stage);
    }
  }

  /// @brief
  /// @param inc stage increment of semaphore. increment of >= num_stages is
  /// equivalent to driving it to completion.
  void increment(u64 inc)
  {
    inc         = min(inc, inner.num_stages);
    u64 current = 0;
    u64 target  = inc;
    while (!inner.stage.compare_exchange_strong(
        current, target, std::memory_order_release, std::memory_order_relaxed))
    {
      target = min(sat_add(current, inc), inner.num_stages);
    }
  }
};

typedef Semaphore *SemaphoreRef;

///
/// @brief Create an independently allocated semaphore object
///
/// @param num_stages: number of stages represented by this semaphore. must be
///  non-zero.
/// @return Semaphore
///
[[nodiscard]] SemaphoreRef create_semaphore(u64                  num_stages,
                                            AllocatorImpl const &allocator);

/// @brief
/// @param sem can be null
void destroy_semaphore(SemaphoreRef sem, AllocatorImpl const &allocator);

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
[[nodiscard]] bool await_semaphores(Span<SemaphoreRef const> await,
                                    Span<u64 const>          stages,
                                    nanoseconds              timeout);

}        // namespace ash
