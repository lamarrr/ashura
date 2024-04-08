#ifndef CORE_GLSL
#define CORE_GLSL
#extension GL_GOOGLE_include_directive : require
const float PI    = 3.1415926535897932384626433832795;
const float GAMMA = 2.2;

vec3 pow_vec3(vec3 v, float p)
{
  return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 to_linear(vec3 v)
{
  return pow_vec3(v, GAMMA);
}

vec3 to_srgb(vec3 v)
{
  return pow_vec3(v, 1.0 / GAMMA);
}

struct ViewTransform
{
  mat4 projection;
  vec4 view_0;
  vec4 view_1;
  vec4 view_2;
  vec4 model_0;
  vec4 model_1;
  vec4 model_2;
};

mat4 to_mvp(ViewTransform t)
{
  return t.projection * mat4(t.view_0, t.view_1, t.view_2, vec4(0, 0, 0, 1)) *
         mat4(t.model_0, t.model_1, t.model_2, vec4(0, 0, 0, 1));
}

struct Edge
{
  vec4 tl;
  vec4 tr;
  vec4 bl;
  vec4 br;
};

#endif