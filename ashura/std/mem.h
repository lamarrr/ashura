#pragma once
#include "ashura/std/traits.h"
#include "ashura/std/types.h"
#include <string.h>

namespace ash
{

constexpr u16 MAX_STANDARD_ALIGNMENT = alignof(max_align_t);

/// @brief Just a hint, this is a common cacheline size. not the actual target's
/// cacheline size
constexpr u16 CACHELINE_ALIGNMENT = 64;

/// @brief Just a hint, this is the common page alignment. not the actual
/// target's page alignment.
constexpr u16 PAGE_ALIGNMENT = 16_KB;

namespace mem
{

template <typename T>
constexpr T align_offset(T alignment, T offset)
{
  return (offset + (alignment - 1)) & ~(alignment - 1);
}

template <typename T>
constexpr bool is_aligned(T alignment, T offset)
{
  return (offset & (alignment - 1)) == 0;
}

template <typename T>
void copy(T const *src, T *dst, usize num)
{
  if (num == 0)
  {
    return;
  }
  memcpy(dst, src, sizeof(T) * num);
}

template <typename T>
void copy(Span<T const> src, Span<T> dst)
{
  if (src.is_empty())
  {
    return;
  }
  memcpy(dst.data(), src.data(), src.size_bytes());
}

template <typename T>
void copy(Span<T const> src, T *dst)
{
  if (src.is_empty())
  {
    return;
  }
  memcpy(dst, src.data(), src.size_bytes());
}

template <typename T>
void move(T const *src, T *dst, usize num)
{
  if (num == 0)
  {
    return;
  }
  memmove(dst, src, sizeof(T) * num);
}

template <typename T>
void move(Span<T const> src, Span<T> dst)
{
  if (src.is_empty())
  {
    return;
  }
  memmove(dst.data(), src.data(), src.size_bytes());
}

template <typename T>
void move(Span<T const> src, T *dst)
{
  if (src.is_empty())
  {
    return;
  }
  memmove(dst, src.data(), src.size_bytes());
}

template <typename T>
void zero(T *dst, usize num)
{
  if (num == 0)
  {
    return;
  }
  memset(dst, 0, sizeof(T) * num);
}

template <typename T>
void zero(Span<T> dst)
{
  if (dst.is_empty())
  {
    return;
  }
  memset(dst.data(), 0, dst.size_bytes());
}

template <typename T>
void fill(T *dst, usize num, u8 byte)
{
  if (num == 0)
  {
    return;
  }
  memset(dst, byte, sizeof(T) * num);
}

template <typename T>
void fill(Span<T> dst, u8 byte)
{
  if (dst.is_empty())
  {
    return;
  }
  memset(dst.data(), byte, dst.size_bytes());
}

/// move-construct object from src to an uninitialized memory range dst and
/// destroy object at src, leaving src uninitialized.
///
/// src and dst must not be same nor overlapping.
template <typename T>
void relocate(T *src, T *uninit_dst, usize num)
{
  if constexpr (TriviallyRelocatable<T>)
  {
    copy(src, uninit_dst, num);
  }
  else
  {
    T       *in  = src;
    T       *out = uninit_dst;
    T *const end = src + num;
    while (in != end)
    {
      new (out) T{(T &&) *in};
      in++;
      out++;
    }
    in = src;
    while (in != end)
    {
      in->~T();
      in++;
    }
  }
}

}        // namespace mem
}        // namespace ash
