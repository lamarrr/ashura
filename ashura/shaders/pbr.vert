#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"
#include "pbr.glsl"

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_uv;

layout(set = 0, binding = 0) uniform Params
{
  PBRParams p;
};

layout(location = 0) out vec3 o_pos;
layout(location = 1) out vec3 o_world_pos;
layout(location = 2) out vec2 o_uv;

void main()
{
  vec4 position       = to_mvp(p.transform) * vec4(i_pos, 1);
  vec4 world_position = affine4(p.transform.model) * vec4(i_pos, 1);
  gl_Position         = position;
  o_pos               = position.xyz;
  o_world_pos         = world_position.xyz;
  o_uv                = i_uv;
}
