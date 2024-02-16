#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

struct RootResources
{
  gfx::Image  color_image                = nullptr;
  gfx::Format color_image_format         = gfx::Format::Undefined;
  gfx::Image  depth_stencil_image        = nullptr;
  gfx::Format depth_stencil_image_format = gfx::Format::Undefined;
  gfx::Extent extent                     = {};
};

struct RootPass
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
