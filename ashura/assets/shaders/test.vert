#version 450

// layout(location = 0) in vec2 in_position;
// layout(location = 1) in vec2 in_uv;
// layout(location = 2) in vec4 in_color;

layout(binding = 0) uniform Buf {
  mat3 transform;
  vec2 uv;
  vec3 normal;
} c;

// layout(set = 2, binding = 2, rgba8ui) uniform writeonly uimage2D im;
//
// layout(location = 0) out vec2 out_uv;
// layout(location = 1) out vec4 out_color;

void main() {
  gl_Position = vec4(0);
}
