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

struct Layout
{
  usize alignment = 1;
  usize size      = 0;

  constexpr Layout append(Layout const &ext) const
  {
    return Layout{.alignment = max(alignment, ext.alignment),
                  .size = mem::align_offset(ext.alignment, size) + ext.size};
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

  constexpr Layout lanes(usize num_lanes) const
  {
    return Layout{.alignment = alignment * num_lanes, .size = size * num_lanes};
  }
};

template <typename T>
constexpr Layout layout = Layout{.alignment = alignof(T), .size = sizeof(T)};

constexpr Layout flex_layout(Span<Layout const> member_layouts)
{
  Layout layout;
  for (usize i = 0; i < member_layouts.size(); i++)
  {
    layout = layout.append(member_layouts[i]);
  }
  return layout.aligned();
}

template <typename H, typename T>
void flex_get(Span<Layout const> layout, H *head, usize i, Span<T> &array)
{
  Layout prefix;
  for (usize m = 0; m < i; m++)
  {
    prefix = layout[m].append(prefix);
  }

  array =
      Span<T>{(T *) align_ptr(layout[i].alignment, ((u8 *) head) + prefix.size),
              layout[i].size / sizeof(T)};
}

template <typename H, typename T>
void flex_get(Span<Layout const> layout, H *head, usize i, T *&ptr)
{
  Span<T> array;
  flex_get(layout, head, i, array);
  ptr = array.data();
}

template <usize N>
struct Flex
{
  Layout members[N];

  constexpr Layout layout() const
  {
    return flex_layout(span(members));
  }

  template <typename H, typename T>
  void get(H *head, usize i, Span<T> &array)
  {
    flex_get(span(members), head, i, array);
  }

  template <typename H, typename T>
  void get(H *head, usize i, T *&ptr)
  {
    flex_get(span(members), head, i, ptr);
  }

  template <typename H, typename... T>
  void unpack(H *head, T &...r)
  {
    usize i = 1;
    (get(head, i++, r), ...);
  }
};

}        // namespace mem
}        // namespace ash
