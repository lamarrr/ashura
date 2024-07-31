/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{
typedef void (*PanicHandler)();

ASH_C_LINKAGE ASH_DLL_EXPORT std::atomic<u64> *panic_count;

ASH_C_LINKAGE ASH_DLL_EXPORT PanicHandler panic_handler;

}        // namespace ash
