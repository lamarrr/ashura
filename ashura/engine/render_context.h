#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/error.h"
#include "ashura/std/hash_map.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// created with sampled, storage, color attachment, and transfer flags
struct Framebuffer
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

/// @param color_format hdr if hdr supported and required.
///
/// scratch images resized when swapchain extents changes
///
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
  static constexpr u32 NUM_TEXTURE_SLOTS = 2048;

  static_assert(NUM_TEXTURE_SLOTS % 64 == 0);

  gfx::DeviceImpl          device                     = {};
  gfx::PipelineCache       pipeline_cache             = nullptr;
  u32                      buffering                  = 0;
  StrHashMap<gfx::Shader>  shader_map                 = {};
  gfx::Format              color_format               = gfx::Format::Undefined;
  gfx::Format              depth_stencil_format       = gfx::Format::Undefined;
  Framebuffer              framebuffer                = {};
  Framebuffer              scratch_framebuffer        = {};
  gfx::DescriptorSetLayout ssbo_layout                = nullptr;
  gfx::DescriptorSetLayout textures_layout            = nullptr;
  gfx::DescriptorSetLayout sampler_layout             = nullptr;
  gfx::DescriptorSet       texture_views              = nullptr;
  gfx::DescriptorSet       color_texture_view         = nullptr;
  gfx::DescriptorSet       scratch_color_texture_view = nullptr;
  Vec<gfx::Object>         released_objects[gfx::MAX_FRAME_BUFFERING] = {};
  alignas(64) u64 texture_slots[NUM_TEXTURE_SLOTS / 64]               = {};

  void init(gfx::DeviceImpl device, bool use_hdr, u32 buffering,
            gfx::Extent initial_extent, StrHashMap<gfx::Shader> shader_map);
  void uninit();

  void recreate_framebuffers(gfx::Extent new_extent);

  gfx::CommandEncoderImpl encoder();
  u32                     ring_index();
  gfx::FrameId            frame_id();
  gfx::FrameId            tail_frame_id();

  Option<gfx::Shader> get_shader(Span<char const> name);

  u16  alloc_texture_slot();
  void dealloc_texture_slot(u16 slot);

  void release(gfx::Image image);
  void release(gfx::ImageView view);
  void release(gfx::Buffer view);
  void release(gfx::BufferView view);
  void release(gfx::DescriptorSet set);
  void release(gfx::Sampler sampler);
  void idle_reclaim();

  void begin_frame(gfx::Swapchain swapchain);
  void end_frame(gfx::Swapchain swapchain);
};

}        // namespace ash
