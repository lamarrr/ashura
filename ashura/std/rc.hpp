/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/alias_count.hpp"
#include "ashura/std/allocator.hpp"
#include "ashura/std/result.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

enum class RcAction : u8
{
    GetCount = 0,
    Alias    = 1,
    Unalias  = 2
};

typedef Fn<usize(RcAction)> RcOp;

/// @param allocator the allocator used to allocate the object
/// @param action       the reference counting action to perform
/// @returns the previous alias count
static constexpr usize rc_noop(RcAction action)
{
    (void) action;
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
struct [[nodiscard]] Rc
{
    typedef H Handle;

    H    handle_;
    RcOp rc_;

    constexpr Rc(H handle, RcOp rc) : handle_{Consume<H>::consume(handle)}, rc_{rc}
    {
    }

    constexpr Rc(Rc const &) = delete;

    constexpr Rc & operator=(Rc const &) = delete;

    constexpr Rc(Rc && other) :
      handle_{Consume<H>::consume(other.handle_)},
      rc_{other.rc_}
    {
        other.rc_ = rc_noop;
    }

    constexpr Rc & operator=(Rc && other)
    {
        if (this == &other) [[unlikely]]
        {
            return *this;
        }

        this->~Rc();
        new (this) Rc{static_cast<Rc &&>(other)};
        return *this;
    }

    constexpr ~Rc()
    {
        uninit();
    }

    constexpr void uninit() const
    {
        rc_(RcAction::Unalias);
    }

    constexpr Rc alias() const
    {
        rc_(RcAction::Alias);
        return Rc{handle_, rc_};
    }

    constexpr usize num_aliases() const
    {
        return rc_(RcAction::GetCount);
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
        return handle_(static_cast<Args &&>(args)...);
    }

    constexpr H operator->() const
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

    static constexpr usize rc_op(Allocated<RcObject> * p, RcAction action)
    {
        switch (action)
        {
            case RcAction::GetCount:
            {
                return p->v.alias_count.count();
            }
            case RcAction::Alias:
            {
                return p->v.alias_count.alias();
            }
            case RcAction::Unalias:
            {
                auto old = p->v.alias_count.unalias();
                if (old == 0)
                {
                    auto allocator = p->allocator;
                    p->~Allocated<RcObject>();
                    allocator->ndealloc(1, p);
                }
                return old;
            }
            default:
                ASH_UNREACHABLE;
        }
    }
};

template <typename T, typename... Args>
constexpr Result<Rc<T *>, Void> rc(Inplace, Allocator allocator, Args &&... args)
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
    Rc<H> t{static_cast<H &&>(handle), base.rc_};
    (void) Consume<Base>::consume(base.handle_);
    base.rc_ = rc_noop;
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
