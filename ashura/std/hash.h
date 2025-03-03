/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

constexpr hash64 hash_combine(hash64 hash_a, hash64 hash_b)
{
  hash_a ^= hash_b + 0x9e37'79b9 + (hash_a << 6) + (hash_a >> 2);
  return hash_a;
}

template <typename... H>
constexpr hash64 hash_combine_n(hash64 hash_a, H... hash_b)
{
  ((hash_a = hash_combine(hash_a, hash_b)), ...);
  return hash_a;
}

hash64 hash_bytes(Span<u8 const> bytes, hash64 seed = 0);

struct SpanHash
{
  hash64 operator()(auto const & range) const
  {
    return hash_bytes(span(range).as_u8());
  }
};

struct BitHash
{
  template <typename T>
  hash64 operator()(T const & a) const
  {
    return hash_bytes(Span<T const>{&a, 1}.as_u8());
  }
};

struct IdentityHash
{
  constexpr hash64 operator()(u8 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(u16 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(u32 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(u64 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(i8 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(i16 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(i32 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(i64 a) const
  {
    return static_cast<hash64>(a);
  }

  constexpr hash64 operator()(f32 a) const
  {
    return static_cast<hash64>(bit_cast<u32>(a));
  }

  constexpr hash64 operator()(f64 a) const
  {
    return bit_cast<hash64>(a);
  }
};

inline constexpr SpanHash     span_hash;
inline constexpr BitHash      bit_hash;
inline constexpr IdentityHash identity_hash;

}    // namespace ash
