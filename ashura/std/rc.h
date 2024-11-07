/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief A reference-counted resource handle
///
/// Requirements
/// ===========
/// - non-type-centric custom callback for uninitializing resources
/// - non-memory resources, i.e. devices
/// - intrusive and extrusive reference counting
///
/// @tparam H : handle type
template <typename H>
struct [[nodiscard]] Rc
{
  typedef H                          Handle;
  typedef Fn<void(H, AllocatorImpl)> Uninit;

  struct Inner
  {
    H             handle      = {};
    AliasCount   *alias_count = nullptr;
    AllocatorImpl allocator   = default_allocator;
    Uninit        uninit      = fn([](H, AllocatorImpl) {});
  };

  Inner inner{};

  void init(H handle, AliasCount &alias_count, AllocatorImpl allocator,
            Uninit uninit)
  {
    inner = Inner{.handle      = handle,
                  .alias_count = &alias_count,
                  .allocator   = allocator,
                  .uninit      = uninit};
  }

  constexpr void uninit() const
  {
    if (inner.alias_count->unalias())
    {
      inner.uninit(inner.handle, inner.allocator);
    }
  }

  constexpr usize num_aliases() const
  {
    return inner.alias_count->count();
  }

  constexpr Rc<H> alias() const
  {
    inner.alias_count->alias();
    return *this;
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

template <typename T>
struct AliasCounted : AliasCount
{
  T data;
};

template <typename T, typename... Args>
Result<Rc<T *>, Void> rc_inplace(AllocatorImpl allocator, Args &&...args)
{
  AliasCounted<T> *object;

  if (!allocator.nalloc(1, object))
  {
    return Err{Void{}};
  }

  new (object) AliasCounted<T>{.data{((Args &&) args)...}};

  return Ok{Rc<T *>{.inner{.handle      = &object->data,
                           .alias_count = static_cast<AliasCount *>(object),
                           .allocator   = allocator,
                           .uninit = fn(object, [](AliasCounted<T> *object, T *,
                                                   AllocatorImpl    allocator) {
                             object->~AliasCounted<T>();
                             allocator.ndealloc(object, 1);
                           })}}};
}

template <typename T>
Result<Rc<T *>, Void> rc(AllocatorImpl allocator, T &&object)
{
  return rc_inplace<T>(allocator, (T &&) object);
}

}        // namespace ash
