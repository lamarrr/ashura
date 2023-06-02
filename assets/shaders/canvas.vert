#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(push_constant) uniform CanvasPushConstants
{
  mat4 transform;
}
push_constants;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

void main()
{
  gl_Position = push_constants.transform * vec4(in_position, 0.f, 1.f);
  out_uv      = in_uv;
  out_color   = in_color;
}
