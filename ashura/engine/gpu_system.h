/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/gpu/gpu.h"
#include "ashura/std/allocators.h"
#include "ashura/std/async.h"
#include "ashura/std/map.h"
#include "ashura/std/option.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class TextureId : u32
{
  Base        = 0,
  White       = 0,
  Black       = 1,
  Transparent = 2,
  Alpha       = 3,
  Red         = 4,
  Green       = 5,
  Blue        = 6,
  Magenta     = 7,
  Cyan        = 8,
  Yellow      = 9
};

inline constexpr u32 NUM_DEFAULT_TEXTURES = 10;

enum class SamplerId : u32
{
  Linear         = 0,
  Nearest        = 1,
  LinearClamped  = 2,
  NearestClamped = 3
};

inline constexpr u32 NUM_DEFAULT_SAMPLERS = 4;

struct Framebuffer
{
  /// @brief created with sampled, storage, color attachment, and transfer flags
  struct Color
  {
    gpu::ImageInfo info = {};

    gpu::ImageViewInfo view_info = {};

    gpu::Image image = nullptr;

    gpu::ImageView view = nullptr;

    gpu::DescriptorSet texture = nullptr;

    static constexpr TextureId texture_id = TextureId::Base;
  };

  /// @brief created with color attachment flag
  struct ColorMsaa
  {
    gpu::ImageInfo info = {};

    gpu::ImageViewInfo view_info = {};

    gpu::Image image = nullptr;

    /// @brief to preserve bandwidth (especially for tiled architectures), preferably
    /// use `StoreOp::DontCare` and `LoadOp::Clear/LoadOp::DontCare` in the render passes.
    gpu::ImageView view = nullptr;

    constexpr gpu::SampleCount sample_count() const
    {
      return info.sample_count;
    }
  };

  struct Depth
  {
    gpu::ImageInfo info = {};

    gpu::ImageViewInfo view_info = {};

    gpu::ImageViewInfo stencil_view_info = {};

    gpu::Image image = nullptr;

    gpu::ImageView view = nullptr;

    gpu::ImageView stencil_view = {};

    gpu::DescriptorSet texture = nullptr;

    static constexpr TextureId texture_id = TextureId::Base;

    gpu::DescriptorSet stencil_texture = nullptr;

    static constexpr TextureId stencil_texture_id = TextureId::Base;
  };

  /// @brief color texture
  Color color = {};

  Option<ColorMsaa> color_msaa = none;

  /// @brief combined depth and stencil aspect attachment
  Depth depth = {};

  constexpr gpu::Viewport viewport() const
  {
    return gpu::Viewport{.offset    = {},
                         .extent    = as_vec2(extent()),
                         .min_depth = 0,
                         .max_depth = 1};
  }

  constexpr gpu::Rect scissor() const
  {
    return gpu::Rect{.offset = {}, .extent = extent()};
  }

  constexpr gpu::Extent3D extent3() const
  {
    return color.info.extent;
  }

  constexpr gpu::Extent extent() const
  {
    return gpu::Extent{extent3().x, extent3().y};
  }
};

struct SamplerHasher
{
  constexpr hash64 operator()(gpu::SamplerInfo const & info) const
  {
    return hash_combine_n(
      (hash64) info.mag_filter, (hash64) info.min_filter,
      (hash64) info.mip_map_mode, (hash64) info.address_mode_u,
      (hash64) info.address_mode_v, (hash64) info.address_mode_w,
      (hash64) bit_cast<u32>(info.mip_lod_bias),
      (hash64) info.anisotropy_enable,
      (hash64) bit_cast<u32>(info.max_anisotropy), (hash64) info.compare_enable,
      (hash64) info.compare_op, (hash64) bit_cast<u32>(info.min_lod),
      (hash64) bit_cast<u32>(info.max_lod), (hash64) info.border_color,
      (hash64) info.unnormalized_coordinates);
  }
};

struct SamplerEq
{
  constexpr bool operator()(gpu::SamplerInfo const & a,
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
  SamplerId    id      = SamplerId::Linear;
};

typedef Map<gpu::SamplerInfo, CachedSampler, SamplerHasher, SamplerEq, u32>
  SamplerCache;

/// @param color_format hdr if hdr supported and required.
///
/// scratch images resized when swapchain extents changes
///
struct GpuSystem
{
  // [ ] for all the formats, get their properties& features. add to gpu api and allow passes to query them
  // [ ] create SRGB/SDR ranking of formats
  // [ ] allow user to override swapchain/texture format

  struct JobQueue
  {
    ArenaPool       arena;
    Vec<Fn<void()>> jobs;

    void clear()
    {
      jobs.clear();
      arena.reclaim();
    }
  };

  struct FrameQueue
  {
    JobQueue pre;
    JobQueue main;
    JobQueue post;
  };

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

  static constexpr u16 NUM_TEXTURE_SLOTS = 1'024;
  static constexpr u16 NUM_SAMPLER_SLOTS = 64;

  gpu::Device * device;

  gpu::PipelineCache pipeline_cache;

  u32 buffering;

  gpu::SampleCount sample_count;

  gpu::Format color_format;

  gpu::Format depth_stencil_format;

  gpu::DescriptorSetLayout ubo_layout;

  gpu::DescriptorSetLayout ssbo_layout;

  gpu::DescriptorSetLayout textures_layout;

  gpu::DescriptorSetLayout samplers_layout;

  gpu::DescriptorSet texture_views;

  gpu::DescriptorSet samplers;

  SamplerCache sampler_cache;

  Framebuffer fb;

  Framebuffer scratch_fb;

  gpu::Image default_image;

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views;

  InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects;

  Bits<u64, NUM_TEXTURE_SLOTS> texture_slots;

  Bits<u64, NUM_SAMPLER_SLOTS> sampler_slots;

  InplaceVec<JobQueue, gpu::MAX_FRAME_BUFFERING> jobs;

  static GpuSystem create(AllocatorImpl allocator, gpu::Device * device,
                          bool use_hdr, u32 buffering,
                          gpu::SampleCount sample_count,
                          gpu::Extent      initial_extent);

  GpuSystem(
    AllocatorImpl allocator, gpu::Device * device,
    gpu::PipelineCache pipeline_cache, u32 buffering,
    gpu::SampleCount sample_count, gpu::Format color_format,
    gpu::Format depth_stencil_format, gpu::DescriptorSetLayout ubo_layout,
    gpu::DescriptorSetLayout ssbo_layout,
    gpu::DescriptorSetLayout textures_layout,
    gpu::DescriptorSetLayout samplers_layout, gpu::DescriptorSet texture_views,
    gpu::DescriptorSet samplers, gpu::Image default_image,
    Array<gpu::ImageView, NUM_DEFAULT_TEXTURES>            default_image_views,
    InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects) :
    device{device},
    pipeline_cache{pipeline_cache},
    buffering{buffering},
    sample_count{sample_count},
    color_format{color_format},
    depth_stencil_format{depth_stencil_format},
    ubo_layout{ubo_layout},
    ssbo_layout{ssbo_layout},
    textures_layout{textures_layout},
    samplers_layout{samplers_layout},
    texture_views{texture_views},
    samplers{samplers},
    sampler_cache{allocator},
    fb{},
    scratch_fb{},
    default_image{default_image},
    default_image_views{default_image_views},
    released_objects{std::move(released_objects)},
    texture_slots{},
    sampler_slots{}
  {
  }

  GpuSystem(GpuSystem const &)             = delete;
  GpuSystem(GpuSystem &&)                  = default;
  GpuSystem & operator=(GpuSystem const &) = delete;
  GpuSystem & operator=(GpuSystem &&)      = default;
  ~GpuSystem()                             = default;

  void uninit()
  {
    release(texture_views);
    for (gpu::ImageView v : default_image_views)
    {
      release(v);
    }
    release(default_image);
    release(samplers);
    release(ubo_layout);
    release(ssbo_layout);
    release(textures_layout);
    release(samplers_layout);
    release(fb);
    release(scratch_fb);
    for (auto const & [_, sampler] : sampler_cache)
    {
      release(sampler.sampler);
    }
    idle_reclaim();
    device->uninit(pipeline_cache);
  }

  void recreate_framebuffers(gpu::Extent new_extent);

  gpu::CommandEncoder & encoder();

  u32 ring_index();

  gpu::FrameId frame_id();

  gpu::FrameId tail_frame_id();

  CachedSampler create_sampler(gpu::SamplerInfo const & info);

  TextureId alloc_texture_id(gpu::ImageView view);

  void release_texture_id(TextureId id);

  SamplerId alloc_sampler_id(gpu::Sampler sampler);

  void release_sampler_id(SamplerId id);

  /// @brief schedule an object for destruction, the object is destructed on the next frame
  void release(gpu::Object object);

  void release(Framebuffer::Color const & fb)
  {
    release(fb.texture);
    release(fb.view);
    release(fb.image);
  }

  void release(Framebuffer::ColorMsaa const & fb)
  {
    release(fb.view);
    release(fb.image);
  }

  void release(Framebuffer::Depth const & fb)
  {
    release(fb.texture);
    release(fb.stencil_texture);
    release(fb.view);
    release(fb.stencil_view);
    release(fb.image);
  }

  void release(Framebuffer const & fb)
  {
    release(fb.color);
    fb.color_msaa.match([&](Framebuffer::ColorMsaa const & f) { release(f); });
    release(fb.depth);
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

  void uninit(GpuSystem & gpu);

  void reserve(GpuSystem & gpu, u64 size);

  void copy(GpuSystem & gpu, Span<u8 const> src);

  void * map(GpuSystem & gpu);

  void unmap(GpuSystem & gpu);

  void flush(GpuSystem & gpu);

  void release(GpuSystem & gpu);
};

}    // namespace ash
