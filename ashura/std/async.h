#include <atomic>

#include "ashura/std/allocator.h"
#include "ashura/std/enum.h"
#include "ashura/std/fn.h"
#include "ashura/std/types.h"

namespace ash
{

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
// - optimized polling for tasks waiting on other tasks
// fn: returns state
//

enum class FutureState : u8
{
  None       = 0,
  Scheduled  = 1,
  Submitted  = 2,
  Preempted  = 3,
  Executing  = 4,
  Suspended  = 5,
  Canceled   = 0xF0 | 6,
  Completing = 0xF0 | 7,
  Completed  = 0xF0 | 8
};

ASH_DEFINE_ENUM_BIT_OPS(FutureState);

enum class FutureError : u8
{
  None     = 0,
  Pending  = 1,
  Canceled = 2
};

struct Future
{
  void suspend();
  void resume();
  void cancel();
  void await();
  void poll();
};

struct Promise
{
  void send_canceled();
  void send_preempted();
  void send_resumed();
  void complete();
};

u32              num_physical_cores();
u32              num_logical_cores();
void             set_thread_name(uid32 tid, Span<char const>);
Span<char const> get_thread_name(uid32 tid);
void             set_thread_affinity(uid32 tid);
void             get_thread_affinity(uid32 tid);

struct CpuExecutor
{
  void await(...);
  void await_any(...);
  void map(int fn, ...);
  void reduce(...);
  void compute(int fn, u32 x, u32 y, u32 z);
};

}        // namespace ash
