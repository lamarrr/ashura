
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace ash
{

Heap const global_heap_object;
usize      heap_used = 0;

// TODO(lamarrr): switch to using global panic and logger
void *HeapInterface::allocate(Allocator self, usize alignment, usize size)
{
  (void) self;
  (void) alignment;
  (void) size;
  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    return malloc(size);
  }
#if WINDOWS_CRT_ALIGNED_ALLOC_SUPPORTED
  return _aligned_malloc(size, alignment);
#else
#  if STDC_ALIGNED_ALLOC_SUPPORTED
  return aligned_alloc(alignment, size);
#  else
  (void) fprintf(stderr,
                 "over-aligned malloc of alignment %" PRIu64 " not supported\n",
                 (u64) alignment);
  (void) fflush(stderr);
  return nullptr;
#  endif
#endif
}

void *HeapInterface::allocate_zeroed(Allocator self, usize alignment,
                                     usize size)
{
  (void) self;
  (void) alignment;
  (void) size;
  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    return calloc(size, 1);
  }

#if WINDOWS_CRT_ALIGNED_ALLOC_SUPPORTED
  void *mem = _aligned_malloc(size, alignment);
  if (mem != nullptr)
  {
    return memset(mem, 0, size);
  }
  return mem;
#else
#  if STDC_ALIGNED_ALLOC_SUPPORTED
  void *mem = aligned_alloc(alignment, size);
  if (mem != nullptr)
  {
    return memset(mem, 0, size);
  }
  return mem;
#  else
  (void) fprintf(stderr,
                 "over-aligned zeroed-alloc of alignment %" PRIu64
                 " not supported\n",
                 (u64) alignment);
  (void) fflush(stderr);
  return nullptr;
#  endif
#endif
}

void *HeapInterface::reallocate(Allocator self, usize alignment, void *memory,
                                usize old_size, usize new_size)
{
  (void) self;
  (void) alignment;
  (void) memory;
  (void) old_size;
  (void) new_size;
  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    return realloc(memory, new_size);
  }

#if WINDOWS_CRT_ALIGNED_ALLOC_SUPPORTED
  return _aligned_realloc(memory, new_size, alignment);
#else
#  if STDC_ALIGNED_ALLOC_SUPPORTED
  // stdc realloc doesn't guarantee preservation of alignment
  void *new_memory = HeapInterface::allocate(self, alignment, new_size);
  if (new_memory != nullptr)
  {
    (void) memcpy(new_memory, memory, old_size);
    HeapInterface::deallocate(self, alignment, memory, old_size);
  }
  return new_memory;
#  else
  (void) fprintf(stderr,
                 "over-aligned zeroed-alloc of alignment %" PRIu64
                 " not supported\n",
                 (u64) alignment);
  (void) fflush(stderr);
  return nullptr;
#  endif
#endif
}

void HeapInterface::deallocate(Allocator self, usize alignment, void *memory,
                               usize size)
{
  (void) self;
  (void) alignment;
  (void) memory;
  (void) size;
  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    free(memory);
    return;
  }

#if WINDOWS_CRT_ALIGNED_ALLOC_SUPPORTED
  _aligned_free(memory);
#else
#  if STDC_ALIGNED_ALLOC_SUPPORTED
  free(memory);
#  else
  (void) fprintf(stderr,
                 "over-aligned malloc of alignment %" PRIu64 " not supported\n",
                 (u64) alignment);
  (void) fflush(stderr);
  return nullptr;
#  endif
#endif
}

}        // namespace ash
