#version 460

layout(binding = 0, location = 0) in vec3 in_position;
layout(binding = 0, location = 1) in vec3 color;

layout(binding = 1, location = 0) in vec2 uv;

layout(location = 0) out vec3 out_position;

layout(set = 0, binding = 0) uniform mat4 projection;
layout(set = 0, binding = 1) uniform mat4 view;
layout(set = 0, binding = 2) uniform mat4 model;
layout(set = 0, binding = 3) uniform sampler2D texture;

layout(set = 1, binding = 0) uniform vec4 color;

void main() {
    gl_Position = vec4();
}