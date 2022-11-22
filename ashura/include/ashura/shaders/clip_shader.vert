#version 460

layout(location = 0) in vec2 in_position;

layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main() {
    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(in_position, 0.0f, 0.0f);
}
