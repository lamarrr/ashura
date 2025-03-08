/// SPDX-License-Identifier: MIT

#pragma once

static constexpr float PI      = 3.1415926535897932384626433832795;
static constexpr float GAMMA   = 2.2;
static constexpr float EPSILON = 0.00000011920929;

float3 pow_v(float3 v, float p)
{
  return float3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

float3 to_linear(float3 v)
{
  return pow_v(v, GAMMA);
}

float3 to_srgb(float3 v)
{
  return pow_v(v, 1.0 / GAMMA);
}

float4 bilerp(float4 v[4], float2 t)
{
  return lerp(lerp(v[0], v[1], t.x), lerp(v[2], v[3], t.x), t.y);
}
