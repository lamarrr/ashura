/// SPDX-License-Identifier: MIT
#include "ashura/std/panic.h"

namespace ash
{

static std::atomic<u64> panic_count_impl = 0;

static void custom_panic_handler()
{
  // no-op
}

ASH_C_LINKAGE ASH_DLL_EXPORT std::atomic<u64> *panic_count = &panic_count_impl;

ASH_C_LINKAGE ASH_DLL_EXPORT PanicHandler panic_handler = custom_panic_handler;

}        // namespace ash