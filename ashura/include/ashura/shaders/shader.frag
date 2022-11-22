#version 460

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform Skinning {
    vec4 overlay_color;
} skin;

layout(set = 1, binding = 0) uniform sampler2D skin_texture;

layout(set = 1, binding = 1) uniform sampler2D clip_mask;

void main() {
    vec4 skin_texture_color = texture(skin_texture, in_position.xy).rgba;
    vec4 color = skin.overlay_color + skin_texture_color * (1 - skin.overlay_color.a);
    float mask = texture(clip_mask, in_position.xy).r;
    out_color = mask * color;
}