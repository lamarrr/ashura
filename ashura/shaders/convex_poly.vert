#version 450

layout(location = 0) in vec2 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec4 i_color;

layout(push_constant) uniform PushConstant
{
  mat3 transform;
}
push_constant;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_color;

void main()
{
  gl_Position = vec4(push_constant.transform * vec3(i_position, 1.0f), 1.0f);
  o_uv        = i_uv;
  o_color     = i_color;
}
