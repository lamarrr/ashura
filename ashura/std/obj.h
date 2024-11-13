/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/mem.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{
namespace obj
{

template <typename T>
constexpr void default_construct(Span<T> dst)
{
  for (T *iter = dst.begin(); iter != dst.end(); iter++)
  {
    new (iter) T{};
  }
}

template <typename T, typename U>
constexpr void move_construct(Span<T> src, U *dst)
{
  for (T *in = src.begin(); in != src.end(); in++, dst++)
  {
    new (dst) T{(T &&) (*in)};
  }
}

template <typename T, typename U>
constexpr void move_construct(Span<T> src, Span<U> dst)
{
  move_construct(src, dst.data());
}

template <typename T, typename U>
constexpr void copy_construct(Span<T> src, U *dst)
{
  for (T *in = src.begin(); in != src.end(); in++, dst++)
  {
    new (dst) T{*in};
  }
}

template <typename T, typename U>
constexpr void copy_construct(Span<T> src, Span<U> dst)
{
  copy_construct(src, dst.data());
}

template <typename T>
constexpr void destruct(Span<T> src)
{
  if constexpr (!TriviallyDestructible<T>)
  {
    for (T *iter = src.begin(); iter != src.end(); iter++)
    {
      iter->~T();
    }
  }
}

template <typename T, typename U>
constexpr void move(Span<T> src, U *dst)
{
  for (T *in = src.begin(); in != src.end(); in++, dst++)
  {
    *in = (T &&) (*dst);
  }
}

template <typename T, typename U>
constexpr void move(Span<T> src, Span<U> dst)
{
  move(src, dst.data());
}

template <typename T, typename U>
constexpr void copy(Span<T> src, U *dst)
{
  for (T *in = src.begin(); in != src.end(); in++, dst++)
  {
    *dst = *in;
  }
}

template <typename T, typename U>
constexpr void copy(Span<T> src, Span<U> dst)
{
  copy(src, dst.data());
}

/// @brief move-construct object from src to an uninitialized memory range
/// dst_mem and destroy object at src_mem, leaving src's objects uninitialized.
template <typename T, typename U>
constexpr void relocate(Span<T> src, U *dst)
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

template <typename T, typename U>
constexpr void relocate(Span<T> src, Span<U> dst)
{
  relocate(src, dst.data());
}

/// @brief same as relocate but for non-overlapping memory placements
///
/// @note src_mem and dst_mem must not be same nor overlapping.
template <typename T, typename U>
constexpr void relocate_non_overlapping(Span<T> src, U *dst)
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

template <typename T, typename U>
constexpr void relocate_non_overlapping(Span<T> src, Span<U> dst)
{
  relocate_non_overlapping(src, dst.data());
}

}        // namespace obj
}        // namespace ash
