/// SPDX-License-Identifier: MIT
#include "ashura/std/hash.hpp"
#include "ashura/std/types.hpp"
#include "xxhash.h"

namespace ash
{

usize hash_bytes(Span<u8 const> bytes, usize seed)
{
    return XXH3_64bits_withSeed(bytes.data(), bytes.size_bytes(), seed);
}

}    // namespace ash
