#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

struct params_t
{
  struct bloom_params_t
  {
    f32 strength{1};
    f32 radius{1};
  } bloom;
  struct luma_threshold_params_t
  {
    f32 defaultColor[3]{0.f, 0.f, 0.f};        // vec3(0)
    f32 defaultOpacity{0.7f};                  // 0
    f32 luminosityThreshold{0.75f};            // 1.f
    f32 smoothWidth{0.01f};                    // 1.0
  } luma_threshold;
};

struct BlurInput
{
  union
  {
    struct
    {
      gfx::Image image;
      u32        mip_level;
      u32        layer;
    } image;
    uid32 view;
  } input, output;
  gfx::Extent src_extent;
  gfx::Extent dst_extent;
  u32         kernel_width;
  u32         kernel_height;
};

struct BlurObject
{
};

// object-clip space blur
//
// - capture scene at object's screen-space area, dilate by the blur extent
// - reserve scratch stencil image with at least size of the dilated area
// - blur captured area
// - render object to offscreen scratch image stencil only
// - using rendered stencil, directly-write (without blending) onto scene again
struct BlurPass
{
  static void init(Pass self, RenderServer *server, uid32 id);
  static void deinit(Pass self, RenderServer *server);
  static void acquire_scene(Pass self, RenderServer *server, uid32 scene);
  static void release_scene(Pass self, RenderServer *server, uid32 scene);
  static void acquire_view(Pass self, RenderServer *server, uid32 view);
  static void release_view(Pass self, RenderServer *server, uid32 view);
  static void release_object(Pass self, RenderServer *server, uid32 scene,
                             uid32 object);
  static void begin(Pass self, RenderServer *server, uid32 view,
                    gfx::CommandEncoderImpl const *encoder);
  static void encode(Pass self, RenderServer *server, uid32 view,
                     PassEncodeInfo const *info);
  static void end(Pass self, RenderServer *server, uid32 view,
                  gfx::CommandEncoderImpl const *encoder);

  static constexpr PassInterface const interface{.init          = init,
                                                 .deinit        = deinit,
                                                 .acquire_scene = acquire_scene,
                                                 .release_scene = release_scene,
                                                 .acquire_view  = acquire_view,
                                                 .release_view  = release_view,
                                                 .release_object =
                                                     release_object,
                                                 .begin  = begin,
                                                 .encode = encode,
                                                 .end    = end};
};

}        // namespace ash
