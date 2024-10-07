/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

struct PunctualLight
{
  Vec4 direction   = {0, 0, 0, 0};        // xyz
  Vec4 position    = {0, 0, 0, 0};        // xyz
  Vec4 color       = {1, 1, 1, 1};
  f32  inner_angle = 0;
  f32  outer_angle = 0;
  f32  intensity   = 0;
  f32  radius      = 0;
};

}        // namespace ash
