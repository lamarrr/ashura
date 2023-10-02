#version 450

// layout(location = 0) in vec2 in_position;
// layout(location = 1) in vec2 in_uv;
// layout(location = 2) in vec4 in_color;

layout(set = 0, binding = 0) uniform Buf {
    mat3 transform;
    vec2 uv;
    vec3 normal;
} c;

layout(set = 0, binding = 1) uniform Bufx {
    mat3 transform;
    vec2 uv;
    vec3 normal;
} d;

layout(set = 0, binding = 3, rgba8ui) uniform readonly uimage2D ro_image2D;
layout(set = 1, binding = 4, rgba8ui) uniform readonly uimageBuffer ro_imageBuffer;
layout(set = 2, binding = 5, rgba8ui) uniform writeonly uimage2D wo_image2D;
layout(set = 3, binding = 6, rgba8ui) uniform writeonly uimageBuffer wo_imageBuffer;
layout(set = 4, binding = 7) uniform sampler2D rSampler2D;
layout(set = 5, binding = 8) uniform samplerBuffer rSamplerBuffer;
layout(set = 6, binding = 9) uniform texture2D rtexture2D;
layout(set = 7, binding = 9) uniform textureBuffer rtextureBuffer;
layout(set = 7, binding = 9) uniform sampler2D rtextureBuffer2;
layout(set = 7, binding = 9, rgba8ui) uniform writeonly uimage2D rtextureBuffer3;
layout(set = 7, binding = 9, rgba8ui) uniform readonly uimageBuffer rtextureBuffer4;
layout(set = 8, binding = 10) uniform sampler2DArray rSampler2DArray;

// layout(location = 0) out vec2 out_uv;
// layout(location = 1) out vec4 out_color;

void main() {
gl_Position = vec4(0);
}
