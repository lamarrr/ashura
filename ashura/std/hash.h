#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef u64 Hash;

constexpr Hash hash_combine(Hash hash_a, Hash hash_b)
{
  hash_a ^= hash_b + 0x9e3779b9 + (hash_a << 6) + (hash_a >> 2);
  return hash_a;
}

Hash hash_bytes(void const *data, usize size);

}        // namespace ash
