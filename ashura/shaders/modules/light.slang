/// SPDX-License-Identifier: MIT

#pragma once

#include "types.slang"

f32 square_falloff_attenuation(f32x3 pos_to_light, f32 light_inv_radius)
{
  f32 dist_square = dot(pos_to_light, pos_to_light);
  f32 factor      = dist_square * light_inv_radius * light_inv_radius;
  f32 smoothness  = max(1.0 - factor * factor, 0.0);
  return (smoothness * smoothness) / max(dist_square, 1e-4);
}

f32 spot_angle_attenuation(f32x3 l, f32x3 light_dir, f32 inner_angle,
                           f32 outer_angle)
{
  // the scale and offset computations can be done CPU-side
  f32 cos_outer   = cos(outer_angle);
  f32 spot_scale  = 1.0 / max(cos(inner_angle) - cos_outer, 1e-4);
  f32 spot_offset = -cos_outer * spot_scale;
  f32 cd          = dot(normalize(-light_dir), l);
  f32 attenuation = clamp(cd * spot_scale + spot_offset, 0.0, 1.0);
  return attenuation * attenuation;
}
