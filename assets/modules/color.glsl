#version 450
#extension GL_GOOGLE_include_directive : require

#include "constants.glsl"

vec3 pow_vec3(vec3 v, float p)
{
  return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 to_linear(vec3 v)
{
  return pow_vec3(v, GAMMA);
}

vec3 to_srgb(vec3 v)
{
  return pow_vec3(v, 1.0 / GAMMA);
}
