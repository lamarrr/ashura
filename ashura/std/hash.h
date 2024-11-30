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

struct StrHasher
{
  hash64 operator()(Span<char const> str) const
  {
    return hash_bytes(str.as_u8());
  }

  hash64 operator()(Span<c8 const> str) const
  {
    return hash_bytes(str.as_u8());
  }

  hash64 operator()(Span<c16 const> str) const
  {
    return hash_bytes(str.as_u8());
  }

  hash64 operator()(Span<c32 const> str) const
  {
    return hash_bytes(str.as_u8());
  }
};

struct BitHasher
{
  template <typename T>
  hash64 operator()(T const & a) const
  {
    return hash_bytes(Span<T const>{&a, 1}.as_u8());
  }
};

constexpr StrHasher str_hash;
constexpr BitHasher bit_hash;

}        // namespace ash
