#include <atomic>
#include <thread>

#include "ashura/std/allocator.h"
#include "ashura/std/types.h"

namespace ash
{

// TODO(lamarrr): instanced tasks
// automatic heuristic division of data points to cache and destructive
// coherence size
//
// resolve function
// - self-suspension
// - optimized polling for tasks waiting on other tasks
// fn: returns state
//
//
// requirements: inter-task communication, inter-task sharing, inter-task data
// flow, reporting cancelation
// SPSC-buffer ring buffer?
//
// i.e chaining of completions across multiple tasks. or coordination.
//
//
//
//
typedef struct Thread_T *Thread;

u32              num_physical_cores();
u32              num_logical_cores();
void             set_thread_name(Thread thread, Span<char const> name);
Span<char const> get_thread_name(Thread thread);
void             set_thread_affinity(Thread thread);
void             get_thread_affinity(Thread thread);

/// - Guarantees Forward Progress
/// - Scatter-gather operations only require one primitive
/// - Primitive can encode state of multiple operations and also be awaited by
/// multiple operations
///
/// initialized to U64_MAX so that the first yield operation will increment the
/// state (with wrap-around) to 0
///
///
/// automatically destroyed on last use
///
typedef struct Barrier_T *Barrier;

// would dead-lock and forward progress guarantees be achievable?
//
//
// order of scheduling and then incrementing the semaphore?
//
// task N finishes before task N-1, and then signals this will make the state
// think the order is preserved.
//
//
// all tasks should execute anyhow. but have dependencies enforced by barriers.
//
// task A will never sync with task B unless there's a barrier between them.
//
// for forward progress guarantee, task scheduled at time N will not be able to
// wait for task scheduled at time N+1? OR HOW Does vulkan do it?
//
//
// TODO(lamarrr): add barrier that can wait for all previously submitted tasks
//
// main thread is always thread 0.
// U32_MAX means any thread.
//
struct ScheduleInfo
{
  u32                 thread       = U32_MAX;
  Span<Barrier const> await        = {};
  Span<u64 const>     await_state  = {};
  Span<Barrier const> signal       = {};
  Span<u64 const>     signal_state = {};
  Fn<void(void *)>    task         = {};
  void               *data         = nullptr;
};

struct SignalInfo
{
  Span<Barrier const> semaphore  = {};
  Span<u64 const>     state      = {};
  u64                 timeout_ns = 0;
};

struct TaskScheduler
{
  virtual Barrier create_barrier(u64 num_states)              = 0;
  virtual void    advance_barrier(Barrier barrier, u64 state) = 0;
  virtual u64     get_barrier_state(Barrier barrier)          = 0;
  virtual void    schedule(ScheduleInfo const &info)          = 0;
  virtual void    await(SignalInfo const &info)               = 0;
  virtual void    signal(SignalInfo const &info)              = 0;
};

}        // namespace ash
