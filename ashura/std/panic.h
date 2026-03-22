/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/fn.h"
#include "ashura/std/types.h"

namespace ash
{


struct PanicException
{
};

typedef struct IPanicHandler * PanicHandler;

using PanicHandleFn = Fn<void()>;

struct [[nodiscard]] IPanicHandler
{
    IPanicHandler * next = nullptr;
    IPanicHandler * prev = nullptr;
    PanicHandleFn   fn   = noop;
};

ASH_C_LINKAGE ASH_DLL_EXPORT u64 * panic_count;

ASH_C_LINKAGE ASH_DLL_EXPORT void handle_panic();

ASH_C_LINKAGE ASH_DLL_EXPORT void noop_panic_handler(PanicHandler);

ASH_C_LINKAGE ASH_DLL_EXPORT void exception_panic_handler(PanicHandler);

ASH_C_LINKAGE ASH_DLL_EXPORT void trap_panic_handler(PanicHandler);

/// @warning This function is not thread-safe. It must be called during program
///          initialization.
ASH_C_LINKAGE ASH_DLL_EXPORT void add_panic_handler(PanicHandler handler);

/// @warning This function is not thread-safe. It must be called during program
///          initialization.
ASH_C_LINKAGE ASH_DLL_EXPORT void remove_panic_handler(PanicHandler handler);

}    // namespace ash
