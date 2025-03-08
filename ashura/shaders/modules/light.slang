/// SPDX-License-Identifier: MIT

#pragma once

float square_falloff_attenuation(float3 pos_to_light, float light_inv_radius)
{
  float dist_square = dot(pos_to_light, pos_to_light);
  float factor      = dist_square * light_inv_radius * light_inv_radius;
  float smoothness  = max(1.0 - factor * factor, 0.0);
  return (smoothness * smoothness) / max(dist_square, 1e-4);
}

float spot_angle_attenuation(float3 l, float3 light_dir, float inner_angle,
                             float outer_angle)
{
  // the scale and offset computations can be done CPU-side
  float cos_outer   = cos(outer_angle);
  float spot_scale  = 1.0 / max(cos(inner_angle) - cos_outer, 1e-4);
  float spot_offset = -cos_outer * spot_scale;
  float cd          = dot(normalize(-light_dir), l);
  float attenuation = clamp(cd * spot_scale + spot_offset, 0.0, 1.0);
  return attenuation * attenuation;
}
