#pragma once
#include "ashura/std/types.h"

namespace ash
{

struct PunctualLight
{
  Vec4  direction;        // xyz
  Vec4  position;         // xyz
  Vec4  color;
  float inner_angle = 0;
  float outer_angle = 0;
  float intensity   = 0;
  float radius      = 0;
};

}        // namespace ash
