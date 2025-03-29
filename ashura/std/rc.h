/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

typedef Fn<usize(AllocatorRef, i32)> AliasOp;

/// @param allocator the allocator used to allocate the object
/// @param direction the reference operation, 0 (get ref count), 1 (increase ref count), -1 (decrease ref count)
/// @returns the previous alias count
static constexpr usize rc_noop(AllocatorRef allocator, i32 op)
{
  (void) allocator;
  (void) op;
  return 0;
}

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
requires (TriviallyCopyable<H>)
struct [[nodiscard]] Rc
{
  typedef H Handle;

  H            handle_;
  AllocatorRef allocator_;
  AliasOp      alias_;

  constexpr Rc(H handle, AllocatorRef allocator, AliasOp alias) :
    handle_{handle},
    allocator_{allocator},
    alias_{alias}
  {
  }

  explicit constexpr Rc() :
    handle_{},
    allocator_{noop_allocator},
    alias_{rc_noop}
  {
  }

  constexpr Rc(Rc const &) = delete;

  constexpr Rc & operator=(Rc const &) = delete;

  constexpr Rc(Rc && other) :
    handle_{other.handle_},
    allocator_{other.allocator_},
    alias_{other.alias_}
  {
    other.handle_    = H{};
    other.allocator_ = noop_allocator;
    other.alias_     = rc_noop;
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
    alias_(allocator_, -1);
  }

  constexpr void reset()
  {
    uninit();
    *this = Rc{};
  }

  constexpr Rc alias() const
  {
    alias_(allocator_, 1);
    return Rc{handle_, allocator_, alias_};
  }

  constexpr usize num_aliases() const
  {
    return alias_(allocator_, 0);
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
    return handle_(forward<Args>(args)...);
  }

  constexpr H operator->() const
  {
    return handle_;
  }

  constexpr operator H() const
  {
    return handle_;
  }
};

template <typename H>
struct IsTriviallyRelocatable<Rc<H>>
{
  static constexpr bool value = true;
};

template <typename T>
struct RcObject
{
  AliasCount alias_count{};
  T          v0;

  static constexpr usize rc_op(RcObject * obj, AllocatorRef allocator, i32 op)
  {
    switch (op)
    {
      case 0:
      {
        return obj->alias_count.count();
      }
      case -1:
      {
        usize const old = obj->alias_count.unalias();
        if (old == 0)
        {
          obj->~RcObject();
          allocator->ndealloc(1, obj);
        }
        return old;
      }
      case 1:
      {
        return obj->alias_count.alias();
      }
      default:
        ASH_UNREACHABLE;
    }
  }
};

template <typename T, typename... Args>
constexpr Result<Rc<T *>, Void> rc(Inplace, AllocatorRef allocator,
                                   Args &&... args)
{
  RcObject<T> * obj;

  if (!allocator->nalloc(1, obj))
  {
    return Err{Void{}};
  }

  new (obj) RcObject<T>{.v0{static_cast<Args &&>(args)...}};

  return Ok{
    Rc<T *>{&obj->v0, allocator, Fn(obj, RcObject<T>::rc_op)}
  };
}

template <typename T>
constexpr Result<Rc<T *>, Void> rc(AllocatorRef allocator, T object)
{
  return rc<T>(inplace, allocator, static_cast<T &&>(object));
}

template <typename Base, typename H>
constexpr Rc<H> transmute(Rc<Base> base, H handle)
{
  Rc<H> t{static_cast<H &&>(handle), base.allocator_, base.alias_};
  base.handle_    = {};
  base.allocator_ = noop_allocator;
  base.alias_     = rc_noop;
  return t;
}

template <typename To, typename From>
constexpr Rc<To> cast(Rc<From> from)
{
  return transmute((Rc<From> &&) from, static_cast<To>(from.get()));
}

}    // namespace ash
