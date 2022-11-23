#version 450

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform Overlay {
    vec4 color;
} overlay;

layout(set = 1, binding = 0) uniform sampler2D skin_texture;

layout(set = 1, binding = 1) uniform sampler2D clip_mask;

void main() {
    vec4 skin_color = texture(skin_texture, in_position.xy).rgba;
    vec4 color =  overlay.color + skin_color * (1 - overlay.color.a);
    float mask = texture(clip_mask, in_position.xy).r;
    out_color = mask * color;
}