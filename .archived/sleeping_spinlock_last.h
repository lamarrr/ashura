
#include <atomic>
#include <chrono>
#include <thread>

#include "stx/lock_status.h"
#include "stx/struct.h"

namespace stx {

namespace impl {

enum class WaitResult { Success, Timeout };

// anyway we want the threads to sleep but not too much
//
// we should use a Spinlock for protecting the data and a exponential back off
// algorithm to await tasks. must unlock before beginning await.
//
//
// we don't need this data structure
//
//
struct PulsatingCountingSemaphore {
  STX_MAKE_PINNED(PulsatingCountingSemaphore)

  PulsatingCountingSemaphore() : lock_status{LockStatus::Unlocked} {}

  WaitResult wait(std::chrono::nanoseconds lock_timeout) {
    // first phase: immediate spinning
    uint64_t iterations = 0;  // say approx 5 instructions (1 CAS, 1
                              // set, 2 branch) @1.0ghz clock reference
    do {
      if (try_lock()) return;
      std::this_thread::sleep_for(
          impl::clamped_exponential_backoff(iterations, lock_timeout));
      iterations++;
    } while (true);
  }

  bool try_lock() {
    LockStatus expected = LockStatus::Unlocked;
    LockStatus target = LockStatus::Locked;

    return lock_status.compare_exchange_strong(
        expected, target, std::memory_order_acquire, std::memory_order_relaxed);
  }

  void unlock() {
    lock_status.store(LockStatus::Unlocked, std::memory_order_release);
  }

 private:
  std::atomic<LockStatus> lock_status;
};
}  // namespace impl