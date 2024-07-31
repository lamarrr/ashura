/// SPDX-License-Identifier: MIT
#include "ashura/std/semaphore.h"
#include "ashura/std/allocator.h"
#include "ashura/std/backoff.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/time.h"
#include <chrono>

namespace ash
{

SemaphoreRef create_semaphore(u64 num_stages, AllocatorImpl const &allocator)
{
  Semaphore *s;
  CHECK(allocator.nalloc(1, &s));
  new (s) Semaphore{};
  s->init(num_stages);
  return (SemaphoreRef) s;
}

void destroy_semaphore(SemaphoreRef sem, AllocatorImpl const &allocator)
{
  sem->reset();
  allocator.ndealloc(sem, 1);
}

bool await_semaphores(Span<SemaphoreRef const> semaphores,
                      Span<u64 const> stages, nanoseconds timeout)
{
  CHECK(semaphores.size() == stages.size());
  usize const n = semaphores.size();
  for (usize i = 0; i < n; i++)
  {
    Semaphore const *s = semaphores[i];
    CHECK(s != nullptr);
    CHECK((stages[i] == U64_MAX) || (stages[i] < s->inner.num_stages));
  }

  steady_clock::time_point begin = steady_clock::time_point{};
  for (usize i = 0; i < n; i++)
  {
    Semaphore const *s     = semaphores[i];
    u64 const        stage = min(stages[i], s->inner.num_stages - 1);
    u64              poll  = 0;
    while (true)
    {
      // always monotonically increasing
      if (stage <= s->inner.stage.load(std::memory_order_acquire))
      {
        break;
      }

      /// we want to avoid syscalls if timeout is 0
      if (timeout == nanoseconds{})
      {
        return false;
      }

      if (begin == steady_clock::time_point{}) [[unlikely]]
      {
        begin = steady_clock::now();
      }

      steady_clock::time_point const curr = steady_clock::now();
      nanoseconds const dur = duration_cast<nanoseconds>(curr - begin);
      if (dur > timeout) [[unlikely]]
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