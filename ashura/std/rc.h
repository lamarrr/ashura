/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/alias_count.h"
#include "ashura/std/allocator.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

enum class RcOp : u8
{
  GetCount = 0,
  Alias    = 1,
  Unalias  = 2
};

typedef Fn<usize(RcOp)> AliasOp;

/// @param allocator the allocator used to allocate the object
/// @param direction the reference operation, 0 (get ref count), 1 (increase ref count), -1 (decrease ref count)
/// @returns the previous alias count
static constexpr usize rc_noop(RcOp op)
{
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

  H       handle_;
  AliasOp alias_;

  constexpr Rc(H handle, AliasOp alias) : handle_{handle}, alias_{alias}
  {
  }

  explicit constexpr Rc() : handle_{}, alias_{rc_noop}
  {
  }

  constexpr Rc(Rc const &) = delete;

  constexpr Rc & operator=(Rc const &) = delete;

  constexpr Rc(Rc && other) : handle_{other.handle_}, alias_{other.alias_}
  {
    other.handle_ = H{};
    other.alias_  = rc_noop;
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

 constexpr ~Rc()
  {
    uninit();
  }

  constexpr void uninit() const
  {
    alias_(RcOp::Unalias);
  }

  constexpr void reset()
  {
    uninit();
    *this = Rc{};
  }

  constexpr Rc alias() const
  {
    alias_(RcOp::Alias);
    return Rc{handle_, alias_};
  }

  constexpr usize num_aliases() const
  {
    return alias_(RcOp::GetCount);
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
  static constexpr bool value = TriviallyRelocatable<H>;
};

template <typename T>
struct RcObject
{
  AtomicAliasCount alias_count;
  T                v;

  template <typename... Args>
  constexpr RcObject(Args &&... args) :
    alias_count{},
    v{static_cast<Args &&>(args)...}
  {
  }

  static constexpr usize rc_op(Allocated<RcObject> * p, RcOp op)
  {
    switch (op)
    {
      case RcOp::GetCount:
      {
        return p->v.alias_count.count();
      }
      case RcOp::Alias:
      {
        return p->v.alias_count.alias();
      }
      case RcOp::Unalias:
      {
        auto const old = p->v.alias_count.unalias();
        if (old == 0)
        {
          p->dealloc();
        }
        return old;
      }
      default:
        ASH_UNREACHABLE;
    }
  }
};

template <typename T, typename... Args>
constexpr Result<Rc<T *>, Void> rc(Inplace, Allocator allocator,
                                   Args &&... args)
{
  Allocated<RcObject<T>> * p;

  if (!allocator->nalloc(1, p))
  {
    return Err{Void{}};
  }

  new (p) Allocated<RcObject<T>>{allocator, static_cast<Args &&>(args)...};

  return Ok{
    Rc<T *>{&p->v.v, Fn(p, RcObject<T>::rc_op)}
  };
}

template <typename T>
constexpr Result<Rc<T *>, Void> rc(Allocator allocator, T object)
{
  return rc<T>(inplace, allocator, static_cast<T &&>(object));
}

template <typename Base, typename H>
constexpr Rc<H> transmute(Rc<Base> base, H handle)
{
  Rc<H> t{static_cast<H &&>(handle), base.alias_};
  base.handle_ = {};
  base.alias_  = rc_noop;
  return t;
}

template <typename To, typename From>
constexpr Rc<To> cast(Rc<From> from)
{
  return transmute((Rc<From> &&) from, static_cast<To>(from.get()));
}

template <typename Handle>
constexpr Rc<Handle> static_rc(Handle handle)
{
  return Rc<Handle>{handle, rc_noop};
}

using RcBlob8  = Rc<Span<u8 const>>;
using RcBlob16 = Rc<Span<u16 const>>;
using RcBlob32 = Rc<Span<u32 const>>;
using RcBlob64 = Rc<Span<u64 const>>;

}    // namespace ash
