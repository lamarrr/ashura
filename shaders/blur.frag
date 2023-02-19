#version 450

layout(location = 0) in vec2 in_st;

// (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216)
layout(set = 0, binding = 0) uniform Uniforms {
  bool x_blur;
  float weight[5];  // TODO(lamarrr): make the length generic and use a gaussian
                    // dist function to fill in the weights
}
uniforms;

layout(set = 0, binding = 1) uniform sampler2D image;

layout(location = 0) out vec4 out_color;

void main() {
  vec2 texture_offset = 1 / textureSize(image, 0);
  vec3 result = texture(image, in_st).rgb *
                uniforms.weight[0];  // current fragment's contribution

  if (uniforms.x_blur) {
    for (int i = 1; i < 5; ++i) {
      result += texture(image, in_st + vec2(texture_offset.x * i, 0)).rgb *
                uniforms.weight[i];
      result += texture(image, in_st - vec2(texture_offset.x * i, 0)).rgb *
                uniforms.weight[i];
    }
  } else {
    for (int i = 1; i < 5; ++i) {
      result += texture(image, in_st + vec2(0, texture_offset.y * i)).rgb *
                uniforms.weight[i];
      result += texture(image, in_st - vec2(0, texture_offset.y * i)).rgb *
                uniforms.weight[i];
    }
  }

  out_color = vec4(result, 1);
}
