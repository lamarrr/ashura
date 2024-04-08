#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"

layout(location = 0) in vec3 in_local_position;
layout(location = 1) in vec2 in_st;

layout(set = 0, binding = 0) uniform Uniforms
{
  ViewTransform transform;
  vec4          camera_position;
  vec4          light_positions[8];
  vec4          light_colors[8];
  int           nlights;
}
uniforms;

layout(location = 0) out vec3 out_world_position;
layout(location = 1) out vec2 out_st;

void main()
{
  vec4 position      = to_mvp(uniforms.transform) * vec4(in_local_position, 1);
  gl_Position        = position;
  out_world_position = position.xyz;
  out_st             = in_st;
}
