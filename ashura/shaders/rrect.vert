#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "core.glsl"
#include "rrect.glsl"

layout(location = 0) out vec2 o_pos;
layout(location = 1) out vec4 o_color;
layout(location = 2) flat out uint o_instance;

layout(std140, set = 0, binding = 0) readonly buffer Params
{
  RRectParams params[];
};

const uint INDEX_BUFFER[]  = {0, 1, 2, 2, 3, 0};
const vec2 VERTEX_BUFFER[] = {vec2(-1, -1), vec2(1, -1), vec2(1, 1),
                              vec2(-1, 1)};

void main()
{
  RRectParams p      = params[gl_InstanceIndex];
  uint        vertex = INDEX_BUFFER[gl_VertexIndex];
  vec2        i_pos  = VERTEX_BUFFER[vertex];
  gl_Position        = to_mvp(p.transform) * vec4(i_pos, 0, 1);
  o_pos              = i_pos * p.aspect_ratio;
  o_color            = p.tint[vertex];
  o_instance         = gl_InstanceIndex;
}