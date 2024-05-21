#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{

/// @capacity: must be a non-zero power of 2
template <typename T>
struct SPSCRingBuffer
{
  alignas(CACHELINE_ALIGNMENT) std::atomic<usize> produce_next_ = 0;
  alignas(CACHELINE_ALIGNMENT) std::atomic<usize> consume_next_ = 0;

  T    *buffer_   = nullptr;
  usize capacity_ = 0;

  usize capacity() const
  {
    return capacity_;
  }

  bool try_consume(T *out)
  {
    usize const p_idx = produce_next_.load(std::memory_order_relaxed);
    usize       c_idx = consume_next_.load(std::memory_order_relaxed);
    if (p_idx == c_idx)
    {
      return false;
    }

    (void) produce_next_.load(std::memory_order_acquire);

    mem::copy(buffer_ + c_idx, out, 1);

    c_idx = (c_idx + 1) & (capacity_ - 1);

    consume_next_.store(c_idx, std::memory_order_relaxed);

    return true;
  }

  bool try_produce(T const &in)
  {
    usize       p_idx = produce_next_.load(std::memory_order_relaxed);
    usize const c_idx = consume_next_.load(std::memory_order_relaxed);
    if (p_idx == c_idx)
    {
      return false;
    }

    mem::copy(&in, buffer_ + p_idx, 1);

    p_idx = (p_idx + 1) & (capacity_ - 1);

    produce_next_.store(p_idx, std::memory_order_release);

    return true;
  }
};
}        // namespace ash