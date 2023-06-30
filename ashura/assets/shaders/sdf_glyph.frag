#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;

layout(push_constant) uniform GlyphProps {
    float thickness; // thickness will range from 0 to 1
    float softness;
    float outline_thickness;
    float outline_softness;
    vec4 color;
    vec4 outline_color;
    mat4 transform;
} props;

layout(set = 0, binding = 0) uniform sampler2D sdf_texture; // VK_FORMAT_R8_UNORM

layout(location = 0) out vec4 out_color;

void main() {
    float signed_distance = texture(sdf_texture, in_uv).r;
    float alpha = smoothstep(1 - props.thickness - props.softness, 1 - props.thickness + props.softness, signed_distance);
    float outline = smoothstep(props.outline_thickness - props.outline_softness, props.outline_thickness + props.outline_softness, signed_distance);
    out_color = vec4(mix(props.outline_color.rgb, props.color.rgb, outline), alpha);
}