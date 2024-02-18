#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

// needed because we need to be able to render a view that is part of another
// view without adding the elements of the view to the root view
//
// view pass should also check if the view has already been rendered for the
// current frame.
//
// TODO(lamarrr): should be able to request render of another view
//
// get metadata for another pass belonging to a view
// i.e. get color attachment for view, get depth attachment for view, get
// framebuffer, renderpass. named tagged and used for a specific purpose,
// possibly referenced by all passes if they need to modify or add data atop of
// it but what if another view modifies it?
//
struct ViewObject
{
  uid32 scene        = UID32_INVALID;
  uid32 scene_object = UID32_INVALID;
  uid32 view         = UID32_INVALID;
  Vec2U extent       = {1, 1};
};

struct ViewPass
{
  // render to view's frame buffer and then composite onto the present view
  // there must be no recursion happening here
  Vec<ViewObject> view_objects = {};
  SparseVec<u32>  id_map       = {};

  Option<uid32>       add_view(ViewObject const &object);
  Option<ViewObject> *get_view(uid32 id);
  void                remove_view(uid32);

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
