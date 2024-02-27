#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/renderer/camera.h"
#include "ashura/renderer/scene.h"
#include "ashura/std/box.h"
#include "ashura/std/types.h"

namespace ash
{

typedef struct MSAAConfig          MSAAConfig;
typedef struct FXAAConfig          FXAAConfig;
typedef struct AAConfig            AAConfig;
typedef struct GaussianBloomConfig GaussianBloomConfig;
typedef struct BloomConfig         BloomConfig;
typedef struct ViewConfig          ViewConfig;

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
  Camera      camera               = {};
  gfx::Extent extent               = {};
  gfx::Format color_format         = gfx::Format::Undefined;
  gfx::Format depth_stencil_format = gfx::Format::Undefined;
  AAConfig    aa                   = {};
  BloomConfig bloom                = {};
  DOFConfig   dof                  = {};
  f32         chromatic_aberration = 0;
};

void frustum_cull(Camera const &camera, Span<Mat4Affine const> local_transforms,
                  Span<Mat4Affine const> global_transforms,
                  Span<Box const> aabb, BitSpan<u64> is_visible);

}        // namespace ash
