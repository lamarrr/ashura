#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_st;

layout(push_constant) uniform PushConstants {
  mat4 transform;
  vec4 overlay;
  vec4 texture_multiplier;
}
push_constants;

layout(location = 0) out vec2 out_st;

void main() {
  gl_Position = push_constants.transform * vec4(in_position, 0.f, 1.f);
  out_st = in_st;
}
