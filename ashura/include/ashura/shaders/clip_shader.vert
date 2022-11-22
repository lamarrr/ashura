#version 460

layout(location = 0) in vec2 in_position;

layout(set = 0, binding = 0) uniform Transform {
    mat4 transform;
} mvp;

void main() {
    gl_Position = mvp.transform * vec4(in_position, 0.0f, 0.0f);
}
