#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

layout(location = 0) out vec2 o_pos;

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  float         radii[4];
  vec2          uv[2];
  vec4          tint;
  vec4          border_color;
  float         border_thickness;
  float         border_softness;
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
  o_pos       = i_pos;
}