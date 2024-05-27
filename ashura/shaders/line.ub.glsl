
// todo(lamarrr):
// add color interp (with thickness accounted for), macros, and config

struct Params
{
  float thickness;
  float edge_smoothness;
  float cap_roundness;
  vec4  color[2];
};

//
// get bary coords using hw interp, in object's space only:
// https://stackoverflow.com/questions/25397586/accessing-barycentric-coordinates-inside-fragment-shader
//
// use loop-blinn for curve.
// using curve get sdf.
//
//
//
// Problem: when cps are same, the line becomes invincible
//
//
//
//