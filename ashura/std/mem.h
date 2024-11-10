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

constexpr usize PAGE_SIZE = PAGE_ALIGNMENT;

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

namespace mem
{

template <typename T, typename U>
void copy(Span<T> src, U *dst)
{
  if (src.is_empty()) [[unlikely]]
  {
    return;
  }

  std::memcpy(dst, src.data(), src.size_bytes());
}

template <typename T, typename U>
void copy(Span<T> src, Span<U> dst)
{
  copy(src, dst.data());
}

template <typename T, typename U>
void move(Span<T> src, U *dst)
{
  if (src.is_empty()) [[unlikely]]
  {
    return;
  }

  std::memmove(dst, src.data(), src.size_bytes());
}

template <typename T, typename U>
void move(Span<T> src, Span<U> dst)
{
  move(src, dst.data());
}

template <typename T>
void zero(T *dst, usize n)
{
  if (n == 0) [[unlikely]]
  {
    return;
  }

  std::memset(dst, 0, sizeof(T) * n);
}

template <typename T>
void zero(Span<T> dst)
{
  zero(dst.data(), dst.size());
}

template <typename T>
void fill(T *dst, usize n, u8 byte)
{
  if (n == 0) [[unlikely]]
  {
    return;
  }

  std::memset(dst, byte, sizeof(T) * n);
}

template <typename T>
void fill(Span<T> dst, u8 byte)
{
  fill(dst.data(), dst.size(), byte);
}

template <typename T>
ASH_FORCE_INLINE T nontemporal_load(T const &src)
{
#if ASH_HAS_BUILTIN(nontemporal_load)
  return __builtin_nontemporal_load(&src);
#else
  return src;
#endif
}

template <typename T>
ASH_FORCE_INLINE void nontemporal_store(T &dst, T data)
{
#if ASH_HAS_BUILTIN(nontemporal_store)
  __builtin_nontemporal_store(data, &dst);
#else
  dst = data;
#endif
}

template <typename T>
ASH_FORCE_INLINE void prefetch(T const *src, int rw, int locality)
{
#if ASH_HAS_BUILTIN(prefetch)
  __builtin_prefetch(src, rw, locality);
#endif
}

}        // namespace mem

[[nodiscard]] inline bool to_c_str(Span<char const> str, Span<char> c_str)
{
  if ((str.size() + 1) > c_str.size()) [[unlikely]]
  {
    return false;
  }

  mem::copy(str, c_str);
  c_str[str.size()] = 0;
  return true;
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
