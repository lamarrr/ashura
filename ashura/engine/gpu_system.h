/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/gpu/gpu.h"
#include "ashura/std/allocators.h"
#include "ashura/std/map.h"
#include "ashura/std/option.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// @details do not change the underlying type. It maps directly to the GPU handle
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

/// @details do not change the underlying type. It maps directly to the GPU handle
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

  constexpr Vec3U extent3() const
  {
    return color.info.extent;
  }

  constexpr Vec2U extent() const
  {
    return Vec2U{extent3().x, extent3().y};
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

struct Sampler
{
  SamplerId    id      = SamplerId::Linear;
  gpu::Sampler sampler = nullptr;
};

typedef Map<gpu::SamplerInfo, Sampler, SamplerHasher, SamplerEq, u32>
  SamplerCache;

struct StagingBuffer
{
  Span<char const> label = "Staging Buffer"_str;

  gpu::Buffer buffer = nullptr;

  u64 size = 0;

  void uninit(gpu::Device & gpu);

  void reserve(gpu::Device & gpu, u64 target_size);

  void grow(gpu::Device & gpu, u64 target_size);

  void assign(gpu::Device & gpu, Span<u8 const> src);

  void * map(gpu::Device & gpu);

  void unmap(gpu::Device & gpu);

  void flush(gpu::Device & gpu);
};

struct GpuTaskQueue
{
  ArenaPool arena_;

  Vec<Dyn<Fn<void()>>> tasks_;

  static GpuTaskQueue make(AllocatorRef allocator);

  GpuTaskQueue(ArenaPool arena, Vec<Dyn<Fn<void()>>> tasks) :
    arena_{std::move(arena)},
    tasks_{std::move(tasks)}
  {
  }

  template <typename Lambda>
  void add(Lambda && task)
  {
    Dyn<Lambda *> lambda =
      dyn(arena_.ref(), static_cast<Lambda &&>(task)).unwrap();
    lambda.allocator_ = noop_allocator;

    tasks_.push(transmute(std::move(lambda), fn(*lambda))).unwrap();
  }

  void run();
};

struct GpuUploadQueue
{
  typedef Fn<void(gpu::CommandEncoder &, gpu::Buffer buffer, Slice64)> Encoder;

  struct Task
  {
    Slice64      slice{};
    Dyn<Encoder> encoder;
  };

  struct UploadBuffer
  {
    StagingBuffer gpu{};
    Vec<u8>       cpu{};
  };

  ArenaPool arena_;

  InplaceVec<UploadBuffer, gpu::MAX_FRAME_BUFFERING> buffers_;

  Vec<Task> tasks_;

  u32 ring_index_;

  static GpuUploadQueue make(u32 buffering, AllocatorRef allocator);

  GpuUploadQueue(ArenaPool                                          arena,
                 InplaceVec<UploadBuffer, gpu::MAX_FRAME_BUFFERING> buffers,
                 Vec<Task>                                          tasks) :
    arena_{std::move(arena)},
    buffers_{std::move(buffers)},
    tasks_{std::move(tasks)},
    ring_index_{0}
  {
  }

  template <typename Encoder>
  void queue(Span<u8 const> buffer, Encoder && encoder)
  {
    UploadBuffer & upload = buffers_[ring_index_];

    Slice64 slice{upload.cpu.size64(), 0};

    upload.cpu.extend(buffer).unwrap();

    slice.span = buffer.size64();

    Dyn<Encoder *> lambda =
      dyn(arena_.ref(), static_cast<Encoder &&>(encoder)).unwrap();
    lambda.allocator_ = noop_allocator;

    tasks_
      .push(Task{.slice   = slice,
                 .encoder = transmute(std::move(lambda), fn(*lambda))})
      .unwrap();
  }

  void encode(gpu::Device & gpu, gpu::CommandEncoder & enc);
};

struct GpuSystem
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

  static constexpr gpu::Format HDR_FORMATS[] = {
    gpu::Format::R16G16B16A16_SFLOAT};

  static constexpr gpu::Format SDR_FORMATS[] = {gpu::Format::B8G8R8A8_UNORM,
                                                gpu::Format::R8G8B8A8_UNORM};

  static constexpr gpu::Format DEPTH_STENCIL_FORMATS[] = {
    gpu::Format::D16_UNORM_S8_UINT, gpu::Format::D24_UNORM_S8_UINT,
    gpu::Format::D32_SFLOAT_S8_UINT};

  static constexpr u16 NUM_TEXTURE_SLOTS = 1'024;
  static constexpr u16 NUM_SAMPLER_SLOTS = 64;

  gpu::Device * device;

  gpu::PipelineCache pipeline_cache;

  u32 buffering;

  gpu::SampleCount sample_count;

  /// @brief hdr if hdr supported and required.
  gpu::Format color_format;

  gpu::Format depth_stencil_format;

  gpu::DescriptorSetLayout ubo_layout;

  gpu::DescriptorSetLayout ssbo_layout;

  gpu::DescriptorSetLayout textures_layout;

  gpu::DescriptorSetLayout samplers_layout;

  gpu::DescriptorSet textures;

  gpu::DescriptorSet samplers;

  SamplerCache sampler_cache;

  Framebuffer fb;

  Framebuffer scratch_fb;

  gpu::Image default_image;

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views;

  Bits<u64, NUM_TEXTURE_SLOTS> texture_slots;

  Bits<u64, NUM_SAMPLER_SLOTS> sampler_slots;

  InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects;

  GpuTaskQueue tasks_;

  GpuUploadQueue upload_;

  static GpuSystem create(AllocatorRef allocator, gpu::Device & device,
                          Span<u8 const> pipeline_cache_data, bool use_hdr,
                          u32 buffering, gpu::SampleCount sample_count,
                          Vec2U initial_extent);

  GpuSystem(
    AllocatorRef allocator, gpu::Device & device,
    gpu::PipelineCache pipeline_cache, u32 buffering,
    gpu::SampleCount sample_count, gpu::Format color_format,
    gpu::Format depth_stencil_format, gpu::DescriptorSetLayout ubo_layout,
    gpu::DescriptorSetLayout ssbo_layout,
    gpu::DescriptorSetLayout textures_layout,
    gpu::DescriptorSetLayout samplers_layout, gpu::DescriptorSet textures,
    gpu::DescriptorSet samplers, gpu::Image default_image,
    Array<gpu::ImageView, NUM_DEFAULT_TEXTURES>            default_image_views,
    InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects,
    GpuTaskQueue tasks, GpuUploadQueue upload) :
    device{&device},
    pipeline_cache{pipeline_cache},
    buffering{buffering},
    sample_count{sample_count},
    color_format{color_format},
    depth_stencil_format{depth_stencil_format},
    ubo_layout{ubo_layout},
    ssbo_layout{ssbo_layout},
    textures_layout{textures_layout},
    samplers_layout{samplers_layout},
    textures{textures},
    samplers{samplers},
    sampler_cache{allocator},
    fb{},
    scratch_fb{},
    default_image{default_image},
    default_image_views{default_image_views},
    texture_slots{},
    sampler_slots{},
    released_objects{std::move(released_objects)},
    tasks_{std::move(tasks)},
    upload_{std::move(upload)}
  {
  }

  GpuSystem(GpuSystem const &)             = delete;
  GpuSystem(GpuSystem &&)                  = default;
  GpuSystem & operator=(GpuSystem const &) = delete;
  GpuSystem & operator=(GpuSystem &&)      = default;
  ~GpuSystem()                             = default;

  void shutdown(Vec<u8> & cache);

  void recreate_framebuffers(Vec2U new_extent);

  gpu::CommandEncoder & encoder();

  u32 ring_index();

  gpu::FrameId frame_id();

  gpu::FrameId tail_frame_id();

  Sampler create_sampler(gpu::SamplerInfo const & info);

  TextureId alloc_texture_id(gpu::ImageView view);

  void release_texture_id(TextureId id);

  SamplerId alloc_sampler_id(gpu::Sampler sampler);

  void release_sampler_id(SamplerId id);

  /// @brief schedule a GPU object for destruction,
  /// the object is destructed on the next frame cycle, this will prevent
  /// the operation from stalling the GPU
  void release(gpu::Object object);

  void release(Framebuffer::Color const & fb);

  void release(Framebuffer::ColorMsaa const & fb);

  void release(Framebuffer::Depth const & fb);

  void release(Framebuffer const & fb);

  template <typename Lambda>
  void add_pre_frame_task(Lambda && task)
  {
    tasks_.add(static_cast<Lambda &&>(task));
  }

  void idle_reclaim();

  void begin_frame(gpu::Swapchain swapchain);

  void submit_frame(gpu::Swapchain swapchain);
};

struct SSBO
{
  Span<char const> label = "SSBO"_str;

  gpu::Buffer buffer = nullptr;

  u64 size = 0;

  gpu::DescriptorSet descriptor = nullptr;

  void uninit(GpuSystem & gpu);

  void reserve(GpuSystem & gpu, u64 target_size);

  void assign(GpuSystem & gpu, Span<u8 const> src);

  void * map(GpuSystem & gpu);

  void unmap(GpuSystem & gpu);

  void flush(GpuSystem & gpu);

  void release(GpuSystem & gpu);
};

}    // namespace ash
