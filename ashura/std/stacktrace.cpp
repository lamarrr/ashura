/// SPDX-License-Identifier: MIT
#include "ashura/std/stacktrace.hpp"

#include "ashura/std/cfg.hpp"

#if ASH_CFG(OS, LINUX)
#    include <dlfcn.h>
#endif

#if ASH_CFG(OS, WINDOWS)
// clang-format off
#    include <windows.h> 
#    include <dbghelp.h>
// clang-format on
#endif

namespace ash
{

// TODO: add support for macOS
// TODO: add more debug info: line number, source, file, threadId
#if ASH_CFG(OS, LINUX)

void stacktrace(StackTraceFn callback)
{
    void ** rbp = (void **) __builtin_frame_address(0);
    usize   i   = 0;

    while (rbp)
    {
        void *  ret_addr = *(rbp + 1);
        Dl_info info;
        if (dladdr(ret_addr, &info) != 0)
        {
            callback(cstr(info.dli_fname), i, cstr(info.dli_sname), info.dli_saddr);
        }
        else
        {
            callback(cstr("<unknown>"), i, cstr("<unknown>"), nullptr);
        }
        rbp = (void **) *rbp;
        i++;
    }
}

#else
#    if ASH_CFG(OS, WINDOWS)

void stacktrace(StackTraceFn callback)
{
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);
    SymInitialize(process, nullptr, TRUE);

    constexpr u32 max_frames = 128;
    void *        frames[max_frames];

    u16 frame_count = CaptureStackBackTrace(0, max_frames, frames, nullptr);

    for (u16 i = 0; i < frame_count; i++)
    {
        void *       ret_addr = frames[i];
        char         symbol_storage[sizeof(SYMBOL_INFO) + 256];
        PSYMBOL_INFO symbol  = reinterpret_cast<PSYMBOL_INFO>(symbol_storage);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen   = 255;

        DWORD64 displacement = 0;
        if (SymFromAddr(process, reinterpret_cast<DWORD64>(ret_addr), &displacement,
                        symbol))
        {
            IMAGEHLP_MODULE64 module_info = {};
            module_info.SizeOfStruct      = sizeof(module_info);
            if (SymGetModuleInfo64(process, reinterpret_cast<DWORD64>(ret_addr),
                                   &module_info))
            {
                callback(cstr(module_info.ModuleName), i, cstr(symbol->Name),
                         reinterpret_cast<void *>(symbol->Address));
            }
            else
            {
                callback(cstr("<unknown>"), i, cstr(symbol->Name),
                         reinterpret_cast<void *>(symbol->Address));
            }
        }
        else
        {
            callback(cstr("<unknown>"), i, cstr("<unknown>"), ret_addr);
        }
    }
}

#    else
#        if ASH_CFG(OS, MACOS)

void stacktrace(StackTraceFn callback)
{
}

#        else

void stacktrace(StackTraceFn callback)
{
}

#        endif
#    endif
#endif

}    // namespace ash
