#pragma once
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/renderer.h"

namespace ash
{

struct BloomDesc
{
  BlurDesc blur                 = {};
  f32      strength             = 1;
  f32      radius               = 1;
  Vec3     default_color        = {};
  f32      default_opacity      = 0.7f;
  f32      luminosity_threshold = 0.75f;
  f32      smooth_width         = 0.01f;
};

struct BloomObject
{
  BloomDesc desc            = {};
  uid32     scene_object_id = UID32_INVALID;
};

struct BloomPass
{
  static void init(Pass self, RenderServer *server, uid32 id);
  static void deinit(Pass self, RenderServer *server);
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
