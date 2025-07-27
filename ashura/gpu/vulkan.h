/// SPDX-License-Identifier: MIT
#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION           1'000'000

#include "ashura/gpu/gpu.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/range.h"
#include "ashura/std/vec.h"
#include "vk_mem_alloc.h"
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

using gpu::Status;

inline constexpr char const * ENGINE_NAME = "Ash";
inline constexpr u32          ENGINE_VERSION =
  VK_MAKE_API_VERSION(ASH_VERSION.variant, ASH_VERSION.major, ASH_VERSION.minor,
                      ASH_VERSION.variant);
inline constexpr char const * CLIENT_NAME    = "Ash Client";
inline constexpr u32          CLIENT_VERSION = ENGINE_VERSION;

inline constexpr u32 MAX_MEMORY_HEAP_PROPERTIES = 32;
inline constexpr u32 MAX_MEMORY_HEAPS           = 16;
inline constexpr u32 NUM_DESCRIPTOR_TYPES       = 11;

typedef VkSampler             Sampler;
typedef VkShaderModule        Shader;
typedef VkPipelineCache       PipelineCache;
typedef VkSurfaceKHR          Surface;
typedef VkQueryPool           TimestampQuery;
typedef VkQueryPool           StatisticsQuery;
typedef struct Instance       Instance;
typedef struct Device         Device;
typedef struct CommandBuffer  CommandBuffer;
typedef struct CommandEncoder CommandEncoder;

#define ASH_DEF_VKPFN(name) PFN_vk##name name = nullptr

struct InstanceTable
{
  ASH_DEF_VKPFN(CreateInstance);
  ASH_DEF_VKPFN(DestroyInstance);
  ASH_DEF_VKPFN(DestroySurfaceKHR);
  ASH_DEF_VKPFN(EnumeratePhysicalDevices);
  ASH_DEF_VKPFN(GetInstanceProcAddr);
  ASH_DEF_VKPFN(GetDeviceProcAddr);

  ASH_DEF_VKPFN(CreateDevice);
  ASH_DEF_VKPFN(EnumerateDeviceExtensionProperties);
  ASH_DEF_VKPFN(EnumerateDeviceLayerProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceFeatures);
  ASH_DEF_VKPFN(GetPhysicalDeviceFormatProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceImageFormatProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceMemoryProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceQueueFamilyProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceSparseImageFormatProperties);

  ASH_DEF_VKPFN(GetPhysicalDeviceSurfaceSupportKHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceSurfaceFormatsKHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceSurfacePresentModesKHR);

  ASH_DEF_VKPFN(CreateDebugUtilsMessengerEXT);
  ASH_DEF_VKPFN(DestroyDebugUtilsMessengerEXT);
  ASH_DEF_VKPFN(SetDebugUtilsObjectNameEXT);
};

struct DeviceTable
{
  // DEVICE OBJECT FUNCTIONS
  ASH_DEF_VKPFN(AllocateCommandBuffers);
  ASH_DEF_VKPFN(AllocateDescriptorSets);
  ASH_DEF_VKPFN(AllocateMemory);
  ASH_DEF_VKPFN(BindBufferMemory);
  ASH_DEF_VKPFN(BindImageMemory);
  ASH_DEF_VKPFN(CreateBuffer);
  ASH_DEF_VKPFN(CreateBufferView);
  ASH_DEF_VKPFN(CreateCommandPool);
  ASH_DEF_VKPFN(CreateComputePipelines);
  ASH_DEF_VKPFN(CreateDescriptorPool);
  ASH_DEF_VKPFN(CreateDescriptorSetLayout);
  ASH_DEF_VKPFN(CreateEvent);
  ASH_DEF_VKPFN(CreateFence);
  ASH_DEF_VKPFN(CreateGraphicsPipelines);
  ASH_DEF_VKPFN(CreateImage);
  ASH_DEF_VKPFN(CreateImageView);
  ASH_DEF_VKPFN(CreatePipelineCache);
  ASH_DEF_VKPFN(CreatePipelineLayout);
  ASH_DEF_VKPFN(CreateQueryPool);
  ASH_DEF_VKPFN(CreateSampler);
  ASH_DEF_VKPFN(CreateSemaphore);
  ASH_DEF_VKPFN(CreateShaderModule);
  ASH_DEF_VKPFN(DestroyBuffer);
  ASH_DEF_VKPFN(DestroyBufferView);
  ASH_DEF_VKPFN(DestroyCommandPool);
  ASH_DEF_VKPFN(DestroyDescriptorPool);
  ASH_DEF_VKPFN(DestroyDescriptorSetLayout);
  ASH_DEF_VKPFN(DestroyDevice);
  ASH_DEF_VKPFN(DestroyEvent);
  ASH_DEF_VKPFN(DestroyFence);
  ASH_DEF_VKPFN(DestroyImage);
  ASH_DEF_VKPFN(DestroyImageView);
  ASH_DEF_VKPFN(DestroyPipeline);
  ASH_DEF_VKPFN(DestroyPipelineCache);
  ASH_DEF_VKPFN(DestroyPipelineLayout);
  ASH_DEF_VKPFN(DestroyQueryPool);
  ASH_DEF_VKPFN(DestroySampler);
  ASH_DEF_VKPFN(DestroySemaphore);
  ASH_DEF_VKPFN(DestroyShaderModule);
  ASH_DEF_VKPFN(DeviceWaitIdle);
  ASH_DEF_VKPFN(FlushMappedMemoryRanges);
  ASH_DEF_VKPFN(FreeCommandBuffers);
  ASH_DEF_VKPFN(FreeDescriptorSets);
  ASH_DEF_VKPFN(FreeMemory);
  ASH_DEF_VKPFN(GetBufferMemoryRequirements);
  ASH_DEF_VKPFN(GetDeviceMemoryCommitment);
  ASH_DEF_VKPFN(GetDeviceQueue);
  ASH_DEF_VKPFN(GetEventStatus);
  ASH_DEF_VKPFN(GetFenceStatus);
  ASH_DEF_VKPFN(GetImageMemoryRequirements);
  ASH_DEF_VKPFN(GetImageSubresourceLayout);
  ASH_DEF_VKPFN(GetPipelineCacheData);
  ASH_DEF_VKPFN(GetQueryPoolResults);
  ASH_DEF_VKPFN(InvalidateMappedMemoryRanges);
  ASH_DEF_VKPFN(MapMemory);
  ASH_DEF_VKPFN(MergePipelineCaches);
  ASH_DEF_VKPFN(ResetCommandPool);
  ASH_DEF_VKPFN(ResetDescriptorPool);
  ASH_DEF_VKPFN(ResetEvent);
  ASH_DEF_VKPFN(ResetFences);
  ASH_DEF_VKPFN(SetEvent);
  ASH_DEF_VKPFN(UpdateDescriptorSets);
  ASH_DEF_VKPFN(UnmapMemory);
  ASH_DEF_VKPFN(WaitForFences);

  ASH_DEF_VKPFN(QueueSubmit);
  ASH_DEF_VKPFN(QueueWaitIdle);

  // COMMAND BUFFER OBJECT FUNCTIONS
  ASH_DEF_VKPFN(BeginCommandBuffer);
  ASH_DEF_VKPFN(CmdBeginQuery);
  ASH_DEF_VKPFN(CmdBindDescriptorSets);
  ASH_DEF_VKPFN(CmdBindIndexBuffer);
  ASH_DEF_VKPFN(CmdBindPipeline);
  ASH_DEF_VKPFN(CmdBindVertexBuffers);
  ASH_DEF_VKPFN(CmdBlitImage);
  ASH_DEF_VKPFN(CmdClearAttachments);
  ASH_DEF_VKPFN(CmdClearColorImage);
  ASH_DEF_VKPFN(CmdClearDepthStencilImage);
  ASH_DEF_VKPFN(CmdCopyBuffer);
  ASH_DEF_VKPFN(CmdCopyBufferToImage);
  ASH_DEF_VKPFN(CmdCopyImage);
  ASH_DEF_VKPFN(CmdCopyImageToBuffer);
  ASH_DEF_VKPFN(CmdCopyQueryPoolResults);
  ASH_DEF_VKPFN(CmdDispatch);
  ASH_DEF_VKPFN(CmdDispatchIndirect);
  ASH_DEF_VKPFN(CmdDraw);
  ASH_DEF_VKPFN(CmdDrawIndexed);
  ASH_DEF_VKPFN(CmdDrawIndexedIndirect);
  ASH_DEF_VKPFN(CmdDrawIndirect);
  ASH_DEF_VKPFN(CmdEndQuery);
  ASH_DEF_VKPFN(CmdFillBuffer);
  ASH_DEF_VKPFN(CmdPipelineBarrier);
  ASH_DEF_VKPFN(CmdPushConstants);
  ASH_DEF_VKPFN(CmdResetEvent);
  ASH_DEF_VKPFN(CmdResetQueryPool);
  ASH_DEF_VKPFN(CmdResolveImage);
  ASH_DEF_VKPFN(CmdSetBlendConstants);
  ASH_DEF_VKPFN(CmdSetDepthBias);
  ASH_DEF_VKPFN(CmdSetDepthBounds);
  ASH_DEF_VKPFN(CmdSetEvent);
  ASH_DEF_VKPFN(CmdSetLineWidth);
  ASH_DEF_VKPFN(CmdSetScissor);
  ASH_DEF_VKPFN(CmdSetStencilCompareMask);
  ASH_DEF_VKPFN(CmdSetStencilReference);
  ASH_DEF_VKPFN(CmdSetStencilWriteMask);
  ASH_DEF_VKPFN(CmdSetViewport);
  ASH_DEF_VKPFN(CmdUpdateBuffer);
  ASH_DEF_VKPFN(CmdWaitEvents);
  ASH_DEF_VKPFN(CmdWriteTimestamp);
  ASH_DEF_VKPFN(EndCommandBuffer);
  ASH_DEF_VKPFN(ResetCommandBuffer);

  ASH_DEF_VKPFN(CmdSetStencilOpEXT);
  ASH_DEF_VKPFN(CmdSetStencilTestEnableEXT);
  ASH_DEF_VKPFN(CmdSetCullModeEXT);
  ASH_DEF_VKPFN(CmdSetFrontFaceEXT);
  ASH_DEF_VKPFN(CmdSetPrimitiveTopologyEXT);
  ASH_DEF_VKPFN(CmdSetDepthBoundsTestEnableEXT);
  ASH_DEF_VKPFN(CmdSetDepthCompareOpEXT);
  ASH_DEF_VKPFN(CmdSetDepthTestEnableEXT);
  ASH_DEF_VKPFN(CmdSetDepthWriteEnableEXT);

  ASH_DEF_VKPFN(CmdBeginRenderingKHR);
  ASH_DEF_VKPFN(CmdEndRenderingKHR);

  ASH_DEF_VKPFN(CreateSwapchainKHR);
  ASH_DEF_VKPFN(DestroySwapchainKHR);
  ASH_DEF_VKPFN(GetSwapchainImagesKHR);
  ASH_DEF_VKPFN(AcquireNextImageKHR);
  ASH_DEF_VKPFN(QueuePresentKHR);

  ASH_DEF_VKPFN(DebugMarkerSetObjectTagEXT);
  ASH_DEF_VKPFN(DebugMarkerSetObjectNameEXT);

  ASH_DEF_VKPFN(CmdDebugMarkerBeginEXT);
  ASH_DEF_VKPFN(CmdDebugMarkerEndEXT);
  ASH_DEF_VKPFN(CmdDebugMarkerInsertEXT);
};

#undef ASH_DEF_VKPFN

enum class AliasId : u16
{
  Undefined = U16_MAX
};

enum class ImageStateId : u16
{
  Undefined = U16_MAX
};

struct DescriptorSet;

struct BindLocation
{
  DescriptorSet * set     = nullptr;
  u32             binding = 0;
  u32             element = 0;

  constexpr bool operator==(BindLocation const & rhs) const
  {
    return set == rhs.set && binding == rhs.binding && element == rhs.element;
  }

  constexpr bool is_valid() const
  {
    return set != nullptr;
  }
};

/// @brief An allocated block of memory that can be aliased by multiple resources.
struct MemoryGroup
{
  VmaAllocation     vma_allocation = nullptr;
  u64               alignment      = 0;
  void *            map            = nullptr;
  SmallVec<u64>     alias_offsets  = {};
  SmallVec<u32>     alias_bindings = {};
  SmallVec<AliasId> alias_ids      = {};    // [ ] stable accross frames

  Layout64 layout() const;
};

struct MemoryInfo
{
  MemoryGroup *   memory_group  = nullptr;
  u32             group_binding = 0;
  u32             alias_binding = 0;
  gpu::MemoryType type          = gpu::MemoryType::Unique;
};

struct BufferBarrier
{
  VkPipelineStageFlags  src_stages = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlags  dst_stages = VK_PIPELINE_STAGE_NONE;
  VkBufferMemoryBarrier barrier    = {};
};

struct ImageBarrier
{
  VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_NONE;
  VkImageMemoryBarrier barrier    = {};
};

struct MemoryBarrier
{
  VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_NONE;
  VkMemoryBarrier      barrier    = {};
};

using BindLocations = SmallVec<BindLocation, 8>;

struct Buffer
{
  VkBuffer      vk_buffer      = nullptr;
  MemoryInfo    memory         = {};
  BindLocations bind_locations = {};
};

struct BufferView
{
  VkBufferView vk_view = nullptr;
  Buffer *     buffer  = nullptr;
};

struct Image
{
  ImageStateId      state_id           = ImageStateId::Undefined;
  gpu::ImageAspects aspects            = gpu::ImageAspects::None;
  bool              is_swapchain_image = false;
  VkImage           vk_image           = nullptr;
  MemoryInfo        memory             = {};
  BindLocations     bind_locations     = {};
};

struct ImageView
{
  VkImageView vk_view = nullptr;
  Image *     image   = nullptr;
};

struct DescriptorSetLayout
{
  VkDescriptorSetLayout vk_layout = nullptr;

  InplaceVec<gpu::DescriptorBindingInfo, gpu::MAX_DESCRIPTOR_SET_BINDINGS>
    bindings = {};

  u32 num_variable_length = 0;
};

using SyncResources = Enum<None, SmallVec<Buffer *, 4>,
                           SmallVec<BufferView *, 4>, SmallVec<ImageView *, 4>>;

struct DescriptorBinding
{
  SyncResources       sync_resources = none;
  gpu::DescriptorType type           = gpu::DescriptorType::Sampler;

  u32 size() const;
};

struct DescriptorSet
{
  VkDescriptorSet vk_set = nullptr;

  VkDescriptorPool vk_pool = nullptr;

  InplaceVec<DescriptorBinding, gpu::MAX_DESCRIPTOR_SET_BINDINGS> bindings = {};

  static void remove_bind_loc(BindLocations &      locations,
                              BindLocation const & loc);

  void update_link(BindLocation const & loc, Buffer * next);

  void update_link(BindLocation const & loc, BufferView * next);

  void update_link(BindLocation const & loc, ImageView * next);
};

struct ComputePipeline
{
  VkPipeline vk_pipeline = nullptr;

  VkPipelineLayout vk_layout = nullptr;

  u32 push_constants_size = 0;

  u32 num_sets = 0;
};

struct GraphicsPipeline
{
  VkPipeline vk_pipeline = nullptr;

  VkPipelineLayout vk_layout = nullptr;

  u32 push_constants_size = 0;

  u32 num_sets = 0;

  InplaceVec<gpu::Format, gpu::MAX_PIPELINE_COLOR_ATTACHMENTS> color_fmts = {};

  Option<gpu::Format> depth_fmt = none;

  Option<gpu::Format> stencil_fmt = none;

  gpu::SampleCount sample_count = gpu::SampleCount::C1;
};

struct Instance final : gpu::Instance
{
  AllocatorRef allocator = {};

  InstanceTable vk_table = {};

  VkInstance vk_instance = nullptr;

  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;

  bool validation_enabled = false;

  explicit Instance() = default;

  Instance(Instance const &)             = delete;
  Instance & operator=(Instance const &) = delete;
  Instance(Instance &&)                  = delete;
  Instance & operator=(Instance &&)      = delete;
  virtual ~Instance() override;

  virtual Result<gpu::Device *, Status>
    create_device(AllocatorRef                allocator,
                  Span<gpu::DeviceType const> preferred_types,
                  u32                         buffering) override;

  virtual gpu::Backend get_backend() override;

  virtual void uninit(gpu::Device * device) override;

  virtual void uninit(gpu::Surface surface) override;
};

struct PhysicalDevice
{
  VkPhysicalDevice vk_phy_dev = nullptr;

  VkPhysicalDeviceFeatures vk_features = {};

  VkPhysicalDeviceProperties vk_properties = {};

  VkPhysicalDeviceMemoryProperties vk_memory_properties = {};
};

/// @param is_optimal false when vulkan returns that the surface is suboptimal
/// or the description is updated by the user
///
/// @param is_out_of_date can't present anymore
/// @param is_optimal recommended but not necessary to resize
/// @param is_zero_sized swapchain is not receiving presentation requests,
/// because the surface requested a zero sized image extent
struct Swapchain
{
  bool is_out_of_date = true;

  bool is_optimal = false;

  bool is_zero_sized = false;

  gpu::SurfaceFormat format = {};

  gpu::ImageUsage usage = gpu::ImageUsage::None;

  gpu::PresentMode present_mode = gpu::PresentMode::Immediate;

  u32x2 extent = {};

  gpu::CompositeAlpha composite_alpha = gpu::CompositeAlpha::None;

  InplaceVec<Image *, gpu::MAX_SWAPCHAIN_IMAGES> images = {};

  InplaceVec<VkImage, gpu::MAX_SWAPCHAIN_IMAGES> vk_images = {};

  u32 current_image = 0;

  VkSwapchainKHR vk_swapchain = nullptr;

  VkSurfaceKHR vk_surface = nullptr;
};

#define ASH_VK_CAST(Handle)                           \
  ASH_FORCE_INLINE inline Handle * ptr(gpu::Handle p) \
  {                                                   \
    return reinterpret_cast<Handle *>(p);             \
  }                                                   \
                                                      \
  ASH_FORCE_INLINE inline Handle & ref(gpu::Handle p) \
  {                                                   \
    return *ptr(p);                                   \
  }

ASH_VK_CAST(Buffer)
ASH_VK_CAST(Image)
ASH_VK_CAST(BufferView)
ASH_VK_CAST(ImageView)
ASH_VK_CAST(DescriptorSetLayout)
ASH_VK_CAST(DescriptorSet)
ASH_VK_CAST(ComputePipeline)
ASH_VK_CAST(GraphicsPipeline)
ASH_VK_CAST(Swapchain)

namespace cmd
{

enum class Type : u8
{
  ResetTimestampQuery    = 0,
  ResetStatisticsQuery   = 1,
  WriteTimestamp         = 2,
  BeginStatistics        = 3,
  EndStatistics          = 4,
  BeginDebugMarker       = 5,
  EndDebugMarker         = 6,
  FillBuffer             = 7,
  CopyBuffer             = 8,
  UpdateBuffer           = 9,
  ClearColorImage        = 10,
  ClearDepthStencilImage = 11,
  CopyImage              = 12,
  CopyBufferToImage      = 13,
  BlitImage              = 14,
  ResolveImage           = 15,
  BeginComputePass       = 16,
  EndComputePass         = 17,
  BeginRendering         = 18,
  EndRendering           = 19,
  BindComputePipeline    = 20,
  BindGraphicsPipeline   = 21,
  BindDescriptorSets     = 22,
  PushConstants          = 23,
  Dispatch               = 24,
  DispatchIndirect       = 25,
  SetGraphicsState       = 26,
  BindVertexBuffers      = 27,
  BindIndexBuffer        = 28,
  Draw                   = 29,
  DrawIndexed            = 30,
  DrawIndirect           = 31,
  DrawIndexedIndirect    = 32,
  Undefined              = U8_MAX
};

struct Command
{
  Type const type_ = Type::Undefined;
};

struct ResetTimestampQuery
{
  Type const          type_ = Type::ResetTimestampQuery;
  gpu::TimestampQuery query = nullptr;
  Slice32             range = {};
};

struct ResetStatisticsQuery
{
  Type const           type_ = Type::ResetStatisticsQuery;
  gpu::StatisticsQuery query = nullptr;
  Slice32              range = {};
};

struct WriteTimestamp
{
  Type const          type_ = Type::WriteTimestamp;
  gpu::TimestampQuery query = nullptr;
  gpu::PipelineStages stage = gpu::PipelineStages::TopOfPipe;
  u32                 index = 0;
};

struct BeginStatistics
{
  Type const           type_ = Type::BeginStatistics;
  gpu::StatisticsQuery query = nullptr;
  u32                  index = 0;
};

struct EndStatistics
{
  Type const           type_ = Type::EndStatistics;
  gpu::StatisticsQuery query = nullptr;
  u32                  index = 0;
};

struct BeginDebugMarker
{
  Type const type_       = Type::BeginDebugMarker;
  Str        region_name = {};
  f32x4      color       = {};
};

struct EndDebugMarker
{
  Type const type_ = Type::EndDebugMarker;
};

struct FillBuffer
{
  Type const  type_  = Type::FillBuffer;
  gpu::Buffer dst    = nullptr;
  u64         offset = 0;
  u64         size   = 0;
  u32         data   = 0;
};

struct CopyBuffer
{
  Type const                  type_  = Type::CopyBuffer;
  gpu::Buffer                 src    = nullptr;
  gpu::Buffer                 dst    = nullptr;
  Span<gpu::BufferCopy const> copies = {};
};

struct UpdateBuffer
{
  Type const     type_      = Type::UpdateBuffer;
  Span<u8 const> src        = {};
  u64            dst_offset = 0;
  gpu::Buffer    dst        = nullptr;
};

struct ClearColorImage
{
  Type const                             type_  = Type::ClearColorImage;
  gpu::Image                             dst    = nullptr;
  gpu::Color                             value  = {};
  Span<gpu::ImageSubresourceRange const> ranges = {};
};

struct ClearDepthStencilImage
{
  Type const                             type_  = Type::ClearDepthStencilImage;
  gpu::Image                             dst    = nullptr;
  gpu::DepthStencil                      value  = {};
  Span<gpu::ImageSubresourceRange const> ranges = {};
};

struct CopyImage
{
  Type const                 type_  = Type::CopyImage;
  gpu::Image                 src    = nullptr;
  gpu::Image                 dst    = nullptr;
  Span<gpu::ImageCopy const> copies = {};
};

struct CopyBufferToImage
{
  Type const                       type_  = Type::CopyBufferToImage;
  gpu::Buffer                      src    = nullptr;
  gpu::Image                       dst    = nullptr;
  Span<gpu::BufferImageCopy const> copies = {};
};

struct BlitImage
{
  Type const                 type_  = Type::BlitImage;
  gpu::Image                 src    = nullptr;
  gpu::Image                 dst    = nullptr;
  Span<gpu::ImageBlit const> blits  = {};
  gpu::Filter                filter = gpu::Filter::Linear;
};

struct ResolveImage
{
  Type const                    type_    = Type::ResolveImage;
  gpu::Image                    src      = nullptr;
  gpu::Image                    dst      = nullptr;
  Span<gpu::ImageResolve const> resolves = {};
};

struct BeginComputePass
{
  Type const type_ = Type::BeginComputePass;
};

struct EndComputePass
{
  Type const type_ = Type::EndComputePass;
};

struct BeginRendering
{
  Type const         type_ = Type::BeginRendering;
  gpu::RenderingInfo info  = {};
};

struct EndRendering
{
  Type const type_ = Type::EndRendering;
};

struct BindComputePipeline
{
  Type const           type_ = Type::BindComputePipeline;
  gpu::ComputePipeline pipeline;
};

struct BindGraphicsPipeline
{
  Type const            type_    = Type::BindGraphicsPipeline;
  gpu::GraphicsPipeline pipeline = nullptr;
};

struct BindDescriptorSets
{
  Type const                     type_           = Type::BindDescriptorSets;
  Span<gpu::DescriptorSet const> sets            = {};
  Span<u32 const>                dynamic_offsets = {};
};

struct PushConstants
{
  Type const     type_    = Type::PushConstants;
  Span<u8 const> constant = {};
};

struct Dispatch
{
  Type const type_       = Type::Dispatch;
  u32x3      group_count = {};
};

struct DispatchIndirect
{
  Type const  type_  = Type::DispatchIndirect;
  gpu::Buffer buffer = nullptr;
  u64         offset = 0;
};

struct SetGraphicsState
{
  Type const         type_ = Type::SetGraphicsState;
  gpu::GraphicsState state = {};
};

struct BindVertexBuffers
{
  Type const              type_   = Type::BindVertexBuffers;
  u32                     binding = 0;
  Span<gpu::Buffer const> buffers = {};
  Span<u64 const>         offsets = {};
};

struct BindIndexBuffer
{
  Type const     type_      = Type::BindIndexBuffer;
  gpu::Buffer    buffer     = nullptr;
  u64            offset     = 0;
  gpu::IndexType index_type = gpu::IndexType::U32;
};

struct Draw
{
  Type const type_     = Type::Draw;
  Slice32    vertices  = {};
  Slice32    instances = {};
};

struct DrawIndexed
{
  Type const type_         = Type::DrawIndexed;
  Slice32    indices       = {};
  Slice32    instances     = {};
  i32        vertex_offset = 0;
};

struct DrawIndirect
{
  Type const  type_      = Type::DrawIndirect;
  gpu::Buffer buffer     = nullptr;
  u64         offset     = 0;
  u32         draw_count = 0;
  u32         stride     = 0;
};

struct DrawIndexedIndirect
{
  Type const  type_      = Type::DrawIndexedIndirect;
  gpu::Buffer buffer     = nullptr;
  u64         offset     = 0;
  u32         draw_count = 0;
  u32         stride     = 0;
};

}    // namespace cmd

enum class HazardType : u8
{
  /// @brief No reads or writes so far
  None            = 0,
  /// @brief Read hazards that need to be awaited. Subsequent reads are overlapped until a write occurs
  Reads           = 1,
  /// @brief Write hazard that needs to be awaited.
  Write           = 2,
  /// @brief Reads after a write that need to be awaited. Subsequent reads are allowed to overlap until a write occurs
  ReadsAfterWrite = 3
};

struct MemoryAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlags        access = VK_ACCESS_NONE;
};

struct BufferAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlags        access = VK_ACCESS_NONE;

  MemoryAccess to_memory() const;
};

struct BufferHazard
{
  u32          alias = U32_MAX;
  HazardType   type  = HazardType::None;
  BufferAccess reads = {};
  BufferAccess write = {};

  MemoryAccess latest_acccess() const;
};

struct ImageAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlags        access = VK_ACCESS_NONE;
  VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;

  MemoryAccess to_memory() const;
};

struct ImageHazard
{
  u32           alias  = U32_MAX;
  HazardType    type   = HazardType::None;
  ImageAccess   reads  = {};
  ImageAccess   write  = {};
  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

  MemoryAccess latest_acccess() const;
};

/// @brief Multiple aliases can be binded to the same memory alias and read from it at the same time.
/// but they can't write to it at the same time.
struct MemoryHazard
{
  u64                             pass = 0;
  Enum<BufferHazard, ImageHazard> binding;
};

/// @brief Descriptor set hazard state
struct DescriptorSetHazard
{
  u64 pass = 0;
};

/// @brief Per-encoder resource state heap
struct ResourceStateHeap
{
  template <typename T>
  using Map = CoreSparseMap<Vec<u16>, Vec<T>>;

  Map<VkImageLayout> image_states = {};
};

enum class DescriptorSetState : u8
{
  ComputeShaderView  = 0,
  GraphicsShaderView = 1,
  Undefined          = U8_MAX
};

enum class AccessState : u8
{
  Read      = 0,
  ReadWrite = 1
};

enum class BufferState : u8
{
  ComputeShaderView  = 0,
  GraphicsShaderView = 1,
  VertexBuffer       = 2,
  IndexBuffer        = 3,
  IndirectBuffer     = 4,
  TransferSrc        = 5,
  TransferDst        = 6,
  Undefined          = U8_MAX
};

enum class ImageState : u8
{
  ComputeShaderView             = 0,
  GraphicsShaderView            = 1,
  ColorAttachment               = 2,
  DepthStencilAttachment        = 3,
  ColorResolveAttachment        = 4,
  DepthStencilResolveAttachment = 5,
  TransferSrc                   = 6,
  TransferDst                   = 7,
  Present                       = 8,
  General                       = 9,
  Undefined                     = U8_MAX
};

struct HazardBarriers
{
  Vec<BufferBarrier> buffer;
  Vec<ImageBarrier>  image;
  Vec<MemoryBarrier> memory;
};

/// @brief Per-encoder resource hazard state heap
struct ResourceHazardHeap
{
  CoreSparseMap<Vec<u16>, Vec<MemoryHazard>> memory_hazards = {};
  CoreSparseMap<Vec<u16>, BitVec<u64>, Vec<DescriptorSetHazard>>
    descriptor_set_hazards = {};

  /*bool has_hazard(AliasId id) const
  {
    return memory_hazards[(usize) id].v0;
  } */

  static void barrier(Image const & image, ImageAccess const & old_state,
                      ImageAccess const & new_state, HazardBarriers & barriers);

  static void barrier(Buffer const & buffer, BufferAccess const & old_state,
                      BufferAccess const & new_state,
                      HazardBarriers &     barriers);

  static void barrier(MemoryAccess const & old_state,
                      MemoryAccess const & new_state,
                      HazardBarriers &     barriers);

  /// @param image image to sync
  /// @param access merged image state for the pass
  /// @param pass the pass temporal id
  /// @param barriers destination to issue barriers
  void access(Image const & image, ImageAccess const & state, u64 pass,
              HazardBarriers & barriers);

  /// @param buffer buffer to sync
  /// @param access merged image state for the pass
  /// @param pass the pass temporal id
  /// @param barriers destination to issue barriers
  void access(Buffer const & buffer, BufferAccess const & state, u64 pass,
              HazardBarriers & barriers);

  /// @param set descriptor set to sync
  /// @param access merged image state for the pass
  /// @param pass the pass temporal id
  /// @param barriers destination to issue barriers
  void access(DescriptorSet const & set, u64 pass, HazardBarriers & barriers);
};

struct PassAccess
{
  Slice32 descriptor_sets = {};
  Slice32 buffers         = {};
  Slice32 images          = {};
};

struct AccessEncoder
{
  Vec<Tuple<gpu::DescriptorSet, DescriptorSetState>> descriptor_sets_ = {};
  Vec<Tuple<gpu::Buffer, BufferState>>               buffers_         = {};
  Vec<Tuple<gpu::Image, ImageState>>                 images_          = {};
  Vec<PassAccess>                                    passes_          = {};

  void begin_pass()
  {
    passes_.push(PassAccess{}).unwrap();
  }

  void end_pass()
  {
  }

  void access(gpu::DescriptorSet set, DescriptorSetState state);
  void access(gpu::Buffer buffer, BufferState state);
  void access(gpu::Image image, ImageState state);
};

struct Encoder
{
  Vec<cmd::Command *> cmds_;
  ArenaPool           pool_;
};

/*



enum class CommandEncoderState : u16
{
  Reset       = 0,
  Begin       = 1,
  RenderPass  = 2,
  ComputePass = 3,
  End         = 4
};



*/

struct CommandEncoder final : gpu::CommandEncoder
{
  AllocatorRef    allocator         = {};
  ArenaPool       arg_pool          = {};
  VkCommandPool   vk_command_pool   = nullptr;
  VkCommandBuffer vk_command_buffer = nullptr;
  Status          status            = Status::Success;

  /*
  bool is_in_render_pass() const
  {
    return state == CommandEncoderState::RenderPass;
  }

  bool is_in_compute_pass() const
  {
    return state == CommandEncoderState::ComputePass;
  }

  bool is_in_pass() const
  {
    return is_in_render_pass() || is_in_compute_pass();
  }

  bool is_recording() const
  {
    return state == CommandEncoderState::Begin ||
           state == CommandEncoderState::RenderPass ||
           state == CommandEncoderState::ComputePass;
  }

  void validate_render_pass_compatible(gpu::GraphicsPipeline pipeline);

  void access_image(Image & image, VkPipelineStageFlags stages,
                    VkAccessFlags access, VkImageLayout layout,
                    u64 pass_timestamp);

  void access_buffer(Buffer & buffer, VkPipelineStageFlags stages,
                     VkAccessFlags access, u64 pass_timestamp);

  void insert_barrier(ImageBarrier const & barrier);

  void insert_barrier(BufferBarrier const & barrier);

  void insert_barrier(MemoryBarrier const & barrier);

  void access_compute_bindings(DescriptorSet const & set, u64 pass_timestamp);

  void access_graphics_bindings(DescriptorSet const & set, u64 pass_timestamp);

  void clear_context()
  {
    state = CommandEncoderState::Begin;
    render_ctx.clear();
    compute_ctx.clear();
  }
  */

  virtual void reset_timestamp_query(gpu::TimestampQuery query,
                                     Slice32             range) override;

  virtual void reset_statistics_query(gpu::StatisticsQuery query,
                                      Slice32              range) override;

  virtual void write_timestamp(gpu::TimestampQuery query,
                               gpu::PipelineStages stage, u32 index) override;

  virtual void begin_statistics(gpu::StatisticsQuery query, u32 index) override;

  virtual void end_statistics(gpu::StatisticsQuery query, u32 index) override;

  virtual void begin_debug_marker(Str region_name, f32x4 color) override;

  virtual void end_debug_marker() override;

  virtual void fill_buffer(gpu::Buffer dst, u64 offset, u64 size,
                           u32 data) override;

  virtual void copy_buffer(gpu::Buffer src, gpu::Buffer dst,
                           Span<gpu::BufferCopy const> copies) override;

  virtual void update_buffer(Span<u8 const> src, u64 dst_offset,
                             gpu::Buffer dst) override;

  virtual void
    clear_color_image(gpu::Image dst, gpu::Color value,
                      Span<gpu::ImageSubresourceRange const> ranges) override;

  virtual void clear_depth_stencil_image(
    gpu::Image dst, gpu::DepthStencil value,
    Span<gpu::ImageSubresourceRange const> ranges) override;

  virtual void copy_image(gpu::Image src, gpu::Image dst,
                          Span<gpu::ImageCopy const> copies) override;

  virtual void
    copy_buffer_to_image(gpu::Buffer src, gpu::Image dst,
                         Span<gpu::BufferImageCopy const> copies) override;

  virtual void blit_image(gpu::Image src, gpu::Image dst,
                          Span<gpu::ImageBlit const> blits,
                          gpu::Filter                filter) override;

  virtual void resolve_image(gpu::Image src, gpu::Image dst,
                             Span<gpu::ImageResolve const> resolves) override;

  virtual void begin_compute_pass() override;

  virtual void end_compute_pass() override;

  virtual void begin_rendering(gpu::RenderingInfo const & info) override;

  virtual void end_rendering() override;

  virtual void bind_compute_pipeline(gpu::ComputePipeline pipeline) override;

  virtual void bind_graphics_pipeline(gpu::GraphicsPipeline pipeline) override;

  virtual void
    bind_descriptor_sets(Span<gpu::DescriptorSet const> descriptor_sets,
                         Span<u32 const> dynamic_offsets) override;

  virtual void push_constants(Span<u8 const> push_constants_data) override;

  virtual void dispatch(u32x3 group_count) override;

  virtual void dispatch_indirect(gpu::Buffer buffer, u64 offset) override;

  virtual void set_graphics_state(gpu::GraphicsState const & state) override;

  virtual void bind_vertex_buffers(Span<gpu::Buffer const> vertex_buffers,
                                   Span<u64 const>         offsets) override;

  virtual void bind_index_buffer(gpu::Buffer index_buffer, u64 offset,
                                 gpu::IndexType index_type) override;

  virtual void draw(Slice32 vertices, Slice32 instances) override;

  virtual void draw_indexed(Slice32 indices, Slice32 instances,
                            i32 vertex_offset) override;

  virtual void draw_indirect(gpu::Buffer buffer, u64 offset, u32 draw_count,
                             u32 stride) override;

  virtual void draw_indexed_indirect(gpu::Buffer buffer, u64 offset,
                                     u32 draw_count, u32 stride) override;
};

struct CommandBuffer final : gpu::CommandBuffer
{
  Device *        dev               = nullptr;
  VkCommandPool   vk_command_pool   = nullptr;
  VkCommandBuffer vk_command_buffer = nullptr;
  Status          status            = Status::Success;

  virtual void commit(gpu::CommandEncoder & encoder) override;
};

struct FrameContext
{
  gpu::FrameId tail_frame = 0;

  gpu::FrameId current_frame = 0;

  u32 ring_index = 0;

  InplaceVec<CommandEncoder, gpu::MAX_FRAME_BUFFERING> encoders = {};

  InplaceVec<gpu::CommandEncoder *, gpu::MAX_FRAME_BUFFERING> encoders_impl =
    {};

  InplaceVec<VkSemaphore, gpu::MAX_FRAME_BUFFERING> acquire_semaphores = {};

  InplaceVec<VkFence, gpu::MAX_FRAME_BUFFERING> submit_fences = {};

  InplaceVec<VkSemaphore, gpu::MAX_FRAME_BUFFERING> submit_semaphores = {};

  Swapchain * swapchain = nullptr;

  u32 buffering() const
  {
    return encoders.size();
  }
};

struct Device final : gpu::Device
{
  AllocatorRef       allocator      = {};
  Instance *         instance       = nullptr;
  PhysicalDevice     phy_dev        = {};
  DeviceTable        vk_table       = {};
  VmaVulkanFunctions vma_table      = {};
  VkDevice           vk_dev         = nullptr;
  u32                queue_family   = 0;
  VkQueue            vk_queue       = nullptr;
  VmaAllocator       vma_allocator  = nullptr;
  FrameContext       frame_ctx      = {};
  Vec<u8>            scratch        = {};
  u64                pass_timestamp = 1;

  void set_resource_name(Str label, void const * resource, VkObjectType type,
                         VkDebugReportObjectTypeEXT debug_type);

  VkResult recreate_swapchain(Swapchain * swapchain);

  Status init_command_encoder(CommandEncoder * enc);

  void uninit(CommandEncoder * enc);

  Status init_frame_context(u32 buffering);

  void uninit();

  virtual gpu::DeviceProperties get_properties() override;

  virtual Result<gpu::FormatProperties, Status>
    get_format_properties(gpu::Format format) override;

  virtual Result<gpu::Buffer, Status>
    create_buffer(gpu::BufferInfo const & info) override;

  virtual Result<gpu::BufferView, Status>
    create_buffer_view(gpu::BufferViewInfo const & info) override;

  virtual Result<gpu::Image, Status>
    create_image(gpu::ImageInfo const & info) override;

  virtual Result<gpu::ImageView, Status>
    create_image_view(gpu::ImageViewInfo const & info) override;

  virtual Result<gpu::MemoryGroup, Status>
    create_memory_group(gpu::MemoryGroupInfo const & info) override;

  virtual Result<gpu::Sampler, Status>
    create_sampler(gpu::SamplerInfo const & info) override;

  virtual Result<gpu::Shader, Status>
    create_shader(gpu::ShaderInfo const & info) override;

  virtual Result<gpu::DescriptorSetLayout, Status> create_descriptor_set_layout(
    gpu::DescriptorSetLayoutInfo const & info) override;

  virtual Result<gpu::DescriptorSet, Status>
    create_descriptor_set(gpu::DescriptorSetInfo const & info) override;

  virtual Result<gpu::PipelineCache, Status>
    create_pipeline_cache(gpu::PipelineCacheInfo const & info) override;

  virtual Result<gpu::ComputePipeline, Status>
    create_compute_pipeline(gpu::ComputePipelineInfo const & info) override;

  virtual Result<gpu::GraphicsPipeline, Status>
    create_graphics_pipeline(gpu::GraphicsPipelineInfo const & info) override;

  virtual Result<gpu::Swapchain, Status>
    create_swapchain(gpu::Surface               surface,
                     gpu::SwapchainInfo const & info) override;

  virtual Result<gpu::TimestampQuery, Status>
    create_timestamp_query(u32 count) override;

  virtual Result<gpu::StatisticsQuery, Status>
    create_statistics_query(u32 count) override;

  virtual void uninit(gpu::Buffer buffer) override;

  virtual void uninit(gpu::BufferView buffer_view) override;

  virtual void uninit(gpu::Image image) override;

  virtual void uninit(gpu::ImageView image_view) override;

  virtual void uninit(gpu::MemoryGroup memory_group) override;

  virtual void uninit(gpu::Sampler sampler) override;

  virtual void uninit(gpu::Shader shader) override;

  virtual void uninit(gpu::DescriptorSetLayout layout) override;

  virtual void uninit(gpu::DescriptorSet set) override;

  virtual void uninit(gpu::PipelineCache cache) override;

  virtual void uninit(gpu::ComputePipeline pipeline) override;

  virtual void uninit(gpu::GraphicsPipeline pipeline) override;

  virtual void uninit(gpu::Swapchain swapchain) override;

  virtual void uninit(gpu::TimestampQuery query) override;

  virtual void uninit(gpu::StatisticsQuery query) override;

  virtual gpu::FrameContext get_frame_context() override;

  virtual Result<Span<u8>, Status> get_memory_map(gpu::Buffer buffer) override;

  virtual Result<Void, Status> invalidate_mapped_memory(gpu::Buffer buffer,
                                                        Slice64 range) override;

  virtual Result<Void, Status> flush_mapped_memory(gpu::Buffer buffer,
                                                   Slice64     range) override;

  virtual Result<usize, Status>
    get_pipeline_cache_size(gpu::PipelineCache cache) override;

  virtual Result<Void, Status> get_pipeline_cache_data(gpu::PipelineCache cache,
                                                       Vec<u8> & out) override;

  virtual Result<Void, Status>
    merge_pipeline_cache(gpu::PipelineCache             dst,
                         Span<gpu::PipelineCache const> srcs) override;

  virtual void
    update_descriptor_set(gpu::DescriptorSetUpdate const & update) override;

  virtual Result<Void, Status> wait_idle() override;

  virtual Result<Void, Status> wait_queue_idle() override;

  virtual Result<Void, Status>
    get_surface_formats(gpu::Surface              surface,
                        Vec<gpu::SurfaceFormat> & formats) override;

  virtual Result<Void, Status>
    get_surface_present_modes(gpu::Surface            surface,
                              Vec<gpu::PresentMode> & modes) override;

  virtual Result<gpu::SurfaceCapabilities, Status>
    get_surface_capabilities(gpu::Surface surface) override;

  virtual Result<gpu::SwapchainState, Status>
    get_swapchain_state(gpu::Swapchain swapchain) override;

  virtual Result<Void, Status>
    invalidate_swapchain(gpu::Swapchain             swapchain,
                         gpu::SwapchainInfo const & info) override;

  virtual Result<Void, Status> begin_frame(gpu::Swapchain swapchain) override;

  virtual Result<Void, Status> submit_frame(gpu::Swapchain swapchain) override;

  virtual Result<Void, Status>
    get_timestamp_query_result(gpu::TimestampQuery query, Slice32 range,
                               Vec<u64> & timestamps) override;

  virtual Result<Void, Status> get_statistics_query_result(
    gpu::StatisticsQuery query, Slice32 range,
    Vec<gpu::PipelineStatistics> & statistics) override;
};

}    // namespace vk
}    // namespace ash
