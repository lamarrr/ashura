#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

// do we need to copy directly from the image?
struct BlurNode
{
  gfx::Offset    offset;
  gfx::Extent    extent;
  gfx::ImageView image = nullptr;
  uid32          view  = UID32_INVALID;
};

struct BlurDesc
{
  gfx::Extent kernel_extent;
  BlurNode    input;
  BlurNode    output;
};

struct BlurObject
{
  BlurDesc desc = {};
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
