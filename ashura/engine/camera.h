#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// @x_mag: The horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @y_mag: The vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @z_near: The distance to the near clipping plane.
/// @z_far: The distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
constexpr Mat4Affine orthographic(f32 x_mag, f32 y_mag, f32 z_near, f32 z_far)
{
  f32 const z_diff = z_near - z_far;
  return Mat4Affine{{{1 / x_mag, 0, 0, 0},
                     {0, 1 / y_mag, 0, 0},
                     {0, 0, 2 / z_diff, (z_far + z_near) / z_diff}}};
}

/// @aspect_ratio: The aspect ratio of the field of view.
/// @y_fov: The vertical field of view in radians. This value
/// SHOULD be less than π.
/// @z_far: The distance to the far clipping plane.
/// @z_near: The distance to the near clipping plane.
inline Mat4 perspective(f32 aspect_ratio, f32 y_fov, f32 z_far, f32 z_near)
{
  f32 const s      = tanf(y_fov * 0.5F);
  f32 const z_diff = z_near - z_far;
  return Mat4{{{1 / (aspect_ratio * s), 0, 0, 0},
               {0, 1 / s, 0, 0},
               {0, 0, (z_far + z_near) / z_diff, (2 * z_far * z_near) / z_diff},
               {0, 0, -1, 0}}};
}

struct ViewTransform
{
  Mat4Affine model      = {};
  Mat4Affine view       = {};
  Mat4       projection = {};

  constexpr Mat4 mul() const
  {
    return projection * view * model;
  }
};

enum class CameraType : u8
{
  Orthographic = 0,
  Perspective  = 1
};

constexpr Mat4 look_at(Vec3 eye, Vec3 center, Vec3 up)
{
  Vec3 const f = normalize(center - eye);
  Vec3 const s = normalize(cross(up, f));
  Vec3 const u = cross(f, s);

  return {{{s.x, s.x, s.x, 0},
           {u.y, u.y, u.y, 0},
           {f.z, f.z, f.z, 0},
           {-dot(s, eye), -dot(u, eye), -dot(f, eye), 1}}};
}

}        // namespace ash
