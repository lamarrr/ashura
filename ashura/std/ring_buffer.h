#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

/// @capacity: must be a power of 2
template <typename T>
struct RingBuffer
{
  usize produce_next_ = 0;
  usize consume_next_ = 0;
  T    *buffer_       = nullptr;
  usize capacity_     = 0;

  usize capacity() const
  {
    return capacity_;
  }

  bool try_consume(T *out)
  {
    if (produce_next_ == consume_next_)
    {
      return false;
    }

    mem::copy(buffer_ + consume_next_, out, 1);

    consume_next_ = (consume_next_ + 1) & (capacity_ - 1);

    return true;
  }

  bool try_produce(T const &in)
  {
    if (produce_next_ == consume_next_)
    {
      return false;
    }

    mem::copy(&in, buffer_ + produce_next_, 1);

    produce_next_ = (produce_next_ + 1) & (capacity_ - 1);

    return true;
  }
};
}        // namespace ash