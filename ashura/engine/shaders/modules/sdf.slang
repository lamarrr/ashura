/// SPDX-License-Identifier: MIT

#pragma once

#include "types.slang"

/// see: https://www.shadertoy.com/view/DlVcW1
f32 exponential_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 1.0;
  f32 r = exp2(-a / k) + exp2(-b / k);
  return -k * log2(r);
}

f32 root_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 2.0;
  f32 x = b - a;
  return 0.5 * (a + b - sqrt(x * x + k * k));
}

f32 sigmoid_smoothmin(f32 a, f32 b, f32 k)
{
  k *= log(2.0);
  f32 x = b - a;
  return a + x / (1.0 - exp2(x / k));
}

f32 quadratic_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 4.0;
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * k * (1.0 / 4.0);
}

f32 cubic_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 6.0;
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

f32 quartic_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 16.0 / 3.0;
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * (4.0 - h) * k * (1.0 / 16.0);
}

f32 circular_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  f32 h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - k * 0.5 * (1.0 + h - sqrt(1.0 - h * (h - 2.0)));
}

f32 circular_geometrical_smoothmin(f32 a, f32 b, f32 k)
{
  k *= 1.0 / (1.0 - sqrt(0.5));
  return max(k, min(a, b)) - length(max(k - f32x2(a, b), 0.0));
}

// https://www.shadertoy.com/view/fsdyzB
// https://iquilezles.org/articles/distfunctions/
// https://iquilezles.org/articles/distfunctions2d/
// length(...+ corner_radii) - corner_radii -> gives the rounding of the
// corners
f32 rrect_sdf(f32x2 pos, f32x2 half_extent, f32 corner_radii)
{
  f32x2 q = abs(pos) - half_extent + corner_radii;
  return min(max(q.x, q.y), 0) + length(max(q, 0)) - corner_radii;
}

f32 msdf_screen_px_range(f32 msd_px_range, f32x2 texture_size, f32x2 tex_coord)
{
  f32x2 unit_range      = f32x2(msd_px_range) / texture_size;
  f32x2 screen_tex_size = f32x2(1.0) / fwidth(tex_coord);
  return max(0.5 * dot(unit_range, screen_tex_size), 1.0);
}

f32 msdf_median(f32 r, f32 g, f32 b)
{
  return max(min(r, g), min(max(r, g), b));
}

// https://github.com/Chlumsky/msdfgen/tree/master?tab=readme-ov-file#using-a-multi-channel-distance-field
f32 msdf(f32x3 msd, f32 msd_px_range, f32x2 texture_size, f32x2 tex_coord)
{
  f32 sd = msdf_median(msd.r, msd.g, msd.b);
  f32 screen_px_distance =
    msdf_screen_px_range(msd_px_range, texture_size, tex_coord) * (sd - 0.5);
  return clamp(screen_px_distance + 0.5, 0.0, 1.0);
}

// [ ] document
f32x4 msdf_blend(f32x3 msd, f32 msd_px_range, f32x2 texture_size,
                 f32x2 tex_coord, f32x4 bg_color, f32x4 fg_color)
{
  f32 alpha = msdf(msd, msd_px_range, texture_size, tex_coord);
  return lerp(bg_color, fg_color, alpha);
}
