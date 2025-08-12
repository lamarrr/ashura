/// SPDX-License-Identifier: MIT
#define XXH_INLINE_ALL
#include "ashura/std/hash.h"
#include "ashura/std/types.h"
#include "xxhash.h"

namespace ash
{

usize hash_bytes(Span<u8 const> bytes, usize seed)
{
  return XXH3_64bits_withSeed(bytes.data(), bytes.size_bytes(), seed);
}

}    // namespace ash
