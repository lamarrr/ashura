#version 460

// only binding 0 is possible for inputs and outputs
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 out_position;

// to use sets and bindings type must be a struct
layout(set = 0, binding = 0) uniform A {
    mat4 a;
    mat4 b;
    mat4 c;
} xc;

layout(set = 0, binding = 1) uniform B {
    mat4 a;
} a;

layout(set = 1, binding = 0) uniform sampler2D xsampl;

void main() {
    gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
}