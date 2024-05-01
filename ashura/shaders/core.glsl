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
  vec4 view[3];
  vec4 model[3];
  vec4 projection[4];
};

mat4 to_mvp(ViewTransform t)
{
  return mat4(t.projection[0], t.projection[1], t.projection[2],
              t.projection[3]) *
         mat4(t.view[0], t.view[1], t.view[2], vec4(0, 0, 0, 1)) *
         mat4(t.model[0], t.model[1], t.model[2], vec4(0, 0, 0, 1));
}

#endif