#pragma once
#include "ashura/integers.h"
#include <cstring>

namespace ash
{

template <typename T>
struct Span
{
  T    *data = nullptr;
  usize size = 0;

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size;
  }

  constexpr T *begin() const
  {
    return data;
  }

  constexpr T *end() const
  {
    return data + size;
  }

  constexpr operator Span<T const>() const
  {
    return Span<T const>{.data = data, .size = size};
  }
};

template <typename T>
constexpr Span<T const> as_const(Span<T> span)
{
  return Span<T const>{.data = span.data, .size = span.size};
}

template <typename T>
constexpr Span<u8> as_u8(Span<T> span)
{
  return Span<u8>{.data = reinterpret_cast<u8 *>(span.data), .size = span.size_bytes()};
}

template <typename T>
constexpr Span<char> as_char(Span<T> span)
{
  return Span<char>{.data = reinterpret_cast<char *>(span.data), .size = span.size_bytes()};
}

template <typename T>
constexpr Span<u8 const> as_u8(Span<T const> span)
{
  return Span<u8 const>{.data = reinterpret_cast<u8 const *>(span.data), .size = span.size_bytes()};
}

template <typename T>
constexpr Span<char const> as_char(Span<T const> span)
{
  return Span<char const>{.data = reinterpret_cast<char const *>(span.data),
                          .size = span.size_bytes()};
}

template <typename T>
constexpr Span<T> slice(Span<T> span, usize offset)
{
  return Span<T>{.data = span.data + offset, .size = span.size - offset};
}

template <typename T>
constexpr Span<T> slice(Span<T> span, usize offset, usize count)
{
  count = ((span.size - offset) > count) ? count : (span.size - offset);
  return Span<T>{.data = span.data + offset, .size = count};
}

template <typename T>
void mem_copy(T *src, T *dst, usize count)
{
  std::memcpy(dst, src, sizeof(T) * count);
}

template <typename T>
void mem_copy(Span<T const> src, Span<T> dst)
{
  std::memcpy(dst.data, src.data, src.size_bytes());
}

template <typename T>
void mem_copy(Span<T const> src, T *dst)
{
  std::memcpy(dst, src.data, src.size_bytes());
}

template <typename T>
void mem_zero(T *dst, usize count)
{
  std::memset(dst, 0, sizeof(T) * count);
}

template <typename T>
void mem_zero(Span<T> dst)
{
  std::memset(dst.data, 0, dst.size_bytes());
}

template <typename T>
void mem_fill(T *dst, usize count, u8 byte)
{
  std::memset(dst, byte, sizeof(T) * count);
}

template <typename T>
void mem_fill(Span<T> dst, u8 byte)
{
  std::memset(dst.data, byte, dst.size_bytes());
}

template <typename Dst, typename T>
constexpr Span<Dst> cast(Span<T> src)
{
  static_assert(sizeof(T) == sizeof(Dst));
  return Span<Dst>{.data = static_cast<Dst *>(src.data), .size = src.size};
}

template <typename T>
Span<T> reinterpret(Span<u8> src)
{
  return Span<T>{.data = reinterpret_cast<T *>(src.data), .size = src.size / sizeof(T)};
}

template <typename T>
Span<T const> reinterpret(Span<u8 const> src)
{
  return Span<T const>{.data = reinterpret_cast<T const *>(src.data), .size = src.size / sizeof(T)};
}

}        // namespace ash
