#include "ashura/std/hash.h"
#include "xxh3.h"

namespace ash
{

Hash hash_bytes(void const *data, usize size)
{
  return XXH3_64bits(data, size);
}

}        // namespace ash