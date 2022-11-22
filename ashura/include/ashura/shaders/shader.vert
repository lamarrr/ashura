#version 460

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec3 out_position;

// to use sets and bindings type must be a struct
layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

void main() {
    gl_Position = mvp.projection * mvp.view * mvp.model * vec4(in_position, 0.0f);
    out_position = in_position;
}
