/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/gpu/gpu.h"
#include "ashura/std/error.h"
#include "ashura/std/hash_map.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

constexpr u32 TEXTURE_WHITE        = 0;
constexpr u32 TEXTURE_BLACK        = 1;
constexpr u32 TEXTURE_TRANSPARENT  = 2;
constexpr u32 TEXTURE_RED          = 3;
constexpr u32 TEXTURE_GREEN        = 4;
constexpr u32 TEXTURE_BLUE         = 5;
constexpr u32 NUM_DEFAULT_TEXTURES = TEXTURE_BLUE + 1;

constexpr u32 SAMPLER_LINEAR          = 0;
constexpr u32 SAMPLER_NEAREST         = 1;
constexpr u32 SAMPLER_LINEAR_CLAMPED  = 2;
constexpr u32 SAMPLER_NEAREST_CLAMPED = 3;
constexpr u32 NUM_DEFAULT_SAMPLERS    = SAMPLER_NEAREST_CLAMPED + 1;

struct FramebufferAttachment
{
  gpu::ImageDesc     desc      = {};
  gpu::ImageViewDesc view_desc = {};
  gpu::Image         image     = nullptr;
  gpu::ImageView     view      = nullptr;
};

/// created with sampled, storage, color attachment, and transfer flags
struct Framebuffer
{
  FramebufferAttachment color         = {};
  FramebufferAttachment depth_stencil = {};
  gpu::DescriptorSet    color_texture = nullptr;
  gpu::Extent           extent        = {};
};

struct SamplerHasher
{
  constexpr Hash operator()(gpu::SamplerDesc const &desc) const
  {
    return hash_combine_n(
        (Hash) desc.mag_filter, (Hash) desc.min_filter,
        (Hash) desc.mip_map_mode, (Hash) desc.address_mode_u,
        (Hash) desc.address_mode_v, (Hash) desc.address_mode_w,
        (Hash) desc.mip_lod_bias, (Hash) desc.anisotropy_enable,
        (Hash) desc.max_anisotropy, (Hash) desc.compare_enable,
        (Hash) desc.compare_op, (Hash) desc.min_lod, (Hash) desc.max_lod,
        (Hash) desc.border_color, (Hash) desc.unnormalized_coordinates);
  }
};

struct SamplerEq
{
  constexpr Hash operator()(gpu::SamplerDesc const &a,
                            gpu::SamplerDesc const &b) const
  {
    return a.mag_filter == b.mag_filter && a.mip_map_mode == b.mip_map_mode &&
           a.address_mode_u == b.address_mode_u &&
           a.address_mode_v == b.address_mode_v &&
           a.address_mode_w == b.address_mode_w &&
           a.mip_lod_bias == b.mip_lod_bias &&
           a.anisotropy_enable == b.anisotropy_enable &&
           a.max_anisotropy == b.max_anisotropy &&
           a.compare_enable == b.compare_enable &&
           a.compare_op == b.compare_op && a.min_lod == b.min_lod &&
           a.max_lod == b.max_lod && a.border_color == b.border_color &&
           a.unnormalized_coordinates == b.unnormalized_coordinates;
  }
};

struct CachedSampler
{
  gpu::Sampler sampler = nullptr;
  u32          slot    = 0;
};

typedef HashMap<gpu::SamplerDesc, CachedSampler, SamplerHasher, SamplerEq, u32>
    SamplerCache;

/// @param color_format hdr if hdr supported and required.
///
/// scratch images resized when swapchain extents changes
///
struct RenderContext
{
  static constexpr gpu::FormatFeatures COLOR_FEATURES =
      gpu::FormatFeatures::ColorAttachment |
      gpu::FormatFeatures::ColorAttachmentBlend |
      gpu::FormatFeatures::StorageImage | gpu::FormatFeatures::SampledImage;
  static constexpr gpu::FormatFeatures DEPTH_STENCIL_FEATURES =
      gpu::FormatFeatures::DepthStencilAttachment |
      gpu::FormatFeatures::SampledImage;
  static constexpr gpu::BufferUsage SSBO_USAGE =
      gpu::BufferUsage::UniformBuffer | gpu::BufferUsage::StorageBuffer |
      gpu::BufferUsage::UniformTexelBuffer |
      gpu::BufferUsage::StorageTexelBuffer | gpu::BufferUsage::IndirectBuffer |
      gpu::BufferUsage::TransferSrc | gpu::BufferUsage::TransferDst;

  static constexpr gpu::Format HDR_COLOR_FORMATS[] = {
      gpu::Format::R16G16B16A16_SFLOAT};

  static constexpr gpu::Format SDR_COLOR_FORMATS[] = {
      gpu::Format::B8G8R8A8_UNORM, gpu::Format::R8G8B8A8_UNORM};

  static constexpr gpu::Format DEPTH_STENCIL_FORMATS[] = {
      gpu::Format::D16_UNORM_S8_UINT, gpu::Format::D24_UNORM_S8_UINT,
      gpu::Format::D32_SFLOAT_S8_UINT};

  static constexpr u16 NUM_TEXTURE_SLOTS        = 1024;
  static constexpr u16 NUM_SAMPLER_SLOTS        = 64;
  static constexpr u16 NUM_SCRATCH_FRAMEBUFFERS = 2;

  Bits<u64, NUM_TEXTURE_SLOTS> texture_slots        = {};
  Bits<u64, NUM_SAMPLER_SLOTS> sampler_slots        = {};
  gpu::DeviceImpl              device               = {};
  gpu::PipelineCache           pipeline_cache       = nullptr;
  u32                          buffering            = 0;
  StrHashMap<gpu::Shader>      shader_map           = {};
  gpu::Format                  color_format         = gpu::Format::Undefined;
  gpu::Format                  depth_stencil_format = gpu::Format::Undefined;
  gpu::DescriptorSetLayout     ubo_layout           = nullptr;
  gpu::DescriptorSetLayout     ssbo_layout          = nullptr;
  gpu::DescriptorSetLayout     textures_layout      = nullptr;
  gpu::DescriptorSetLayout     samplers_layout      = nullptr;
  gpu::DescriptorSet           texture_views        = nullptr;
  gpu::DescriptorSet           samplers             = nullptr;
  Vec<gpu::Object>             released_objects[gpu::MAX_FRAME_BUFFERING] = {};
  SamplerCache                 sampler_cache                              = {};
  Framebuffer                  screen_fb                                  = {};
  Framebuffer                  scratch_fbs[NUM_SCRATCH_FRAMEBUFFERS]      = {};
  gpu::Image                   default_image = nullptr;
  gpu::ImageView               default_image_views[NUM_DEFAULT_TEXTURES] = {};

  void init(gpu::DeviceImpl device, bool use_hdr, u32 buffering,
            gpu::Extent initial_extent, StrHashMap<gpu::Shader> shader_map);
  void uninit();

  void recreate_framebuffers(gpu::Extent new_extent);

  gpu::CommandEncoderImpl encoder();
  u32                     ring_index();
  gpu::FrameId            frame_id();
  gpu::FrameId            tail_frame_id();

  Option<gpu::Shader> get_shader(Span<char const> name);
  CachedSampler       create_sampler(gpu::SamplerDesc const &desc);

  u32  alloc_texture_slot();
  void release_texture_slot(u32 slot);
  u32  alloc_sampler_slot();
  void release_sampler_slot(u32 slot);

  void release(gpu::Image image);
  void release(gpu::ImageView view);
  void release(gpu::Buffer view);
  void release(gpu::BufferView view);
  void release(gpu::DescriptorSetLayout layout);
  void release(gpu::DescriptorSet set);
  void release(gpu::Sampler sampler);
  void release(FramebufferAttachment fb)
  {
    release(fb.image);
    release(fb.view);
  }
  void release(Framebuffer fb)
  {
    release(fb.color);
    release(fb.depth_stencil);
    release(fb.color_texture);
  }

  void idle_reclaim();

  void begin_frame(gpu::Swapchain swapchain);
  void end_frame(gpu::Swapchain swapchain);
};

}        // namespace ash
