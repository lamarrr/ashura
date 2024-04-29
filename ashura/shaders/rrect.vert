#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

layout(location = 0) out vec2 o_pos;
layout(location = 1) out vec4 o_color;

layout(std140, set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  vec4          radii;
  vec4          uv;
  vec4          tint[4];
  vec2          aspect_ratio;
}
u_params;

const uint INDEX_BUFFER[]  = {0, 1, 2, 2, 3, 0};
const vec2 VERTEX_BUFFER[] = {vec2(-1, -1), vec2(1, -1), vec2(1, 1),
                              vec2(-1, 1)};

void main()
{
  uint vertex = INDEX_BUFFER[gl_VertexIndex];
  vec2 i_pos  = VERTEX_BUFFER[vertex];
  gl_Position = to_mvp(u_params.transform) * vec4(i_pos, 0, 1);
  o_pos       = i_pos * u_params.aspect_ratio;
  o_color     = u_params.tint[vertex];
}