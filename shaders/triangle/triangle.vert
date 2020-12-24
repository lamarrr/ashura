#version 450

layout(set = 0, binding = 0) uniform ViewParameters {
  mat4 model;
  mat4 camera_view;
  mat4 camera_projection;
};

layout(location = 0) in vec3 in_fragment_position;
layout(location = 1) in vec3 in_fragment_color;

layout(location = 0) out vec4 out_fragment_color;

// down is positive y

void main() {
  gl_Position =
      camera_projection * camera_view * model * vec4(in_fragment_position, 1.0);
  out_fragment_color = vec4(in_fragment_color, 1.0);
}
