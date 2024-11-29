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
/// - support for non-memory resources, i.e. devices
/// - intrusive and extrusive reference counting
///
/// @tparam H : handle type
template <typename H>
struct [[nodiscard]] Rc
{
  typedef H                       Handle;
  typedef Fn<void(AllocatorImpl)> Uninit;

  struct Inner
  {
    H             handle      = {};
    AliasCount *  alias_count = nullptr;
    AllocatorImpl allocator   = {};
    Uninit        uninit      = noop;
  };

  Inner inner{};

  constexpr Rc(H handle, AliasCount & alias_count, AllocatorImpl allocator,
               Uninit uninit) :
      inner{.handle      = handle,
            .alias_count = &alias_count,
            .allocator   = allocator,
            .uninit      = uninit}
  {
  }

  explicit Rc() = default;

  constexpr Rc(Rc const &) = delete;

  constexpr Rc & operator=(Rc const &) = delete;

  constexpr Rc(Rc && other) : inner{other.inner}
  {
    other.inner = Inner{};
  }

  constexpr Rc & operator=(Rc && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    uninit();
    new (this) Rc{static_cast<Rc &&>(other)};
    return *this;
  }

  ~Rc()
  {
    uninit();
  }

  constexpr void uninit() const
  {
    if (inner.alias_count == nullptr)
    {
      return;
    }

    if (inner.alias_count->unalias())
    {
      inner.uninit(inner.allocator);
    }
  }

  constexpr void reset()
  {
    uninit();
    inner = Inner{};
  }

  bool is_valid() const
  {
    return inner.alias_count != nullptr;
  }

  constexpr usize num_aliases() const
  {
    return inner.alias_count->count();
  }

  constexpr Rc alias() const
  {
    inner.alias_count->alias();
    return Rc{inner.handle, *inner.alias_count, inner.allocator, inner.uninit};
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
  constexpr decltype(auto) operator()(Args &&... args) const
  {
    return inner.handle(forward<Args>(args)...);
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
constexpr Result<Rc<T *>, Void> rc_inplace(AllocatorImpl allocator,
                                           Args &&... args)
{
  AliasCounted<T> * object;

  if (!allocator.nalloc(1, object))
  {
    return Err{Void{}};
  }

  new (object) AliasCounted<T>{.data{static_cast<Args &&>(args)...}};

  return Ok{
      Rc<T *>{&object->data, *object, allocator,
              fn(object, [](AliasCounted<T> * object, AllocatorImpl allocator) {
                object->~AliasCounted<T>();
                allocator.ndealloc(object, 1);
              })}
  };
}

template <typename T>
constexpr Result<Rc<T *>, Void> rc(AllocatorImpl allocator, T object)
{
  return rc_inplace<T>(allocator, static_cast<T &&>(object));
}

template <typename Base, typename H>
constexpr Rc<H> transmute(Rc<Base> && base, H handle)
{
  Rc<H> t{static_cast<H &&>(handle, base.inner.allocator, base.inner.uninit)};
  base.inner.handle    = {};
  base.inner.allocator = noop_allocator;
  base.inner.uninit    = noop;
  return t;
}

template <typename To, typename From>
constexpr Rc<To> cast(Rc<From> && from)
{
  return transmute((Rc<From> &&) from, static_cast<To>(from.get()));
}

}        // namespace ash
