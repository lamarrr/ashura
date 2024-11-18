/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename H>
struct [[nodiscard]] Dyn
{
  typedef H                       Handle;
  typedef Fn<void(AllocatorImpl)> Uninit;

  struct Inner
  {
    H             handle    = {};
    AllocatorImpl allocator = {};
    Uninit        uninit    = noop;
  };

  Inner inner{};

  constexpr Dyn(H handle, AllocatorImpl allocator, Uninit uninit) :
      inner{.handle = handle, .allocator = allocator, .uninit = uninit}
  {
  }

  explicit constexpr Dyn() = default;

  constexpr Dyn(Dyn const &) = delete;

  constexpr Dyn &operator=(Dyn const &) = delete;

  constexpr Dyn(Dyn &&other) : inner{other.inner}
  {
    other.inner = Inner{};
  }

  constexpr Dyn &operator=(Dyn &&other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    uninit();
    new (this) Dyn{(Dyn &&) other};
  }

  constexpr ~Dyn()
  {
    uninit();
  }

  constexpr void uninit() const
  {
    inner.uninit(inner.allocator);
  }

  constexpr void reset()
  {
    uninit();
    inner = Inner{};
  }

  constexpr H get() const
  {
    return inner.handle;
  }

  constexpr decltype(auto) operator*() const
  {
    return *inner.handle;
  }

  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&...args) const
  {
    return inner.handle(static_cast<Args>(args)...);
  }

  constexpr H operator->() const
  {
    return inner.handle;
  }
};

template <typename T, typename... Args>
Result<Dyn<T *>, Void> dyn_inplace(AllocatorImpl allocator, Args &&...args)
{
  T *object;
  if (!allocator.nalloc(1, object)) [[unlikely]]
  {
    return Err{Void{}};
  }

  new (object) T{((Args &&) args)...};

  return Ok{Dyn<T *>{object, allocator,
                     fn(object, [](T *object, AllocatorImpl allocator) {
                       object->~T();
                       allocator.ndealloc(object, 1);
                     })}};
}

template <typename T>
Result<Dyn<T *>, Void> dyn(AllocatorImpl allocator, T object)
{
  return dyn_inplace<T>(allocator, (T &&) object);
}

template <typename Base, typename H>
Dyn<H> transmute(Dyn<Base> &&base, H handle)
{
  Dyn<H> t{(H &&) handle, base.inner.allocator, base.inner.uninit};
  base.inner.handle    = {};
  base.inner.allocator = noop_allocator;
  base.inner.uninit    = noop;
  return t;
}

}        // namespace ash
