#version 450

layout(set = 0, binding = 0) uniform ViewParameters {
  vec2 reserved;
  mat4 model;
  mat4 camera_view;
  mat4 camera_projection;
};

layout(location = 0) in vec3 in_fragment_position;
layout(location = 1) in vec2 in_fragment_texture_coordinates;

layout(location = 0) out vec2 out_fragment_texture_coordinates;

// down is positive y

void main() {
  gl_Position =
      camera_projection * camera_view * model * vec4(in_fragment_position, 1.0);
  out_fragment_texture_coordinates = in_fragment_texture_coordinates;
}
