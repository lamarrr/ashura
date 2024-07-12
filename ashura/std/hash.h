/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

constexpr Hash hash_combine(Hash hash_a, Hash hash_b)
{
  hash_a ^= hash_b + 0x9e3779b9 + (hash_a << 6) + (hash_a >> 2);
  return hash_a;
}

template <typename... H>
constexpr Hash hash_combine_n(Hash hash_a, H... hash_b)
{
  ((hash_a = hash_combine(hash_a, hash_b)), ...);
  return hash_a;
}

Hash hash_bytes(Span<u8 const> bytes);

}        // namespace ash
