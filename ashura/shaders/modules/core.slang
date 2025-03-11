/// SPDX-License-Identifier: MIT

#pragma once

#include "types.slang"

static constexpr f32 PI      = 3.1415926535897932384626433832795;
static constexpr f32 GAMMA   = 2.2;
static constexpr f32 EPSILON = 0.00000011920929;

f32x3 pow_v(f32x3 v, f32 p)
{
  return f32x3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

f32x3 to_linear(f32x3 v)
{
  return pow_v(v, GAMMA);
}

f32x3 to_srgb(f32x3 v)
{
  return pow_v(v, 1.0 / GAMMA);
}

float4 bilerp(float4 v[4], float2 t)
{
  return lerp(lerp(v[0], v[1], t.x), lerp(v[2], v[3], t.x), t.y);
}
