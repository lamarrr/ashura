/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/obj.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

inline constexpr usize DEFAULT_LAMBDA_ALIGNMENT = 32;

inline constexpr usize DEFAULT_LAMBDA_CAPACITY = 48;

template <typename Sig, usize Alignment = DEFAULT_LAMBDA_ALIGNMENT,
          usize Capacity = DEFAULT_LAMBDA_CAPACITY>
struct Lambda;

/// @brief In-Place/Stack-Allocated and Type-Erased Move-Only Function.
/// It only requires that the erased type be relocatable (moved and destroyed).
/// In order to prevent accessing elements from dynamic offsets, we require that the Type be placeable at the start of the storage.
///
/// x64 minimum size of Lambda = 3 Pointers = 24-bytes
/// x64 ideal configuration: Alignment = 32 bytes, Capacity = 48 bytes (Gives 64 bytes total. Fits exactly into one cacheline.
///
/// @tparam R return type
/// @tparam Args argument types
/// @tparam Alignment alignment of the internal storage in bytes
/// @tparam Capacity capacity of the internal storage in bytes
template <usize Alignment, usize Capacity, typename R, typename... Args>
requires (Alignment > 0 && Capacity > 0)
struct Lambda<R(Args...), Alignment, Capacity>
{
  static constexpr usize ALIGNMENT = Alignment;

  static constexpr usize CAPACITY = Capacity;

  using Thunk = R (*)(void *, Args...);

  /// @brief An object lifecycle function that relocates and destroys an object.
  /// When the destination memory is nullptr, the object is to be destroyed.
  /// Otherwise, it should relocate itself to the destination memory.
  using Lifecycle = void (*)(void *, void *);

  template <typename T>
  static constexpr auto LIFECYCLE = [](void * src_mem, void * dst_mem) {
    T * src = reinterpret_cast<T *>(src_mem);
    T * dst = reinterpret_cast<T *>(dst_mem);

    if (dst_mem == nullptr) [[unlikely]]
    {
      obj::destruct(Span{src, 1});
    }
    else
    {
      obj::relocate_nonoverlapping(Span{src, 1}, dst);
    }
  };

  alignas(ALIGNMENT) mutable u8 storage_[CAPACITY];

  Thunk thunk_;

  Lifecycle lifecycle_;

  template <typename Functor>
  requires (ALIGNMENT >= alignof(Functor) && CAPACITY >= sizeof(Functor) &&
            Callable<Functor, Args...> &&
            Convertible<CallResult<Functor, Args...>, R>)
  constexpr Lambda(Functor   functor,
                   Thunk     thunk = &FunctorThunk<Functor, R(Args...)>::thunk,
                   Lifecycle lifecycle = LIFECYCLE<Functor>) :
    thunk_{thunk},
    lifecycle_{lifecycle}
  {
    new (storage_) Functor{static_cast<Functor &&>(functor)};
  }

  constexpr Lambda(Lambda const &) = delete;

  constexpr Lambda & operator=(Lambda const &) = delete;

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Lambda(Lambda<R(Args...), SrcAlignment, SrcCapacity> && other) :
    thunk_{other.thunk_},
    lifecycle_{other.lifecycle_}
  {
    other.lifecycle_(other.storage_, storage_);
    other.lifecycle_ = noop;
    other.thunk_     = nullptr;
  }

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Lambda &
    operator=(Lambda<R(Args...), SrcAlignment, SrcCapacity> && other)
  {
    if constexpr (ALIGNMENT == SrcAlignment && CAPACITY == SrcCapacity)
    {
      if (this == &other) [[unlikely]]
      {
        return *this;
      }
    }

    lifecycle_(storage_, nullptr);
    other.lifecycle_(other.storage_, storage_);
    lifecycle_       = other.lifecycle_;
    other.lifecycle_ = noop;
    thunk_           = other.thunk_;
    other.thunk_     = nullptr;

    return *this;
  }

  constexpr ~Lambda()
  {
    lifecycle_(storage_, nullptr);
  }

  constexpr R operator()(Args... args) const
  {
    return thunk_(storage_, static_cast<Args &&>(args)...);
  }
};

}    // namespace ash
