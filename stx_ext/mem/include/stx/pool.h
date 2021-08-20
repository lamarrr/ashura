#pragma once

#include <cinttypes>

// needed since the app has a lot of allocations.
// we don't care about the types, we just want fine cacheline patterns
//
struct AnyPool {
  // trivially destructible blocks
  // non-trivially destructible blocks
  explicit AnyPool(uint64_t);
  AnyPool(AnyPool const&) = delete;
  AnyPool& operator=(AnyPool const&) = delete;

  uint64_t chunk_size_;
};
