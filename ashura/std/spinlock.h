#pragma once
#include <atomic>

#include "ashura/std/struct.h"

namespace ash
{
// a rarely-contended lock.
//
// desirable for low-latency scenarios.
// typically used when the operations on the objects being guarded/locked are
// very short.
//
// less desirable for multi-contended and prolonged operations which would cause
// busy waiting.
struct SpinLock
{
  std::atomic<bool> lock_status{false};

  void lock()
  {
    bool expected = false;
    bool target   = true;
    while (!lock_status.compare_exchange_strong(
        expected, target, std::memory_order_acquire, std::memory_order_relaxed))
    {
      expected = false;
    }
  }

  bool try_lock()
  {
    bool expected = false;
    bool target   = true;
    lock_status.compare_exchange_strong(
        expected, target, std::memory_order_acquire, std::memory_order_relaxed);
    return expected;
  }

  void unlock()
  {
    lock_status.store(false, std::memory_order_release);
  }
};

template <typename Resource>
struct LockGuard
{
  ASH_MAKE_PINNED(LockGuard)

  explicit LockGuard(Resource &resource) : m_resource{&resource}
  {
    m_resource->lock();
  }

  ~LockGuard()
  {
    m_resource->unlock();
  }

  Resource *m_resource;
};
}        // namespace ash
