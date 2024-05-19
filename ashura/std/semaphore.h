#pragma once
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
///
typedef struct Semaphore_T *Semaphore;

///
///@brief Create a semaphore object
///
///@param num_stages: number of stages represented by this semaphore. must be
/// non-zero.
///@return Semaphore
///
Semaphore create_semaphore(u64 num_stages);

/// @brief
/// @param semaphore: can be null
void destroy_semaphore(Semaphore sem);

/// @brief
/// @param semaphore: non-null
/// @return
u64 get_semaphore_stage(Semaphore sem);

///
///@brief
///
///@param semaphore: semaphore
///@param stage: stage of the semaphore currently executing. must be <=
/// semaphore.num_stages or == U64_MAX. where stage of num_stages and U64_MAX
/// means
/// completion of the last stage of the operation. must be monotonically
/// increasing for each call to signal_semaphore.
///
void signal_semaphore(Semaphore sem, u64 stage);

///
/// @brief
/// @param await: semaphores to wait for
/// @param stages: stages of the semaphores to wait for. must be <=
/// semaphore.num_stages or == U64_MAX.
/// @param timeout_ns: timeout in nanoseconds to stop attempting to wait for the
/// semaphore. U64_MAX for an infinite timeout.
/// @return: true if all semaphores completed the expected stages before the
/// timeout.
bool await_semaphores(Span<Semaphore const> await, Span<u64 const> stages,
                      u64 timeout_ns);

}        // namespace ash
