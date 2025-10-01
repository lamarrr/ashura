/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/shaders/items.gen.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/allocators.h"
#include "ashura/std/async.h"
#include "ashura/std/dict.h"
#include "ashura/std/option.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

typedef struct IGpuFramePlan *                  GpuFramePlan;
typedef struct IGpuFrame *                      GpuFrame;
typedef Fn<void(GpuFrame, gpu::CommandEncoder)> GpuPassFn;
typedef Dyn<GpuPassFn>                          GpuPass;
typedef Fn<void()>                              GpuFrameTaskFn;
typedef Dyn<GpuFrameTaskFn>                     GpuFrameTask;
typedef struct IGpuSys *                        GpuSys;

inline constexpr u32 NUM_DEFAULT_TEXTURES = 16;

inline constexpr u32 NUM_DEFAULT_SAMPLERS = 60;

enum class [[nodiscard]] GpuBufferId : u32
{
};

enum class [[nodiscard]] CpuBufferId : u32
{
};

typedef Dict<gpu::SamplerInfo, Tuple<SamplerIndex, gpu::Sampler>, BitHash,
             BitEq, u32>
  SamplerCache;

/// @brief Created with sampled, storage, color attachment, and transfer flags
struct [[nodiscard]] ColorImage
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

  gpu::DescriptorSet sampled_textures = nullptr;

  gpu::DescriptorSet storage_textures = nullptr;

  gpu::DescriptorSet input_attachments = nullptr;

  static constexpr TextureIndex sampled_texture_index = TextureIndex::Default;

  static constexpr TextureIndex storage_texture_index = TextureIndex::Default;

  static constexpr TextureIndex input_attachment_index = TextureIndex::Default;

  u32x3 extent() const;

  void uninit(gpu::Device device);
};

/// @brief Created with color attachment flag
struct [[nodiscard]] ColorMsaaImage
{
  gpu::ImageInfo info = {};

  gpu::ImageViewInfo view_info = {};

  gpu::Image image = nullptr;

  /// @brief To preserve bandwidth (especially for tiled architectures), preferably
  /// use `StoreOp::DontCare` and `LoadOp::Clear/LoadOp::DontCare` in the render passes.
  gpu::ImageView view = nullptr;

  gpu::SampleCount sample_count() const;

  u32x3 extent() const;

  void uninit(gpu::Device device);
};

struct [[nodiscard]] DepthStencilImage
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

  gpu::DescriptorSet depth_sampled_textures = nullptr;

  gpu::DescriptorSet depth_storage_textures = nullptr;

  gpu::DescriptorSet depth_input_attachments = nullptr;

  static constexpr TextureIndex depth_sampled_texture_index =
    TextureIndex::Default;

  static constexpr TextureIndex depth_storage_texture_index =
    TextureIndex::Default;

  static constexpr TextureIndex depth_input_attachment_index =
    TextureIndex::Default;

  u32x3 extent() const;

  void uninit(gpu::Device device);
};

struct [[nodiscard]] Framebuffer
{
  /// @brief Color image
  ColorImage color = {};

  Option<ColorMsaaImage> color_msaa = none;

  /// @brief Combined depth and stencil aspect attachment
  Option<DepthStencilImage> depth_stencil = none;

  u32x3 extent() const;

  void uninit(gpu::Device device);
};

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

  gpu::DescriptorSet uniform_buffer = nullptr;

  gpu::DescriptorSet read_storage_buffer = nullptr;

  gpu::DescriptorSet read_write_storage_buffer = nullptr;

  void uninit(gpu::Device device);

  static GpuBuffer create(GpuSys sys, u64 capacity, gpu::BufferUsage usage,
                          Str label, Allocator scratch);
};

struct [[nodiscard]] GpuBufferSpan
{
  GpuBuffer buffer = {};
  Slice64   slice  = {};
};

struct GpuQueries
{
  gpu::TimestampQuery timestamps = nullptr;

  gpu::StatisticsQuery statistics = nullptr;

  Vec<u64> cpu_timestamps;

  Vec<gpu::PipelineStatistics> cpu_statistics;

  void uninit(gpu::Device device);

  u32 timestamps_capacity() const;

  u32 statistics_capacity() const;

  static GpuQueries create(Allocator allocator, gpu::Device device,
                           Span<char const> label, u32 timestamps_capacity,
                           u32 statistics_capacity, Allocator scratch);
};

struct GpuFrameCfg
{
  u64 min_buffer_size         = 5_MB;
  u64 max_buffer_size         = 4_GB;
  u64 min_scratch_buffer_size = 5_MB;
  u64 max_scratch_buffer_size = 1_GB;
  u32 max_scratch_buffers     = 4;
  u32 min_scratch_images      = 3;
  u32 max_scratch_images      = 5;
};

struct GpuSysCfg
{
  u32 bindless_samplers_capacity                   = 1'024;
  u32 bindless_sampled_textures_capacity           = 8'192;
  u32 bindless_storage_textures_capacity           = 8'192;
  u32 bindless_uniform_texel_buffers_capacity      = 1'024;
  u32 bindless_storage_texel_buffers_capacity      = 1'024;
  u32 bindless_uniform_buffers_capacity            = 1'024;
  u32 bindless_read_storage_buffers_capacity       = 1'024;
  u32 bindless_read_write_storage_buffers_capacity = 1'024;
  u32 bindless_input_attachments_capacity          = 1'024;
  u32 frame_timestamps_capacity                    = 8'192;
  u32 frame_statistics_capacity                    = 4'096;
};

struct GpuSysPreferences
{
  u32                            buffering               = 4;
  u32x2                          initial_extent          = {1'920, 1'080};
  GpuSysCfg                      cfg                     = {};
  Span<gpu::Format const>        color_formats           = {};
  Span<gpu::Format const>        depth_stencil_formats   = {};
  Span<gpu::SurfaceFormat const> swapchain_formats       = {};
  Span<gpu::PresentMode const>   swapchain_present_modes = {};
  gpu::CompositeAlpha swapchain_composite_alpha = gpu::CompositeAlpha::Opaque;
};

struct GpuDescriptorsLayout
{
  /// @brief Bindless samplers
  gpu::DescriptorSetLayout samplers = nullptr;

  u32 samplers_capacity = 0;

  /// @brief Bindless textures
  gpu::DescriptorSetLayout sampled_textures = nullptr;

  u32 sampled_textures_capacity = 0;

  /// @brief Bindless storage textures
  gpu::DescriptorSetLayout storage_textures = nullptr;

  u32 storage_textures_capacity = 0;

  gpu::DescriptorSetLayout uniform_texel_buffers = nullptr;

  u32 uniform_texel_buffers_capacity = 0;

  gpu::DescriptorSetLayout storage_texel_buffers = nullptr;

  u32 storage_texel_buffers_capacity = 0;

  /// @brief Single dynamic-offset constant buffer
  gpu::DescriptorSetLayout uniform_buffer = nullptr;

  /// @brief Single dynamic-offset storage buffer
  gpu::DescriptorSetLayout read_storage_buffer = nullptr;

  /// @brief Single dynamic-offset storage buffer
  gpu::DescriptorSetLayout read_write_storage_buffer = nullptr;

  /// @brief Bindless static offset uniform buffers
  gpu::DescriptorSetLayout uniform_buffers = nullptr;

  u32 uniform_buffer_capacity = 0;

  /// @brief Bindless static offset storage buffers
  gpu::DescriptorSetLayout read_storage_buffers = nullptr;

  u32 read_storage_buffers_capacity = 0;

  /// @brief Bindless static offset writable storage buffers
  gpu::DescriptorSetLayout read_write_storage_buffers = nullptr;

  u32 read_write_storage_buffers_capacity = 0;

  /// @brief Bindless input attachments
  gpu::DescriptorSetLayout input_attachments = nullptr;

  u32 input_attachments_capacity = 0;

  void uninit(gpu::Device device);

  static GpuDescriptorsLayout create(gpu::Device device, Str label,
                                     GpuSysCfg const & cfg, Allocator scratch);
};

struct GpuDescriptors
{
  gpu::DescriptorSet samplers = nullptr;

  BitVec<u64> samplers_slots = {};

  gpu::DescriptorSet sampled_textures = nullptr;

  BitVec<u64> sampled_textures_slots = {};

  void uninit(gpu::Device device);

  static GpuDescriptors create(GpuSys sys, Str label, Allocator scratch);
};

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

enum class GpuFramePlanState : u8
{
  Reset     = 0,
  Recording = 1,
  Recorded  = 2,
  Submitted = 3,
  Executed  = 4
};

/// @brief A prepared frame ready to be executed on the render thread
struct IGpuFramePlan
{
  Allocator allocator_;

  GpuSys sys_;

  Vec<GpuFrameTask> pre_frame_tasks_;

  Vec<GpuFrameTask> post_frame_tasks_;

  Vec<GpuFrameTask> frame_completed_tasks_;

  Vec<u8> gpu_buffer_data_;

  Vec<Slice64> gpu_buffer_entries_;

  Vec<u8> cpu_buffer_data_;

  Vec<Slice64> cpu_buffer_entries_;

  Vec<u64> scratch_buffer_sizes_;

  u32 num_scratch_images_;

  Vec<GpuPass> passes_;

  GpuFrameTargetInfo target_;

  GpuFramePlanState state_;

  Dyn<Semaphore> semaphore_;

  u64 submission_stage_;

  ArenaPool arena_;

  IGpuFramePlan(Allocator allocator, GpuSys sys, Dyn<Semaphore> semaphore) :
    allocator_{allocator},
    sys_{sys},
    pre_frame_tasks_{allocator},
    post_frame_tasks_{allocator},
    frame_completed_tasks_{allocator},
    gpu_buffer_data_{allocator},
    gpu_buffer_entries_{allocator},
    cpu_buffer_data_{allocator},
    cpu_buffer_entries_{allocator},
    scratch_buffer_sizes_{allocator},
    num_scratch_images_{0},
    passes_{allocator},
    target_{},
    state_{GpuFramePlanState::Reset},
    semaphore_{std::move(semaphore)},
    submission_stage_{0},
    arena_{allocator}
  {
  }

  void uninit();

  void set_target(GpuFrameTargetInfo target);

  void reserve_scratch_buffers(Span<u64 const> sizes);

  void reserve_scratch_images(u32 num_scratch_images);

  void add_preframe_task(GpuFrameTask && task);

  template <Callable Lambda>
  void add_preframe_task(Lambda && task)
  {
    return add_preframe_task(
      dyn_lambda<GpuFrameTaskFn>(arena_, static_cast<Lambda &&>(task))
        .unwrap());
  }

  void add_postframe_task(GpuFrameTask && task);

  template <Callable Lambda>
  void add_postframe_task(Lambda && task)
  {
    return add_postframe_task(
      dyn_lambda<GpuFrameTaskFn>(arena_, static_cast<Lambda &&>(task))
        .unwrap());
  }

  void add_pass(GpuPass && pass);

  template <Callable<GpuFrame, gpu::CommandEncoder> Lambda>
  void add_pass(Lambda && task)
  {
    return add_pass(
      dyn_lambda<GpuPassFn>(arena_, static_cast<Lambda &&>(task)).unwrap());
  }

  CpuBufferId push_cpu(Span<u8 const> data);

  template <typename T>
  CpuBufferId push_cpu(Span<T> data)
  {
    return push_cpu(data.as_u8().as_const());
  }

  GpuBufferId push_gpu(Span<u8 const> data);

  template <typename T>
  GpuBufferId push_gpu(Span<T> data)
  {
    return push_gpu(data.as_u8().as_const());
  }

  GpuSys sys() const;

  gpu::Device device() const;

  void begin();

  void end();

  void reset();

  /// @brief Await completion of the frame
  /// @returns true if the frame completed before the timeout period
  bool await(nanoseconds timeout);
};

struct TexelBufferUnion
{
  static constexpr gpu::BufferUsage USAGE =
    gpu::BufferUsage::TransferSrc | gpu::BufferUsage::TransferDst |
    gpu::BufferUsage::UniformTexelBuffer |
    gpu::BufferUsage::StorageTexelBuffer | gpu::BufferUsage::UniformBuffer |
    gpu::BufferUsage::StorageBuffer | gpu::BufferUsage::IndexBuffer |
    gpu::BufferUsage::VertexBuffer | gpu::BufferUsage::IndirectBuffer;

  struct View
  {
    gpu::BufferView    view                  = nullptr;
    gpu::Format        format                = gpu::Format::Undefined;
    gpu::DescriptorSet uniform_texel_buffers = nullptr;
    gpu::DescriptorSet storage_texel_buffers = nullptr;

    static constexpr TextureIndex uniform_texel_buffer_index =
      TextureIndex::Default;

    static constexpr TextureIndex storage_texel_buffer_index =
      TextureIndex::Default;
  };

  // [ ] The texel will not fit all image types? it'll make the memory usage 4x larger than necessary; i.e.
  // aliased image is R8G8B8A8_UNORM and TEXEL will always have R32G32B32A32_SFLOAT
  static constexpr gpu::Format FORMATS[] = {gpu::Format::R8_UNORM,
                                            gpu::Format::R8_SNORM,
                                            gpu::Format::R8_UINT,
                                            gpu::Format::R8_SINT,
                                            gpu::Format::R8G8B8A8_UNORM,
                                            gpu::Format::R8G8B8A8_SNORM,
                                            gpu::Format::R8G8B8A8_UINT,
                                            gpu::Format::R8G8B8A8_SINT,
                                            gpu::Format::R16_UINT,
                                            gpu::Format::R16_SINT,
                                            gpu::Format::R16_SFLOAT,
                                            gpu::Format::R16G16_UINT,
                                            gpu::Format::R16G16_SINT,
                                            gpu::Format::R16G16_SFLOAT,
                                            gpu::Format::R32_UINT,
                                            gpu::Format::R32_SINT,
                                            gpu::Format::R32_SFLOAT,
                                            gpu::Format::R32G32_UINT,
                                            gpu::Format::R32G32_SINT,
                                            gpu::Format::R32G32_SFLOAT,
                                            gpu::Format::R32G32B32A32_UINT,
                                            gpu::Format::R32G32B32A32_SINT,
                                            gpu::Format::R32G32B32A32_SFLOAT};

  static constexpr u32 NUM_VIEWS = 23;

  gpu::Buffer            buffer = {};
  Array<View, NUM_VIEWS> views  = {};

  View interpret(gpu::Format format) const;

  void uninit(gpu::Device device);

  static TexelBufferUnion create(GpuSys sys, u32x2 target_size, Str label,
                                 Allocator scratch);
};

struct ImageUnion
{
  ColorImage        color         = {};
  DepthStencilImage depth_stencil = {};
  TexelBufferUnion  texel         = {};
  gpu::Alias        alias         = nullptr;

  void uninit(gpu::Device device);

  static ImageUnion create(GpuSys sys, u32x2 target_size,
                           gpu::Format color_format,
                           gpu::Format depth_stencil_format, Str label,
                           Allocator scratch);
};

struct ScratchImages
{
  Vec<ImageUnion> images;

  void uninit(gpu::Device device);

  static ScratchImages create(GpuSys sys, u32 num_scratch, u32x2 target_size,
                              gpu::Format color_format,
                              gpu::Format depth_stencil_format, Str label,
                              Allocator allocator, Allocator scratch);
};

struct ScratchBuffers
{
  Vec<GpuBuffer> buffers;

  void uninit(gpu::Device device);

  static ScratchBuffers create(GpuSys sys, Span<u64 const> sizes, Str label,
                               Allocator allocator, Allocator scratch);

  void grow(GpuSys sys, Span<u64 const> sizes, Str label, Allocator allocator,
            Allocator scratch);
};

struct GpuFrameResources
{
  GpuBuffer      buffer          = {};
  ScratchBuffers scratch_buffers = {};
  ScratchImages  scratch_images  = {};
  GpuQueries     queries         = {};

  void uninit(gpu::Device device);
};

// [ ] handle the case where swapchain might be deferred or not have images
// [ ] we should sync and update texture configurations when swapchain config changes
// [ ] to recreate swapchain, how do we wait correctly on the swapchain image presentation before re-creating?

enum class ScratchTexureType : u8
{
  SampledColor                = 0,
  StorageColor                = 1,
  InputAttachmentColor        = 2,
  SampledDepthStencil         = 3,
  StorageDepthStencil         = 4,
  InputAttachmentDepthStencil = 5
};

struct ScratchTexture
{
  u32               image = 0;
  ScratchTexureType type  = ScratchTexureType::SampledColor;
};

struct SampledTextures
{
};

inline constexpr SampledTextures sampled_textures;

using TextureSet = Enum<ScratchTexture, SampledTextures>;

enum class GpuFrameState : u8
{
  Reset     = 0,
  Recording = 1,
  Recorded  = 2,
  Submitted = 2,
  Completed = 3
};

/// @brief GpuFrame represents the state needed to render a frame on the GPU.
/// This object is prepared on the main/scheduling thread and submitted to the GPU on a render thread.
/// In order to ensure maximum execution overlap, we defer as much work as possible to the render thread.
/// We thus have copies of some of the resources across the frames.
/// The swapchain is also recreated and acquired on the render thread. The primary color target might
/// thus, be out-of-sync with the swapchain extent, in that case, we blit from the target to the swapchain image.
struct IGpuFrame
{
  Allocator allocator_;

  gpu::Device dev_;

  GpuSys sys_;

  u32 id_;

  GpuFrameState state_;

  u64 scope_frame_id_;

  GpuFrameTargetInfo target_info_;

  GpuFrameCfg cfg_;

  GpuFrameResources resources_;

  u32 next_timestamp_;

  u32 next_statistics_;

  Dyn<Semaphore> semaphore_;

  u64 submission_stage_;

  gpu::CommandEncoder command_encoder_;

  gpu::CommandBuffer command_buffer_;

  /// @brief Currently bounded frame plan
  GpuFramePlan current_plan_;

  IGpuFrame(Allocator allocator, gpu::Device device, GpuSys sys, u32 id,
            Dyn<Semaphore> semaphore, gpu::CommandEncoder command_encoder,
            gpu::CommandBuffer command_buffer) :
    allocator_{allocator},
    dev_{device},
    sys_{sys},
    id_{id},
    state_{GpuFrameState::Reset},
    scope_frame_id_{0},
    target_info_{},
    cfg_{},
    resources_{},
    next_timestamp_{0},
    next_statistics_{0},
    semaphore_{std::move(semaphore)},
    submission_stage_{0},
    command_encoder_{command_encoder},
    command_buffer_{command_buffer},
    current_plan_{nullptr}
  {
  }

  void uninit();

  gpu::Device dev() const;

  GpuSys sys() const;

  /// @brief Swapchain images must be manually acquired and blitted to
  gpu::Swapchain swapchain() const;

  gpu::DescriptorSet sampled_textures() const;

  gpu::DescriptorSet samplers() const;

  gpu::CommandEncoder command_encoder() const;

  gpu::CommandBuffer command_buffer() const;

  Option<Tuple<gpu::TimestampQuery, u32>> allocate_timestamp();

  Option<Tuple<gpu::StatisticsQuery, u32>> allocate_statistics();

  Span<ImageUnion const> get_scratch_images() const;

  Span<GpuBuffer const> get_scratch_buffers() const;

  GpuBufferSpan get(GpuBufferId id);

  Span<u8 const> get(CpuBufferId id);

  template <typename T>
  Span<T const> get(CpuBufferId id)
  {
    return get(id).reinterpret<T>();
  }

  gpu::DescriptorSet get(TextureSet tex);

  void begin();

  void cmd(GpuFramePlan plan);

  void end();

  /// @brief Submit frame to the GPU
  void submit();

  /// @brief The frame has finished executing on the GPU, run completion tasks or fetch data from GPU
  /// @return true if the frame has been executed, otherwise, false.
  bool try_complete(nanoseconds timeout);

  /// @brief Reset the frame for recording
  void reset();

  /// @brief Await completion of the frame
  /// @returns true if the frame completed before the timeout period
  bool await(nanoseconds timeout);
};

/// @brief A GpuSys has two primary components: The GpuFramePlan and the GpuFrame.
///
/// The GpuFramePlan describes the operations that need to be executed for a specific frame. It also contains the
/// description of all resources that will need to be allocated/copied to the GPU in order to execute the frame.
///
///
/// The GpuFrame contains the GPU resources that are needed to actually execute the frame.
///
/// The GpuFramePlan can be continously prepared whilst the frames are executing on the render thread so the GPU is never
/// starved of work. This also means we can submit frames as soon as they are ready and immediately start doing other work
/// without any waits on the CPU.
///
/// To keep the queue saturated, we'd ideally want to have atleast N + 1 FramePlans and N+1 Frames ???????????
///
struct IGpuSys
{
  static constexpr u32 MAX_BUFFERING = 4;

  bool initialized_;

  Allocator allocator_;

  gpu::Device dev_;

  gpu::Surface surface_;

  GpuSysCfg cfg_;

  gpu::DeviceProperties props_;

  gpu::PipelineCache pipeline_cache_;

  u32 buffering_;

  /// @brief Hdr if hdr supported and required.
  gpu::Format color_format_;

  gpu::Format depth_stencil_format_;

  gpu::SampleCount sample_count_;

  GpuDescriptorsLayout descriptors_layout_;

  gpu::Swapchain swapchain_;

  gpu::QueueScope queue_scope_;

  IFutex resources_lock_;

  SamplerCache sampler_cache_;

  GpuDescriptors descriptors_;

  gpu::Image default_image_;

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views_;

  u32 frame_ring_index_;

  Vec<Dyn<GpuFrame>> frames_;

  Vec<Dyn<GpuFramePlan>> plans_;

  Scheduler scheduler_;

  ThreadId thread_id_;

  IGpuSys() :
    initialized_{false},
    allocator_{},
    dev_{nullptr},
    surface_{nullptr},
    cfg_{},
    props_{},
    pipeline_cache_{nullptr},
    buffering_{0},
    color_format_{gpu::Format::Undefined},
    depth_stencil_format_{gpu::Format::Undefined},
    descriptors_layout_{},
    swapchain_{nullptr},
    queue_scope_{nullptr},
    resources_lock_{},
    sampler_cache_{},
    descriptors_{},
    default_image_{nullptr},
    default_image_views_{},
    frame_ring_index_{0},
    frames_{},
    plans_{},
    scheduler_{nullptr},
    thread_id_{ThreadId::Undefined}
  {
  }

  IGpuSys(IGpuSys const &)             = delete;
  IGpuSys(IGpuSys &&)                  = delete;
  IGpuSys & operator=(IGpuSys const &) = delete;
  IGpuSys & operator=(IGpuSys &&)      = delete;
  ~IGpuSys()                           = default;

  /// @brief Uninitialize the gpu system and get the pipeline cache data
  void uninit(Vec<u8> & cache);

  /// @brief Initialize the GPU system with the provided pipeline cache data and preference
  ///
  void init(Allocator allocator, gpu::Device device,
            Span<u8 const> pipeline_cache_data, gpu::Surface surface,
            GpuSysPreferences const & preferences, Scheduler scheduler,
            ThreadId thread_id);

  SamplerIndex create_cached_sampler(gpu::SamplerInfo const & info);

  /// @brief Allocate a texture slot for the image and bind it to the textures descriptors
  /// at the start of the next frame
  TextureIndex alloc_texture_index(gpu::ImageView view);

  /// @brief Release a texture slot and unbind it from the textures descriptors at the start
  /// of the next frame
  void release_texture_index(TextureIndex index);

  gpu::Device device();

  Allocator allocator() const;

  GpuFramePlan plan();

  gpu::Format color_format() const;

  gpu::Format depth_stencil_format() const;

  gpu::SampleCount sample_count() const;

  gpu::PipelineCache pipeline_cache() const;

  GpuDescriptorsLayout const & descriptors_layout() const;

  gpu::DescriptorSet samplers() const;

  gpu::DescriptorSet sampled_textures() const;

  void submit_frame();
};

}    // namespace ash
