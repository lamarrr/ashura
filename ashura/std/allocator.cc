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

bool HeapAllocator::alloc(Layout layout, u8 *& mem)
{
  if (layout.size == 0)
  {
    mem = nullptr;
    return true;
  }

  if (layout.alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 * m = (u8 *) std::malloc(layout.size); m != nullptr)
    {
      mem = m;
      return true;
    }
    mem = nullptr;
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 * m = (u8 *) _aligned_malloc(layout.size, layout.alignment);
      m != nullptr)
  {
    mem = m;
    return true;
  }
  mem = nullptr;
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  if (u8 * m = (u8 *) std::aligned_alloc(layout.alignment, layout.size);
      m != nullptr)
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
    (u64) layout.alignment);
  (void) std::fflush(stderr);
  mem = nullptr;
  return false;
#endif
}

bool HeapAllocator::zalloc(Layout layout, u8 *& mem)
{
  if (layout.size == 0)
  {
    mem = nullptr;
    return true;
  }

  if (layout.alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 * m = (u8 *) std::calloc(layout.size, 1); m != nullptr)
    {
      mem = m;
      return true;
    }
    mem = nullptr;
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 * m = (u8 *) _aligned_malloc(layout.size, layout.alignment);
      m != nullptr)
  {
    std::memset(m, 0, layout.size);
    mem = m;
    return true;
  }
  mem = nullptr;
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  if (u8 * m = (u8 *) std::aligned_alloc(layout.alignment, layout.size);
      m != nullptr)
  {
    std::memset(m, 0, layout.size);
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

bool HeapAllocator::realloc(Layout layout, usize new_size, u8 *& mem)
{
  if (new_size == 0)
  {
    HeapAllocator::dealloc(layout, mem);
    mem = nullptr;
    return true;
  }

  // preserve mem if allocation fails

  if (layout.alignment <= MAX_STANDARD_ALIGNMENT)
  {
    if (u8 * m = (u8 *) std::realloc(mem, new_size); m != nullptr)
    {
      mem = m;
      return true;
    }
    return false;
  }

#if USE_WIN32CRT_ALIGNED_ALLOC
  if (u8 * m = (u8 *) _aligned_realloc(mem, new_size, layout.alignment);
      m != nullptr)
  {
    mem = m;
    return true;
  }
  return false;
#endif

#if USE_STDC_ALIGNED_ALLOC
  // stdc realloc doesn't guarantee preservation of alignment
  if (u8 * m = (u8 *) std::aligned_alloc(layout.alignment, new_size);
      m != nullptr)
  {
    (void) std::memcpy(m, mem, layout.size);
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
               (u64) layout.alignment);
  (void) std::fflush(stderr);
  return false;
#endif
}

void HeapAllocator::dealloc(Layout layout, u8 * mem)
{
  (void) layout;
  (void) mem;
  if (layout.alignment <= MAX_STANDARD_ALIGNMENT)
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
    (u64) layout.alignment);
  (void) std::fflush(stderr);
#endif
}

}    // namespace ash
