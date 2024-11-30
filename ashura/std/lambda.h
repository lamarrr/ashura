/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/obj.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

static constexpr usize DEFAULT_LAMBDA_ALIGNMENT = 32;
static constexpr usize DEFAULT_LAMBDA_CAPACITY  = 48;

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
struct Lambda<R(Args...), Alignment, Capacity>
{
  static constexpr usize ALIGNMENT = Alignment;
  static constexpr usize CAPACITY  = Capacity;

  using Thunk = R (*)(void *, Args...);

  using Lifecycle = PFnLifecycle;

  alignas(ALIGNMENT) mutable u8 storage[CAPACITY];

  Thunk thunk = nullptr;

  Lifecycle lifecycle = noop;

  explicit constexpr Lambda() = default;

  template <AnyFunctor Functor>
  requires (ALIGNMENT >= alignof(Functor) && CAPACITY >= sizeof(Functor) &&
            Callable<Functor, Args...> &&
            Convertible<CallResult<Functor, Args...>, R>)
  constexpr Lambda(Functor && functor,
                   Thunk thunk = &FunctorThunk<Functor, R(Args...)>::thunk) :
      thunk{thunk},
      lifecycle{pFn_LIFECYCLE<Functor>}
  {
    new (storage) Functor{static_cast<Functor &&>(functor)};
  }

  constexpr Lambda(Lambda const &) = delete;

  constexpr Lambda & operator=(Lambda const &) = delete;

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Lambda(Lambda<R(Args...), SrcAlignment, SrcCapacity> && other) :
      thunk{other.thunk},
      lifecycle{other.lifecycle}
  {
    other.lifecycle(other.storage, storage);
    other.thunk     = nullptr;
    other.lifecycle = noop;
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

    lifecycle(storage, nullptr);
    other.lifecycle(other.storage, storage);
    lifecycle       = other.lifecycle;
    other.lifecycle = noop;
    thunk           = other.thunk;
    other.thunk     = nullptr;

    return *this;
  }

  constexpr ~Lambda()
  {
    lifecycle(storage, nullptr);
  }

  constexpr R operator()(Args... args) const
  {
    return thunk(storage, static_cast<Args &&>(args)...);
  }
};

}        // namespace ash
