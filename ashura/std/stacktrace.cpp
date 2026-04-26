/// SPDX-License-Identifier: MIT
#include "ashura/std/stacktrace.hpp"

#include "ashura/std/cfg.hpp"

#if ASH_CFG(OS, LINUX)
#    include <dlfcn.h>
#endif

namespace ash
{

#if ASH_CFG(OS, LINUX)

void walk_stack_linux()
{
    void ** rbp = (void **) __builtin_frame_address(0);
    usize   i   = 0;

    while (rbp)
    {
        void *  ret_addr = *(rbp + 1);
        Dl_info info;
        if (dladdr(ret_addr, &info) != 0)
        {
            (void)
              info.dli_fname;    // file name of shared object that contains the address
            (void) info.dli_sname;    // name of nearest symbol
            (void) info.dli_saddr;    // start address of the symbol
            (void) i;
        }
        else
        {
        }
        rbp = (void **) *rbp;
        i++;
    }
}
#endif

#if ASH_CFG(OS, WINDOWS)
void walk_stack_windows()
{
    void ** rbp = (void **) __builtin_frame_address(0);

    while (rbp)
    {
        void * ret_addr = *(rbp + 1);

        HANDLE       process = GetCurrentProcess();
        char         buffer[sizeof(SYMBOL_INFO) + 256];
        PSYMBOL_INFO symbol  = (PSYMBOL_INFO) buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen   = 255;

        DWORD64 displacement = 0;

        if (SymFromAddr(process, (DWORD64) ret_addr, &displacement, symbol))
        {
            (void) symbol->Name;       // name of the symbol
            (void) symbol->Address;    // address of the symbol
            (void) i;
        }
        else
        {
        }

        rbp = (void **) *rbp;
        i++;
    }
}
#endif

}    // namespace ash
