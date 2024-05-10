#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/renderer/scene.h"
#include "ashura/renderer/view.h"
#include "ashura/std/error.h"
#include "ashura/std/hash_map.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

/// created with sampled, storage, color attachment, and transfer flags
struct FramebufferAttachments
{
  gfx::ImageDesc     color_image_desc              = {};
  gfx::ImageDesc     depth_stencil_image_desc      = {};
  gfx::ImageViewDesc color_image_view_desc         = {};
  gfx::ImageViewDesc depth_stencil_image_view_desc = {};
  gfx::Image         color_image                   = nullptr;
  gfx::Image         depth_stencil_image           = nullptr;
  gfx::ImageView     color_image_view              = nullptr;
  gfx::ImageView     depth_stencil_image_view      = nullptr;
  gfx::Extent        extent                        = {};
};

using ShaderMap = StrHashMap<gfx::Shader>;

/// @color_format: hdr if hdr supported and required
/// scratch images resized when swapchain extents changes
struct RenderContext
{
  static constexpr gfx::FormatFeatures COLOR_FEATURES =
      gfx::FormatFeatures::ColorAttachment |
      gfx::FormatFeatures::ColorAttachmentBlend |
      gfx::FormatFeatures::StorageImage | gfx::FormatFeatures::SampledImage;
  static constexpr gfx::FormatFeatures DEPTH_STENCIL_FEATURES =
      gfx::FormatFeatures::DepthStencilAttachment |
      gfx::FormatFeatures::SampledImage;
  static constexpr gfx::BufferUsage SSBO_USAGE =
      gfx::BufferUsage::UniformBuffer | gfx::BufferUsage::StorageBuffer |
      gfx::BufferUsage::UniformTexelBuffer |
      gfx::BufferUsage::StorageTexelBuffer | gfx::BufferUsage::IndirectBuffer |
      gfx::BufferUsage::TransferSrc | gfx::BufferUsage::TransferDst;

  gfx::DeviceImpl          device                  = {};
  gfx::PipelineCache       pipeline_cache          = nullptr;
  u32                      buffering               = 0;
  ShaderMap                shader_map              = {};
  gfx::Format              color_format            = gfx::Format::Undefined;
  gfx::Format              depth_stencil_format    = gfx::Format::Undefined;
  FramebufferAttachments   framebuffer_attachments = {};
  FramebufferAttachments   scratch_framebuffer_attachments         = {};
  gfx::RenderPass          render_pass                             = nullptr;
  gfx::RenderPass          color_render_pass                       = nullptr;
  gfx::Framebuffer         framebuffer                             = nullptr;
  gfx::Framebuffer         scratch_framebuffer                     = nullptr;
  gfx::DescriptorSetLayout ssbo_layout                             = nullptr;
  gfx::DescriptorSetLayout textures_layout                         = nullptr;
  Vec<Tuple<gfx::FrameId, gfx::Framebuffer>> released_framebuffers = {};
  Vec<Tuple<gfx::FrameId, gfx::Image>>       released_images       = {};
  Vec<Tuple<gfx::FrameId, gfx::ImageView>>   released_image_views  = {};

  void init(gfx::DeviceImpl p_device, bool p_use_hdr,
            u32 p_max_frames_in_flight, gfx::Extent p_initial_extent,
            ShaderMap p_shader_map);
  void uninit();

  void recreate_framebuffers(gfx::Extent new_extent);

  gfx::CommandEncoderImpl encoder();
  u32                     ring_index();
  gfx::FrameId            frame_id();
  gfx::FrameId            tail_frame_id();

  Option<gfx::Shader> get_shader(Span<char const> name);

  void release(gfx::Framebuffer framebuffer);
  void release(gfx::Image image);
  void release(gfx::ImageView view);
  void purge();
  void idle_purge();

  void begin_frame(gfx::Swapchain swapchain);
  void end_frame(gfx::Swapchain swapchain);
};

}        // namespace ash
