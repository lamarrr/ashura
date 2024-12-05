/// SPDX-License-Identifier: MIT
#define XXH_INLINE_ALL
#include "ashura/std/hash.h"
#include "ashura/std/types.h"
#include "xxhash.h"

namespace ash
{

hash64 hash_bytes(Span<u8 const> bytes, hash64 seed)
{
  return XXH3_64bits_withSeed(bytes.data(), bytes.size_bytes(), seed);
}

}        // namespace ash
