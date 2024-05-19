#include "ashura/std/semaphore.h"
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include <atomic>
#include <chrono>

namespace ash
{
struct SemaphoreImpl
{
  u64              num_stages = 1;
  std::atomic<u64> stage      = 0;
};

Semaphore create_semaphore(u64 num_stages)
{
  CHECK(num_stages > 0);
  SemaphoreImpl *s;
  CHECK(default_allocator.t_alloc(1, &s));
  new (s) SemaphoreImpl{.num_stages = num_stages};
  return (Semaphore) s;
}

void destroy_semaphore(Semaphore sem)
{
  if (sem == nullptr)
  {
    return;
  }
  SemaphoreImpl *s = (SemaphoreImpl *) sem;
  CHECK(s->stage.load(std::memory_order_relaxed) == s->num_stages);
  s->~SemaphoreImpl();

  default_allocator.t_dealloc(s, 1);
}

u64 get_semaphore_stage(Semaphore sem)
{
  CHECK(sem != nullptr);
  SemaphoreImpl const *s = (SemaphoreImpl const *) sem;
  return s->stage.load(std::memory_order_acquire);
}

void signal_semaphore(Semaphore sem, u64 stage)
{
  CHECK(sem != nullptr);
  SemaphoreImpl *s = (SemaphoreImpl *) sem;
  CHECK((stage == U64_MAX) || (stage <= s->num_stages));
  stage               = (stage == U64_MAX) ? s->num_stages : stage;
  u64 const old_stage = s->stage.exchange(stage, std::memory_order_release);
  CHECK(stage >= old_stage);
}

inline void exponential_backoff(u64 poll_count)
{
  if (poll_count > 16 && poll_count <= 64)
  {
    std::this_thread::yield();
  }
  else if (poll_count > 64)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

bool await_semaphores(Span<Semaphore const> semaphores, Span<u64 const> stages,
                      u64 timeout_ns)
{
  CHECK(semaphores.size() == stages.size());
  for (usize i = 0; i < semaphores.size(); i++)
  {
    CHECK(semaphores[i] != nullptr);
    SemaphoreImpl const *s = (SemaphoreImpl const *) semaphores[i];
    CHECK((stages[i] == U64_MAX) || (stages[i] <= s->num_stages));
  }

  auto const begin = std::chrono::steady_clock::now();
  for (usize i = 0; i < semaphores.size(); i++)
  {
    SemaphoreImpl const *s = (SemaphoreImpl const *) semaphores[i];
    u64 const stage        = (stages[i] == U64_MAX) ? s->num_stages : stages[i];
    u64       poll_count   = 0;
    while (true)
    {
      poll_count++;
      if ((stage > s->stage.load(std::memory_order_relaxed)) &&
          (stage > s->stage.load(std::memory_order_acquire)))
      {
        break;
      }

      auto const curr = std::chrono::steady_clock::now();
      auto const dur =
          std::chrono::duration_cast<std::chrono::nanoseconds>(curr - begin);
      if (((u64) dur.count()) > timeout_ns)
      {
        return false;
      }

      exponential_backoff(poll_count);
    }
  }

  return true;
}
}        // namespace ash