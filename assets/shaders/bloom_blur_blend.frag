#version 450

layout(location = 0) in vec2 in_uv;

layout(set = 0, binding = 0) uniform Uniforms
{
  float exposure;
}
uniforms;

layout(set = 0, binding = 1) uniform sampler2D scene_texture;
layout(set = 0, binding = 2) uniform sampler2D bloom_blur_texture;

layout(location = 0) out vec4 out_color;

void main()
{
  const float gamma       = 2.2;
  vec3        hdr_color   = texture(scene_texture, in_uv).rgb;
  vec3        bloom_color = texture(bloom_blur_texture, in_uv).rgb;
  vec3        blended     = hdr_color + bloom_color;        // additive blending

  // tone mapping
  vec3 result = vec3(1) - exp(-hdr_color * uniforms.exposure);
  // perform gamma correction
  result = pow(result, vec3(1 / gamma));

  out_color = vec4(result, 1);
}
