/// SPDX-License-Identifier: MIT

#pragma once

#include "types.slang"

/// SIGGRAPH 2015 - Bandwidth-Efficient Rendering, Marius Bjorge, ARM
/// (https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_notes.pdf)
/// KAWASE multi-tap downsampling
f32x4 downsample(SamplerState s, Texture2D t, f32x2 uv, f32x2 radius)
{
  f32x4 sum = t.Sample(s, uv) * f32x4(4.0) + t.Sample(s, uv + radius) +
              t.Sample(s, uv - radius) +
              t.Sample(s, uv + f32x2(radius.x, -radius.y)) +
              t.Sample(s, uv + f32x2(-radius.x, radius.y));
  return sum * (1 / 8.0);
}

f32x4 downsample_clamped(SamplerState s, Texture2D t, f32x2 uv, f32x2 radius,
                         f32x2 min_uv, f32x2 max_uv)
{
  f32x4 sum =
    t.Sample(s, clamp(uv, min_uv, max_uv)) * f32x4(4.0) +
    t.Sample(s, clamp(uv + radius, min_uv, max_uv)) +
    t.Sample(s, clamp(uv - radius, min_uv, max_uv)) +
    t.Sample(s, clamp(uv + f32x2(radius.x, -radius.y), min_uv, max_uv)) +
    t.Sample(s, clamp(uv + f32x2(-radius.x, radius.y), min_uv, max_uv));
  return sum * (1 / 8.0);
}

/// SIGGRAPH 2015 - Bandwidth-Efficient Rendering, Marius Bjorge, ARM
/// KAWASE multi-tap upsampling
f32x4 upsample(SamplerState s, Texture2D t, f32x2 uv, f32x2 radius)
{
  f32x4 sum = t.Sample(s, uv + f32x2(-radius.x * 2, 0)) +
              t.Sample(s, uv + f32x2(-radius.x, radius.y)) * 2.0 +
              t.Sample(s, uv + f32x2(0, radius.y * 2)) +
              t.Sample(s, uv + f32x2(radius.x, radius.y)) * 2.0 +
              t.Sample(s, uv + f32x2(radius.x * 2, 0)) +
              t.Sample(s, uv + f32x2(radius.x, -radius.y)) * 2.0 +
              t.Sample(s, uv + f32x2(0, -radius.y * 2)) +
              t.Sample(s, uv + f32x2(-radius.x, -radius.y)) * 2.0;
  return sum * (1 / 12.0);
}

f32x4 upsample_clamped(SamplerState s, Texture2D t, f32x2 uv, f32x2 radius,
                       f32x2 min_uv, f32x2 max_uv)
{
  f32x4 sum =
    t.Sample(s, clamp(uv + f32x2(-radius.x * 2, 0), min_uv, max_uv)) +
    t.Sample(s, clamp(uv + f32x2(-radius.x, radius.y), min_uv, max_uv)) * 2.0 +
    t.Sample(s, clamp(uv + f32x2(0, radius.y * 2), min_uv, max_uv)) +
    t.Sample(s, clamp(uv + f32x2(radius.x, radius.y), min_uv, max_uv)) * 2.0 +
    t.Sample(s, clamp(uv + f32x2(radius.x * 2, 0), min_uv, max_uv)) +
    t.Sample(s, clamp(uv + f32x2(radius.x, -radius.y), min_uv, max_uv)) * 2.0 +
    t.Sample(s, clamp(uv + f32x2(0, -radius.y * 2), min_uv, max_uv)) +
    t.Sample(s, clamp(uv + f32x2(-radius.x, -radius.y), min_uv, max_uv)) * 2.0;
  return sum * (1 / 12.0);
}
