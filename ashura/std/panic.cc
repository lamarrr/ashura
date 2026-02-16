/// SPDX-License-Identifier: MIT
#include "ashura/std/panic.h"
#include "ashura/std/list.h"

namespace ash
{

static u64 panic_count_impl = 0;

void noop_panic_handler(PanicHandler)
{
}

void exception_panic_handler(PanicHandler)
{
    throw PanicException{};
}

void trap_panic_handler(PanicHandler)
{
    __builtin_trap();
}

ASH_C_LINKAGE ASH_DLL_EXPORT u64 * panic_count = &panic_count_impl;
ASH_C_LINKAGE ASH_DLL_EXPORT List<IPanicHandler> panic_handlers{};

ASH_C_LINKAGE ASH_DLL_EXPORT void handle_panic()
{
    for (auto & handler : panic_handlers)
    {
        handler.fn();
    }
}

ASH_C_LINKAGE ASH_DLL_EXPORT void add_panic_handler(PanicHandler handler)
{
    panic_handlers.push_back(handler);
}

ASH_C_LINKAGE ASH_DLL_EXPORT void remove_panic_handler(PanicHandler handler)
{
    panic_handlers.pop_at(handler);
}

}    // namespace ash
