#pragma once

#include <atomic>
#include <cinttypes>
#include <utility>

#include "stx/lock_status.h"
#include "stx/struct.h"

namespace stx {

template <typename Resource>
struct LockGuard {
  STX_MAKE_PINNED(LockGuard)

  explicit LockGuard(Resource& iresource) : resource{&iresource} {
    resource->lock();
  }

  ~LockGuard() { resource->unlock(); }

 private:
  Resource* resource;
};

// a rarely-contended lock.
//
// desirable for low-latency scenarios.
// typically used when the operations on the objects being guarded/locked are
// very short.
//
// less desirable for multi-contended and frequently updated memory regions.
struct SpinLock {
  STX_MAKE_PINNED(SpinLock)

  SpinLock() : lock_status{LockStatus::Unlocked} {}

  void lock() {
    LockStatus expected = LockStatus::Unlocked;
    LockStatus target = LockStatus::Locked;
    while (!lock_status.compare_exchange_strong(expected, target,
                                                std::memory_order_acquire,
                                                std::memory_order_relaxed)) {
      expected = LockStatus::Unlocked;
    }
  }

  void unlock() {
    lock_status.store(LockStatus::Unlocked, std::memory_order_release);
  }

 private:
  std::atomic<LockStatus> lock_status;
};

}  // namespace stx
