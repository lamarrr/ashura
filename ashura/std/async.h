#include <atomic>

#include "ashura/allocator.h"
#include "ashura/types.h"

namespace ash
{

struct Promise
{
};

// TODO(lamarrr): instanced tasks
// automatic heuristic division of data points to cache and destructive
// coherence size
//
// awaiting tasks
// priority
// tracing
// schedule timepoint
// resolve function
// thread-pinning
// -barriers?
// - self-suspension
//
// - optimized polling for tasks waiting on other tasks
//
// fn: returns state
//
struct Task
{
  u64 (*fn)(void *data, u64 instance)                           = nullptr;
  void *data                                                    = nullptr;
  void (*resolve_fn)(void *data, void *resolve_data, u64 state) = nullptr;
  void *resolve_data                                            = nullptr;
  u64   target_state                                            = 0;
  u64   instances                                               = 0;
  u32   runs_on_thread                                          = -1;
};

struct Thread
{
  static u32  get_num_physical_cores();
  static u32  get_num_logical_cores();
  void        set_name(char const *);
  char const *get_name();
  void        set_affinity();
};

// how to perform cross-thread work-stealing
struct TaskChunk
{
  CpuTask         *tasks = nullptr;
  atomic_uint64_t *task_assignment;
  atomic_uint64_t *called_instances = 0;
};

// false-sharing is okay for rarely executed tasks
struct ThreadPoolExecutor
{
  // allocators for each thread/worker?
  //
  //
  //
  //
  // must not use std::function or any allocating type. stx::Fn only
  //
  //
  //
};

}        // namespace ash
