
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"

#if (!ASH_CFG(OS, WINDOWS)) && \
    (ASH_CFG(COMPILER, CLANG) || ASH_CFG(COMPILER, GNUC))
#  define USE_STDC_ALIGNED_ALLOC 1
#else
#  define USE_STDC_ALIGNED_ALLOC 0
#endif

#if ASH_CFG(OS, WINDOWS)
#  define USE_WIN32CRT_ALIGNED_ALLOC 1
#else
#  define USE_WIN32CRT_ALIGNED_ALLOC 0
#endif

#if (USE_STDC_ALIGNED_ALLOC || USE_WIN32CRT_ALIGNED_ALLOC)
#  define HAS_ALIGNED_ALLOC 1
#else
#  define HAS_ALIGNED_ALLOC 0
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace ash
{

bool HeapInterface::alloc(Allocator self, usize alignment, usize size, u8 **mem)
{
  (void) self;
  *mem = nullptr;

  if (size == 0)
  {
    return true;
  }

  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 *m = (u8 *) std::malloc(size); m != nullptr)
    {
      *mem = m;
      return true;
    }
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 *m = (u8 *) _aligned_malloc(size, alignment); m != nullptr)
  {
    *mem = m;
    return true;
  }
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  if (u8 *m = (u8 *) std::aligned_alloc(alignment, size); m != nullptr)
  {
    *mem = m;
    return true;
  }
  return false;
#endif

#if !HAS_ALIGNED_ALLOC
  (void) std::fprintf(
      stderr, "over-aligned malloc of alignment %" PRIu64 " not supported\n",
      (u64) alignment);
  (void) std::fflush(stderr);
  return false;
#endif
}

bool HeapInterface::alloc_zeroed(Allocator self, usize alignment, usize size,
                                 u8 **mem)
{
  (void) self;

  if (size == 0)
  {
    *mem = nullptr;
    return true;
  }

  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 *m = (u8 *) std::calloc(size, 1); m != nullptr)
    {
      *mem = m;
      return true;
    }
    *mem = nullptr;
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 *m = (u8 *) _aligned_malloc(size, alignment); m != nullptr)
  {
    std::memset(m, 0, size);
    *mem = m;
    return true;
  }
  *mem = nullptr;
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  if (u8 *m = (u8 *) std::aligned_alloc(alignment, size); m != nullptr)
  {
    std::memset(m, 0, size);
    *mem = m;
    return true;
  }
  *mem = nullptr;
  return false;
#endif

#if !HAS_ALIGNED_ALLOC
  (void) std::fprintf(stderr,
                      "over-aligned zeroed-alloc of alignment %" PRIu64
                      " not supported\n",
                      (u64) alignment);
  (void) std::fflush(stderr);
  *mem = nullptr;
  return false;
#endif
}

bool HeapInterface::realloc(Allocator self, usize alignment, usize old_size,
                            usize new_size, u8 **mem)
{
  if (new_size == 0)
  {
    HeapInterface::dealloc(self, alignment, *mem, old_size);
    *mem = nullptr;
    return true;
  }

  // preserve mem if allocation fails

  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 *m = (u8 *) std::realloc(*mem, new_size); m != nullptr)
    {
      *mem = m;
      return true;
    }
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 *m = (u8 *) _aligned_realloc(*mem, new_size, alignment); m != nullptr)
  {
    *mem = m;
    return true;
  }
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  // stdc realloc doesn't guarantee preservation of alignment
  if (u8 *m = (u8 *) std::aligned_alloc(alignment, new_size); m != nullptr)
  {
    (void) std::memcpy(m, *mem, old_size);
    std::free(*mem);
    *mem = m;
  }
  return false;
#endif

#if !HAS_ALIGNED_ALLOC
  fprintf(stderr,
          "over-aligned zeroed-alloc of alignment %" PRIu64 " not supported\n",
          (u64) alignment);
  (void) std::fflush(stderr);
  return false;
#endif
}

void HeapInterface::dealloc(Allocator self, usize alignment, u8 *mem,
                            usize size)
{
  (void) self;
  (void) alignment;
  (void) mem;
  (void) size;
  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    free(mem);
    return;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  _aligned_free(mem);
#endif

#if USE_STDC_ALIGNED_ALLOC
  std::free(mem);
#endif

#if !HAS_ALIGNED_ALLOC
  (void) std::fprintf(
      stderr, "over-aligned malloc of alignment %" PRIu64 " not supported\n",
      (u64) alignment);
  (void) std::fflush(stderr);
#endif
}

}        // namespace ash
