/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/obj.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

typedef void (*ErasedDestroy)(void * storage);

typedef void (*ErasedRelocate)(void * src_storage, void * dst_storage);

template <typename T>
inline constexpr ErasedDestroy pFn_DESTRUCT =
    [](void * storage) { reinterpret_cast<T *>(storage)->~T(); };

template <typename T>
inline constexpr ErasedRelocate pFn_RELOCATE =
    [](void * src_storage, void * dst_storage) {
      T * src = reinterpret_cast<T *>(src_storage);
      T * dst = reinterpret_cast<T *>(dst_storage);
      obj::relocate(Span{src, 1}, dst);
    };

template <typename T>
inline constexpr ErasedRelocate pFn_RELOCATE_NON_OVERLAPPING =
    [](void * src_storage, void * dst_storage) {
      T * src = reinterpret_cast<T *>(src_storage);
      T * dst = reinterpret_cast<T *>(dst_storage);
      obj::relocate_non_overlapping(Span{src, 1}, dst);
    };

static constexpr usize DEFAULT_LAMBDA_CAPACITY = 40;

static constexpr usize DEFAULT_LAMBDA_ALIGNMENT = 64;

template <typename Sig, usize Alignment = DEFAULT_LAMBDA_ALIGNMENT,
          usize Capacity = DEFAULT_LAMBDA_CAPACITY>
struct Lambda;

/// @brief In-Place/Stack-Allocated and Type-Erased Move-Only Function.
/// It only requires that the erased type be relocatable (moved and destroyed).
/// In order to prevent accessing elements from dynamic offsets, we require that the Type be placeable at the start of the storage.
///
/// x64 minimum size of Lambda = 4 Pointers = 32-bytes
/// x64 ideal configuration: Alignment = 64 bytes, Capacity = 40 bytes (Gives 64 bytes total. Fits exactly into one cacheline.
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

  alignas(ALIGNMENT) mutable u8 storage[CAPACITY];

  Thunk thunk = nullptr;

  ErasedRelocate relocator = noop;

  ErasedDestroy destructor = noop;

  explicit constexpr Lambda() = default;

  template <AnyFunctor Functor>
  requires (ALIGNMENT >= alignof(Functor) && CAPACITY >= sizeof(Functor) &&
            Callable<Functor, Args...> &&
            Convertible<CallResult<Functor, Args...>, R>)
  constexpr Lambda(Functor && functor,
                   Thunk thunk = &FunctorThunk<Functor, R(Args...)>::thunk) :
      thunk{thunk},
      relocator{pFn_RELOCATE_NON_OVERLAPPING<Functor>},
      destructor{pFn_DESTRUCT<Functor>}
  {
    new (storage) Functor{static_cast<Functor &&>(functor)};
  }

  constexpr Lambda(Lambda const &) = delete;

  constexpr Lambda & operator=(Lambda const &) = delete;

  template <usize SrcAlignment, usize SrcCapacity>
  requires (ALIGNMENT >= SrcAlignment && CAPACITY >= SrcCapacity)
  constexpr Lambda(Lambda<R(Args...), SrcAlignment, SrcCapacity> && other) :
      thunk{other.thunk},
      relocator{other.relocator},
      destructor{other.destructor}
  {
    other.relocator(other.storage, storage);
    other.thunk      = nullptr;
    other.relocator  = noop;
    other.destructor = noop;
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

    destructor(storage);
    other.relocator(other.storage, storage);
    relocator        = other.relocator;
    other.relocator  = noop;
    destructor       = other.destructor;
    other.destructor = noop;
    thunk            = other.thunk;
    other.thunk      = nullptr;

    return *this;
  }

  constexpr ~Lambda()
  {
    destructor(storage);
  }

  constexpr R operator()(Args... args) const
  {
    return thunk(storage, static_cast<Args &&>(args)...);
  }
};

// TODO (lamarrr):
template <AnyFunctor Functor>
constexpr auto lambda(Functor && functor)
{
  using Sig = int(int, int);
  return Lambda<Sig, alignof(Functor), sizeof(Functor)>{
      static_cast<Functor &&>(functor)};
}

}        // namespace ash
