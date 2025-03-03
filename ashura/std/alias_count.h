/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/error.h"
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{

/// @brief Alias counting/tracking, similar to reference counting but 0-based.
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
///
/// References
/// ===========
///
/// https://lwn.net/Articles/693038/
///
struct AliasCount
{
  /// @brief Guards against overflow.
  static constexpr usize MAX = USIZE_MAX - 1;

  /// @brief number of other aliases. range: [0, MAX], overflow is
  /// well-checked.
  usize count_{0};

  /// @brief called before sharing an object
  /// @returns returns the old alias count
  usize alias()
  {
    std::atomic_ref count{count_};
    usize           expected = 0;
    usize           desired  = 1;
    while (!count.compare_exchange_weak(
      expected, desired, std::memory_order_release, std::memory_order_relaxed))
    {
      CHECK(expected <= MAX, "");
      desired = expected + 1;
    }
    return expected;
  }

  /// @brief called when done with an object.
  /// @brief returns the old alias count. If 0 then the resource is ready to be released.
  ///
  /// WARNING: if accompanied by a destructive reclamation procedure and
  /// `unalias` is called again after it has already returned 0, it will lead
  /// to a double-release (i.e. double-free).
  [[nodiscard]] usize unalias()
  {
    usize           expected = 0;
    usize           desired  = 0;
    std::atomic_ref count{count_};
    while (!count.compare_exchange_weak(
      expected, desired, std::memory_order_release, std::memory_order_relaxed))
    {
      desired = max((usize) 1, expected) - 1;
    }
    return expected;
  }

  [[nodiscard]] usize count()
  {
    std::atomic_ref count{count_};
    return count.load(std::memory_order_relaxed);
  }
};

}    // namespace ash
