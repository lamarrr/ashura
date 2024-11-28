#include "ashura/std/types.h"

namespace ash
{

static_assert(sizeof(char) == sizeof(u8));
static_assert(sizeof(c8) == sizeof(u8));
static_assert(sizeof(c16) == sizeof(u16));
static_assert(sizeof(c32) == sizeof(u32));

}        // namespace ash