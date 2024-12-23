/// SPDX-License-Identifier: MIT
#include "ashura/std/panic.h"

namespace ash
{

static u64 panic_count_impl = 0;

void noop_panic_handler()
{
}

void exception_panic_handler()
{
  throw Panic{};
}

ASH_C_LINKAGE ASH_DLL_EXPORT u64 * panic_count = &panic_count_impl;

ASH_C_LINKAGE ASH_DLL_EXPORT PanicHandler panic_handler =
#if ASH_CFG(PANIC_HANDLER, EXCEPTION)
  exception_panic_handler;
#else
  noop_panic_handler;
#endif

}    // namespace ash
