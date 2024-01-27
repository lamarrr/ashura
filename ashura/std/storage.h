#pragma once
#include "ashura/std/types.h"

namespace ash
{

template <typename T, usize Capacity = 1>
struct Storage
{
  static_assert(Capacity > 0);
  static constexpr usize capacity = Capacity;
  alignas(T) u8 rep[sizeof(T) * Capacity];

  operator T *()
  {
    return reinterpret_cast<T *>(rep);
  }

  operator T const *() const
  {
    return reinterpret_cast<T const *>(rep);
  }

  operator void *()
  {
    return rep;
  }

  operator void const *() const
  {
    return rep;
  }
};

template <usize Alignment, usize ByteCapacity>
struct UntypedStorage
{
  static_assert(ByteCapacity > 0);
  static_assert(ByteCapacity % Alignment == 0);
  static constexpr usize byte_capacity = ByteCapacity;

  template <typename T>
  static constexpr usize capacity = ByteCapacity / sizeof(T);

  alignas(Alignment) u8 rep[ByteCapacity];

  template <typename T>
  operator T *()
  {
    static_assert(Alignment % alignof(T) == 0);
    static_assert(ByteCapacity / sizeof(T) > 0);
    return reinterpret_cast<T *>(rep);
  }

  template <typename T>
  operator T const *() const
  {
    static_assert(Alignment % alignof(T) == 0);
    static_assert(ByteCapacity / sizeof(T) > 0);
    return reinterpret_cast<T const *>(rep);
  }

  operator void *()
  {
    return rep;
  }

  operator void const *() const
  {
    return rep;
  }
};

}        // namespace ash
