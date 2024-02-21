#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

// do we need to copy directly from the image?
typedef struct BlurDesc   BlurDesc;
typedef struct BlurObject BlurObject;
typedef struct BlurPass   BlurPass;

struct BlurDesc
{
  u32         blur_radius = 4;
  gfx::Offset src_offset;
  gfx::Extent src_extent;
  gfx::Image  src_image       = nullptr;
  u32         src_mip_level   = 0;
  u32         src_array_layer = 0;
  gfx::Offset dst_offset;
  gfx::Extent dst_extent;
  gfx::Image  dst_image       = nullptr;
  u32         dst_mip_level   = 0;
  u32         dst_array_layer = 0;
};

struct BlurObject
{
  BlurDesc desc            = {};
  uid32    scene_object_id = UID32_INVALID;
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
  gfx::ComputePipeline     pipeline              = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  static void              init(Pass self, RenderServer *server, uid32 id);
  static void              deinit(Pass self, RenderServer *server);
  static void acquire_scene(Pass self, RenderServer *server, uid32 scene);
  static void release_scene(Pass self, RenderServer *server, uid32 scene);
  static void acquire_view(Pass self, RenderServer *server, uid32 view);
  static void release_view(Pass self, RenderServer *server, uid32 view);
  static void release_object(Pass self, RenderServer *server, uid32 scene,
                             uid32 object);
  static void begin(Pass self, RenderServer *server, PassBeginInfo const *info);
  static void encode(Pass self, RenderServer *server,
                     PassEncodeInfo const *info);
  static void end(Pass self, RenderServer *server, PassEndInfo const *info);

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
