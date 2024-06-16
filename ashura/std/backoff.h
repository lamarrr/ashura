#pragma once
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include <emmintrin.h>
#include <thread>

namespace ash
{

inline void yielding_backoff(u64 poll)
{
  if (poll < 8)
  {
    return;
  }
  if (poll < 16)
  {
    return _mm_pause();
  }
  return std::this_thread::yield();
}

inline void sleepy_backoff(u64 poll, nanoseconds sleep)
{
  if (poll < 8)
  {
    return;
  }
  if (poll < 16)
  {
    return _mm_pause();
  }
  if (poll <= 64)
  {
    return std::this_thread::yield();
  }
  return std::this_thread::sleep_for(sleep);
}

}        // namespace ash