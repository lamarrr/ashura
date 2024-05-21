
// todo(lamarrr):
// add color interp (with thickness accounted for), macros, and config

// has a weird behaviour when the cp are same
/// see: https://www.shadertoy.com/view/MtS3Dy
///
//
// https://www.shadertoy.com/view/lts3Df
//https://www.shadertoy.com/view/4tj3Dy
//
// 
// get bary coords using hw interp, in object's space only:
// https://stackoverflow.com/questions/25397586/accessing-barycentric-coordinates-inside-fragment-shader
//
// use loop-blinn for curve.
// using curve get sdf.
//
vec2 bezier_distance_vector(vec2 b0, vec2 b1, vec2 b2)
{
  float a = det(b0, b2), b = 2.0 * det(b1, b0), d = 2.0 * det(b2, b1);
  float f   = b * d - a * a;
  vec2  d21 = b2 - b1, d10 = b1 - b0, d20 = b2 - b0;
  vec2  gf  = 2.0 * (b * d21 + d * d10 + a * d20);
  gf        = vec2(gf.y, -gf.x);
  vec2  pp  = -f * gf / dot(gf, gf);
  vec2  d0p = b0 - pp;
  float ap = det(d0p, d20), bp = 2.0 * det(d10, d0p);
  // (note that 2*ap+bp+dp=2*a+b+d=4*area(b0,b1,b2))
  float t = clamp((ap + bp) / (2.0 * a + b + d), 0.0, 1.0);
  return mix(mix(b0, b1, t), mix(b1, b2, t), t);
}

float bezier_sdf(vec2 t, vec2 b0, vec2 b1, vec2 b2)
{
  return length(bezier_distance_vector(b0 - t, b1 - t, b2 - t));
}