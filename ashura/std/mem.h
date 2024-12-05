/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/traits.h"
#include "ashura/std/types.h"
#include <cstring>

namespace ash
{

inline constexpr usize MAX_STANDARD_ALIGNMENT = alignof(max_align_t);

/// @brief Just a hint, this is a common cacheline size. not the actual target's
/// cacheline size
inline constexpr usize CACHELINE_ALIGNMENT = 64;

/// @brief Just a hint, this is the common page alignment. not the actual
/// target's page alignment.
inline constexpr usize PAGE_ALIGNMENT = 16_KB;

inline constexpr usize PAGE_SIZE = PAGE_ALIGNMENT;

template <typename T>
constexpr T align_offset(T alignment, T offset)
{
  return (offset + (alignment - 1)) & ~(alignment - 1);
}

template <typename T>
T * align_ptr(usize alignment, T * p)
{
  return (T *) align_offset(alignment, (uptr) p);
}

template <typename T>
constexpr bool is_aligned(T alignment, T offset)
{
  return (offset & (alignment - 1)) == 0;
}

template <typename T>
bool is_ptr_aligned(usize alignment, T * p)
{
  return is_aligned(alignment, (uptr) p);
}

namespace mem
{

template <typename T, NonConst U>
void copy(Span<T> src, U * dst)
{
  if (src.is_empty()) [[unlikely]]
  {
    return;
  }

  std::memcpy(dst, src.data(), src.size_bytes());
}

template <typename T, NonConst U>
void copy(Span<T> src, Span<U> dst)
{
  copy(src, dst.data());
}

template <typename T, NonConst U>
void move(Span<T> src, U * dst)
{
  if (src.is_empty()) [[unlikely]]
  {
    return;
  }

  std::memmove(dst, src.data(), src.size_bytes());
}

template <typename T, NonConst U>
void move(Span<T> src, Span<U> dst)
{
  move(src, dst.data());
}

template <NonConst T>
void zero(T * dst, usize n)
{
  if (n == 0) [[unlikely]]
  {
    return;
  }

  std::memset(dst, 0, sizeof(T) * n);
}

template <NonConst T>
void zero(Span<T> dst)
{
  zero(dst.data(), dst.size());
}

template <NonConst T>
void fill(T * dst, usize n, u8 byte)
{
  if (n == 0) [[unlikely]]
  {
    return;
  }

  std::memset(dst, byte, sizeof(T) * n);
}

template <NonConst T>
void fill(Span<T> dst, u8 byte)
{
  fill(dst.data(), dst.size(), byte);
}

template <typename T, typename U>
bool eq(Span<T> a, Span<U> b)
{
  return (a.size_bytes() == b.size_bytes()) &&
         (std::memcmp(a.data(), b.data(), a.size_bytes()) == 0);
}

template <typename T>
ASH_FORCE_INLINE T nontemporal_load(T const & src)
{
#if ASH_HAS_BUILTIN(nontemporal_load)
  return __builtin_nontemporal_load(&src);
#else
  return src;
#endif
}

template <typename T>
ASH_FORCE_INLINE void nontemporal_store(T & dst, T data)
{
#if ASH_HAS_BUILTIN(nontemporal_store)
  __builtin_nontemporal_store(data, &dst);
#else
  dst = data;
#endif
}

template <typename T>
ASH_FORCE_INLINE void prefetch(T const * src, int rw, int locality)
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
/// @param alignment non-zero power-of-2 alignment of the type
/// @param size byte-size of the type
struct Layout
{
  usize alignment = 1;
  usize size      = 0;

  constexpr Layout append(Layout const & ext) const
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
  Layout members[N]{};

  constexpr Layout layout() const
  {
    Layout l;
    for (Layout const & m : members)
    {
      l = l.append(m);
    }
    return l.aligned();
  }

  template <typename T>
  void unpack_at(void const *& stack, usize i, Span<T> & span) const
  {
    stack             = align_ptr(members[i].alignment, stack);
    usize const count = members[i].size / sizeof(T);
    span              = Span{(T *) stack, count};
    stack             = ((u8 const *) stack) + members[i].size;
  }

  template <typename T>
  void unpack_at(void const *& stack, usize i, T *& ptr) const
  {
    Span<T> span;
    unpack_at(stack, i, span);
    ptr = span.data();
  }

  template <typename... T>
  requires (sizeof...(T) == N)
  void unpack(void const * stack, T &... p) const
  {
    usize i = 0;
    (unpack_at(stack, i++, p), ...);
  }
};

struct StrEq
{
  bool operator()(Span<char const> a, Span<char const> b) const
  {
    return mem::eq(a, b);
  }

  bool operator()(Span<c8 const> a, Span<c8 const> b) const
  {
    return mem::eq(a, b);
  }

  bool operator()(Span<c16 const> a, Span<c16 const> b) const
  {
    return mem::eq(a, b);
  }

  bool operator()(Span<c32 const> a, Span<c32 const> b) const
  {
    return mem::eq(a, b);
  }
};

struct BitEq
{
  template <typename T>
  bool operator()(T const & a, T const & b) const
  {
    return mem::eq(Span{&a, 1}, Span{&b, 1});
  }
};

constexpr StrEq str_eq;
constexpr BitEq bit_eq;

}        // namespace ash
