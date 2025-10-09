/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

constexpr auto hash_combine(usize a, usize b)
{
  a ^= b + 0x9e37'79b9 + (a << 6) + (a >> 2);
  return a;
}

template <typename... H>
constexpr auto hash_combine(usize a, usize b, H... c)
{
  a = hash_combine(a, b);
  ((a = hash_combine(a, c)), ...);
  return a;
}

usize hash_bytes(Span<u8 const> bytes, usize seed = 0);

struct SpanHash
{
  auto operator()(auto const & range) const
  {
    return hash_bytes(span(range).as_u8());
  }
};

struct BitHash
{
  template <typename T>
  auto operator()(T const & a) const
  {
    return hash_bytes(Span<T const>{&a, 1}.as_u8());
  }
};

struct IdentityHash
{
  constexpr auto operator()(u8 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(u16 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(u32 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(u64 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(i8 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(i16 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(i32 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(i64 a) const
  {
    return static_cast<usize>(a);
  }

  constexpr auto operator()(f32 a) const
  {
    return static_cast<usize>(bit_cast<u32>(a));
  }

  constexpr auto operator()(f64 a) const
  {
    return bit_cast<usize>(a);
  }
};

inline constexpr SpanHash     span_hash;
inline constexpr BitHash      bit_hash;
inline constexpr IdentityHash identity_hash;

inline constexpr auto hash(Integral auto i)
{
  return identity_hash(i);
}

inline constexpr auto hash(FloatingPoint auto f)
{
  return identity_hash(f);
}

template <typename T>
inline constexpr auto hash(T const & obj) -> decltype(obj.hash())
{
  return obj.hash();
}

template <typename T>
concept Hashable = requires (T const & obj) {
  { static_cast<usize>(hash(obj)) };
};

struct DefaultHasher
{
  template <Hashable T>
  inline constexpr auto operator()(T const & obj)
  {
    return hash(obj);
  }
};

template <typename Hasher, typename T>
concept HashableWith = requires (T const & obj, Hasher const & hasher) {
  { static_cast<usize>(hasher(obj)) };
};

}    // namespace ash
