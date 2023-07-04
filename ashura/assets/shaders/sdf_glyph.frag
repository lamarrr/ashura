#version 450

layout(location = 0) in vec2 in_uv;

layout(push_constant) uniform GlyphProps {
    mat3 transform;
    float thickness; // thickness will range from 0 to 1
    float outline_thickness;
    vec4 color;
    vec4 outline_color;
} props;

layout(set = 0, binding = 0) uniform sampler2D sdf_texture; /// VK_FORMAT_R8_UNORM

layout(location = 0) out vec4 out_color;

void main() {
    float signed_distance = texture(sdf_texture, in_uv).r;
    float alpha = smoothstep(1 - props.thickness, 1 - props.thickness, signed_distance);
    float outline = smoothstep(props.outline_thickness, props.outline_thickness, signed_distance);
    out_color = vec4(mix(props.outline_color.rgb, props.color.rgb, outline), alpha);
}