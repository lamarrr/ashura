#ifndef _RRECT_GLSL_
#define _RRECT_GLSL_

#extension GL_GOOGLE_include_directive : require
#include "core.glsl"

struct RRectParams
{
  ViewTransform transform;
  vec4          radii;
  vec4          uv;
  vec4          tint[4];
  vec2          aspect_ratio;
  uint          albedo;
};

#endif