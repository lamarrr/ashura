/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

typedef Fn<void(AllocatorImpl)> DynUninit;

template <typename H>
requires (TriviallyCopyable<H>)
struct [[nodiscard]] Dyn
{
  typedef H Handle;

  H             handle_;
  AllocatorImpl allocator_;
  DynUninit     uninit_;

  constexpr Dyn(H handle, AllocatorImpl allocator, DynUninit uninit) :
    handle_{handle},
    allocator_{allocator},
    uninit_{uninit}
  {
  }

  explicit constexpr Dyn() : handle_{}, allocator_{}, uninit_{noop}
  {
  }

  constexpr Dyn(Dyn const &) = delete;

  constexpr Dyn & operator=(Dyn const &) = delete;

  constexpr Dyn(Dyn && other) :
    handle_{other.handle_},
    allocator_{other.allocator_},
    uninit_{other.uninit_}
  {
    other.handle_    = H{};
    other.allocator_ = AllocatorImpl{};
    other.uninit_    = noop;
  }

  constexpr Dyn & operator=(Dyn && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    uninit();
    new (this) Dyn{static_cast<Dyn &&>(other)};
    return *this;
  }

  constexpr ~Dyn()
  {
    uninit();
  }

  constexpr void uninit() const
  {
    uninit_(allocator_);
  }

  constexpr void reset()
  {
    uninit();
    *this = Dyn{};
  }

  constexpr H get() const
  {
    return handle_;
  }

  constexpr decltype(auto) operator*() const
  {
    return *handle_;
  }

  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&... args) const
  {
    return handle_(static_cast<Args>(args)...);
  }

  constexpr H operator->() const
  {
    return handle_;
  }
};

template <typename T, typename... Args>
constexpr Result<Dyn<T *>, Void> dyn_inplace(AllocatorImpl allocator,
                                             Args &&... args)
{
  T * object;
  if (!allocator.nalloc(1, object)) [[unlikely]]
  {
    return Err{Void{}};
  }

  constexpr auto uninit = +[](T * object, AllocatorImpl allocator) {
    obj::destruct(Span{object, 1});
    allocator.ndealloc(object, 1);
  };

  new (object) T{static_cast<Args &&>(args)...};

  return Ok{
    Dyn<T *>{object, allocator, fn(object, uninit)}
  };
}

template <typename H>
struct IsTriviallyRelocatable<Dyn<H>>
{
  static constexpr bool value = true;
};

template <typename T>
constexpr Result<Dyn<T *>, Void> dyn(AllocatorImpl allocator, T object)
{
  return dyn_inplace<T>(allocator, static_cast<T &&>(object));
}

template <typename Base, typename H>
constexpr Dyn<H> transmute(Dyn<Base> base, H handle)
{
  Dyn<H> t{static_cast<H &&>(handle), base.allocator_, base.uninit_};
  base.handle_    = {};
  base.allocator_ = noop_allocator;
  base.uninit_    = noop;
  return t;
}

template <typename To, typename From>
constexpr Dyn<To> cast(Dyn<From> from)
{
  return transmute(static_cast<Dyn<From> &&>(from),
                   static_cast<To>(from.get()));
}

}    // namespace ash
