#pragma once
#include "ashura/engine/error.h"
#include "ashura/engine/render_graph.h"
#include "ashura/engine/scene.h"
#include "ashura/engine/view.h"
#include "ashura/gfx/gfx.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

/// @remove_scene: remove all pass resources associated with a scene object.
/// @acquire_screen_color_image:
///
/// @add_object: once an object is added to the scene, if it is not at the end
/// of the tree, then the tree should be re-sorted based on depth, sort indices,
/// resize object cull masks for all views
/// @remove_object: remove object and all its children
struct Renderer
{
  gfx::DeviceImpl    device         = {};
  gfx::PipelineCache pipeline_cache = nullptr;
  gfx::FrameContext  frame_context  = nullptr;
  gfx::Swapchain     swapchain      = nullptr;
  RenderGraph       *rdg            = nullptr;

  void tick();
  // sky render pass
  // render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
  // perform bloom, blur, msaa on 3d scene
  // render UI pass + custom shaders, blur ???
  // copy and composite 3d and 2d scenes
};

}        // namespace ash
