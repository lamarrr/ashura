/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

struct Panic
{
};

typedef void (*PanicHandler)();

ASH_C_LINKAGE ASH_DLL_EXPORT u64 * panic_count;

ASH_C_LINKAGE ASH_DLL_EXPORT PanicHandler panic_handler;

}    // namespace ash
