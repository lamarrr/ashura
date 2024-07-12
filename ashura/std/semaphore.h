/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/time.h"
#include "ashura/std/types.h"

namespace ash
{

/// - Guarantees Forward Progress
/// - Scatter-gather operations only require one primitive
/// - Primitive can encode state of multiple operations and also be awaited by
/// multiple operations at once.
/// - Task ordering is established by the `state` which describes the number of
/// steps needed to complete a task, and can be awaited by other tasks.
///
/// Semaphore can only move from state `i` to state `j` where `j` >= `i`.
///
/// semaphore should ideally not be destroyed before completion as there could
/// possibly be other tasks awaiting it.
///
/// Semaphores never overflow. so it can have a maximum of U64_MAX stages.
typedef struct Semaphore_T *Semaphore;

///
///@brief Create a semaphore object
///
///@param num_stages: number of stages represented by this semaphore. must be
/// non-zero.
///@return Semaphore
///
[[nodiscard]] Semaphore create_semaphore(u64 num_stages);

/// @brief
/// @param sem can be null
void destroy_semaphore(Semaphore sem);

/// @brief
/// @param sem non-null
/// @return
[[nodiscard]] u64 get_semaphore_stage(Semaphore sem);

/// @brief
/// @param sem non-null
/// @return
[[nodiscard]] u64 get_num_semaphore_stages(Semaphore sem);

/// @brief
/// @param sem non-null
/// @return
[[nodiscard]] bool is_semaphore_completed(Semaphore sem);

///
///@brief
///
///@param semaphore: semaphore, non-null.
///@param stage: stage of the semaphore currently executing. stage >= num_stages
/// means completion of the last stage of the operation. must be monotonically
/// increasing for each call to signal_semaphore.
///
void signal_semaphore(Semaphore sem, u64 stage);

/// @brief
/// @param sem non-null
/// @param inc stage increment of semaphore. increment of >= num_stages is
/// equivalent to driving it to completion.
void increment_semaphore(Semaphore sem, u64 inc);

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
[[nodiscard]] bool await_semaphores(Span<Semaphore const> await,
                                    Span<u64 const>       stages,
                                    nanoseconds           timeout);

}        // namespace ash
