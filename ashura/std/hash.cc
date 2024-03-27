#include "ashura/std/hash.h"
#include "xxh3.h"

namespace ash
{

Hash hash_bytes(Span<u8 const> bytes)
{
  return XXH3_64bits(bytes.data(), bytes.size());
}

}        // namespace ash