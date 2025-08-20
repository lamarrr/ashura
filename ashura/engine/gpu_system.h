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

typedef struct IGpuFramePlan * GpuFramePlan;
typedef struct IGpuFrame *     GpuFrame;
typedef Fn<void(GpuFrame)>     GpuPassFn;
typedef Dyn<GpuPassFn>         GpuPass;
typedef Fn<void()>             GpuFrameTaskFn;
typedef Dyn<GpuFrameTaskFn>    GpuFrameTask;
typedef struct IGpuSys *       GpuSys;

// do not change the underlying type. It maps directly to the GPU handle
enum class [[nodiscard]] TextureId : u32
{
  Base = 0,

  /// @brief Nominal Texture Ids for the GPU system
  Transparent        = 0,     // [0, 0, 0, 0]
  RedTransparent     = 1,     // [1, 0, 0, 0]
  GreenTransparent   = 2,     // [0, 1, 0, 0]
  BlueTransparent    = 3,     // [0, 0, 1, 0]
  YellowTransparent  = 4,     // [1, 1, 0, 0]
  MagentaTransparent = 5,     // [1, 0, 1, 0]
  CyanTransparent    = 6,     // [0, 1, 1, 0]
  WhiteTransparent   = 7,     // [1, 1, 1, 0]
  Black              = 8,     // [0, 0, 0, 1]
  Red                = 9,     // [1, 0, 0, 1]
  Green              = 10,    // [0, 1, 0, 1]
  Blue               = 11,    // [0, 0, 1, 1]
  Yellow             = 12,    // [1, 1, 0, 1]
  Magenta            = 13,    // [1, 0, 1, 1]
  Cyan               = 14,    // [0, 1, 1, 1]
  White              = 15     // [1, 1, 1, 1]
};

inline constexpr u32 NUM_DEFAULT_TEXTURES = 16;

// do not change the underlying type. It maps directly to the GPU handle
enum class [[nodiscard]] SamplerId : u32
{
  LinearRepeatTransparentFloat           = 0,
  LinearRepeatTransparentInt             = 1,
  LinearRepeatBlackFloat                 = 2,
  LinearRepeatBlackInt                   = 3,
  LinearRepeatWhiteFloat                 = 4,
  LinearRepeatWhiteInt                   = 5,
  LinearMirroredRepeatTransparentFloat   = 6,
  LinearMirroredRepeatTransparentInt     = 7,
  LinearMirroredRepeatBlackFloat         = 8,
  LinearMirroredRepeatBlackInt           = 9,
  LinearMirroredRepeatWhiteFloat         = 10,
  LinearMirroredRepeatWhiteInt           = 11,
  LinearEdgeClampTransparentFloat        = 12,
  LinearEdgeClampTransparentInt          = 13,
  LinearEdgeClampBlackFloat              = 14,
  LinearEdgeClampBlackInt                = 15,
  LinearEdgeClampWhiteFloat              = 16,
  LinearEdgeClampWhiteInt                = 17,
  LinearBorderClampTransparentFloat      = 18,
  LinearBorderClampTransparentInt        = 19,
  LinearBorderClampBlackFloat            = 20,
  LinearBorderClampBlackInt              = 21,
  LinearBorderClampWhiteFloat            = 22,
  LinearBorderClampWhiteInt              = 23,
  LinearMirrorEdgeClampTransparentFloat  = 24,
  LinearMirrorEdgeClampTransparentInt    = 25,
  LinearMirrorEdgeClampBlackFloat        = 26,
  LinearMirrorEdgeClampBlackInt          = 27,
  LinearMirrorEdgeClampWhiteFloat        = 28,
  LinearMirrorEdgeClampWhiteInt          = 29,
  NearestRepeatTransparentFloat          = 30,
  NearestRepeatTransparentInt            = 31,
  NearestRepeatBlackFloat                = 32,
  NearestRepeatBlackInt                  = 33,
  NearestRepeatWhiteFloat                = 34,
  NearestRepeatWhiteInt                  = 35,
  NearestMirroredRepeatTransparentFloat  = 36,
  NearestMirroredRepeatTransparentInt    = 37,
  NearestMirroredRepeatBlackFloat        = 38,
  NearestMirroredRepeatBlackInt          = 39,
  NearestMirroredRepeatWhiteFloat        = 40,
  NearestMirroredRepeatWhiteInt          = 41,
  NearestEdgeClampTransparentFloat       = 42,
  NearestEdgeClampTransparentInt         = 43,
  NearestEdgeClampBlackFloat             = 44,
  NearestEdgeClampBlackInt               = 45,
  NearestEdgeClampWhiteFloat             = 46,
  NearestEdgeClampWhiteInt               = 47,
  NearestBorderClampTransparentFloat     = 48,
  NearestBorderClampTransparentInt       = 49,
  NearestBorderClampBlackFloat           = 50,
  NearestBorderClampBlackInt             = 51,
  NearestBorderClampWhiteFloat           = 52,
  NearestBorderClampWhiteInt             = 53,
  NearestMirrorEdgeClampTransparentFloat = 54,
  NearestMirrorEdgeClampTransparentInt   = 55,
  NearestMirrorEdgeClampBlackFloat       = 56,
  NearestMirrorEdgeClampBlackInt         = 57,
  NearestMirrorEdgeClampWhiteFloat       = 58,
  NearestMirrorEdgeClampWhiteInt         = 59
};

inline constexpr u32 NUM_DEFAULT_SAMPLERS = 60;

enum class [[nodiscard]] GpuBufferId : u32
{
};

enum class [[nodiscard]] BufferId : u32
{
};

typedef Dict<gpu::SamplerInfo, Tuple<SamplerId, gpu::Sampler>, BitHash, BitEq,
             u32>
  SamplerCache;

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

  u32x3 extent() const;

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

  gpu::SampleCount sample_count() const;

  u32x3 extent() const;

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

  u32x3 extent() const;

  void uninit(gpu::Device device);
};

struct [[nodiscard]] Framebuffer
{
  /// @brief color texture
  ColorTexture color = {};

  Option<ColorMsaaTexture> color_msaa = none;

  /// @brief combined depth and stencil aspect attachment
  DepthStencilTexture depth_stencil = {};

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
  u32 min_scratch_textures    = 2;
  u32 max_scratch_textures    = 4;
};

struct GpuSysCfg
{
  u32 bindless_samplers_capacity                   = 1'024;
  u32 bindless_sampled_textures_capacity           = 8'192;
  u32 bindless_storage_textures_capacity           = 8'192;
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
  /// @brief bindless samplers
  gpu::DescriptorSetLayout samplers = nullptr;

  u32 samplers_capacity = 0;

  /// @brief bindless textures
  gpu::DescriptorSetLayout sampled_textures = nullptr;

  u32 sampled_textures_capacity = 0;

  /// @brief bindless storage textures
  gpu::DescriptorSetLayout storage_textures = nullptr;

  u32 storage_textures_capacity = 0;

  /// @brief single dynamic-offset constant buffer
  gpu::DescriptorSetLayout uniform_buffer = nullptr;

  /// @brief single dynamic-offset storage buffer
  gpu::DescriptorSetLayout read_storage_buffer = nullptr;

  /// @brief single dynamic-offset storage buffer
  gpu::DescriptorSetLayout read_write_storage_buffer = nullptr;

  /// @brief bindless static offset uniform buffers
  gpu::DescriptorSetLayout uniform_buffers = nullptr;

  u32 uniform_buffer_capacity = 0;

  /// @brief bindless static offset storage buffers
  gpu::DescriptorSetLayout read_storage_buffers = nullptr;

  u32 read_storage_buffers_capacity = 0;

  /// @brief bindless static offset writable storage buffers
  gpu::DescriptorSetLayout read_write_storage_buffers = nullptr;

  u32 read_write_storage_buffers_capacity = 0;

  /// @brief bindless input attachments
  gpu::DescriptorSetLayout input_attachments = nullptr;

  u32 input_attachments_capacity = 0;

  void uninit(gpu::Device device);

  static GpuDescriptorsLayout create(gpu::Device device, Str label,
                                     GpuSysCfg const & cfg, Allocator scratch);
};

struct GpuDescriptors
{
  gpu::DescriptorSet samplers = nullptr;

  u32 samplers_capacity = 0;

  SparseVec<> samplers_map = {};

  gpu::DescriptorSet sampled_textures = nullptr;

  u32 sampled_textures_capacity = 0;

  SparseVec<> sampled_textures_map = {};

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

using GpuFrameTarget = ColorTexture;

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

  Vec<GpuFrameTask> pre_frame_tasks_;

  Vec<GpuFrameTask> post_frame_tasks_;

  Vec<GpuFrameTask> frame_completed_tasks_;

  Vec<u8> gpu_buffer_data_;

  Vec<Slice64> gpu_buffer_entries_;

  Vec<u8> cpu_buffer_data_;

  Vec<Slice64> cpu_buffer_entries_;

  Vec<u64> scratch_buffer_sizes_;

  u32 num_scratch_textures_;

  Vec<GpuPass> passes_;

  GpuFrameTargetInfo target_;

  GpuFramePlanState state_;

  Dyn<Semaphore> semaphore_;

  u64 submission_stage_;

  ArenaPool arena_;

  IGpuFramePlan(Allocator allocator, Dyn<Semaphore> semaphore) :
    allocator_{allocator},
    pre_frame_tasks_{allocator},
    post_frame_tasks_{allocator},
    frame_completed_tasks_{allocator},
    gpu_buffer_data_{allocator},
    gpu_buffer_entries_{allocator},
    cpu_buffer_data_{allocator},
    cpu_buffer_entries_{allocator},
    scratch_buffer_sizes_{allocator},
    num_scratch_textures_{0},
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

  void reserve_scratch_textures(u32 num_scratch_textures);

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

  template <Callable<GpuFrame &, gpu::CommandEncoder> Lambda>
  void add_pass(Lambda && task)
  {
    return add_pass(
      dyn_lambda<GpuPassFn>(arena_, static_cast<Lambda &&>(task)).unwrap());
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

  /// @brief await completion of the frame
  /// @returns true if the frame completed before the timeout period
  bool await(nanoseconds timeout);
};

struct TextureUnion
{
  ColorTexture        color         = {};
  DepthStencilTexture depth_stencil = {};
  gpu::Alias          alias         = nullptr;

  void uninit(gpu::Device device);

  static TextureUnion create(GpuSys sys, u32x2 target_size,
                             gpu::Format color_format,
                             gpu::Format depth_stencil_format, Str label,
                             Allocator scratch);
};

struct ScratchTextures
{
  Vec<TextureUnion> textures;

  void uninit(gpu::Device device);

  static ScratchTextures create(GpuSys sys, u32 num_scratch, u32x2 target_size,
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
  GpuBuffer       buffer           = {};
  GpuFrameTarget  target           = {};
  ScratchBuffers  scratch_buffers  = {};
  ScratchTextures scratch_textures = {};
  GpuQueries      queries          = {};

  void uninit(gpu::Device device);
};

// [ ] we want to be able to start recording CPU-side work for this frame whilst the GPU work is still executing-----------------
// [ ] handle the case where swapchain might be deferred or not have images
// [ ] we should sync and update texture configurations when swapchain config changes

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

  u32 next_scratch_texture_;

  u32 next_timestamp_;

  u32 next_statistics_;

  Dyn<Semaphore> semaphore_;

  u64 submission_stage_;

  gpu::CommandEncoder command_encoder_;

  gpu::CommandBuffer command_buffer_;

  /// @brief currently bounded frame plan
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
    next_scratch_texture_{0},
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

  /// @brief swapchain images must be manually acquired and blitted to
  gpu::Swapchain swapchain() const;

  ColorTexture target() const;

  gpu::DescriptorSet sampled_textures() const;

  gpu::DescriptorSet samplers() const;

  gpu::CommandEncoder command_encoder() const;

  gpu::CommandBuffer command_buffer() const;

  Option<Tuple<gpu::TimestampQuery, u32>> allocate_timestamp();

  Option<Tuple<gpu::StatisticsQuery, u32>> allocate_statistics();

  void get_scratch_textures(Span<TextureUnion> textures);

  void get_scratch_buffers(Span<GpuBuffer> buffers);

  GpuBufferSpan get(GpuBufferId id);

  Span<u8 const> get(BufferId id);

  template <typename T>
  Span<T const> get(BufferId id)
  {
    return get(id).reinterpret<T>();
  }

  void begin();

  void cmd(GpuFramePlan plan);

  void end();

  /// @brief submit frame to the GPU
  void submit();

  /// @brief the frame has finished executing on the GPU, run completion tasks or fetch data from GPU
  /// @return true if the frame has been executed, otherwise, false.
  bool try_complete(nanoseconds timeout);

  /// @brief reset the frame for recording
  void reset();

  /// @brief await completion of the frame
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
  static constexpr u32 MAX_SWAPCHAIN_BUFFERING = 4;

  bool initialized_;

  Allocator allocator_;

  gpu::Device dev_;

  gpu::Surface surface_;

  GpuSysCfg cfg_;

  gpu::DeviceProperties props_;

  gpu::PipelineCache pipeline_cache_;

  u32 buffering_;

  /// @brief hdr if hdr supported and required.
  gpu::Format color_format_;

  gpu::Format depth_stencil_format_;

  GpuDescriptorsLayout descriptors_layout_;

  gpu::Swapchain swapchain_;

  gpu::QueueScope queue_scope_;

  SpinLock resources_lock_;

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

  /// @brief uninitialize the gpu system and get the pipeline cache data
  void uninit(Vec<u8> & cache);

  /// @brief initialize the GPU system with the provided pipeline cache data and preference
  ///
  void init(Allocator allocator, gpu::Device device,
            Span<u8 const> pipeline_cache_data, gpu::Surface surface,
            GpuSysPreferences const & preferences, Scheduler scheduler,
            ThreadId thread_id);

  SamplerId create_cached_sampler(gpu::SamplerInfo const & info);

  /// @brief allocate a texture slot for the image and bind it to the textures descriptors
  /// at the start of the next frame
  TextureId alloc_texture_id(gpu::ImageView view);

  /// @brief release a texture slot and unbind it from the textures descriptors at the start
  /// of the next frame
  void release_texture_id(TextureId id);

  GpuFramePlan plan();

  void submit_frame();
};

}    // namespace ash
