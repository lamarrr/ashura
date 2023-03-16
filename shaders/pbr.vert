#version 450

layout(location = 0) in vec3 in_local_position;
layout(location = 1) in vec2 in_st;

layout(set = 0, binding = 0) uniform Uniforms
{
  mat4 model_view_projection;
  vec4 camera_position;
  vec4 light_positions[8];
  vec4 light_colors[8];
  int  nlights;
}
uniforms;

layout(location = 0) out vec3 out_world_position;
layout(location = 1) out vec2 out_st;

void main()
{
  out_world_position = (uniforms.model_view_projection * vec4(in_local_position, 1)).xyz;
  out_st             = in_st;
}
