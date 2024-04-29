#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

/// @x_mag: The horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @y_mag: The vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @z_far: The distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
/// @z_near: The distance to the near clipping plane.
struct OrthographicCamera
{
  f32 x_mag  = 0;
  f32 y_mag  = 0;
  f32 z_far  = 0;
  f32 z_near = 0;

  constexpr Mat4 to_projection_mat() const
  {
    f32 const z_diff = z_near - z_far;
    return Mat4{{{1 / x_mag, 0, 0, 0},
                 {0, 1 / y_mag, 0, 0},
                 {0, 0, 2 / z_diff, (z_far + z_near) / z_diff},
                 {0, 0, 0, 1}}};
  }
};

/// @aspect_ratio: The aspect ratio of the field of view.
/// @y_fov: The vertical field of view in radians. This value
/// SHOULD be less than π.
/// @z_far: The distance to the far clipping plane.
/// @z_near: The distance to the near clipping plane.
struct PerspectiveCamera
{
  f32 aspect_ratio = 0;
  f32 y_fov        = 0;
  f32 z_far        = 0;
  f32 z_near       = 0;

  Mat4 to_projection_mat() const
  {
    f32 const s      = tanf(y_fov * 0.5F);
    f32 const z_diff = z_near - z_far;
    return Mat4{
        {{1 / (aspect_ratio * s), 0, 0, 0},
         {0, 1 / s, 0, 0},
         {0, 0, (z_far + z_near) / z_diff, (2 * z_far * z_near) / z_diff},
         {0, 0, -1, 0}}};
  }
};

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

struct Camera
{
  union
  {
    PerspectiveCamera  perspective;
    OrthographicCamera orthographic;
  };
  CameraType type = CameraType::Orthographic;

  Mat4 to_projection_matrix() const
  {
    if (type == CameraType::Orthographic)
      return orthographic.to_projection_mat();
    return perspective.to_projection_mat();
  }
};

// rotate about pivot point on one or more axis
// camera controller class
constexpr Mat4Affine look_at(Vec3 position);
constexpr Mat4Affine move_camera_to(Vec3 position);

}        // namespace ash
