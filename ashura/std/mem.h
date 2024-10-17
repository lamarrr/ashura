/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/traits.h"
#include "ashura/std/types.h"
#include <cstring>

namespace ash
{

constexpr usize MAX_STANDARD_ALIGNMENT = alignof(max_align_t);

/// @brief Just a hint, this is a common cacheline size. not the actual target's
/// cacheline size
constexpr usize CACHELINE_ALIGNMENT = 64;

/// @brief Just a hint, this is the common page alignment. not the actual
/// target's page alignment.
constexpr usize PAGE_ALIGNMENT = 16_KB;

namespace mem
{

template <typename T>
constexpr T align_offset(T alignment, T offset)
{
  return (offset + (alignment - 1)) & ~(alignment - 1);
}

template <typename T>
T *align_ptr(usize alignment, T *p)
{
  return (T *) align_offset(alignment, (uptr) p);
}

template <typename T>
constexpr bool is_aligned(T alignment, T offset)
{
  return (offset & (alignment - 1)) == 0;
}

template <typename T>
bool is_ptr_aligned(usize alignment, T *p)
{
  return is_aligned(alignment, (uptr) p);
}

template <typename T>
void copy(T const *src, T *dst, usize num)
{
  if (num == 0)
  {
    return;
  }
  std::memcpy(dst, src, sizeof(T) * num);
}

template <typename T>
void copy(Span<T const> src, Span<T> dst)
{
  if (src.is_empty())
  {
    return;
  }
  std::memcpy(dst.data(), src.data(), src.size_bytes());
}

template <typename T>
void copy(Span<T const> src, T *dst)
{
  if (src.is_empty())
  {
    return;
  }
  std::memcpy(dst, src.data(), src.size_bytes());
}

template <typename T>
void move(T const *src, T *dst, usize num)
{
  if (num == 0)
  {
    return;
  }
  std::memmove(dst, src, sizeof(T) * num);
}

template <typename T>
void move(Span<T const> src, Span<T> dst)
{
  if (src.is_empty())
  {
    return;
  }
  std::memmove(dst.data(), src.data(), src.size_bytes());
}

template <typename T>
void move(Span<T const> src, T *dst)
{
  if (src.is_empty())
  {
    return;
  }
  std::memmove(dst, src.data(), src.size_bytes());
}

template <typename T>
void zero(T *dst, usize num)
{
  if (num == 0)
  {
    return;
  }
  std::memset(dst, 0, sizeof(T) * num);
}

template <typename T>
void zero(Span<T> dst)
{
  if (dst.is_empty())
  {
    return;
  }
  std::memset(dst.data(), 0, dst.size_bytes());
}

template <typename T>
void fill(T *dst, usize num, u8 byte)
{
  if (num == 0)
  {
    return;
  }
  std::memset(dst, byte, sizeof(T) * num);
}

template <typename T>
void fill(Span<T> dst, u8 byte)
{
  if (dst.is_empty())
  {
    return;
  }
  std::memset(dst.data(), byte, dst.size_bytes());
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

/// @brief Memory layout of a type
/// @param alignment non-zero alignment of the type
/// @param size byte-size of the type
struct Layout
{
  usize alignment = 1;
  usize size      = 0;

  constexpr Layout append(Layout const &ext) const
  {
    return Layout{.alignment = max(alignment, ext.alignment),
                  .size      = align_offset(ext.alignment, size) + ext.size};
  }

  constexpr Layout array(usize n) const
  {
    return Layout{.alignment = alignment, .size = size * n};
  }

  constexpr Layout aligned() const
  {
    return Layout{.alignment = alignment,
                  .size      = align_offset(alignment, size)};
  }

  constexpr Layout lanes(usize n) const
  {
    return Layout{.alignment = alignment * n, .size = size * n};
  }
};

template <typename T>
constexpr Layout layout = Layout{.alignment = alignof(T), .size = sizeof(T)};

/// @brief A Flex is a struct with multiple variable-sized members packed into a
/// single address. It ensures the correct calculation of the alignments,
/// offsets, and sizing requirements of the types and the resulting struct.
/// @tparam N number of members in the flexible struct
/// @param members memory layout of the members
template <usize N>
struct Flex
{
  Layout members[N];

  constexpr Layout layout() const
  {
    Layout l;
    for (Layout const &m : members)
    {
      l = l.append(m);
    }
    return l.aligned();
  }

  template <typename T>
  void unpack_at(void const *&stack, usize i, Span<T> &span)
  {
    stack             = align_ptr(members[i].alignment, stack);
    usize const count = members[i].size / sizeof(T);
    span              = Span{(T *) stack, count};
    stack             = ((u8 const *) stack) + members[i].size;
  }

  template <typename T>
  void unpack_at(void const *&stack, usize i, T *&ptr)
  {
    Span<T> span;
    unpack_at(stack, i, span);
    ptr = span.data();
  }

  template <typename... T>
  void unpack(void const *stack, T &...p)
  {
    static_assert(sizeof...(T) == N);
    usize i = 0;
    (unpack_at(stack, i++, p), ...);
  }
};

}        // namespace mem
}        // namespace ash
