/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/gpu/gpu.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/async.h"
#include "ashura/std/vec.h"

// clang-format off
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_VULKAN_VERSION           1'000'000

#include "vulkan/vulkan_core.h"
#include "vk_mem_alloc.h"

// clang-format on

namespace ash
{
namespace vk
{

using gpu::Status;

inline constexpr char const ENGINE_NAME[] = "Ash";

inline constexpr u32 ENGINE_VERSION = VK_MAKE_API_VERSION(
  ASH_VERSION.variant, ASH_VERSION.major, ASH_VERSION.minor, ASH_VERSION.patch);

inline constexpr u32 ENGINE_VULKAN_VERSION = VK_API_VERSION_1_1;

inline constexpr char const CLIENT_NAME[] = "Ash Client";

inline constexpr u32 CLIENT_VERSION = ENGINE_VERSION;

typedef struct IBuffer *              Buffer;
typedef struct IBufferView *          BufferView;
typedef struct IImage *               Image;
typedef struct IImageView *           ImageView;
typedef struct IAlias *               Alias;
typedef VkSampler                     Sampler;
typedef VkShaderModule                Shader;
typedef struct IDescriptorSetLayout * DescriptorSetLayout;
typedef struct IDescriptorSet *       DescriptorSet;
typedef VkPipelineCache               PipelineCache;
typedef struct IComputePipeline *     ComputePipeline;
typedef struct IGraphicsPipeline *    GraphicsPipeline;
typedef VkQueryPool                   TimestampQuery;
typedef VkQueryPool                   StatisticsQuery;
typedef VkSurfaceKHR                  Surface;
typedef struct ISwapchain *           Swapchain;
typedef struct IQueueScope *          QueueScope;
typedef struct ICommandEncoder *      CommandEncoder;
typedef struct ICommandBuffer *       CommandBuffer;
typedef struct IDevice *              Device;
typedef struct IInstance *            Instance;

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
  ASH_DEF_VKPFN(GetPhysicalDeviceFeatures2KHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceFormatProperties2KHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceImageFormatProperties2KHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceMemoryProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceMemoryProperties2KHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceProperties2KHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceQueueFamilyProperties2KHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceSparseImageFormatProperties2KHR);

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

enum class AliasId : u32
{
  Undefined = U32_MAX
};

enum class DescriptorSetId : u32
{
  Undefined = U32_MAX
};

struct IDescriptorSet;

struct BindLocation
{
  DescriptorSet set     = nullptr;
  u32           binding = 0;
  u32           element = 0;

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
struct IAlias
{
  AliasId       id             = AliasId::Undefined;
  VmaAllocation vma_allocation = nullptr;
  Layout64      layout         = {.alignment = 1, .size = 0};
  void *        map            = nullptr;
};

struct MemoryInfo
{
  Alias           alias   = nullptr;
  u32             element = 0;
  gpu::MemoryType type    = gpu::MemoryType::Unique;
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

using BindLocations = SmallVec<BindLocation, 8, 0>;

struct IBuffer
{
  VkBuffer         vk             = nullptr;
  gpu::BufferUsage usage          = gpu::BufferUsage::None;
  bool             host_mapped    = false;
  u64              size           = 0;
  MemoryInfo       memory         = {};
  BindLocations    bind_locations = {};
};

struct IBufferView
{
  VkBufferView  vk             = nullptr;
  Buffer        buffer         = nullptr;
  Slice64       slice          = {};
  BindLocations bind_locations = {};
};

struct IImage
{
  VkImage           vk                 = nullptr;
  gpu::ImageType    type               = gpu::ImageType::Type1D;
  gpu::ImageUsage   usage              = gpu::ImageUsage::None;
  gpu::ImageAspects aspects            = gpu::ImageAspects::None;
  gpu::SampleCount  sample_count       = gpu::SampleCount::None;
  u32x3             extent             = {};
  u32               mip_levels         = 0;
  u32               array_layers       = 0;
  bool              is_swapchain_image = false;
  MemoryInfo        memory             = {};
};

struct IImageView
{
  VkImageView   vk             = nullptr;
  Image         image          = nullptr;
  gpu::Format   format         = gpu::Format::Undefined;
  Slice32       mip_levels     = {};
  Slice32       array_layers   = {};
  BindLocations bind_locations = {};
};

struct IDescriptorSetLayout
{
  VkDescriptorSetLayout vk = nullptr;

  SmallVec<gpu::DescriptorBindingInfo, 1, 0> bindings = {};

  u32 num_variable_length = 0;

  bool is_readonly = false;
};

using SyncResources = Enum<None, SmallVec<Option<IBuffer &>, 4, 0>,
                           SmallVec<Option<IBufferView &>, 4, 0>,
                           SmallVec<Option<IImageView &>, 4, 0>>;

struct DescriptorBinding
{
  SyncResources sync_resources = none;

  gpu::DescriptorType type = gpu::DescriptorType::Sampler;

  u32 count = 0;

  u32 sync_size() const;
};

struct IDescriptorSet
{
  VkDescriptorSet vk = nullptr;

  VkDescriptorPool vk_pool = nullptr;

  DescriptorSetId id = DescriptorSetId::Undefined;

  bool is_readonly = false;

  SmallVec<DescriptorBinding, 1, 0> bindings = {};

  static void remove_bind_loc(BindLocations &      locations,
                              BindLocation const & loc);

  void update_link(u32 binding, u32 first_element,
                   Span<gpu::BufferBinding const> buffers);

  void update_link(u32 binding, u32 first_element,
                   Span<gpu::BufferView const> buffer_views);

  void update_link(u32 binding, u32 first_element,
                   Span<gpu::ImageBinding const> images);
};

struct IComputePipeline
{
  VkPipeline vk = nullptr;

  VkPipelineLayout vk_layout = nullptr;

  u32 push_constants_size = 0;

  u32 num_sets = 0;
};

struct IGraphicsPipeline
{
  VkPipeline vk = nullptr;

  VkPipelineLayout vk_layout = nullptr;

  u32 push_constants_size = 0;

  u32 num_sets = 0;

  SmallVec<gpu::Format, 8, 0> color_fmts = {};

  Option<gpu::Format> depth_fmt = none;

  Option<gpu::Format> stencil_fmt = none;

  gpu::SampleCount sample_count = gpu::SampleCount::C1;

  u32 num_vertex_attributes = 0;
};

struct IInstance final : gpu::IInstance
{
  Allocator allocator_;

  InstanceTable table_;

  VkInstance vk_;

  VkDebugUtilsMessengerEXT vk_debug_messenger_;

  bool validation_enabled_;

  explicit IInstance(Allocator allocator, InstanceTable table,
                     VkInstance               instance,
                     VkDebugUtilsMessengerEXT debug_messenger,
                     bool                     validation_enabled) :
    allocator_{allocator},
    table_{table},
    vk_{instance},
    vk_debug_messenger_{debug_messenger},
    validation_enabled_{validation_enabled}
  {
  }

  IInstance(IInstance const &)             = delete;
  IInstance & operator=(IInstance const &) = delete;
  IInstance(IInstance &&)                  = delete;
  IInstance & operator=(IInstance &&)      = delete;
  virtual ~IInstance() override;

  virtual Result<gpu::Device, Status>
    create_device(Allocator                   allocator,
                  Span<gpu::DeviceType const> preferred_types) override;

  virtual gpu::Backend get_backend() override;

  virtual void uninit(gpu::Device device) override;

  virtual void uninit(gpu::Surface surface) override;
};

struct IPhysicalDevice
{
  VkPhysicalDevice vk = nullptr;

  VkPhysicalDeviceFeatures vk_features = {};

  VkPhysicalDeviceProperties vk_properties = {};

  VkPhysicalDeviceMemoryProperties vk_memory_properties = {};

  VkPhysicalDeviceDescriptorIndexingPropertiesEXT vk_descriptor_properties = {};
};

struct SwapchainPreference
{
  Vec<char>           label               = {};
  gpu::Surface        surface             = nullptr;
  gpu::SurfaceFormat  format              = {};
  gpu::ImageUsage     usage               = gpu::ImageUsage::None;
  u32                 preferred_buffering = 0;
  gpu::PresentMode    present_mode        = gpu::PresentMode::Immediate;
  u32x2               preferred_extent    = {};
  gpu::CompositeAlpha composite_alpha     = gpu::CompositeAlpha::None;
};

/// @param is_out_of_date can't present anymore
/// @param is_optimal recommended but not necessary to resize
/// @param is_zero_sized swapchain is not receiving presentation requests,
/// because the surface requested a zero sized image extent
struct ISwapchain
{
  VkSwapchainKHR vk = nullptr;

  VkSurfaceKHR vk_surface = nullptr;

  SmallVec<Image, 8, 0> images = {};

  SmallVec<VkSemaphore, 8, 0> acquire_semaphores = {};

  u32 ring_index = 0;

  Option<u32> current_image = none;

  Option<u32> current_semaphore = none;

  bool is_deferred = true;

  bool is_out_of_date = true;

  bool is_optimal = false;

  gpu::SurfaceFormat format = {};

  gpu::ImageUsage usage = gpu::ImageUsage::None;

  gpu::PresentMode present_mode = gpu::PresentMode::Immediate;

  u32x2 extent = {};

  gpu::CompositeAlpha composite_alpha = gpu::CompositeAlpha::None;

  SwapchainPreference preference = {};
};

#define ASH_VK_CAST(Handle)                            \
  ASH_FORCE_INLINE Handle ptr(ash::gpu::Handle p)      \
  {                                                    \
    return reinterpret_cast<ash::vk::Handle>(p);       \
  }                                                    \
                                                       \
  ASH_FORCE_INLINE I##Handle & ref(ash::gpu::Handle p) \
  {                                                    \
    return *ptr(p);                                    \
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

enum class Type : usize
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
  BeginRendering         = 16,
  EndRendering           = 17,
  BindPipeline           = 18,
  BindDescriptorSets     = 19,
  PushConstants          = 20,
  Dispatch               = 21,
  DispatchIndirect       = 22,
  SetGraphicsState       = 23,
  BindVertexBuffers      = 24,
  BindIndexBuffer        = 25,
  Draw                   = 26,
  DrawIndexed            = 27,
  DrawIndirect           = 28,
  DrawIndexedIndirect    = 29
};

struct alignas(8) Cmd
{
  Type const type;
  Cmd *      next = nullptr;
};

struct alignas(8) ResetTimestampQuery
{
  Type const  type  = Type::ResetTimestampQuery;
  Cmd *       next  = nullptr;
  VkQueryPool query = nullptr;
  Slice32     range = {};
};

struct alignas(8) ResetStatisticsQuery
{
  Type const  type  = Type::ResetStatisticsQuery;
  Cmd *       next  = nullptr;
  VkQueryPool query = nullptr;
  Slice32     range = {};
};

struct alignas(8) WriteTimestamp
{
  Type const              type   = Type::WriteTimestamp;
  Cmd *                   next   = nullptr;
  VkQueryPool             query  = nullptr;
  VkPipelineStageFlagBits stages = VK_PIPELINE_STAGE_NONE;
  u32                     index  = 0;
};

struct alignas(8) BeginStatistics
{
  Type const  type  = Type::BeginStatistics;
  Cmd *       next  = nullptr;
  VkQueryPool query = nullptr;
  u32         index = 0;
};

struct alignas(8) EndStatistics
{
  Type const  type  = Type::EndStatistics;
  Cmd *       next  = nullptr;
  VkQueryPool query = nullptr;
  u32         index = 0;
};

struct alignas(8) BeginDebugMarker
{
  Type const                 type = Type::BeginDebugMarker;
  Cmd *                      next = nullptr;
  VkDebugMarkerMarkerInfoEXT info = {};
};

struct alignas(8) EndDebugMarker
{
  Type const type = Type::EndDebugMarker;
  Cmd *      next = nullptr;
};

struct alignas(8) FillBuffer
{
  Type const type  = Type::FillBuffer;
  Cmd *      next  = nullptr;
  VkBuffer   dst   = nullptr;
  Slice64    range = {};
  u32        data  = 0;
};

struct alignas(8) CopyBuffer
{
  Type const               type   = Type::CopyBuffer;
  Cmd *                    next   = nullptr;
  VkBuffer                 src    = nullptr;
  VkBuffer                 dst    = nullptr;
  Span<VkBufferCopy const> copies = {};
};

struct alignas(8) UpdateBuffer
{
  Type const     type       = Type::UpdateBuffer;
  Cmd *          next       = nullptr;
  Span<u8 const> src        = {};
  u64            dst_offset = 0;
  VkBuffer       dst        = nullptr;
};

struct alignas(8) ClearColorImage
{
  Type const                          type       = Type::ClearColorImage;
  Cmd *                               next       = nullptr;
  VkImage                             dst        = nullptr;
  VkImageLayout                       dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkClearColorValue                   value      = {};
  Span<VkImageSubresourceRange const> ranges     = {};
};

struct alignas(8) ClearDepthStencilImage
{
  Type const                          type       = Type::ClearDepthStencilImage;
  Cmd *                               next       = nullptr;
  VkImage                             dst        = nullptr;
  VkImageLayout                       dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkClearDepthStencilValue            value      = {};
  Span<VkImageSubresourceRange const> ranges     = {};
};

struct alignas(8) CopyImage
{
  Type const              type       = Type::CopyImage;
  Cmd *                   next       = nullptr;
  VkImage                 src        = nullptr;
  VkImageLayout           src_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImage                 dst        = nullptr;
  VkImageLayout           dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  Span<VkImageCopy const> copies     = {};
};

struct alignas(8) CopyBufferToImage
{
  Type const                    type       = Type::CopyBufferToImage;
  Cmd *                         next       = nullptr;
  VkBuffer                      src        = nullptr;
  VkImage                       dst        = nullptr;
  VkImageLayout                 dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  Span<VkBufferImageCopy const> copies     = {};
};

struct alignas(8) BlitImage
{
  Type const              type       = Type::BlitImage;
  Cmd *                   next       = nullptr;
  VkImage                 src        = nullptr;
  VkImageLayout           src_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImage                 dst        = nullptr;
  VkImageLayout           dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  Span<VkImageBlit const> blits      = {};
  VkFilter                filter     = VK_FILTER_LINEAR;
};

struct alignas(8) ResolveImage
{
  Type const                 type       = Type::ResolveImage;
  Cmd *                      next       = nullptr;
  VkImage                    src        = nullptr;
  VkImageLayout              src_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImage                    dst        = nullptr;
  VkImageLayout              dst_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  Span<VkImageResolve const> resolves   = {};
};

struct alignas(8) BeginRendering
{
  Type const      type = Type::BeginRendering;
  Cmd *           next = nullptr;
  VkRenderingInfo info = {};
};

struct alignas(8) EndRendering
{
  Type const type = Type::EndRendering;
  Cmd *      next = nullptr;
};

struct alignas(8) BindPipeline
{
  Type const          type       = Type::BindPipeline;
  Cmd *               next       = nullptr;
  VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
  VkPipeline          pipeline   = nullptr;
};

struct alignas(8) BindDescriptorSets
{
  Type const                  type            = Type::BindDescriptorSets;
  Cmd *                       next            = nullptr;
  VkPipelineBindPoint         bind_point      = VK_PIPELINE_BIND_POINT_MAX_ENUM;
  VkPipelineLayout            layout          = nullptr;
  Span<VkDescriptorSet const> sets            = {};
  Span<u32 const>             dynamic_offsets = {};
};

struct alignas(8) PushConstants
{
  Type const       type      = Type::PushConstants;
  Cmd *            next      = nullptr;
  VkPipelineLayout layout    = nullptr;
  Span<u8 const>   constants = {};
};

struct alignas(8) Dispatch
{
  Type const type        = Type::Dispatch;
  Cmd *      next        = nullptr;
  u32x3      group_count = {};
};

struct alignas(8) DispatchIndirect
{
  Type const type   = Type::DispatchIndirect;
  Cmd *      next   = nullptr;
  VkBuffer   buffer = nullptr;
  u64        offset = 0;
};

struct alignas(8) SetGraphicsState
{
  Type const         type  = Type::SetGraphicsState;
  Cmd *              next  = nullptr;
  gpu::GraphicsState state = {};
};

struct alignas(8) BindVertexBuffers
{
  Type const           type    = Type::BindVertexBuffers;
  Cmd *                next    = nullptr;
  Span<VkBuffer const> buffers = {};
  Span<u64 const>      offsets = {};
};

struct alignas(8) BindIndexBuffer
{
  Type const  type       = Type::BindIndexBuffer;
  Cmd *       next       = nullptr;
  VkBuffer    buffer     = nullptr;
  u64         offset     = 0;
  VkIndexType index_type = VK_INDEX_TYPE_UINT32;
};

struct alignas(8) Draw
{
  Type const type      = Type::Draw;
  Cmd *      next      = nullptr;
  Slice32    vertices  = {};
  Slice32    instances = {};
};

struct alignas(8) DrawIndexed
{
  Type const type          = Type::DrawIndexed;
  Cmd *      next          = nullptr;
  Slice32    indices       = {};
  Slice32    instances     = {};
  i32        vertex_offset = 0;
};

struct alignas(8) DrawIndirect
{
  Type const type       = Type::DrawIndirect;
  Cmd *      next       = nullptr;
  VkBuffer   buffer     = nullptr;
  u64        offset     = 0;
  u32        draw_count = 0;
  u32        stride     = 0;
};

struct alignas(8) DrawIndexedIndirect
{
  Type const type       = Type::DrawIndexedIndirect;
  Cmd *      next       = nullptr;
  VkBuffer   buffer     = nullptr;
  u64        offset     = 0;
  u32        draw_count = 0;
  u32        stride     = 0;
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

struct MemAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
};

struct BufferMemState
{
  /// @brief Alias element
  u32 element = 0;
};

struct ImageMemState
{
  /// @brief Alias element
  u32 element = 0;

  /// @brief Current image layout
  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

using MemState = Enum<None, BufferMemState, ImageMemState>;

struct Hazard
{
  HazardType type     = HazardType::None;
  MemAccess  latest   = {};
  MemAccess  previous = {};
  MemState   state    = none;
};

struct HazardBarriers
{
  struct Stage
  {
    VkPipelineStageFlags src = VK_PIPELINE_STAGE_NONE;
    VkPipelineStageFlags dst = VK_PIPELINE_STAGE_NONE;
  };

  Vec<Tuple<VkPipelineStageFlags, VkPipelineStageFlags, VkBufferMemoryBarrier>>
    buffers_;

  Vec<Tuple<VkPipelineStageFlags, VkPipelineStageFlags, VkMemoryBarrier,
            VkBufferMemoryBarrier>>
    mem_buffers_;

  Vec<Tuple<VkPipelineStageFlags, VkPipelineStageFlags, VkImageMemoryBarrier>>
    images_;

  Vec<Tuple<VkPipelineStageFlags, VkPipelineStageFlags, VkMemoryBarrier,
            VkImageMemoryBarrier>>
    mem_images_;

  HazardBarriers(Allocator allocator) :
    buffers_{allocator},
    mem_buffers_{allocator},
    images_{allocator},
    mem_images_{allocator}
  {
  }

  HazardBarriers(HazardBarriers const &)             = delete;
  HazardBarriers(HazardBarriers &&)                  = delete;
  HazardBarriers & operator=(HazardBarriers const &) = delete;
  HazardBarriers & operator=(HazardBarriers &&)      = delete;
  ~HazardBarriers()                                  = default;

  void clear();

  void buffer(VkPipelineStageFlags src, VkPipelineStageFlags dst,
              VkBufferMemoryBarrier const & buffer);

  void buffer(VkPipelineStageFlags src, VkPipelineStageFlags dst,
              VkMemoryBarrier const &       mem,
              VkBufferMemoryBarrier const & buffer);

  void image(VkPipelineStageFlags src, VkPipelineStageFlags dst,
             VkImageMemoryBarrier const & image);

  void image(VkPipelineStageFlags src, VkPipelineStageFlags dst,
             VkMemoryBarrier const & mem, VkImageMemoryBarrier const & image);

  void barrier(IImage const & image, MemAccess old_access,
               VkImageLayout old_layout, MemAccess new_access,
               VkImageLayout new_layout);

  void discard_barrier(IImage const & image, MemAccess old_access,
                       MemAccess new_access, VkImageLayout new_layout);

  void barrier(IBuffer const & buffer, MemAccess old_access,
               MemAccess new_access);

  void discard_barrier(IBuffer const & buffer, MemAccess old_access,
                       MemAccess new_access);
};

/// @brief Global synchronization state
struct DeviceResourceStates
{
  CoreSparseMap<Vec<u32>,      // id-to-index map
                Vec<Hazard>    // memory state
                >
    alias_;

  CoreSparseMap<Vec<u32>    // id-to-index map
                >
    descriptor_sets_;

  ReadWriteLock<IFutex> lock_;

  DeviceResourceStates(Allocator allocator) :
    alias_{allocator},
    descriptor_sets_{allocator},
    lock_{}
  {
  }

  DeviceResourceStates(DeviceResourceStates const &)             = delete;
  DeviceResourceStates(DeviceResourceStates &&)                  = delete;
  DeviceResourceStates & operator=(DeviceResourceStates const &) = delete;
  DeviceResourceStates & operator=(DeviceResourceStates &&)      = delete;
  ~DeviceResourceStates()                                        = default;
};

/// @brief Encoder-local synchronization state
struct EncoderResourceStates
{
  static constexpr VkPipelineStageFlags GRAPHICS_DESCRIPTOR_STAGES =
    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

  static constexpr VkPipelineStageFlags COMPUTE_DESCRIPTOR_STAGES =
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

  CoreSparseMap<
    Vec<u32>,       // id-to-index map
    Vec<Hazard>,    // memory hazard so far
    BitVec<u64>,    // was the resource accessed?
    Vec<u32>        // the last pass the memory was accessed: initially U32_MAX
    >
    alias_;

  CoreSparseMap<
    Vec<u32>,    // id-to-index map
    Vec<
      u32>    // the last pass the descriptor set was accessed: initially U32_MAX
    >
    descriptor_sets_;

  EncoderResourceStates(Allocator allocator) :
    alias_{allocator},
    descriptor_sets_{allocator}
  {
  }

  EncoderResourceStates(DeviceResourceStates const &)              = delete;
  EncoderResourceStates(EncoderResourceStates &&)                  = delete;
  EncoderResourceStates & operator=(EncoderResourceStates const &) = delete;
  EncoderResourceStates & operator=(EncoderResourceStates &&)      = delete;
  ~EncoderResourceStates()                                         = default;

  /// @param image image to sync
  /// @param access merged image state for the pass
  /// @param pass the pass temporal id
  /// @param barriers destination to issue barriers
  void access(IImage const & image, MemAccess const & access,
              VkImageLayout layout, u32 pass, HazardBarriers & barriers);

  void access(IImageView const & image, MemAccess const & access,
              VkImageLayout layout, u32 pass, HazardBarriers & barriers);

  /// @param buffer buffer to sync
  /// @param access merged image state for the pass
  /// @param pass the pass temporal id
  /// @param barriers destination to issue barriers
  void access(IBuffer const & buffer, MemAccess const & access, u32 pass,
              HazardBarriers & barriers);

  /// @param set descriptor set to sync
  /// @param access merged image state for the pass
  /// @param pass the pass temporal id
  /// @param barriers destination to issue barriers
  void access(IDescriptorSet const & set, u32 pass,
              VkPipelineStageFlags shader_stages, HazardBarriers & barriers);

  void rebuild(DeviceResourceStates const & upstream);

  void commit(DeviceResourceStates & upstream);
};

struct CommandTracker
{
  struct Entry
  {
    u32 commands        = 0;
    u32 buffers         = 0;
    u32 images          = 0;
    u32 descriptor_sets = 0;
  };

  Vec<Tuple<Buffer, VkPipelineStageFlags, VkAccessFlags>> buffers_;
  Vec<Tuple<Image, VkPipelineStageFlags, VkAccessFlags, VkImageLayout>> images_;

  Vec<Tuple<DescriptorSet, VkShaderStageFlags>> descriptor_sets_;
  Vec<Entry>                                    passes_;
  cmd::Cmd *                                    first_cmd_;
  cmd::Cmd *                                    last_cmd_;

  CommandTracker(Allocator allocator) :
    buffers_{allocator},
    images_{allocator},
    descriptor_sets_{allocator},
    passes_{allocator},
    first_cmd_{nullptr},
    last_cmd_{nullptr}
  {
  }

  CommandTracker(CommandTracker const &)             = delete;
  CommandTracker(CommandTracker &&)                  = delete;
  CommandTracker & operator=(CommandTracker const &) = delete;
  CommandTracker & operator=(CommandTracker &&)      = delete;
  ~CommandTracker()                                  = default;

  u32 begin_pass();

  void command(cmd::Cmd * cmd);

  void end_pass();

  void track(Buffer buffer, VkPipelineStageFlags stages, VkAccessFlags access);

  void track(Image image, VkPipelineStageFlags stages, VkAccessFlags access,
             VkImageLayout layout);

  void track(ImageView image, VkPipelineStageFlags stages, VkAccessFlags access,
             VkImageLayout layout);

  void track(DescriptorSet set, VkShaderStageFlags stages);

  void reset();
};

enum class CommandBufferState : u8
{
  Reset     = 0,
  Recording = 1,
  Recorded  = 2,
  Submitted = 3
};

struct PassContext
{
  Option<IGraphicsPipeline &> graphics_pipeline = none;

  Option<IComputePipeline &> compute_pipeline = none;

  SmallVec<gpu::RenderingAttachment, 8, 0> color_attachments = {};

  Option<gpu::RenderingAttachment> depth_attachment = none;

  Option<gpu::RenderingAttachment> stencil_attachment = none;

  SmallVec<DescriptorSet, 8, 0> descriptor_sets = {};

  SmallVec<Buffer, 8, 0> vertex_buffers = {};

  Option<IBuffer &> index_buffer = none;

  gpu::IndexType index_type = gpu::IndexType::U16;

  u64 index_buffer_offset = 0;

  bool has_graphics_state = false;

  PassContext(Allocator allocator) :
    color_attachments{allocator},
    descriptor_sets{allocator},
    vertex_buffers{allocator}
  {
  }

  PassContext(PassContext const &)             = delete;
  PassContext(PassContext &&)                  = delete;
  PassContext & operator=(PassContext const &) = delete;
  PassContext & operator=(PassContext &&)      = delete;
  ~PassContext()                               = default;

  void clear();
};

struct ICommandEncoder final : gpu::ICommandEncoder
{
  enum class Pass : u8
  {
    None    = 0,
    Compute = 1,
    Render  = 2
  };

  Device               dev_;
  ArenaPool            arena_;
  Status               status_;
  CommandBufferState   state_;
  Pass                 pass_;
  CommandTracker       tracker_;
  PassContext          ctx_;
  Option<ISwapchain &> swapchain_;

  ICommandEncoder(IDevice & dev, Allocator allocator) :
    dev_{&dev},
    arena_{allocator},
    status_{Status::Success},
    state_{CommandBufferState::Reset},
    pass_{Pass::None},
    tracker_{allocator},
    ctx_{allocator},
    swapchain_{none}
  {
  }

  ICommandEncoder(ICommandEncoder const &)             = delete;
  ICommandEncoder(ICommandEncoder &&)                  = delete;
  ICommandEncoder & operator=(ICommandEncoder const &) = delete;
  ICommandEncoder & operator=(ICommandEncoder &&)      = delete;
  ~ICommandEncoder()                                   = default;

  template <typename CmdImpl>
  CmdImpl * push(CmdImpl const & cmd)
  {
    CmdImpl * p_cmd;
    if (!arena_.nalloc(1, p_cmd))
    {
      return nullptr;
    }

    new (p_cmd) CmdImpl{cmd};

    tracker_.command((cmd::Cmd *) p_cmd);

    return p_cmd;
  }

  virtual void begin() override;

  virtual Result<Void, Status> end() override;

  virtual void reset() override;

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

  virtual void fill_buffer(gpu::Buffer dst, Slice64 range, u32 data) override;

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

  virtual void present(gpu::Swapchain swapchain) override;
};

struct ICommandBuffer final : gpu::ICommandBuffer
{
  Device                dev_;
  VkCommandPool         vk_pool_;
  VkCommandBuffer       vk_;
  Option<ISwapchain &>  swapchain_;
  Status                status_;
  CommandBufferState    state_;
  EncoderResourceStates resource_states_;
  ArenaPool             arena_;

  ICommandBuffer(IDevice & dev, VkCommandPool vk_pool,
                 VkCommandBuffer vk_buffer, Allocator allocator) :
    dev_{&dev},
    vk_pool_{vk_pool},
    vk_{vk_buffer},
    swapchain_{none},
    status_{Status::Success},
    state_{CommandBufferState::Reset},
    resource_states_{allocator},
    arena_{allocator}
  {
  }

  ICommandBuffer(ICommandBuffer const &)             = delete;
  ICommandBuffer(ICommandBuffer &&)                  = delete;
  ICommandBuffer & operator=(ICommandBuffer const &) = delete;
  ICommandBuffer & operator=(ICommandBuffer &&)      = delete;
  ~ICommandBuffer()                                  = default;

  virtual void begin() override;

  virtual Result<Void, Status> end() override;

  virtual void reset() override;

  virtual void record(gpu::CommandEncoder encoder) override;

  void commit_resource_states();
};

struct IQueueScope
{
  u64                         buffering_;
  u64                         frame_;
  u64                         ring_index_;
  SmallVec<VkSemaphore, 4, 0> submit_semaphores_;
  SmallVec<VkFence, 4, 0>     submit_fences_;

  IQueueScope(u64 buffering, SmallVec<VkSemaphore, 4, 0> submit_semaphores,
              SmallVec<VkFence, 4, 0> submit_fences) :
    buffering_{buffering},
    frame_{0},
    ring_index_{0},
    submit_semaphores_{std::move(submit_semaphores)},
    submit_fences_{std::move(submit_fences)}
  {
  }

  IQueueScope(IQueueScope const &)             = delete;
  IQueueScope(IQueueScope &&)                  = delete;
  IQueueScope & operator=(IQueueScope const &) = delete;
  IQueueScope & operator=(IQueueScope &&)      = delete;
  ~IQueueScope()                               = default;
};

struct IDevice final : gpu::IDevice
{
  Allocator            allocator_;
  Instance             instance_;
  IPhysicalDevice      phy_;
  DeviceTable          table_;
  VmaVulkanFunctions   vma_table_;
  VkDevice             vk_dev_;
  u32                  queue_family_;
  VkQueue              vk_queue_;
  VmaAllocator         vma_allocator_;
  DeviceResourceStates resource_states_;

  IDevice(Allocator allocator, IInstance & instance, IPhysicalDevice phy_dev,
          DeviceTable vk_table, VmaVulkanFunctions vma_table, VkDevice vk_dev,
          u32 queue_family, VkQueue vk_queue, VmaAllocator vma_allocator) :
    allocator_{allocator},
    instance_{&instance},
    phy_{phy_dev},
    table_{vk_table},
    vma_table_{vma_table},
    vk_dev_{vk_dev},
    queue_family_{queue_family},
    vk_queue_{vk_queue},
    vma_allocator_{vma_allocator},
    resource_states_{allocator}
  {
  }

  IDevice(IDevice const &)             = delete;
  IDevice(IDevice &&)                  = delete;
  IDevice & operator=(IDevice const &) = delete;
  IDevice & operator=(IDevice &&)      = delete;
  ~IDevice()                           = default;

  void set_resource_name(Str label, void const * resource, VkObjectType type,
                         VkDebugReportObjectTypeEXT debug_type,
                         Allocator                  scratch);

  AliasId allocate_alias_id();

  void release_alias_id(AliasId id);

  DescriptorSetId allocate_descriptor_set_id();

  void release_descriptor_set_id(DescriptorSetId id);

  void uninit();

  virtual Result<gpu::Buffer, Status>
    create_buffer(gpu::BufferInfo const & info) override;

  virtual Result<gpu::BufferView, Status>
    create_buffer_view(gpu::BufferViewInfo const & info) override;

  virtual Result<gpu::Image, Status>
    create_image(gpu::ImageInfo const & info) override;

  virtual Result<gpu::ImageView, Status>
    create_image_view(gpu::ImageViewInfo const & info) override;

  virtual Result<gpu::Alias, Status>
    create_alias(gpu::AliasInfo const & info) override;

  Result<gpu::Alias, Status> create_shim_alias(gpu::AliasInfo const & info);

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
    create_swapchain(gpu::SwapchainInfo const & info) override;

  Result<Void, Status> recreate_swapchain(Swapchain swapchain);

  virtual Result<gpu::TimestampQuery, Status>
    create_timestamp_query(gpu::TimestampQueryInfo const & info) override;

  virtual Result<gpu::StatisticsQuery, Status>
    create_statistics_query(gpu::StatisticsQueryInfo const & info) override;

  virtual Result<gpu::CommandEncoder, Status>
    create_command_encoder(gpu::CommandEncoderInfo const & info) override;

  virtual Result<gpu::CommandBuffer, Status>
    create_command_buffer(gpu::CommandBufferInfo const & info) override;

  virtual Result<gpu::QueueScope, Status>
    create_queue_scope(gpu::QueueScopeInfo const & info) override;

  virtual void uninit(gpu::Buffer buffer) override;

  virtual void uninit(gpu::BufferView buffer_view) override;

  virtual void uninit(gpu::Image image) override;

  virtual void uninit(gpu::ImageView image_view) override;

  virtual void uninit(gpu::Alias alias) override;

  virtual void uninit(gpu::Sampler sampler) override;

  virtual void uninit(gpu::Shader shader) override;

  virtual void uninit(gpu::DescriptorSetLayout layout) override;

  virtual void uninit(gpu::DescriptorSet set) override;

  virtual void uninit(gpu::PipelineCache cache) override;

  virtual void uninit(gpu::ComputePipeline pipeline) override;

  virtual void uninit(gpu::GraphicsPipeline pipeline) override;

  void release(ISwapchain & swapchain);

  virtual void uninit(gpu::Swapchain swapchain) override;

  virtual void uninit(gpu::TimestampQuery query) override;

  virtual void uninit(gpu::StatisticsQuery query) override;

  virtual void uninit(gpu::CommandEncoder encoder) override;

  virtual void uninit(gpu::CommandBuffer buffer) override;

  virtual void uninit(gpu::QueueScope scope) override;

  virtual gpu::DeviceProperties get_properties() override;

  virtual Result<gpu::FormatProperties, Status>
    get_format_properties(gpu::Format format) override;

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

  virtual gpu::QueueScopeState
    get_queue_scope_state(gpu::QueueScope scope) override;

  virtual Result<Void, Status> await_idle() override;

  virtual Result<Void, Status> await_queue_idle() override;

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
    get_timestamp_query_result(gpu::TimestampQuery query, u32 first,
                               Span<u64> timestamps) override;

  virtual Result<Void, Status> get_statistics_query_result(
    gpu::StatisticsQuery query, u32 first,
    Span<gpu::PipelineStatistics> statistics) override;

  virtual Result<Void, Status> acquire_next(gpu::Swapchain swapchain) override;

  virtual Result<u64, Status> submit(gpu::CommandBuffer buffer,
                                     gpu::QueueScope    scope) override;

  virtual Result<Void, Status>
    await_queue_scope_idle(gpu::QueueScope scope, nanoseconds timeout) override;

  virtual Result<Void, Status>
    await_queue_scope_frame(gpu::QueueScope scope, u64 frame,
                            nanoseconds timeout) override;
};

}    // namespace vk
}    // namespace ash
