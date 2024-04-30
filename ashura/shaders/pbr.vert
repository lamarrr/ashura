#version 450
#extension GL_GOOGLE_include_directive : require

#include "core.glsl"
#include "pbr.glsl"

layout(location = 0) in vec3 i_pos;
layout(location = 1) in vec2 i_uv;

layout(set = 0, binding = 0) uniform Params
{
  ViewTransform transform;
  vec4          albedo;        // only xyz
  float         metallic;
  float         roughness;
  float         normal;
  float         occlusion;
  vec4          emissive;        // only xyz
}
u_params;

layout(location = 0) out vec3 o_pos;
layout(location = 1) out vec2 o_uv;

void main()
{
  vec4 position = to_mvp(u_params.transform) * vec4(i_pos, 1);
  gl_Position   = position;
  o_pos         = position.xyz;
  o_uv          = i_uv;
}
