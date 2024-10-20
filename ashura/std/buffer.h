/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{

template <typename T>
struct Buffer
{
  T    *data_     = nullptr;
  usize capacity_ = 0;
  usize size_     = 0;

  constexpr T *data() const
  {
    return data_;
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr bool extend(Span<T const> in)
  {
    if ((size_ + in.size()) > capacity_)
    {
      return false;
    }

    copy(in, Span{data_ + size_, in.size()});

    size_ += in.size();
    return true;
  }
};

template <typename T>
constexpr Buffer<T> buffer(Span<T> span)
{
  return Buffer<T>{.data_ = span.data(), .capacity_ = span.size(), .size_ = 0};
}

template <typename T>
constexpr Span<T> span(Buffer<T> buffer)
{
  return Span<T>{.data_ = buffer.data(), .size_ = buffer.size()};
}

/// @capacity: must be a non-zero power of 2
template <typename T>
struct RingBuffer
{
  T    *data_         = nullptr;
  usize capacity_     = 0;
  usize size_         = 0;
  usize consume_next_ = 0;

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr bool try_consume(T *out)
  {
    if (size_ == 0)
    {
      return false;
    }

    mem::copy(data_ + consume_next_, out, 1);

    consume_next_ = (consume_next_ + 1) & (capacity_ - 1);
    size_--;

    return true;
  }

  constexpr bool try_produce(T const &in)
  {
    if (size_ == capacity_)
    {
      return false;
    }

    usize const produce_next = (consume_next_ + size_) & (capacity_ - 1);
    mem::copy(&in, data_ + produce_next, 1);

    size_++;

    return true;
  }
};

/// @param capacity must be a non-zero power of 2
template <typename T>
struct SPSCRingBuffer
{
  alignas(CACHELINE_ALIGNMENT) usize produce_next_ = 0;
  alignas(CACHELINE_ALIGNMENT) usize consume_next_ = 0;

  T    *data_     = nullptr;
  usize capacity_ = 0;

  usize capacity() const
  {
    return capacity_;
  }

  bool try_consume(T *out)
  {
    std::atomic_ref p{produce_next_};
    std::atomic_ref c{consume_next_};
    usize const     p_idx = p.load(std::memory_order_relaxed);
    usize           c_idx = c.load(std::memory_order_relaxed);
    if (p_idx == c_idx)
    {
      return false;
    }

    (void) p.load(std::memory_order_acquire);

    mem::copy(data_ + c_idx, out, 1);

    c_idx = (c_idx + 1) & (capacity_ - 1);

    c.store(c_idx, std::memory_order_relaxed);

    return true;
  }

  bool try_produce(T const &in)
  {
    std::atomic_ref p{produce_next_};
    std::atomic_ref c{consume_next_};
    usize           p_idx = p.load(std::memory_order_relaxed);
    usize const     c_idx = c.load(std::memory_order_relaxed);
    if (p_idx == c_idx)
    {
      return false;
    }

    mem::copy(&in, data_ + p_idx, 1);

    p_idx = (p_idx + 1) & (capacity_ - 1);

    p.store(p_idx, std::memory_order_release);

    return true;
  }
};

}        // namespace ash