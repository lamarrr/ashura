/// SPDX-License-Identifier: MIT
#include "ashura/engine/systems.h"

namespace ash
{

Systems sys = {};

ASH_C_LINKAGE ASH_DLL_EXPORT void hook_systems(Systems const * psys)
{
  sys = *psys;
}

}    // namespace ash
