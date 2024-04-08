#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  Edge          tint;
  Edge          border_color;
  vec4          border_radii;
  float         border_thickness;
  vec2          uv0;
  vec2          uv1;
}
params;

layout(set = 1, binding = 0) uniform sampler2D base_color;

void main()
{
}