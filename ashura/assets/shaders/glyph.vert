#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(push_constant) uniform PushConstant {
  mat3 transform;
} push_constant;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

void main() {
  gl_Position = vec4(push_constant.transform * vec3(in_position, 1.0f), 1.0f);
  out_uv = in_uv;
  out_color = in_color;
}
