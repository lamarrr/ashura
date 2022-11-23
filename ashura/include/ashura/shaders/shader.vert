#version 450

layout(location = 0) in vec2 in_position;

layout(location = 0) out vec2 out_position;

// to use sets and bindings type must be a struct
layout(set = 0, binding = 0) uniform Transform {
    mat4 value;
} transform;

void main() {
    gl_Position = transform.value * vec4(in_position, 0.0f, 0.0f);
    out_position = in_position;
}
