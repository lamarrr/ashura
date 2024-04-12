#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

layout(location = 0) in vec2 i_rel_position;
layout(location = 0) out vec2 o_rel_position;

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  vec4          tint[4];
  vec4          border_color[4];
  vec4          border_radii;
  float         border_thickness;
  vec2          uv0;
  vec2          uv1;
}
u_params;

void main()
{
  vec4 position  = to_mvp(u_params.transform) * vec4(i_rel_position, 0, 1);
  gl_Position    = position;
  o_rel_position = i_rel_position;
}