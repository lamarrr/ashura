/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/log.h"
#include "ashura/std/obj.h"
#include "ashura/std/types.h"

namespace ash
{

static constexpr usize DEFAULT_SUPER_ALIGNMENT = 32;
static constexpr usize DEFAULT_SUPER_CAPACITY  = 48;

template <typename Base, usize Alignment = DEFAULT_SUPER_ALIGNMENT,
          usize Capacity = DEFAULT_SUPER_CAPACITY>
requires (Alignment >= alignof(Base) && Capacity >= sizeof(Base))
struct Super
{
  static constexpr usize ALIGNMENT = Alignment;
  static constexpr usize CAPACITY  = Capacity;

  using Lifecycle = PFnLifecycle;
  using Slicer    = Base * (*) (void *);

  static Base * noop_slicer(void *)
  {
    logger->panic("Tried to slice an empty Super type");
  }

  alignas(ALIGNMENT) mutable u8 storage[CAPACITY];

  Slicer slicer = noop_slicer;

  Lifecycle lifecycle = noop;

  explicit constexpr Super() = default;

  template <typename Object>
  requires (Derives<Super, Object> && ALIGNMENT >= alignof(Object) &&
            CAPACITY >= sizeof(Object))
  constexpr Super(Object && object) :
      slicer{+[](void * storage) -> Base * {
        Object * ptr = reinterpret_cast<Object *>(storage);
        return ptr;
      }},
      lifecycle{pFn_LIFECYCLE<Object>}
  {
    new (storage) Object{static_cast<Object &&>(object)};
  }

  constexpr Super(Super const &) = delete;

  constexpr Super & operator=(Super const &) = delete;

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Super(Super<Base, SrcAlignment, SrcCapacity> && other) :
      slicer{other.slicer},
      lifecycle{other.lifecycle}
  {
    other.lifecycle(other.storage, storage);
    other.lifecycle = noop;
    other.slicer    = noop_slicer;
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

    lifecycle(storage, nullptr);
    other.lifecycle(other.storage, storage);
    lifecycle       = other.lifecycle;
    other.lifecycle = noop;
    slicer          = other.slicer;
    other.slicer    = noop_slicer;

    return *this;
  }

  constexpr operator Base &() const
  {
    return get();
  }

  constexpr Base & get() const
  {
    return *slicer(storage);
  }

  constexpr ~Super()
  {
    lifecycle(storage, nullptr);
  }
};

}        // namespace ash
