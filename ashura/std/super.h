/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

inline constexpr usize DEFAULT_SUPER_ALIGNMENT = 32;

inline constexpr usize DEFAULT_SUPER_CAPACITY = 48;

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
          obj::destruct(Span{src, 1});
        }
        else
        {
          obj::relocate_nonoverlapping(Span{src, 1}, dst);
          *base_ptr = dst;
        }
      };

  alignas(ALIGNMENT) mutable u8 storage_[CAPACITY];

  Base * base_;

  Lifecycle lifecycle_;

  template <typename Object>
  requires (Derives<Object, Base> && ALIGNMENT >= alignof(Object) &&
            CAPACITY >= sizeof(Object))
  constexpr Super(Object object, Lifecycle lifecycle = LIFECYCLE<Object>) :
      lifecycle_{lifecycle}
  {
    base_ = new (storage_) Object{static_cast<Object &&>(object)};
  }

  constexpr Super(Super const &) = delete;

  constexpr Super & operator=(Super const &) = delete;

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Super(Super<Base, SrcAlignment, SrcCapacity> && other) :
      lifecycle_{other.lifecycle_}
  {
    other.lifecycle_(other.storage_, storage_, &base_);
    other.lifecycle_ = noop;
    other.base_      = nullptr;
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

    lifecycle_(storage_, nullptr, nullptr);
    other.lifecycle_(other.storage_, storage_, &base_);
    lifecycle_       = other.lifecycle_;
    other.lifecycle_ = noop;
    other.base_      = nullptr;

    return *this;
  }

  constexpr operator Base &() const
  {
    return *base_;
  }

  constexpr Base & get() const
  {
    return *base_;
  }

  constexpr ~Super()
  {
    lifecycle_(storage_, nullptr, nullptr);
  }
};

}        // namespace ash
