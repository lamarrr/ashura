/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/gpu/gpu.h"
#include "ashura/std/allocators.h"
#include "ashura/std/async.h"
#include "ashura/std/dict.h"
#include "ashura/std/option.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// @details do not change the underlying type. It maps directly to the GPU handle
enum class [[nodiscard]] TextureId : u32
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
enum class [[nodiscard]] SamplerId : u32
{
  LinearBlack    = 0,
  NearestBlack   = 1,
  LinearClamped  = 2,
  NearestClamped = 3
};

enum class [[nodiscard]] GpuBufferId : u32
{
};

enum class [[nodiscard]] BufferId : u32
{
};

inline constexpr u32 NUM_DEFAULT_SAMPLERS = 4;

/// @brief created with sampled, storage, color attachment, and transfer flags
struct [[nodiscard]] ColorTexture
{
  static constexpr gpu::FormatFeatures FEATURES =
    gpu::FormatFeatures::ColorAttachment |
    gpu::FormatFeatures::ColorAttachmentBlend |
    gpu::FormatFeatures::StorageImage | gpu::FormatFeatures::SampledImage;

  static constexpr gpu::ImageUsage USAGE =
    gpu::ImageUsage::ColorAttachment | gpu::ImageUsage::InputAttachment |
    gpu::ImageUsage::Sampled | gpu::ImageUsage::Storage |
    gpu::ImageUsage::TransferDst | gpu::ImageUsage::TransferSrc;

  static constexpr gpu::Format HDR_FORMATS[] = {
    gpu::Format::R16G16B16A16_SFLOAT};

  static constexpr gpu::Format SDR_FORMATS[] = {gpu::Format::B8G8R8A8_UNORM,
                                                gpu::Format::R8G8B8A8_UNORM};

  gpu::ImageInfo info = {};

  gpu::ImageViewInfo view_info = {};

  gpu::Image image = nullptr;

  gpu::ImageView view = nullptr;

  gpu::DescriptorSet sampled_texture = nullptr;

  gpu::DescriptorSet storage_texture = nullptr;

  gpu::DescriptorSet input_attachment = nullptr;

  static constexpr TextureId sampled_texture_id = TextureId::Base;

  static constexpr TextureId storage_texture_id = TextureId::Base;

  constexpr u32x3 extent() const
  {
    return info.extent;
  }

  void uninit(gpu::Device device);
};

/// @brief created with color attachment flag
struct [[nodiscard]] ColorMsaaTexture
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

  constexpr u32x3 extent() const
  {
    return info.extent;
  }

  void uninit(gpu::Device device);
};

struct [[nodiscard]] DepthStencilTexture
{
  static constexpr gpu::FormatFeatures FEATURES =
    gpu::FormatFeatures::DepthStencilAttachment |
    gpu::FormatFeatures::SampledImage;

  static constexpr gpu::ImageUsage USAGE =
    gpu::ImageUsage::DepthStencilAttachment | gpu::ImageUsage::InputAttachment |
    gpu::ImageUsage::Sampled | gpu::ImageUsage::Storage |
    gpu::ImageUsage::TransferDst | gpu::ImageUsage::TransferSrc;

  static constexpr gpu::Format FORMATS[] = {gpu::Format::D16_UNORM_S8_UINT,
                                            gpu::Format::D24_UNORM_S8_UINT,
                                            gpu::Format::D32_SFLOAT_S8_UINT};

  gpu::ImageInfo info = {};

  gpu::ImageViewInfo depth_view_info = {};

  gpu::ImageViewInfo stencil_view_info = {};

  gpu::Image image = nullptr;

  gpu::ImageView depth_view = nullptr;

  gpu::ImageView stencil_view = {};

  gpu::DescriptorSet depth_sampled_texture = nullptr;

  gpu::DescriptorSet depth_storage_texture = nullptr;

  gpu::DescriptorSet depth_input_attachment = nullptr;

  static constexpr TextureId sampled_depth_texture_id = TextureId::Base;

  static constexpr TextureId storage_depth_texture_id = TextureId::Base;

  constexpr u32x3 extent() const
  {
    return info.extent;
  }

  void uninit(gpu::Device device);
};

struct [[nodiscard]] Framebuffer
{
  /// @brief color texture
  ColorTexture color = {};

  Option<ColorMsaaTexture> color_msaa = none;

  /// @brief combined depth and stencil aspect attachment
  DepthStencilTexture depth_stencil = {};

  constexpr u32x3 extent() const
  {
    return color.extent();
  }

  void uninit(gpu::Device device);
};

struct SamplerHasher
{
  constexpr usize operator()(gpu::SamplerInfo const & info) const
  {
    return bit_hash(info);
  }
};

struct SamplerEq
{
  constexpr bool operator()(gpu::SamplerInfo const & a,
                            gpu::SamplerInfo const & b) const
  {
    // [ ] fix; make the labels empty
    return obj::byte_eq(a, b);
  }
};

typedef Dict<gpu::SamplerInfo, SamplerId, SamplerHasher, SamplerEq, u32>
  SamplerDict;

struct GpuFrame;

struct FrameGraph;

struct GpuSystem;

typedef Fn<void(GpuFrame &)> GpuPassFn;

typedef Dyn<GpuPassFn> GpuPass;

struct [[nodiscard]] GpuBuffer
{
  static constexpr gpu::BufferUsage USAGE =
    gpu::BufferUsage::TransferSrc | gpu::BufferUsage::TransferDst |
    gpu::BufferUsage::UniformTexelBuffer |
    gpu::BufferUsage::StorageTexelBuffer | gpu::BufferUsage::UniformBuffer |
    gpu::BufferUsage::StorageBuffer | gpu::BufferUsage::IndexBuffer |
    gpu::BufferUsage::VertexBuffer | gpu::BufferUsage::IndirectBuffer;

  u64 capacity = 0;

  gpu::BufferUsage usage = gpu::BufferUsage::None;

  gpu::Buffer buffer = nullptr;

  gpu::DescriptorSet const_buffer = nullptr;

  gpu::DescriptorSet read_struct_buffer = nullptr;

  gpu::DescriptorSet read_write_struct_buffer = nullptr;

  void uninit(gpu::Device device);

  static GpuBuffer create(gpu::Device device, GpuSystem const & system,
                          u64 capacity, gpu::BufferUsage usage, Str label,
                          Allocator scratch);
};

struct [[nodiscard]] GpuBufferSpan
{
  GpuBuffer buffer = {};
  Slice64   slice  = {};
};

enum class GpuFrameState : u8
{
  Reset        = 0,
  Constructing = 1,
  Constructed  = 2,
  Encoding     = 3,
  Encoded      = 4,
  Executing    = 5
};

struct GpuQueries
{
  static constexpr u32 DEFAULT_NUM_FRAME_TIMESTAMPS = 8'192;

  static constexpr u32 DEFAULT_NUM_FRAME_STATISTICS = 4'096;

  f32 time_period = 1;

  gpu::TimestampQuery timestamps = nullptr;

  u32 next_timestamp = 0;

  gpu::StatisticsQuery statistics = nullptr;

  u32 next_statistics = 0;

  Vec<u64> cpu_timestamps;

  Vec<gpu::PipelineStatistics> cpu_statistics;

  void uninit(gpu::Device device);

  u32 timestamps_capacity() const
  {
    return size32(cpu_timestamps);
  }

  u32 statistics_capacity() const
  {
    return size32(cpu_statistics);
  }

  Option<Tuple<gpu::TimestampQuery, u32>> allocate_timestamp();

  Option<Tuple<gpu::StatisticsQuery, u32>> allocate_statistics();

  void reset();

  static GpuQueries create(Allocator allocator, gpu::Device device,
                           Span<char const> label, f32 time_period,
                           u32 timestamps_capacity, u32 statistics_capacity,
                           Allocator scratch);
};

struct GpuDescriptorsLayout
{
  static constexpr u32 DEFAULT_SAMPLED_TEXTURES_CAPACITY = 16'384;

  static constexpr u32 DEFAULT_SAMPLERS_CAPACITY = 512;

  gpu::DescriptorSetLayout samplers = nullptr;

  gpu::DescriptorSetLayout sampled_textures = nullptr;

  gpu::DescriptorSetLayout storage_textures = nullptr;

  gpu::DescriptorSetLayout const_buffer = nullptr;

  gpu::DescriptorSetLayout read_struct_buffer = nullptr;

  gpu::DescriptorSetLayout read_write_struct_buffer = nullptr;

  gpu::DescriptorSetLayout const_buffers = nullptr;

  gpu::DescriptorSetLayout read_struct_buffers = nullptr;

  gpu::DescriptorSetLayout read_write_struct_buffers = nullptr;

  gpu::DescriptorSetLayout input_attachments = nullptr;

  // u32 num_samplers,
  // u32 num_sampled_textures

  void uninit(gpu::Device device);
};

struct GpuDescriptors
{
  gpu::DescriptorSet samplers = nullptr;

  gpu::DescriptorSet sampled_textures = nullptr;

  void uninit(gpu::Device device);

  static GpuDescriptors create(GpuDescriptorsLayout const & layout);
};

using GpuFrameTask = Dyn<Fn<void()>>;

struct GpuFrameTargetInfo
{
  u32x2 extent = {0, 0};

  gpu::Format color_format = gpu::Format::Undefined;

  gpu::Format depth_stencil_format = gpu::Format::Undefined;

  constexpr bool operator==(GpuFrameTargetInfo const & rhs) const
  {
    return obj::byte_eq(*this, rhs);
  }
};

using GpuFrameTarget = ColorTexture;

/// @brief A prepared frame ready to be executed on the render thread
struct GpuFramePlan
{
  Allocator allocator_;

  Vec<GpuFrameTask> pre_frame_tasks_;

  Vec<GpuFrameTask> post_frame_tasks_;

  Vec<GpuFrameTask> frame_completed_tasks_;

  Vec<u8> gpu_buffer_data_;

  Vec<Slice64> gpu_buffer_entries_;

  Vec<u8> cpu_buffer_data_;

  Vec<Slice64> cpu_buffer_entries_;

  u64 scratch_buffer_reserve_size_;

  Vec<GpuPass> passes_;

  GpuFrameTargetInfo target_;

  ArenaPool arena_;

  GpuFramePlan(Allocator allocator) :
    allocator_{allocator},
    pre_frame_tasks_{allocator},
    post_frame_tasks_{allocator},
    frame_completed_tasks_{allocator},
    gpu_buffer_data_{allocator},
    gpu_buffer_entries_{allocator},
    cpu_buffer_data_{allocator},
    cpu_buffer_entries_{allocator},
    scratch_buffer_reserve_size_{0},
    passes_{allocator},
    target_{},
    arena_{allocator}
  {
  }

  // [ ]
  void set_target(GpuFrameTargetInfo target);

  void reserve_scratch_buffer(u64 size);

  void add_preframe_task(GpuFrameTask && task);

  template <Callable Lambda>
  void add_preframe_task(Lambda && task);

  void add_postframe_task(GpuFrameTask && task);

  template <Callable Lambda>
  void add_postframe_task(Lambda && task);

  void add_pass(GpuPass && pass);

  template <Callable<FrameGraph &, gpu::CommandEncoder &> Lambda>
  void add_pass(Lambda && task)
  {
    auto      lambda = dyn(arena_, static_cast<Lambda &&>(task)).unwrap();
    GpuPassFn func{lambda.get()};
    return add_pass(transmute(std::move(lambda), func));
  }

  BufferId push_cpu(Span<u8 const> data);

  template <typename T>
  BufferId push_cpu(Span<T> data)
  {
    return push_cpu(data.as_u8().as_const());
  }

  GpuBufferId push_gpu(Span<u8 const> data);

  template <typename T>
  GpuBufferId push_gpu(Span<T> data)
  {
    return push_gpu(data.as_u8().as_const());
  }

  void begin();

  void end();

  void reset();
};

struct TextureUnion
{
  ColorTexture        color;
  DepthStencilTexture depth_stencil;

  void uninit(gpu::Device device);

  static TextureUnion create(gpu::Device device, GpuSystem const & sys,
                             u32x2 target_size, gpu::Format color_format,
                             gpu::Format depth_stencil_format, Str label,
                             Allocator scratch);
};

struct ScratchTextures
{
  Vec<TextureUnion> textures;
  gpu::MemoryGroup  memory_group = nullptr;

  void uninit(gpu::Device device);

  static ScratchTextures create(gpu::Device device, GpuSystem const & system,
                                u32 num_scratch, u32x2 target_size,
                                gpu::Format color_format,
                                gpu::Format depth_stencil_format, Str label,
                                Allocator allocator, Allocator scratch);
};

struct GpuFrameResources
{
  GpuBuffer       buffer           = {};
  GpuFrameTarget  target           = {};
  ScratchTextures scratch_textures = {};
  GpuBuffer       scratch_buffer   = {};
  GpuQueries      queries          = {};

  void uninit(gpu::Device device);
};

// [ ] relax limits and raise errors instead
// [ ] target extent: get swapchain extent at frame begin??? no, rebuild target and scratch if extent and formats not same
//
// [ ] single scratch buffer with bounds; min max;------- won't work with ping-ponging
//
// [ ] await_frame_complete()
// [ ] execute_frame_complete_tasks() -- need to be scheduled
//
// [ ] we want to be able to start recording CPU-side work for this frame whilst the GPU work is still executing-----------------
// [ ] handle the case where swapchain might be deferred or not have images
// [ ] how and when to sync texture extents or formats; is there a general concept to mutable state gotten from the execution?
//  ----------- post-complete task; Option<Diff> ; i.e. Option<u32x2> new_swapchain_extent;
//
//
// [ ] we should sync and update texture configurations when swapchain config changes

/// @brief GpuFrame represents the state needed to render a frame on the GPU.
/// This object is prepared on the main/scheduling thread and submitted to the GPU on a render thread.
/// In order to ensure maximum execution overlap, we defer as much work as possible to the render thread.
/// We thus have copies of some of the resources across the frames.
/// The swapchain is also recreated and acquired on the render thread. The primary color target might
/// thus, be out-of-sync with the swapchain extent, in that case, we blit from the target to the swapchain image.
struct GpuFrame
{
  static constexpr u32 DEFAULT_NUM_SCRATCH_TEXTURES = 4;

  Allocator allocator_;

  gpu::Device device_;

  GpuSystem * system_;

  u32 id_;

  GpuFrameState state_;

  gpu::Swapchain swapchain_;

  GpuFrameTargetInfo target_info_;

  GpuFrameResources resources_;

  u32 next_scratch_texture_;

  Semaphore semaphore_;

  gpu::QueueScope queue_scope_;

  gpu::CommandEncoder command_encoder_;

  gpu::CommandBuffer command_buffer_;

  // [ ] needed for methods that need to get context info; i.e. buffer, textuyres
  GpuFramePlan * current_plan_;

  GpuFrame(Allocator allocator, gpu::Device device, GpuSystem * system, u32 id,
           gpu::Swapchain swapchain, Semaphore semaphore,
           gpu::QueueScope queue_scope, gpu::CommandEncoder command_encoder,
           gpu::CommandBuffer command_buffer) :
    allocator_{allocator},
    device_{device},
    system_{system},
    id_{id},
    state_{GpuFrameState::Reset},
    swapchain_{swapchain},
    target_info_{},
    resources_{},
    next_scratch_texture_{0},
    semaphore_{semaphore},
    queue_scope_{queue_scope},
    command_encoder_{command_encoder},
    command_buffer_{command_buffer},
    current_plan_{nullptr}
  {
  }

  void uninit();

  Option<Tuple<gpu::TimestampQuery, u32>> allocate_timestamp();

  Option<Tuple<gpu::StatisticsQuery, u32>> allocate_statistics();

  void get_scratch_textures(u32 num_scratch, Vec<TextureUnion> & textures);

  gpu::Device device() const;

  GpuSystem * system() const;

  gpu::Swapchain swapchain() const;

  ColorTexture target() const;

  gpu::DescriptorSet sampled_textures() const;

  gpu::DescriptorSet samplers() const;

  gpu::CommandEncoder command_encoder() const;

  gpu::CommandBuffer command_buffer() const;

  GpuBufferSpan get(GpuBufferId id);

  Span<u8 const> get(BufferId id);

  template <typename T>
  Span<T const> get(BufferId id)
  {
    return get(id).reinterpret<T>();
  }

  void begin_(GpuFramePlan * plan);

  void end_();

  void execute_();

  void complete_();

  void reset_();

  void await_();
};

struct GpuSystem
{
  static constexpr u32 NUM_TEXTURE_SLOTS = 2'048;

  static constexpr u32 NUM_SAMPLER_SLOTS = 128;

  gpu::Device * device_;

  Vec<GpuFrame, 4> frame_data_;

  u32 frame_ring_index_;

  gpu::DeviceProperties props_;

  gpu::PipelineCache pipeline_cache_;

  u32 buffering_;

  gpu::SampleCount sample_count_;

  /// @brief hdr if hdr supported and required.
  gpu::Format color_format_;

  gpu::Format depth_stencil_format_;

  GpuDescriptorsLayout layouts_;

  GpuDescriptors descriptors_;

  SamplerDict sampler_dict_;

  gpu::Image default_image_;

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views_;

  // [ ] managing laRGE number of textures, samplers; sync

  static GpuSystem create(Allocator allocator, gpu::Device device,
                          Span<u8 const> pipeline_cache_data, bool use_hdr,
                          u32 buffering, gpu::SampleCount sample_count,
                          u32x2 initial_extent);

  GpuSystem(Allocator allocator, gpu::Device & device,
            gpu::DeviceProperties props, gpu::PipelineCache pipeline_cache,
            u32 buffering, gpu::SampleCount sample_count,
            gpu::Format color_format, gpu::Format depth_stencil_format,
            GpuDescriptorsLayout descriptors_layout, gpu::Image default_image,
            Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views,
            FrameGraph                                  frame_graph) :
    device_{&device},
    props_{props},
    pipeline_cache_{pipeline_cache},
    buffering_{buffering},
    sample_count_{sample_count},
    color_format_{color_format},
    depth_stencil_format_{depth_stencil_format},
    descriptors_layout_{descriptors_layout},
    sampler_cache_{allocator},
    default_image_{default_image},
    default_image_views_{default_image_views},
    texture_slots_{},
    sampler_slots_{},
    frame_graph_{std::move(frame_graph)}
  {
  }

  GpuSystem(GpuSystem const &)             = delete;
  GpuSystem(GpuSystem &&)                  = default;
  GpuSystem & operator=(GpuSystem const &) = delete;
  GpuSystem & operator=(GpuSystem &&)      = default;
  ~GpuSystem()                             = default;

  void shutdown(Vec<u8> & cache);

  Sampler create_sampler(gpu::SamplerInfo const & info);

  TextureId alloc_texture_id(gpu::ImageView view);

  void release_texture_id(TextureId id);

  SamplerId alloc_sampler_id(gpu::Sampler sampler);

  void release_sampler_id(SamplerId id);

  GpuFrame & frame();
};

}    // namespace ash
