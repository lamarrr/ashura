/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/backoff.h"
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{

struct SpinLock
{
  std::atomic<bool> flag_{false};

  void lock()
  {
    bool expected = false;
    bool target   = true;
    u64  poll     = 0;
    while (!flag_.compare_exchange_strong(
        expected, target, std::memory_order_acquire, std::memory_order_relaxed))
    {
      expected = false;
      yielding_backoff(poll);
      poll++;
    }
  }

  [[nodiscard]] bool try_lock()
  {
    bool expected = false;
    bool target   = true;
    flag_.compare_exchange_strong(expected, target, std::memory_order_acquire,
                                  std::memory_order_relaxed);
    return expected;
  }

  void unlock()
  {
    flag_.store(false, std::memory_order_release);
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
}        // namespace ash
