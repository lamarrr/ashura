/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename H>
struct Unique
{
  typedef H                          Handle;
  typedef Fn<void(H, AllocatorImpl)> Uninit;

  struct Inner
  {
    H             handle    = {};
    AllocatorImpl allocator = default_allocator;
    Uninit        uninit    = fn([](H, AllocatorImpl) {});
  };

  Inner inner{};

  void init(H handle, AllocatorImpl allocator, Uninit uninit)
  {
    inner = Inner{.handle = handle, .allocator = allocator, .uninit = uninit};
  }

  constexpr void uninit() const
  {
    inner.uninit(inner.handle, inner.allocator);
  }

  constexpr H get() const
  {
    return inner.handle;
  }

  constexpr auto &operator*() const
  {
    return *inner.handle;
  }

  constexpr H operator->() const
  {
    return inner.handle;
  }
};

template <typename T, typename... Args>
Result<Unique<T *>, Void> unique_inplace(AllocatorImpl allocator,
                                         Args &&...args)
{
  T *object;
  if (!allocator.nalloc(1, object))
  {
    return Err{Void{}};
  }

  new (object) T{((Args &&) args)...};

  return Ok{Unique<T *>{
      .inner = {.handle    = object,
                .allocator = allocator,
                .uninit =
                    fn(object, [](T *object, T *, AllocatorImpl allocator) {
                      object->~T();
                      allocator.ndealloc(object, 1);
                    })}}};
}

template <typename T>
Result<Unique<T *>, Void> unique(AllocatorImpl allocator, T &&object)
{
  return unique_inplace(allocator, (T &&) object);
}

}        // namespace ash
