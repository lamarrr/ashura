#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef struct Orthographic Orthographic;
typedef struct Perspective  Perspective;
typedef struct Camera       Camera;

/// @x_mag: The horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @y_mag: The vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @z_far: The distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
/// @z_near: The distance to the near clipping plane.
struct Orthographic
{
  f32 x_mag  = 0;
  f32 y_mag  = 0;
  f32 z_far  = 0;
  f32 z_near = 0;
};

/// @aspect_ratio: The aspect ratio of the field of view.
/// @y_fov: The vertical field of view in radians. This value
/// SHOULD be less than π.
/// @z_far: The distance to the far clipping plane.
/// @z_near: The distance to the near clipping plane.
struct Perspective
{
  f32 aspect_ratio = 0;
  f32 y_fov        = 0;
  f32 z_far        = 0;
  f32 z_near       = 0;
};

struct Camera
{
  Mat4Affine model      = {};
  Mat4Affine view       = {};
  Mat4       projection = {};
};

}        // namespace ash