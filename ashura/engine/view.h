#pragma once
#include "ashura/engine/camera.h"
#include "ashura/engine/scene.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/types.h"

namespace ash
{

struct MSAAConfig
{
  gfx::SampleCount sample_count = gfx::SampleCount::None;
};

struct FXAAConfig
{
};

enum class AATechnique : u8
{
  None = 0,
  MSAA = 1,
  FXAA = 2
};

struct AAConfig
{
  union
  {
    char       none_ = 0;
    MSAAConfig msaa;
    FXAAConfig fxaa;
  };
  AATechnique technique = AATechnique::None;
};

enum class BloomTechnique : u8
{
  None     = 0,
  Gaussian = 1
};

struct GaussianBloomConfig
{
  u32  blur_radius          = 4;
  f32  strength             = 1;
  f32  radius               = 1;
  Vec3 default_color        = {};
  f32  default_opacity      = 0.7f;
  f32  luminosity_threshold = 0.75f;
  f32  smooth_width         = 0.01f;
};

struct BloomConfig
{
  union
  {
    char                none_ = 0;
    GaussianBloomConfig gaussian;
  };
  BloomTechnique technique = BloomTechnique::None;
};

// Depth of Field
struct DOFConfig
{
};

struct ViewConfig
{
  ViewTransform transform            = {};
  AAConfig      aa                   = {};
  BloomConfig   bloom                = {};
  DOFConfig     dof                  = {};
  f32           chromatic_aberration = 0;
};

/// https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron/blob/b92d559bd083f44df9f8f42a6ad149c1584ae94c/src/common/Misc/Misc.cpp#L265
/// https://bruop.github.io/frustum_culling/
///
/// exploits the fact that in clip-space all vertices in the view frustum will
/// obey:
///
/// -w <= x <= w
/// -w <= y <= w
///  0 <= z <= w
///
constexpr bool is_outside_frustum(Mat4 const &mvp, Box const &box)
{
  constexpr u8   NUM_CORNERS = 8;
  constexpr auto to_vec4     = [](Vec3 a) { return Vec4{a.x, a.y, a.z, 1}; };
  Vec4 const     corners[NUM_CORNERS] = {
      mvp * to_vec4(box.offset),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, 0, 0}),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, box.extent.y, 0}),
      mvp * to_vec4(box.offset + Vec3{0, box.extent.y, 0}),
      mvp * to_vec4(box.offset + Vec3{0, 0, box.extent.z}),
      mvp * to_vec4(box.offset + Vec3{box.extent.x, 0, box.extent.z}),
      mvp * to_vec4(box.offset + box.extent),
      mvp * to_vec4(box.offset + Vec3{0, box.extent.y, box.extent.z})};
  u8 left   = 0;
  u8 right  = 0;
  u8 top    = 0;
  u8 bottom = 0;
  u8 back   = 0;

  for (u8 i = 0; i < NUM_CORNERS; i++)
  {
    Vec4 const corner = corners[i];

    if (corner.x < -corner.w)
    {
      left++;
    }

    if (corner.x > corner.w)
    {
      right++;
    }

    if (corner.y < -corner.w)
    {
      bottom++;
    }

    if (corner.y > corner.w)
    {
      top++;
    }

    if (corner.z < 0)
    {
      back++;
    }
  }

  return left == NUM_CORNERS || right == NUM_CORNERS || top == NUM_CORNERS ||
         bottom == NUM_CORNERS || back == NUM_CORNERS;
}

constexpr void frustum_cull(Mat4 const            &mvp,
                            Span<Mat4Affine const> global_transform,
                            Span<Box const> aabb, BitSpan<u64> is_visible)
{
  for (u32 i = 0; i < (u32) aabb.size(); i++)
  {
    is_visible[i] = !is_outside_frustum(mvp * global_transform[i], aabb[i]);
  }
}

}        // namespace ash
