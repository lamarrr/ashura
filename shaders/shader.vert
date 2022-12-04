#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_st;

layout(set = 0, binding = 0) uniform Transform { mat4 value; }
transform;

layout(location = 0) out vec2 out_st;

void main() {
  gl_Position = transform.value * vec4(in_position, 0.f, 1.f);
  in_st = out_st;
}
