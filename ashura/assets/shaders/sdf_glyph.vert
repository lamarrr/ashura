#version 450

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

layout(push_constant) uniform GlyphProps {
    mat3 transform;
    vec4 stroke_color;
    vec4 outline_color;
    float outline_min_0;
    float outline_min_1;
    float outline_max_0;
    float outline_max_1;
    float soft_edge_min;
    float soft_edge_max;
    uint flags;
} props;

layout(location = 0) out vec2 out_uv;


void main() {
    gl_Position = vec4(props.transform * vec3(in_position, 1.0), 1.0);
    out_uv = in_uv;
}
