
#include <atomic>
#include <chrono>
#include <thread>

#include "stx/lock_status.h"
#include "stx/struct.h"

namespace stx {

namespace impl {
constexpr std::chrono::nanoseconds clamped_exponential_backoff(
    uint64_t iteration, std::chrono::nanoseconds max_wait_time) {
  constexpr auto i64_min_cmp = [](int64_t a, int64_t b) {
    return a < b ? a : b;
  };

  uint64_t time_u64 =
      static_cast<uint64_t>(1) << std::clamp<uint64_t>(
          iteration, 0,
          62 /* 62 so we can leave the sign bit when casting to i64 */);
  int64_t time =
      i64_min_cmp(static_cast<int64_t>(time_u64), max_wait_time.count());

  return std::chrono::nanoseconds{time};
}
}  // namespace impl

enum class LockResult { Success, Timeout };

struct ResponsiveSpinLock {
  STX_MAKE_PINNED(ResponsiveSpinLock)

  ResponsiveSpinLock() : lock_status{LockStatus::Unlocked} {}

  LockResult lock(std::chrono::nanoseconds lock_timeout) {
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
}  // namespace stx