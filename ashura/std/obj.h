/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{
namespace obj
{

template <NonConst T>
constexpr void default_construct(Span<T> dst)
{
  for (T * iter = dst.begin(); iter != dst.end(); iter++)
  {
    new (iter) T{};
  }
}

template <NonConst T, NonConst U>
constexpr void move_construct(Span<T> src, U * dst)
{
  for (T * in = src.begin(); in != src.end(); in++, dst++)
  {
    new (dst) T{static_cast<T &&>(*in)};
  }
}

template <NonConst T, NonConst U>
constexpr void move_construct(Span<T> src, Span<U> dst)
{
  move_construct(src, dst.data());
}

template <typename T, NonConst U>
constexpr void copy_construct(Span<T> src, U * dst)
{
  for (T * in = src.begin(); in != src.end(); in++, dst++)
  {
    new (dst) T{*in};
  }
}

template <typename T, NonConst U>
constexpr void copy_construct(Span<T> src, Span<U> dst)
{
  copy_construct(src, dst.data());
}

template <typename T>
constexpr void destruct(Span<T> src)
{
  if constexpr (!TriviallyDestructible<T>)
  {
    for (T * iter = src.begin(); iter != src.end(); iter++)
    {
      iter->~T();
    }
  }
}

template <typename T, NonConst U>
constexpr void move_assign(Span<T> src, U * dst)
{
  for (T * in = src.begin(); in != src.end(); in++, dst++)
  {
    *in = static_cast<T &&>(*dst);
  }
}

template <typename T, NonConst U>
constexpr void move_assign(Span<T> src, Span<U> dst)
{
  move_assign(src, dst.data());
}

template <typename T, NonConst U>
constexpr void copy_assign(Span<T> src, U * dst)
{
  for (T * in = src.begin(); in != src.end(); in++, dst++)
  {
    *dst = *in;
  }
}

template <typename T, NonConst U>
constexpr void copy_assign(Span<T> src, Span<U> dst)
{
  copy_assign(src, dst.data());
}

/// @brief move-construct object from src to an uninitialized memory range
/// dst_mem and destroy object at src_mem, leaving src's objects uninitialized.
template <NonConst T>
constexpr void relocate(Span<T> src, T * dst)
{
  if constexpr (TriviallyRelocatable<T>)
  {
    mem::move(src, dst);
  }
  else
  {
    move_construct(src, dst);
    destruct(src);
  }
}

template <NonConst T>
constexpr void relocate(Span<T> src, Span<T> dst)
{
  relocate(src, dst.data());
}

/// @brief same as relocate but for non-overlapping memory placements
///
/// @note src_mem and dst_mem must not be same nor overlapping.
template <NonConst T>
constexpr void relocate_non_overlapping(Span<T> src, T * dst)
{
  if constexpr (TriviallyRelocatable<T>)
  {
    mem::copy(src, dst);
  }
  else
  {
    move_construct(src, dst);
    destruct(src);
  }
}

template <NonConst T>
constexpr void relocate_non_overlapping(Span<T> src, Span<T> dst)
{
  relocate_non_overlapping(src, dst.data());
}

}        // namespace obj

using PFnDestruct = void (*)(void *);

using PFnRelocate = void (*)(void *, void *);

using PFnLifecycle = void (*)(void *, void *);

template <typename T>
inline constexpr PFnDestruct pFn_DESTRUCT = [](void * mem) {
  T * obj = reinterpret_cast<T *>(mem);

  obj::destruct(Span{obj, 1});
};

template <typename T>
inline constexpr PFnRelocate pFn_RELOCATE = [](void * src_mem, void * dst_mem) {
  T * src = reinterpret_cast<T *>(src_mem);
  T * dst = reinterpret_cast<T *>(dst_mem);

  obj::relocate_non_overlapping(Span{src, 1}, dst);
};

/// @brief An object lifecycle function that relocates and destroys an object.
/// When the destination memory is nullptr,
/// the object is to be destroyed.
/// Otherwise, it should relocate itself to the destination memory.
template <typename T>
inline constexpr PFnLifecycle pFn_LIFECYCLE =
    [](void * src_mem, void * dst_mem) {
      T * src = reinterpret_cast<T *>(src_mem);
      T * dst = reinterpret_cast<T *>(dst_mem);

      if (dst_mem == nullptr) [[unlikely]]
      {
        src->~T();
      }
      else
      {
        obj::relocate_non_overlapping(Span{src, 1}, dst);
      }
    };

}        // namespace ash
