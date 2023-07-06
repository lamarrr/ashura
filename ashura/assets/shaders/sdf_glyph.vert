#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

layout(push_constant) uniform GlyphProps {
    mat3 transform;
    float thickness;
    float outline_thickness;
    vec4 color;
    vec4 outline_color;
} props;

layout(location = 0) out vec2 out_uv;

void main() {
    gl_Position = vec4(props.transform * vec3(in_position, 1.0f), 1.0f);
    out_uv = in_uv;
}
