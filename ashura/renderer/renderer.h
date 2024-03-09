#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/renderer/scene.h"
#include "ashura/renderer/view.h"
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
/// @color_format: hdr if hdr supported and required
/// scratch images resized when swapchain extents changes
struct Renderer
{
  gfx::DeviceImpl    device                   = {};
  gfx::PipelineCache pipeline_cache           = nullptr;
  gfx::FrameContext  frame_context            = nullptr;
  gfx::Swapchain     swapchain                = nullptr;
  gfx::FrameInfo     frame_info               = {};
  gfx::Format        color_format             = gfx::Format::B8G8R8A8_UNORM;
  gfx::Format        depth_stencil_format     = gfx::Format::D32_SFLOAT_S8_UINT;
  gfx::Format        depth_format             = gfx::Format::D32_SFLOAT;
  gfx::Format        stencil_format           = gfx::Format::S8_UINT;
  gfx::Image         scratch_color_image      = nullptr;
  gfx::ImageView     scratch_color_image_view = nullptr;
  gfx::Image         scratch_depth_stencil_image                   = nullptr;
  gfx::ImageView     scratch_depth_stencil_image_view              = nullptr;
  Vec<Tuple<gfx::FrameId, gfx::Framebuffer>> released_framebuffers = {};

  Option<gfx::Shader> get_shader(Span<char const> name);
  void                release(gfx::Framebuffer framebuffer);
  // template <typename T, typename... Args>
  // T *allocate_param(Args &&...args);
  // void release_attachment(ImageAttachment const &attachment);

  // TODO(lamarrr): can we release attachments after the pass ends?
  template <typename Binding, typename Reg, typename Exe>
  void add_pass(Span<char const> name, Reg &&registration, Exe &&execution);

  void begin_frame();
  void end_frame();

  void tick();
  // sky render pass
  // render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
  // perform bloom, blur, msaa on 3d scene
  // render UI pass + custom shaders, blur ???
  // copy and composite 3d and 2d scenes
};

}        // namespace ash
