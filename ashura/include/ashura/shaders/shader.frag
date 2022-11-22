#version 460

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform Skinning {
    vec4 color;
    vec2 alpha_blend;
    float opacity;
} skin;

layout(set = 1, binding = 0) uniform sampler2D skin_texture;

layout(set = 1, binding = 1) uniform sampler2D clip_mask;

void main() {
    vec4 skin_color = skin.opacity * (skin.alpha_blend.x * skin.color + skin.alpha_blend.y * texture(skin_texture, in_position.xy));
    float mask = texture(clip_mask, in_position.xy).r;
    out_color = mask * skin_color;
}