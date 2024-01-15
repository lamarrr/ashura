#pragma once
#include "ashura/allocator.h"
#include "ashura/integrals.h"

namespace ash
{

// only for purely trivial types
// TODO(lamarrr): write a C-based implementation
template <typename T>
struct TrivialVec
{
  T    *data     = nullptr;
  usize size     = 0;
  usize capacity = 0;

  [[nodiscard]] bool reserve(AllocatorImpl allocator, usize target_size)
  {
    if (target_size <= capacity)
    {
      return true;
    }
    usize const target_capacity = target_size + (target_size >> 1);
    T *new_data = (T *) allocator.reallocate(data, sizeof(T) * target_capacity,
                                             alignof(T));
    if (new_data == nullptr)
    {
      return false;
    }
    data     = new_data;
    capacity = target_capacity;
    return true;
  }

  [[nodiscard]] bool grow_size(AllocatorImpl allocator, usize growth)
  {
    if (!reserve(allocator, size + growth))
    {
      return false;
    }
    size += growth;
    return true;
  }

  void fill(T const &element, usize begin, usize num)
  {
    for (usize i = begin; i < begin + num && i < size; i++)
    {
      data[i] = element;
    }
  }

  [[nodiscard]] bool push(AllocatorImpl allocator, T const &element)
  {
    if (!reserve(allocator, size + 1))
    {
      return false;
    }
    data[size] = element;
    size++;

    return true;
  }

  void clear()
  {
    size = 0;
  }

  void deallocate(AllocatorImpl allocator)
  {
    allocator.deallocate(data);
    data     = nullptr;
    size     = 0;
    capacity = 0;
  }

  T *begin()
  {
    return data;
  }

  T *end()
  {
    return data + size;
  }

  constexpr T &operator[](usize i) const
  {
    return data[i];
  }
};

}        // namespace ash
