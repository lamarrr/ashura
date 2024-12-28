/// SPDX-License-Identifier: MIT
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/mem.h"
#include <cstring>

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

NoopAllocator noop_allocator_impl{};
HeapAllocator heap_allocator_impl{};

bool HeapAllocator::alloc(usize alignment, usize size, u8 *& mem)
{
  if (size == 0)
  {
    mem = nullptr;
    return true;
  }

  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 * m = (u8 *) std::malloc(size); m != nullptr)
    {
      mem = m;
      return true;
    }
    mem = nullptr;
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 * m = (u8 *) _aligned_malloc(size, alignment); m != nullptr)
  {
    mem = m;
    return true;
  }
  mem = nullptr;
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  if (u8 * m = (u8 *) std::aligned_alloc(alignment, size); m != nullptr)
  {
    mem = m;
    return true;
  }
  mem = nullptr;
  return false;
#endif

#if !HAS_ALIGNED_ALLOC
  (void) std::fprintf(
    stderr, "over-aligned malloc of alignment %" PRIu64 " not supported\n",
    (u64) alignment);
  (void) std::fflush(stderr);
  mem = nullptr;
  return false;
#endif
}

bool HeapAllocator::zalloc(usize alignment, usize size, u8 *& mem)
{
  if (size == 0)
  {
    mem = nullptr;
    return true;
  }

  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 * m = (u8 *) std::calloc(size, 1); m != nullptr)
    {
      mem = m;
      return true;
    }
    mem = nullptr;
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 * m = (u8 *) _aligned_malloc(size, alignment); m != nullptr)
  {
    std::memset(m, 0, size);
    mem = m;
    return true;
  }
  mem = nullptr;
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  if (u8 * m = (u8 *) std::aligned_alloc(alignment, size); m != nullptr)
  {
    std::memset(m, 0, size);
    mem = m;
    return true;
  }
  mem = nullptr;
  return false;
#endif

#if !HAS_ALIGNED_ALLOC
  (void) std::fprintf(stderr,
                      "over-aligned zeroed-alloc of alignment %" PRIu64
                      " not supported\n",
                      (u64) alignment);
  (void) std::fflush(stderr);
  mem = nullptr;
  return false;
#endif
}

bool HeapAllocator::realloc(usize alignment, usize old_size, usize new_size,
                            u8 *& mem)
{
  if (new_size == 0)
  {
    HeapAllocator::dealloc(alignment, old_size, mem);
    mem = nullptr;
    return true;
  }

  // preserve mem if allocation fails

  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 * m = (u8 *) std::realloc(mem, new_size); m != nullptr)
    {
      mem = m;
      return true;
    }
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 * m = (u8 *) _aligned_realloc(mem, new_size, alignment); m != nullptr)
  {
    mem = m;
    return true;
  }
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  // stdc realloc doesn't guarantee preservation of alignment
  if (u8 * m = (u8 *) std::aligned_alloc(alignment, new_size); m != nullptr)
  {
    (void) std::memcpy(m, mem, old_size);
    std::free(mem);
    mem = m;
    return true;
  }
  return false;
#endif

#if !HAS_ALIGNED_ALLOC
  std::fprintf(stderr,
               "over-aligned zeroed-alloc of alignment %" PRIu64
               " not supported\n",
               (u64) alignment);
  (void) std::fflush(stderr);
  return false;
#endif
}

void HeapAllocator::dealloc(usize alignment, usize size, u8 * mem)
{
  (void) alignment;
  (void) mem;
  (void) size;
  if (alignment <= MAX_STANDARD_ALIGNMENT)
  {
    std::free(mem);
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

}    // namespace ash
