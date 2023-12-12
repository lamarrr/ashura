
#include "ashura/allocator.h"
#include "ashura/cfg.h"

#if ASH_CFG(OS, POSIX) && (ASH_CFG(COMPILER, CLANG) || ASH_CFG(COMPILER, GNUC))
#  define STDC_ALIGNED_ALLOC_SUPPORTED 1
#else
#  define STDC_ALIGNED_ALLOC_SUPPORTED 0
#endif

#if ASH_CFG(OS, WINDOWS) && ASH_CFG(COMPILER, MSVC)
#  define WINDOWS_CRT_ALIGNED_ALLOC_SUPPORTED 1
#else
#  define WINDOWS_CRT_ALIGNED_ALLOC_SUPPORTED 0
#endif

#include <stdlib.h>

namespace ash
{

Heap const heap;

// TODO(lamarrr): handle nullptrs, 0 resizes
// malloc 0 is okay

}        // namespace ash
