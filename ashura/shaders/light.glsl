#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

#include "core.glsl"

float get_square_falloff_attenuation(vec3 pos_to_light, float light_inv_radius)
{
  float distance_square = dot(pos_to_light, pos_to_light);
  float factor          = distance_square * light_inv_radius * light_inv_radius;
  float smooth_factor   = max(1.0 - factor * factor, 0.0);
  return (smooth_factor * smooth_factor) / max(distance_square, 1e-4);
}

float get_spot_angle_attenuation(vec3 l, vec3 light_dir, float inner_angle,
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

struct PunctualLight
{
  vec4  direction;        // xyz
  vec4  position;         // xyz
  vec4  color;
  float inner_angle;
  float outer_angle;
  float intensity;
  float radius;
};

#endif