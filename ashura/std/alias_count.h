
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{

/// @struct Alias counting/tracking, similar to reference counting but 0-based.
///
/// having access to and being able to reference this struct already implies a
/// reference count of 1, meaning total refs = 1 + num_other_aliases.
///
/// this requires that the accessing scope knows it has access to it or not and
/// can in some cases be statically checked.
///
/// the reference count is initialized by default. 0-based means initialization
/// can be a memset if lock-free.
///
/// NOTE: just like reference-counting, this only guarantees synchronization of
/// the operation it represents and instruction ordering relative to it.
///
/// it uses relaxed operations in situations where it has exclusive access to
/// the resource.
///
/// References
/// ===========
///
/// https://lwn.net/Articles/693038/
///
struct alias_count
{
  /// range: [0, USIZE_MAX], overflow is well-checked.
  std::atomic<usize> num_others_{0};

  void acquire()
  {
    do
    {
      usize expected = 0;
      if (num_others_.compare_exchange_strong(expected, 1,
                                              std::memory_order_relaxed,
                                              std::memory_order_relaxed))
      {
        return;
      }
    } while (false);
    usize const old = num_others_.fetch_add(1, std::memory_order_acquire);
    // overflow check, transition from MAX_ALIAS_COUNT -> 0, is illegal
    CHECK(old < USIZE_MAX);
  }

  /// @brief returns true if it is the exclusive owner of the shared object.
  /// subsequent calls to release() after it returns true will still return
  /// true.
  ///
  /// WARNING: if accompanied by a destructive reclamantion procedure, it will
  /// lead to a double-release (i.e. double-free) if `release` is called again
  /// after num_others reaches 0. alternatively, the scope could have an atomic
  /// to check for free-s.
  bool release()
  {
    if (num_others_.load(std::memory_order_relaxed) == 0)
    {
      return false;
    }
    num_others_.fetch_sub(1, std::memory_order_release);
    return false;
  }
};

}        // namespace ash