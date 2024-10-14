/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/time.h"
#include "ashura/std/types.h"

#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
#  include <emmintrin.h>
#endif

#include <thread>

namespace ash
{

inline void yielding_backoff(u64 poll)
{
  if (poll < 8)
  {
    return;
  }

#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
  if (poll < 16)
  {
    return _mm_pause();
  }
#endif
  return std::this_thread::yield();
}

inline void sleepy_backoff(u64 poll, nanoseconds sleep)
{
  if (poll < 8)
  {
    return;
  }

#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
  if (poll < 16)
  {
    return _mm_pause();
  }
#endif

  if (poll <= 64)
  {
    return std::this_thread::yield();
  }
  return std::this_thread::sleep_for(sleep);
}

}        // namespace ash