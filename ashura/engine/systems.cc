/// SPDX-License-Identifier: MIT
#include "ashura/engine/systems.h"
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"

namespace ash
{

// [ ] fix
Systems sys = {};

ASH_C_LINKAGE ASH_DLL_EXPORT void hook_system(Systems const * psys)
{
  sys = *psys;
}

}    // namespace ash
