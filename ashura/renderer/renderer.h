#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/renderer/scene.h"
#include "ashura/renderer/shader.h"
#include "ashura/renderer/view.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

/// @color_images, @depth_stencil_image format must be same as render context's
struct RenderTarget
{
  Span<gfx::ImageView const> color_images;
  gfx::ImageView             depth_stencil_image   = nullptr;
  gfx::ImageAspects          depth_stencil_aspects = gfx::ImageAspects::None;
  Vec2U                      extent                = {};
  Vec2U                      scissor_offset        = {};
  Vec2U                      scissor_extent        = {};
};

/// with views for each image mip level
struct Scratch
{
  gfx::FrameId            frame_stamp                    = 0;
  gfx::ImageDesc          color_image_desc               = {};
  gfx::ImageDesc          depth_stencil_image_desc       = {};
  Vec<gfx::ImageViewDesc> color_image_view_descs         = {};
  Vec<gfx::ImageViewDesc> depth_stencil_image_view_descs = {};
  gfx::Image              color_image                    = nullptr;
  gfx::Image              depth_stencil_image            = nullptr;
  Vec<gfx::ImageView>     color_image_views              = {};
  Vec<gfx::ImageView>     depth_stencil_image_views      = {};
};

/// @color_format: hdr if hdr supported and required
/// scratch images resized when swapchain extents changes
struct Renderer
{
  gfx::DeviceImpl          device               = {};
  gfx::PipelineCache       pipeline_cache       = nullptr;
  gfx::FrameContext        frame_context        = nullptr;
  gfx::Swapchain           swapchain            = nullptr;
  gfx::FrameInfo           frame_info           = {};
  gfx::SwapchainInfo       swapchain_info       = {};
  gfx::Format              color_format         = gfx::Format::Undefined;
  gfx::Format              depth_stencil_format = gfx::Format::Undefined;
  Scratch                  scatch               = {};
  Vec<UniformHeap>         frame_uniform_heaps  = {};
  gfx::DescriptorSetLayout uniform_layout       = nullptr;
  gfx::CommandEncoderImpl  encoder              = {};
  Vec<Tuple<gfx::FrameId, gfx::Framebuffer>> released_framebuffers = {};
  Vec<Tuple<gfx::FrameId, gfx::Image>>       released_images       = {};
  Vec<Tuple<gfx::FrameId, gfx::ImageView>>   released_image_views  = {};

  Option<gfx::Shader> get_shader(Span<char const> name);

  void release(gfx::Framebuffer framebuffer)
  {
    ENSURE(released_framebuffers.push(frame_info.current, framebuffer));
  }

  void release(gfx::Image image)
  {
    ENSURE(released_images.push(frame_info.current, image));
  }

  void release(gfx::ImageView view)
  {
    ENSURE(released_image_views.push(frame_info.current, view));
  }

  // sky render pass
  // render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
  // perform bloom, blur, msaa on 3d scene
  // render UI pass + custom shaders, blur ???
  // copy and composite 3d and 2d scenes
  void begin_frame();
  void record_frame();
  void end_frame();
};

}        // namespace ash
