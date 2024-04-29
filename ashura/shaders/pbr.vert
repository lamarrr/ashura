#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"
#include "pbr.glsl"

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_st;

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  vec4          base_color_factor;
  float         metallic_factor;
  float         roughness_factor;
  float         normal_scale;
  float         occlusion_strength;
  vec4          emissive_factor;
  float         emissive_strength;
}
u_params;

layout(location = 0) out vec3 o_pos;
layout(location = 1) out vec2 o_st;

void main()
{
  vec4 position = to_mvp(u_params.transform) * vec4(i_pos, 1);
  gl_Position   = position;
  o_pos         = position.xyz;
  o_st          = i_st;
}
