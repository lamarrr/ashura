#ifndef RRECT_GLSL
#define RRECT_GLSL

#extension GL_GOOGLE_include_directive : require
#include "core.glsl"

struct RRectParams
{
  ViewTransform transform;
  vec4          radii;
  vec4          uv;
  vec4          tint[4];
  vec2          aspect_ratio;
};

#endif