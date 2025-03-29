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

/// @brief created with sampled, storage, color attachment, and transfer flags
struct ColorTexture
{
  static constexpr gpu::FormatFeatures FEATURES =
    gpu::FormatFeatures::ColorAttachment |
    gpu::FormatFeatures::ColorAttachmentBlend |
    gpu::FormatFeatures::StorageImage | gpu::FormatFeatures::SampledImage;

  static constexpr gpu::Format HDR_FORMATS[] = {
    gpu::Format::R16G16B16A16_SFLOAT};

  static constexpr gpu::Format SDR_FORMATS[] = {gpu::Format::B8G8R8A8_UNORM,
                                                gpu::Format::R8G8B8A8_UNORM};

  gpu::ImageInfo info = {};

  gpu::ImageViewInfo view_info = {};

  gpu::Image image = nullptr;

  gpu::ImageView view = nullptr;

  gpu::DescriptorSet texture = nullptr;

  static constexpr TextureId texture_id = TextureId::Base;

  constexpr Vec3U extent() const
  {
    return info.extent;
  }
};

/// @brief created with color attachment flag
struct ColorMsaaTexture
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

  constexpr Vec3U extent() const
  {
    return info.extent;
  }
};

struct DepthTexture
{
  static constexpr gpu::FormatFeatures FEATURES =
    gpu::FormatFeatures::DepthStencilAttachment |
    gpu::FormatFeatures::SampledImage;

  static constexpr gpu::Format FORMATS[] = {gpu::Format::D16_UNORM_S8_UINT,
                                            gpu::Format::D24_UNORM_S8_UINT,
                                            gpu::Format::D32_SFLOAT_S8_UINT};

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

  constexpr Vec3U extent() const
  {
    return info.extent;
  }
};

struct Framebuffer
{
  /// @brief color texture
  ColorTexture color = {};

  Option<ColorMsaaTexture> color_msaa = none;

  /// @brief combined depth and stencil aspect attachment
  DepthTexture depth = {};

  constexpr Vec3U extent() const
  {
    return color.extent();
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

struct GpuSystem;

struct GpuBuffer
{
  Str label_;

  gpu::Buffer buffer_ = nullptr;

  u64 size_ = 0;

  u64 capacity_ = 0;

  gpu::BufferUsage usage_;

  GpuBuffer(Str label, gpu::BufferUsage usage);
  GpuBuffer(GpuBuffer const &)             = default;
  GpuBuffer(GpuBuffer &&)                  = default;
  GpuBuffer & operator=(GpuBuffer const &) = default;
  GpuBuffer & operator=(GpuBuffer &&)      = default;
  ~GpuBuffer()                             = default;

  bool is_valid() const;

  void release(GpuSystem & gpu);

  void reserve_exact(GpuSystem & gpu, u64 target_capacity, bool defer);

  void assign(GpuSystem & gpu, Span<u8 const> src);

  void * map(GpuSystem & gpu);

  void unmap(GpuSystem & gpu);

  void flush(GpuSystem & gpu);
};

struct ShaderBuffer
{
  static constexpr gpu::BufferUsage USAGE =
    gpu::BufferUsage::UniformBuffer | gpu::BufferUsage::StorageBuffer |
    gpu::BufferUsage::UniformTexelBuffer |
    gpu::BufferUsage::StorageTexelBuffer | gpu::BufferUsage::IndirectBuffer |
    gpu::BufferUsage::TransferSrc | gpu::BufferUsage::TransferDst;

  Str label_;

  gpu::Buffer buffer_ = nullptr;

  u64 size_ = 0;

  u64 capacity_ = 0;

  gpu::BufferUsage usage_;

  gpu::DescriptorSet descriptor_;

  ShaderBuffer(Str label, gpu::BufferUsage usage = USAGE);
  ShaderBuffer(ShaderBuffer const &)             = default;
  ShaderBuffer(ShaderBuffer &&)                  = default;
  ShaderBuffer & operator=(ShaderBuffer const &) = default;
  ShaderBuffer & operator=(ShaderBuffer &&)      = default;
  ~ShaderBuffer()                                = default;

  bool is_valid() const;

  void release(GpuSystem & gpu);

  void uninit(GpuSystem & gpu);

  void reserve_exact(GpuSystem & gpu, u64 target_capacity, bool defer);

  void assign(GpuSystem & gpu, Span<u8 const> src);

  void * map(GpuSystem & gpu);

  void unmap(GpuSystem & gpu);

  void flush(GpuSystem & gpu);
};

typedef ShaderBuffer StructuredBuffer;

typedef ShaderBuffer ConstantBuffer;

struct GpuQueries
{
  f32 time_period_;

  gpu::TimeStampQuery timestamps_ = nullptr;

  u32 timespans_capacity_;

  gpu::StatisticsQuery statistics_;

  u32 statistics_capacity_;

  Vec<u64> cpu_timestamps_;

  Vec<gpu::PipelineStatistics> cpu_statistics_;

  Vec<Str> timespan_labels_;

  Vec<Str> statistics_labels_;

  static GpuQueries create(AllocatorRef allocator, gpu::Device & dev,
                           f32 time_period, u32 num_timespans,
                           u32 num_statistics);

  GpuQueries(AllocatorRef allocator, f32 time_period,
             gpu::TimeStampQuery timestamps, u32 timespans_capacity,
             gpu::StatisticsQuery statistics, u32 statistics_capacity) :
    time_period_{time_period},
    timestamps_{timestamps},
    timespans_capacity_{timespans_capacity},
    statistics_{statistics},
    statistics_capacity_{statistics_capacity},
    cpu_timestamps_{allocator},
    cpu_statistics_{allocator},
    timespan_labels_{allocator},
    statistics_labels_{allocator}
  {
  }

  GpuQueries(GpuQueries const &)             = delete;
  GpuQueries & operator=(GpuQueries const &) = delete;
  GpuQueries(GpuQueries &&)                  = default;
  GpuQueries & operator=(GpuQueries &&)      = default;
  ~GpuQueries()                              = default;

  void uninit(gpu::Device & dev);

  void begin_frame(gpu::Device & dev, gpu::CommandEncoder & enc);

  Option<u32> begin_timespan(gpu::CommandEncoder & enc, Str label);

  void end_timespan(gpu::CommandEncoder & enc, u32 id);

  Option<u32> begin_statistics(gpu::CommandEncoder & enc, Str label);

  void end_statistics(gpu::CommandEncoder & enc, u32 id);
};

struct FrameGraph
{
  typedef Fn<void(FrameGraph &, gpu::CommandEncoder &)> PassEncoderFn;
  typedef Dyn<PassEncoderFn>                            PassEncoder;

  typedef Fn<void(gpu::CommandEncoder &, gpu::Buffer buffer, Slice64)>
                               UploadEncoderFn;
  typedef Dyn<UploadEncoderFn> UploadEncoder;

  typedef Fn<void()>  TaskFn;
  typedef Dyn<TaskFn> Task;

  struct Pass
  {
    Str         label;
    PassEncoder encoder;
  };

  struct Upload
  {
    UploadEncoder encoder;
    Slice64       slice;
  };

  struct FrameData
  {
    StructuredBuffer sb{"FrameGraph::StructuredBuffer"_str};
    GpuBuffer        staging{"FrameGraph::StagingBuffer"_str,
                      gpu::BufferUsage::TransferSrc |
                        gpu::BufferUsage::TransferDst};

    void release(GpuSystem & gpu);
  };

  typedef InplaceVec<FrameData, gpu::MAX_FRAME_BUFFERING> BufferedFrameData;

  BufferedFrameData frame_data_;
  u32               ring_index_;
  bool              uploaded_;
  Vec<u8>           sb_data_;
  Vec<Slice32>      sb_entries_;
  Vec<u8>           staging_data_;
  Vec<Upload>       uploads_;
  Vec<Task>         tasks_;
  Vec<Pass>         passes_;
  ArenaPool         arena_;

  FrameGraph(AllocatorRef allocator) :
    frame_data_{},
    ring_index_{0},
    uploaded_{false},
    sb_data_{allocator},
    sb_entries_{allocator},
    staging_data_{allocator},
    uploads_{allocator},
    tasks_{allocator},
    passes_{allocator},
    arena_{allocator}
  {
  }

  FrameGraph(FrameGraph const &)             = delete;
  FrameGraph & operator=(FrameGraph const &) = delete;
  FrameGraph(FrameGraph &&)                  = default;
  FrameGraph & operator=(FrameGraph &&)      = default;
  ~FrameGraph()                              = default;

  u32 push_ssbo(Span<u8 const> data);

  Tuple<StructuredBuffer, Slice32> get_structured_buffer(u32 id);

  void add_pass(Pass pass);

  template <typename Lambda>
  void add_pass(Str label, Lambda && task)
  {
    // relocate lambda to heap
    auto lambda       = dyn(arena_, static_cast<Lambda &&>(task)).unwrap();
    // allocator is noop-ed but destructor still runs when the dynamic object is
    // uninitialized. the memory is freed by at the end of the frame anyway so
    // no need to free it
    lambda.allocator_ = noop_allocator;

    PassEncoderFn f{lambda.get()};

    return add_pass(
      Pass{.label = label, .encoder = transmute(std::move(lambda), f)});
  }

  template <typename Lambda>
  void add_task(Lambda && task)
  {
    auto lambda       = dyn(arena_, static_cast<Lambda &&>(task)).unwrap();
    lambda.allocator_ = noop_allocator;
    TaskFn f{lambda.get()};

    tasks_.push(transmute(std::move(lambda), f)).unwrap();
  }

  template <typename Encoder>
  void upload(Span<u8 const> buffer, Encoder && encoder)
  {
    CHECK(!uploaded_, "");

    auto const offset = staging_data_.size64();

    staging_data_.extend(buffer).unwrap();

    Slice64 const slice{offset, buffer.size64()};

    auto lambda = dyn(arena_, static_cast<Encoder &&>(encoder)).unwrap();

    lambda.allocator_ = noop_allocator;

    UploadEncoderFn f{lambda.get()};

    uploads_
      .push(Upload{.encoder = transmute(std::move(lambda), f), .slice = slice})
      .unwrap();
  }

  void execute(GpuSystem & gpu);

  void acquire(GpuSystem & gpu);

  void release(GpuSystem & gpu);
};

struct GpuSystem
{
  static constexpr u32 NUM_TEXTURE_SLOTS = 2'048;

  static constexpr u32 NUM_SAMPLER_SLOTS = 128;

  static constexpr u32 NUM_FRAME_TIMESPANS = 2'048;

  static constexpr u32 NUM_FRAME_STATISTICS = 4'096;

  static constexpr u32 NUM_SCRATCH_COLOR_TEXTURES = 2;

  static constexpr u32 NUM_SCRATCH_DEPTH_TEXTURES = 1;

  gpu::Device * device_;

  gpu::DeviceProperties props_;

  gpu::PipelineCache pipeline_cache_;

  u32 buffering_;

  gpu::SampleCount sample_count_;

  /// @brief hdr if hdr supported and required.
  gpu::Format color_format_;

  gpu::Format depth_format_;

  gpu::DescriptorSetLayout cb_layout_;

  gpu::DescriptorSetLayout sb_layout_;

  gpu::DescriptorSetLayout textures_layout_;

  gpu::DescriptorSetLayout samplers_layout_;

  gpu::DescriptorSet textures_;

  gpu::DescriptorSet samplers_;

  SamplerCache sampler_cache_;

  Framebuffer fb_;

  ColorTexture scratch_color_[NUM_SCRATCH_COLOR_TEXTURES];

  DepthTexture scratch_depth_[NUM_SCRATCH_DEPTH_TEXTURES];

  gpu::Image default_image_;

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views_;

  Bits<u64, NUM_TEXTURE_SLOTS> texture_slots_;

  Bits<u64, NUM_SAMPLER_SLOTS> sampler_slots_;

  InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects_;

  FrameGraph frame_graph_;

  InplaceVec<GpuQueries, gpu::MAX_FRAME_BUFFERING> queries_;

  static GpuSystem create(AllocatorRef allocator, gpu::Device & device,
                          Span<u8 const> pipeline_cache_data, bool use_hdr,
                          u32 buffering, gpu::SampleCount sample_count,
                          Vec2U initial_extent);

  GpuSystem(
    AllocatorRef allocator, gpu::Device & device, gpu::DeviceProperties props,
    gpu::PipelineCache pipeline_cache, u32 buffering,
    gpu::SampleCount sample_count, gpu::Format color_format,
    gpu::Format depth_format, gpu::DescriptorSetLayout cb_layout,
    gpu::DescriptorSetLayout sb_layout,
    gpu::DescriptorSetLayout textures_layout,
    gpu::DescriptorSetLayout samplers_layout, gpu::DescriptorSet textures,
    gpu::DescriptorSet samplers, gpu::Image default_image,
    Array<gpu::ImageView, NUM_DEFAULT_TEXTURES>            default_image_views,
    InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects,
    FrameGraph                                             frame_graph,
    InplaceVec<GpuQueries, gpu::MAX_FRAME_BUFFERING>       queries) :
    device_{&device},
    props_{props},
    pipeline_cache_{pipeline_cache},
    buffering_{buffering},
    sample_count_{sample_count},
    color_format_{color_format},
    depth_format_{depth_format},
    cb_layout_{cb_layout},
    sb_layout_{sb_layout},
    textures_layout_{textures_layout},
    samplers_layout_{samplers_layout},
    textures_{textures},
    samplers_{samplers},
    sampler_cache_{allocator},
    fb_{},
    scratch_color_{},
    scratch_depth_{},
    default_image_{default_image},
    default_image_views_{default_image_views},
    texture_slots_{},
    sampler_slots_{},
    released_objects_{std::move(released_objects)},
    frame_graph_{std::move(frame_graph)},
    queries_{std::move(queries)}
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

  void release(ColorTexture tex);

  void release(ColorMsaaTexture tex);

  void release(DepthTexture tex);

  void release(Framebuffer tex);

  void idle_reclaim();

  template <typename Encoder>
  void upload(Span<u8 const> buffer, Encoder && encoder)
  {
    frame_graph_.upload(buffer, static_cast<Encoder &&>(encoder));
  }

  template <typename Lambda>
  void add_task(Lambda && task)
  {
    frame_graph_.add_task(static_cast<Lambda &&>(task));
  }

  void frame(gpu::Swapchain swapchain);

  Option<u32> begin_timespan(Str label);

  void end_timespan(u32 id);

  Option<u32> begin_statistics(Str label);

  void end_statistics(u32 id);
};

}    // namespace ash
