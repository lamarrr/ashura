#ifndef _CORE_GLSL_
#define _CORE_GLSL_

const float PI      = 3.1415926535897932384626433832795;
const float GAMMA   = 2.2;
const float EPSILON = 0.00000011920929;

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
  vec4 model[3];
  vec4 view[3];
  vec4 projection[4];
};

mat4 affine4(vec4 v[3])
{
  return mat4(v[0], v[1], v[2], vec4(0, 0, 0, 1));
}

mat4 to_mat4(vec4 v[4])
{
  return mat4(v[0], v[1], v[2], v[3]);
}

mat4 to_mvp(ViewTransform t)
{
  return to_mat4(t.projection) * affine4(t.view) * affine4(t.model);
}

#endif