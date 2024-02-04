#pragma once
#include "ashura/std/types.h"
#include <string.h>

namespace ash
{
namespace mem
{
constexpr usize align_offset(usize alignment, usize offset)
{
  return (offset + (alignment - 1)) / alignment;
}

template <typename T>
void copy(T *src, T *dst, usize count)
{
  memcpy(dst, src, sizeof(T) * count);
}

template <typename T>
void copy(Span<T const> src, Span<T> dst)
{
  memcpy(dst.data, src.data(), src.size_bytes());
}

template <typename T>
void copy(Span<T const> src, T *dst)
{
  memcpy(dst, src.data(), src.size_bytes());
}

template <typename T>
void move(T *src, T *dst, usize count)
{
  memmove(dst, src, sizeof(T) * count);
}

template <typename T>
void move(Span<T const> src, Span<T> dst)
{
  memmove(dst.data(), src.data(), src.size_bytes());
}

template <typename T>
void move(Span<T const> src, T *dst)
{
  memmove(dst, src.data(), src.size_bytes());
}

template <typename T>
void zero(T *dst, usize count)
{
  memset(dst, 0, sizeof(T) * count);
}

inline void zero(void *dst, usize size)
{
  memset(dst, 0, size);
}

template <typename T>
void zero(Span<T> dst)
{
  memset(dst.data(), 0, dst.size_bytes());
}

template <typename T>
void fill(T *dst, usize count, u8 byte)
{
  memset(dst, byte, sizeof(T) * count);
}

template <typename T>
void fill(Span<T> dst, u8 byte)
{
  memset(dst.data(), byte, dst.size_bytes());
}

}        // namespace mem
}        // namespace ash
