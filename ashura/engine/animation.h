/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// [ ] Animation forward and backwards
/// [ ] Animation cancelation
/// [ ] https://create.roblox.com/docs/ui/animation#style
/// [ ] must be possible to make it complete at a speed/framerate, i.e. during
/// automated testing

/// @brief Frame-independent smooth animation ticker
///
///
/// @param t [0, 1]
/// @param iterations [0, F32_MAX]
/// @param cycle [0, F32_MAX]
struct AnimationTick
{
  f32  duration   = 1;
  f32  iterations = 1;
  f32  speed      = 1;
  f32  elapsed    = 0;
  f32  t          = 0;
  f32  cycle      = 0;
  bool loop       = false;
  bool alternate  = false;

  void pause()
  {
    speed = 0;
  }

  void resume()
  {
    speed = 1;
  }

  void reset();

  f32 tick(f32 dt)
  {
    damplerp(0, 0, 0, 0);

    return 0;
  }
};

}        // namespace ash
