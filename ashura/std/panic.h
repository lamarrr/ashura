/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/types.h"

namespace ash
{

// [ ] add backtrace support

struct Panic
{
};

typedef void (*PanicHandleFunc)();

struct [[nodiscard]] IPanicHandler
{
    IPanicHandler * next = nullptr;
    IPanicHandler * prev = nullptr;
    PanicHandleFunc func = noop;
};

ASH_C_LINKAGE ASH_DLL_EXPORT u64 * panic_count;

ASH_C_LINKAGE ASH_DLL_EXPORT void handle_panic();

ASH_C_LINKAGE ASH_DLL_EXPORT void noop_panic_handler();

ASH_C_LINKAGE ASH_DLL_EXPORT void exception_panic_handler();

ASH_C_LINKAGE ASH_DLL_EXPORT void trap_panic_handler();

/// @warning This function is not thread-safe. It must be called during program
///          initialization.
ASH_C_LINKAGE ASH_DLL_EXPORT void add_panic_handler(IPanicHandler * handler);

/// @warning This function is not thread-safe. It must be called during program
///          initialization.
ASH_C_LINKAGE ASH_DLL_EXPORT void remove_panic_handler(IPanicHandler * handler);

}    // namespace ash
