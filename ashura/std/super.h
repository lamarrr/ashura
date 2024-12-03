/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

static constexpr usize DEFAULT_SUPER_ALIGNMENT = 32;

static constexpr usize DEFAULT_SUPER_CAPACITY = 48;

template <typename Base, usize Alignment = DEFAULT_SUPER_ALIGNMENT,
          usize Capacity = DEFAULT_SUPER_CAPACITY>
requires (Alignment >= alignof(Base) && Capacity >= sizeof(Base))
struct Super
{
  static constexpr usize ALIGNMENT = Alignment;

  static constexpr usize CAPACITY = Capacity;

  using Lifecycle = void (*)(void *, void *, Base **);

  /// @brief An object lifecycle function that relocates and destroys an object.
  /// When the destination memory is nullptr, the object is to be destroyed.
  /// Otherwise, it should relocate itself to the destination memory and adjust the base pointer.
  template <typename T>
  static constexpr auto LIFECYCLE =
      [](void * src_mem, void * dst_mem, Base ** base_ptr) {
        T * src = reinterpret_cast<T *>(src_mem);
        T * dst = reinterpret_cast<T *>(dst_mem);

        if (dst_mem == nullptr) [[unlikely]]
        {
          src->~T();
        }
        else
        {
          obj::relocate_non_overlapping(Span{src, 1}, dst);
          *base_ptr = dst;
        }
      };

  alignas(ALIGNMENT) mutable u8 storage[CAPACITY];

  Base * base_ptr;

  Lifecycle lifecycle;

  template <typename Object>
  requires (Derives<Super, Object> && ALIGNMENT >= alignof(Object) &&
            CAPACITY >= sizeof(Object))
  constexpr Super(Object object, Lifecycle lifecycle = LIFECYCLE<Object>) :
      lifecycle{lifecycle}
  {
    base_ptr = new (storage) Object{static_cast<Object &&>(object)};
  }

  constexpr Super(Super const &) = delete;

  constexpr Super & operator=(Super const &) = delete;

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Super(Super<Base, SrcAlignment, SrcCapacity> && other) :
      lifecycle{other.lifecycle}
  {
    other.lifecycle(other.storage, storage, &base_ptr);
    other.lifecycle = noop;
    other.base_ptr  = nullptr;
  }

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Super & operator=(Super<Base, SrcAlignment, SrcCapacity> && other)
  {
    if constexpr (ALIGNMENT == SrcAlignment && CAPACITY == SrcCapacity)
    {
      if (this == &other) [[unlikely]]
      {
        return *this;
      }
    }

    lifecycle(storage, nullptr, nullptr);
    other.lifecycle(other.storage, storage, &base_ptr);
    lifecycle       = other.lifecycle;
    other.lifecycle = noop;
    other.base_ptr  = nullptr;

    return *this;
  }

  constexpr operator Base &() const
  {
    return *base_ptr;
  }

  constexpr Base & get() const
  {
    return *base_ptr;
  }

  constexpr ~Super()
  {
    lifecycle(storage, nullptr, nullptr);
  }
};

}        // namespace ash
