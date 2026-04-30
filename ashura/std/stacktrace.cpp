/// SPDX-License-Identifier: MIT
#include "ashura/std/stacktrace.hpp"

#include "ashura/std/cfg.hpp"

#if ASH_CFG(OS, LINUX)
#    include <cxxabi.h>
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
// TODO: add more debug info: line number, source, file, threadId, nominal threadname, system thread name
// TODO: demangle symbols
// TODO: make the stack trace be formatted into a vec before being printed so it can be displayed well
#if ASH_CFG(OS, LINUX)

struct StackFrame
{
    StackFrame * next     = nullptr;
    void *       ret_addr = nullptr;
};

void stacktrace(StackTraceFn callback)
{
    // On Linux, we can use the frame pointer to walk the call stack. This is more accurate than using backtrace, but it requires that the code be compiled with frame pointers (which is the default on x86-64). We can use dladdr to get the symbol information for each return address.
    auto * frame_ptr = reinterpret_cast<StackFrame *>(__builtin_frame_address(0));
    usize  i         = 0;

    while (frame_ptr != nullptr)
    {
        void *  ret_addr = frame_ptr->ret_addr;
        Dl_info info;
        if (dladdr(ret_addr, &info) != 0)
        {
            auto sname =
              info.dli_sname == nullptr ?
                nullptr :
                abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, nullptr);
            callback(info.dli_fname == nullptr ? cstr("<unknown>") :
                                                 cstr(info.dli_fname),
                     i, sname == nullptr ? cstr("<unknown>") : cstr(sname),
                     info.dli_saddr == nullptr ? ret_addr : info.dli_saddr);
            free(sname);
        }
        else
        {
            callback(cstr("<unknown>"), i, cstr("<unknown>"), ret_addr);
        }

        if (frame_ptr->next <= frame_ptr)
        {
            break;
        }
        frame_ptr = frame_ptr->next;
        i++;
    }
}

#else
#    if ASH_CFG(OS, WINDOWS)

void stacktrace(StackTraceFn callback)
{
    // Windows doesn't generally doesn't have frame pointers for system libraries,
    // so we use CaptureStackBackTrace to get the return addresses and then use SymFromAddr to get the symbol information for each address. This is less
    // accurate than using frame pointers, but it works even when frame pointers are not available.

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
                callback(
                  module_info.ModuleName == nullptr ? cstr("<unknown>") :
                                                      cstr(module_info.ModuleName),
                  i, symbol->Name == nullptr ? cstr("<unknown>") : cstr(symbol->Name),
                  reinterpret_cast<void *>(symbol->Address));
            }
            else
            {
                callback(cstr("<unknown>"), i,
                         symbol->Name == nullptr ? cstr("<unknown>") :
                                                   cstr(symbol->Name),
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
