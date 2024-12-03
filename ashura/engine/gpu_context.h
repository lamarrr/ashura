/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/gpu/gpu.h"
#include "ashura/std/error.h"
#include "ashura/std/map.h"
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
  gpu::ImageInfo     info      = {};
  gpu::ImageViewInfo view_info = {};
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

// [ ] use: enum to bits, and float to bits, not casted
struct SamplerHasher
{
  constexpr hash64 operator()(gpu::SamplerInfo const & info) const
  {
    return hash_combine_n(
        (hash64) info.mag_filter, (hash64) info.min_filter,
        (hash64) info.mip_map_mode, (hash64) info.address_mode_u,
        (hash64) info.address_mode_v, (hash64) info.address_mode_w,
        (hash64) info.mip_lod_bias, (hash64) info.anisotropy_enable,
        (hash64) info.max_anisotropy, (hash64) info.compare_enable,
        (hash64) info.compare_op, (hash64) info.min_lod, (hash64) info.max_lod,
        (hash64) info.border_color, (hash64) info.unnormalized_coordinates);
  }
};

struct SamplerEq
{
  constexpr hash64 operator()(gpu::SamplerInfo const & a,
                              gpu::SamplerInfo const & b) const
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

typedef Map<gpu::SamplerInfo, CachedSampler, SamplerHasher, SamplerEq, u32>
    SamplerCache;

/// @param color_format hdr if hdr supported and required.
///
/// scratch images resized when swapchain extents changes
///
struct GpuContext
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

  static constexpr u16 NUM_TEXTURE_SLOTS        = 1'024;
  static constexpr u16 NUM_SAMPLER_SLOTS        = 64;
  static constexpr u16 NUM_SCRATCH_FRAMEBUFFERS = 2;

  gpu::Device * device;

  gpu::PipelineCache pipeline_cache;

  u32 buffering;

  gpu::Format color_format;

  gpu::Format depth_stencil_format;

  gpu::DescriptorSetLayout ubo_layout;

  gpu::DescriptorSetLayout ssbo_layout;

  gpu::DescriptorSetLayout textures_layout;

  gpu::DescriptorSetLayout samplers_layout;

  gpu::DescriptorSet texture_views;

  gpu::DescriptorSet samplers;

  SamplerCache sampler_cache;

  Framebuffer screen_fb;

  Array<Framebuffer, NUM_SCRATCH_FRAMEBUFFERS> scratch_fbs;

  gpu::Image default_image;

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views;

  InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects;

  Bits<u64, NUM_TEXTURE_SLOTS> texture_slots;

  Bits<u64, NUM_SAMPLER_SLOTS> sampler_slots;

  static GpuContext create(AllocatorImpl allocator, gpu::Device * device,
                           bool use_hdr, u32 buffering,
                           gpu::Extent initial_extent);

  GpuContext(
      AllocatorImpl allocator, gpu::Device * device,
      gpu::PipelineCache pipeline_cache, u32 buffering,
      gpu::Format color_format, gpu::Format depth_stencil_format,
      gpu::DescriptorSetLayout ubo_layout, gpu::DescriptorSetLayout ssbo_layout,
      gpu::DescriptorSetLayout textures_layout,
      gpu::DescriptorSetLayout samplers_layout,
      gpu::DescriptorSet texture_views, gpu::DescriptorSet samplers,
      gpu::Image                                  default_image,
      Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views,
      InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects) :
      device{device},
      pipeline_cache{pipeline_cache},
      buffering{buffering},
      color_format{color_format},
      depth_stencil_format{depth_stencil_format},
      ubo_layout{ubo_layout},
      ssbo_layout{ssbo_layout},
      textures_layout{textures_layout},
      samplers_layout{samplers_layout},
      texture_views{texture_views},
      samplers{samplers},
      sampler_cache{allocator},
      screen_fb{},
      scratch_fbs{},
      default_image{default_image},
      default_image_views{default_image_views},
      released_objects{std::move(released_objects)},
      texture_slots{},
      sampler_slots{}
  {
  }

  GpuContext(GpuContext const &)             = delete;
  GpuContext(GpuContext &&)                  = default;
  GpuContext & operator=(GpuContext const &) = delete;
  GpuContext & operator=(GpuContext &&)      = default;

  ~GpuContext()
  {
    release(default_image);
    for (gpu::ImageView v : default_image_views)
    {
      release(v);
    }
    release(texture_views);
    release(samplers);
    release(ubo_layout);
    release(ssbo_layout);
    release(textures_layout);
    release(samplers_layout);
    release(screen_fb);
    for (Framebuffer & f : scratch_fbs)
    {
      release(f);
    }
    for (auto const & [_, sampler] : sampler_cache)
    {
      release(sampler.sampler);
    }
    idle_reclaim();
    device->uninit_pipeline_cache(pipeline_cache);
  }

  void recreate_framebuffers(gpu::Extent new_extent);

  gpu::CommandEncoder & encoder();

  u32 ring_index();

  gpu::FrameId frame_id();

  gpu::FrameId tail_frame_id();

  CachedSampler create_sampler(gpu::SamplerInfo const & info);

  u32 alloc_texture_slot();

  void release_texture_slot(u32 slot);

  u32 alloc_sampler_slot();

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

  void submit_frame(gpu::Swapchain swapchain);
};

struct SSBO
{
  gpu::Buffer buffer = nullptr;

  u64 size = 0;

  gpu::DescriptorSet descriptor = nullptr;

  Span<char const> label = "SSBO"_str;

  void uninit(GpuContext & ctx);

  void reserve(GpuContext & ctx, u64 size);

  void copy(GpuContext & ctx, Span<u8 const> src);

  void * map(GpuContext & ctx);

  void unmap(GpuContext & ctx);

  void flush(GpuContext & ctx);

  void release(GpuContext & ctx);
};

}        // namespace ash
