#include "ashura/std/semaphore.h"
#include "ashura/std/allocator.h"
#include "ashura/std/backoff.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/time.h"
#include <chrono>

namespace ash
{

struct CpuSemaphore
{
  u64              num_stages = 1;
  std::atomic<u64> stage      = 0;
};

Semaphore create_semaphore(u64 num_stages)
{
  CHECK(num_stages > 0);
  CpuSemaphore *s;
  CHECK(default_allocator.nalloc(1, &s));
  new (s) CpuSemaphore{.num_stages = num_stages};
  return (Semaphore) s;
}

void destroy_semaphore(Semaphore sem)
{
  if (sem == nullptr)
  {
    return;
  }
  CpuSemaphore *s = (CpuSemaphore *) sem;
  CHECK(s->stage.load(std::memory_order_relaxed) == s->num_stages);
  s->~CpuSemaphore();
  default_allocator.ndealloc(s, 1);
}

u64 get_semaphore_stage(Semaphore sem)
{
  CHECK(sem != nullptr);
  CpuSemaphore const *s = (CpuSemaphore const *) sem;
  return s->stage.load(std::memory_order_acquire);
}

u64 get_num_semaphore_stages(Semaphore sem)
{
  CHECK(sem != nullptr);
  CpuSemaphore const *s = (CpuSemaphore const *) sem;
  return s->num_stages;
}

bool is_semaphore_completed(Semaphore sem)
{
  CHECK(sem != nullptr);
  CpuSemaphore const *s = (CpuSemaphore const *) sem;
  return (s->stage.load(std::memory_order_relaxed) == s->num_stages) &&
         (s->stage.load(std::memory_order_acquire) == s->num_stages);
}

void signal_semaphore(Semaphore sem, u64 stage)
{
  CHECK(sem != nullptr);
  CpuSemaphore *s = (CpuSemaphore *) sem;
  stage           = min(stage, s->num_stages);
  u64 current     = 0;
  while (!s->stage.compare_exchange_strong(
      current, stage, std::memory_order_release, std::memory_order_relaxed))
  {
    CHECK(current <= stage);
  }
}

void increment_semaphore(Semaphore sem, u64 inc)
{
  CHECK(sem != nullptr);
  CpuSemaphore *s = (CpuSemaphore *) sem;
  inc             = min(inc, s->num_stages);
  u64 current     = 0;
  u64 target      = inc;
  while (!s->stage.compare_exchange_strong(
      current, target, std::memory_order_release, std::memory_order_relaxed))
  {
    target = min(current + inc, s->num_stages);
  }
}

bool await_semaphores(Span<Semaphore const> semaphores, Span<u64 const> stages,
                      u64 timeout_ns)
{
  CHECK(semaphores.size() == stages.size());
  usize const n = semaphores.size();
  for (usize i = 0; i < n; i++)
  {
    CpuSemaphore const *s = (CpuSemaphore const *) semaphores[i];
    CHECK(s != nullptr);
    CHECK((stages[i] == U64_MAX) || (stages[i] <= s->num_stages));
  }

  steady_clock::time_point begin = steady_clock::time_point{};
  for (usize i = 0; i < n; i++)
  {
    CpuSemaphore const *s     = (CpuSemaphore const *) semaphores[i];
    u64 const           stage = min(stages[i], s->num_stages);
    u64                 poll  = 0;
    while (true)
    {
      // always monotinically increasing. use relaxed load for first
      // comparision, this enables fast backoff without introducing a memory
      // barrier.
      if ((stage < s->stage.load(std::memory_order_relaxed)) &&
          (stage < s->stage.load(std::memory_order_acquire)))
      {
        break;
      }

      /// we want to avoid syscalls if timeout is 0
      if (timeout_ns == 0)
      {
        return false;
      }

      if (begin == steady_clock::time_point{}) [[unlikely]]
      {
        begin = steady_clock::now();
      }
      auto const curr = steady_clock::now();
      auto const dur  = duration_cast<nanoseconds>(curr - begin);
      if (((u64) dur.count()) > timeout_ns) [[unlikely]]
      {
        return false;
      }

      yielding_backoff(poll);
      poll++;
    }
  }

  return true;
}
}        // namespace ash