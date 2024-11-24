/// SPDX-License-Identifier: MIT
#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1000000

#include "ashura/gpu/gpu.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/vec.h"
#include "vk_mem_alloc.h"
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

using gpu::Status;

constexpr char const *ENGINE_NAME    = "Ash";
constexpr u32         ENGINE_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);
constexpr char const *CLIENT_NAME    = "Ash Client";
constexpr u32         CLIENT_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);

constexpr u32 MAX_MEMORY_HEAP_PROPERTIES = 32;
constexpr u32 MAX_MEMORY_HEAPS           = 16;
constexpr u8  NUM_DESCRIPTOR_TYPES       = 11;

typedef VkSampler             Sampler;
typedef VkShaderModule        Shader;
typedef VkPipelineCache       PipelineCache;
typedef VkSurfaceKHR          Surface;
typedef VkQueryPool           TimestampQuery;
typedef VkQueryPool           StatisticsQuery;
typedef struct Instance       Instance;
typedef struct Device         Device;
typedef struct CommandEncoder CommandEncoder;

struct InstanceTable
{
  PFN_vkCreateInstance           CreateInstance           = nullptr;
  PFN_vkDestroyInstance          DestroyInstance          = nullptr;
  PFN_vkDestroySurfaceKHR        DestroySurfaceKHR        = nullptr;
  PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices = nullptr;
  PFN_vkGetInstanceProcAddr      GetInstanceProcAddr      = nullptr;
  PFN_vkGetDeviceProcAddr        GetDeviceProcAddr        = nullptr;

  PFN_vkCreateDevice                       CreateDevice = nullptr;
  PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties =
      nullptr;
  PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties = nullptr;
  PFN_vkGetPhysicalDeviceFeatures      GetPhysicalDeviceFeatures      = nullptr;
  PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties =
      nullptr;
  PFN_vkGetPhysicalDeviceImageFormatProperties
      GetPhysicalDeviceImageFormatProperties = nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties =
      nullptr;
  PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties = nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties
      GetPhysicalDeviceQueueFamilyProperties = nullptr;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties
      GetPhysicalDeviceSparseImageFormatProperties = nullptr;

  PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR =
      nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
      GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR =
      nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
      GetPhysicalDeviceSurfacePresentModesKHR = nullptr;

  PFN_vkCreateDebugUtilsMessengerEXT  CreateDebugUtilsMessengerEXT  = nullptr;
  PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;
  PFN_vkSetDebugUtilsObjectNameEXT    SetDebugUtilsObjectNameEXT    = nullptr;
};

struct DeviceTable
{
  // DEVICE OBJECT FUNCTIONS
  PFN_vkAllocateCommandBuffers       AllocateCommandBuffers       = nullptr;
  PFN_vkAllocateDescriptorSets       AllocateDescriptorSets       = nullptr;
  PFN_vkAllocateMemory               AllocateMemory               = nullptr;
  PFN_vkBindBufferMemory             BindBufferMemory             = nullptr;
  PFN_vkBindImageMemory              BindImageMemory              = nullptr;
  PFN_vkCreateBuffer                 CreateBuffer                 = nullptr;
  PFN_vkCreateBufferView             CreateBufferView             = nullptr;
  PFN_vkCreateCommandPool            CreateCommandPool            = nullptr;
  PFN_vkCreateComputePipelines       CreateComputePipelines       = nullptr;
  PFN_vkCreateDescriptorPool         CreateDescriptorPool         = nullptr;
  PFN_vkCreateDescriptorSetLayout    CreateDescriptorSetLayout    = nullptr;
  PFN_vkCreateEvent                  CreateEvent                  = nullptr;
  PFN_vkCreateFence                  CreateFence                  = nullptr;
  PFN_vkCreateGraphicsPipelines      CreateGraphicsPipelines      = nullptr;
  PFN_vkCreateImage                  CreateImage                  = nullptr;
  PFN_vkCreateImageView              CreateImageView              = nullptr;
  PFN_vkCreatePipelineCache          CreatePipelineCache          = nullptr;
  PFN_vkCreatePipelineLayout         CreatePipelineLayout         = nullptr;
  PFN_vkCreateQueryPool              CreateQueryPool              = nullptr;
  PFN_vkCreateSampler                CreateSampler                = nullptr;
  PFN_vkCreateSemaphore              CreateSemaphore              = nullptr;
  PFN_vkCreateShaderModule           CreateShaderModule           = nullptr;
  PFN_vkDestroyBuffer                DestroyBuffer                = nullptr;
  PFN_vkDestroyBufferView            DestroyBufferView            = nullptr;
  PFN_vkDestroyCommandPool           DestroyCommandPool           = nullptr;
  PFN_vkDestroyDescriptorPool        DestroyDescriptorPool        = nullptr;
  PFN_vkDestroyDescriptorSetLayout   DestroyDescriptorSetLayout   = nullptr;
  PFN_vkDestroyDevice                DestroyDevice                = nullptr;
  PFN_vkDestroyEvent                 DestroyEvent                 = nullptr;
  PFN_vkDestroyFence                 DestroyFence                 = nullptr;
  PFN_vkDestroyImage                 DestroyImage                 = nullptr;
  PFN_vkDestroyImageView             DestroyImageView             = nullptr;
  PFN_vkDestroyPipeline              DestroyPipeline              = nullptr;
  PFN_vkDestroyPipelineCache         DestroyPipelineCache         = nullptr;
  PFN_vkDestroyPipelineLayout        DestroyPipelineLayout        = nullptr;
  PFN_vkDestroyQueryPool             DestroyQueryPool             = nullptr;
  PFN_vkDestroySampler               DestroySampler               = nullptr;
  PFN_vkDestroySemaphore             DestroySemaphore             = nullptr;
  PFN_vkDestroyShaderModule          DestroyShaderModule          = nullptr;
  PFN_vkDeviceWaitIdle               DeviceWaitIdle               = nullptr;
  PFN_vkFlushMappedMemoryRanges      FlushMappedMemoryRanges      = nullptr;
  PFN_vkFreeCommandBuffers           FreeCommandBuffers           = nullptr;
  PFN_vkFreeDescriptorSets           FreeDescriptorSets           = nullptr;
  PFN_vkFreeMemory                   FreeMemory                   = nullptr;
  PFN_vkGetBufferMemoryRequirements  GetBufferMemoryRequirements  = nullptr;
  PFN_vkGetDeviceMemoryCommitment    GetDeviceMemoryCommitment    = nullptr;
  PFN_vkGetDeviceQueue               GetDeviceQueue               = nullptr;
  PFN_vkGetEventStatus               GetEventStatus               = nullptr;
  PFN_vkGetFenceStatus               GetFenceStatus               = nullptr;
  PFN_vkGetImageMemoryRequirements   GetImageMemoryRequirements   = nullptr;
  PFN_vkGetImageSubresourceLayout    GetImageSubresourceLayout    = nullptr;
  PFN_vkGetPipelineCacheData         GetPipelineCacheData         = nullptr;
  PFN_vkGetQueryPoolResults          GetQueryPoolResults          = nullptr;
  PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges = nullptr;
  PFN_vkMapMemory                    MapMemory                    = nullptr;
  PFN_vkMergePipelineCaches          MergePipelineCaches          = nullptr;
  PFN_vkResetCommandPool             ResetCommandPool             = nullptr;
  PFN_vkResetDescriptorPool          ResetDescriptorPool          = nullptr;
  PFN_vkResetEvent                   ResetEvent                   = nullptr;
  PFN_vkResetFences                  ResetFences                  = nullptr;
  PFN_vkSetEvent                     SetEvent                     = nullptr;
  PFN_vkUpdateDescriptorSets         UpdateDescriptorSets         = nullptr;
  PFN_vkUnmapMemory                  UnmapMemory                  = nullptr;
  PFN_vkWaitForFences                WaitForFences                = nullptr;

  PFN_vkQueueSubmit   QueueSubmit   = nullptr;
  PFN_vkQueueWaitIdle QueueWaitIdle = nullptr;

  // COMMAND BUFFER OBJECT FUNCTIONS
  PFN_vkBeginCommandBuffer        BeginCommandBuffer        = nullptr;
  PFN_vkCmdBeginQuery             CmdBeginQuery             = nullptr;
  PFN_vkCmdBindDescriptorSets     CmdBindDescriptorSets     = nullptr;
  PFN_vkCmdBindIndexBuffer        CmdBindIndexBuffer        = nullptr;
  PFN_vkCmdBindPipeline           CmdBindPipeline           = nullptr;
  PFN_vkCmdBindVertexBuffers      CmdBindVertexBuffers      = nullptr;
  PFN_vkCmdBlitImage              CmdBlitImage              = nullptr;
  PFN_vkCmdClearAttachments       CmdClearAttachments       = nullptr;
  PFN_vkCmdClearColorImage        CmdClearColorImage        = nullptr;
  PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage = nullptr;
  PFN_vkCmdCopyBuffer             CmdCopyBuffer             = nullptr;
  PFN_vkCmdCopyBufferToImage      CmdCopyBufferToImage      = nullptr;
  PFN_vkCmdCopyImage              CmdCopyImage              = nullptr;
  PFN_vkCmdCopyImageToBuffer      CmdCopyImageToBuffer      = nullptr;
  PFN_vkCmdCopyQueryPoolResults   CmdCopyQueryPoolResults   = nullptr;
  PFN_vkCmdDispatch               CmdDispatch               = nullptr;
  PFN_vkCmdDispatchIndirect       CmdDispatchIndirect       = nullptr;
  PFN_vkCmdDraw                   CmdDraw                   = nullptr;
  PFN_vkCmdDrawIndexed            CmdDrawIndexed            = nullptr;
  PFN_vkCmdDrawIndexedIndirect    CmdDrawIndexedIndirect    = nullptr;
  PFN_vkCmdDrawIndirect           CmdDrawIndirect           = nullptr;
  PFN_vkCmdEndQuery               CmdEndQuery               = nullptr;
  PFN_vkCmdFillBuffer             CmdFillBuffer             = nullptr;
  PFN_vkCmdPipelineBarrier        CmdPipelineBarrier        = nullptr;
  PFN_vkCmdPushConstants          CmdPushConstants          = nullptr;
  PFN_vkCmdResetEvent             CmdResetEvent             = nullptr;
  PFN_vkCmdResetQueryPool         CmdResetQueryPool         = nullptr;
  PFN_vkCmdResolveImage           CmdResolveImage           = nullptr;
  PFN_vkCmdSetBlendConstants      CmdSetBlendConstants      = nullptr;
  PFN_vkCmdSetDepthBias           CmdSetDepthBias           = nullptr;
  PFN_vkCmdSetDepthBounds         CmdSetDepthBounds         = nullptr;
  PFN_vkCmdSetEvent               CmdSetEvent               = nullptr;
  PFN_vkCmdSetLineWidth           CmdSetLineWidth           = nullptr;
  PFN_vkCmdSetScissor             CmdSetScissor             = nullptr;
  PFN_vkCmdSetStencilCompareMask  CmdSetStencilCompareMask  = nullptr;
  PFN_vkCmdSetStencilReference    CmdSetStencilReference    = nullptr;
  PFN_vkCmdSetStencilWriteMask    CmdSetStencilWriteMask    = nullptr;
  PFN_vkCmdSetViewport            CmdSetViewport            = nullptr;
  PFN_vkCmdUpdateBuffer           CmdUpdateBuffer           = nullptr;
  PFN_vkCmdWaitEvents             CmdWaitEvents             = nullptr;
  PFN_vkCmdWriteTimestamp         CmdWriteTimestamp         = nullptr;
  PFN_vkEndCommandBuffer          EndCommandBuffer          = nullptr;
  PFN_vkResetCommandBuffer        ResetCommandBuffer        = nullptr;

  PFN_vkCmdSetStencilOpEXT             CmdSetStencilOpEXT             = nullptr;
  PFN_vkCmdSetStencilTestEnableEXT     CmdSetStencilTestEnableEXT     = nullptr;
  PFN_vkCmdSetCullModeEXT              CmdSetCullModeEXT              = nullptr;
  PFN_vkCmdSetFrontFaceEXT             CmdSetFrontFaceEXT             = nullptr;
  PFN_vkCmdSetPrimitiveTopologyEXT     CmdSetPrimitiveTopologyEXT     = nullptr;
  PFN_vkCmdSetDepthBoundsTestEnableEXT CmdSetDepthBoundsTestEnableEXT = nullptr;
  PFN_vkCmdSetDepthCompareOpEXT        CmdSetDepthCompareOpEXT        = nullptr;
  PFN_vkCmdSetDepthTestEnableEXT       CmdSetDepthTestEnableEXT       = nullptr;
  PFN_vkCmdSetDepthWriteEnableEXT      CmdSetDepthWriteEnableEXT      = nullptr;

  PFN_vkCmdBeginRenderingKHR CmdBeginRenderingKHR = nullptr;
  PFN_vkCmdEndRenderingKHR   CmdEndRenderingKHR   = nullptr;

  PFN_vkCreateSwapchainKHR    CreateSwapchainKHR    = nullptr;
  PFN_vkDestroySwapchainKHR   DestroySwapchainKHR   = nullptr;
  PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = nullptr;
  PFN_vkAcquireNextImageKHR   AcquireNextImageKHR   = nullptr;
  PFN_vkQueuePresentKHR       QueuePresentKHR       = nullptr;

  PFN_vkDebugMarkerSetObjectTagEXT  DebugMarkerSetObjectTagEXT  = nullptr;
  PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT = nullptr;

  PFN_vkCmdDebugMarkerBeginEXT  CmdDebugMarkerBeginEXT  = nullptr;
  PFN_vkCmdDebugMarkerEndEXT    CmdDebugMarkerEndEXT    = nullptr;
  PFN_vkCmdDebugMarkerInsertEXT CmdDebugMarkerInsertEXT = nullptr;
};

struct BufferAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
};

struct ImageAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
  VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

enum class AccessSequence : u8
{
  None           = 0,
  Reads          = 1,
  Write          = 2,
  ReadAfterWrite = 3
};

struct BufferState
{
  BufferAccess   access[2] = {};
  AccessSequence sequence  = AccessSequence::None;
};

struct ImageState
{
  ImageAccess    access[2] = {};
  AccessSequence sequence  = AccessSequence::None;
};

struct Buffer
{
  gpu::BufferInfo info           = {};
  VkBuffer        vk_buffer      = nullptr;
  VmaAllocation   vma_allocation = nullptr;
  BufferState     state          = {};
};

struct BufferView
{
  gpu::BufferViewInfo info    = {};
  VkBufferView        vk_view = nullptr;
};

constexpr u32 COLOR_ASPECT_IDX   = 0;
constexpr u32 DEPTH_ASPECT_IDX   = 0;
constexpr u32 STENCIL_ASPECT_IDX = 1;

struct Image
{
  gpu::ImageInfo    info                = {};
  bool              is_swapchain_image  = false;
  VkImage           vk_image            = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  ImageState        states[2]           = {};
  u32               num_aspects         = 0;
};

struct ImageView
{
  gpu::ImageViewInfo info    = {};
  VkImageView        vk_view = nullptr;
};

struct DescriptorSetLayout
{
  gpu::DescriptorBindingInfo bindings[gpu::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  VkDescriptorSetLayout      vk_layout                    = nullptr;
  u32                        sizing[NUM_DESCRIPTOR_TYPES] = {};
  u32                        num_bindings                 = 0;
  u32                        num_variable_length          = 0;
};

/// used to track stateful resource access
/// @param images only valid if `type` is a descriptor type that access images
/// @param param buffers: only valid if `type` is a descriptor type that access
/// buffers
struct DescriptorBinding
{
  union
  {
    void   **sync_resources = nullptr;
    Image  **images;
    Buffer **buffers;
  };
  u32                 count              = 0;
  gpu::DescriptorType type               = gpu::DescriptorType::Sampler;
  bool                is_variable_length = false;
  u32                 max_count          = 0;
};

struct DescriptorSet
{
  VkDescriptorSet   vk_set                                     = nullptr;
  DescriptorBinding bindings[gpu::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  u32               num_bindings                               = 0;
  u32               pool                                       = 0;
};

struct DescriptorPool
{
  VkDescriptorPool vk_pool                     = nullptr;
  u32              avail[NUM_DESCRIPTOR_TYPES] = {};
};

/// @param pool_size each pool will have `pool_size` of each descriptor type
struct DescriptorHeap
{
  AllocatorImpl   allocator    = {};
  DescriptorPool *pools        = nullptr;
  u32             pool_size    = 0;
  u8             *scratch      = nullptr;
  u32             num_pools    = 0;
  usize           scratch_size = 0;
};

struct ComputePipeline
{
  VkPipeline       vk_pipeline         = nullptr;
  VkPipelineLayout vk_layout           = nullptr;
  u32              push_constants_size = 0;
  u32              num_sets            = 0;
};

struct GraphicsPipeline
{
  VkPipeline       vk_pipeline                                 = nullptr;
  VkPipelineLayout vk_layout                                   = nullptr;
  u32              push_constants_size                         = 0;
  u32              num_sets                                    = 0;
  gpu::Format      colors[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS] = {};
  gpu::Format      depth[1]                                    = {};
  gpu::Format      stencil[1]                                  = {};
  u32              num_colors                                  = 0;
  u32              num_depths                                  = 0;
  u32              num_stencils                                = 0;
};

struct Instance final : gpu::Instance
{
  AllocatorImpl            allocator          = {};
  InstanceTable            vk_table           = {};
  VkInstance               vk_instance        = nullptr;
  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;
  bool                     validation_enabled = false;

  // [ ] fix
  explicit Instance() {};
  Instance(Instance const &)            = delete;
  Instance &operator=(Instance const &) = delete;
  Instance(Instance &&)                 = delete;
  Instance &operator=(Instance &&)      = delete;
  virtual ~Instance() override;

  virtual Result<gpu::Device *, Status>
      create_device(AllocatorImpl               allocator,
                    Span<gpu::DeviceType const> preferred_types,
                    u32                         buffering) override;

  virtual gpu::Backend get_backend() override;

  virtual void uninit_device(gpu::Device *device) override;

  virtual void uninit_surface(gpu::Surface surface) override;
};

struct PhysicalDevice
{
  VkPhysicalDevice                 vk_phy_dev           = nullptr;
  VkPhysicalDeviceFeatures         vk_features          = {};
  VkPhysicalDeviceProperties       vk_properties        = {};
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
  gpu::SwapchainInfo  info            = {};
  bool                is_out_of_date  = true;
  bool                is_optimal      = false;
  bool                is_zero_sized   = false;
  gpu::SurfaceFormat  format          = {};
  gpu::ImageUsage     usage           = gpu::ImageUsage::None;
  gpu::PresentMode    present_mode    = gpu::PresentMode::Immediate;
  gpu::Extent         extent          = {};
  gpu::CompositeAlpha composite_alpha = gpu::CompositeAlpha::None;
  Image               image_impls[gpu::MAX_SWAPCHAIN_IMAGES] = {};
  gpu::Image          images[gpu::MAX_SWAPCHAIN_IMAGES]      = {};
  VkImage             vk_images[gpu::MAX_SWAPCHAIN_IMAGES]   = {};
  u32                 num_images                             = 0;
  u32                 current_image                          = 0;
  VkSwapchainKHR      vk_swapchain                           = nullptr;
  VkSurfaceKHR        vk_surface                             = nullptr;
};

enum class CommandEncoderState : u16
{
  Reset       = 0,
  Begin       = 1,
  RenderPass  = 2,
  ComputePass = 3,
  End         = 4
};

enum class CommandType : usize
{
  None                = 0,
  BindDescriptorSets  = 1,
  BindPipeline        = 2,
  PushConstants       = 3,
  SetGraphicsState    = 4,
  BindVertexBuffer    = 5,
  BindIndexBuffer     = 6,
  Draw                = 7,
  DrawIndexed         = 8,
  DrawIndirect        = 9,
  DrawIndexedIndirect = 10
};

struct Command
{
  CommandType type = CommandType::None;
  union
  {
    char                                     none_ = 0;
    Tuple<DescriptorSet **, u32, u32 *, u32> set;
    GraphicsPipeline                        *pipeline;
    gpu::GraphicsState                       state;
    Tuple<u8 *, u32>                         push_constant;
    Tuple<u32, Buffer *, u64>                vertex_buffer;
    Tuple<Buffer *, u64, gpu::IndexType>     index_buffer;
    Tuple<u32, u32, u32, u32>                draw;
    Tuple<u32, u32, i32, u32, u32>           draw_indexed;
    Tuple<Buffer *, u64, u32, u32>           draw_indirect;
  };
};

struct RenderPassContext
{
  gpu::Rect render_area = {};
  u32       num_layers  = 0;
  gpu::RenderingAttachment
      color_attachments[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS]          = {};
  gpu::RenderingAttachment depth_attachment[1]                        = {};
  gpu::RenderingAttachment stencil_attachment[1]                      = {};
  u32                      num_color_attachments                      = 0;
  u32                      num_depth_attachments                      = 0;
  u32                      num_stencil_attachments                    = 0;
  ArenaPool                arg_pool                                   = {};
  ArenaPool                command_pool                               = {};
  Vec<Command>             commands                                   = {};
  Buffer                  *vertex_buffers[gpu::MAX_VERTEX_ATTRIBUTES] = {};
  u32                      num_vertex_buffers                         = 0;
  Buffer                  *index_buffer                               = nullptr;
  gpu::IndexType           index_type          = gpu::IndexType::Uint16;
  u64                      index_buffer_offset = 0;
  GraphicsPipeline        *pipeline            = nullptr;
  bool                     has_state           = false;

  void reset()
  {
    render_area             = {};
    num_layers              = 0;
    num_color_attachments   = 0;
    num_depth_attachments   = 0;
    num_stencil_attachments = 0;
    commands.reset();
    command_pool.reset();
    arg_pool.reset();
    num_vertex_buffers  = 0;
    index_buffer        = nullptr;
    index_buffer_offset = 0;
    pipeline            = nullptr;
    has_state           = false;
  }
};

struct ComputePassContext
{
  DescriptorSet   *sets[gpu::MAX_PIPELINE_DESCRIPTOR_SETS] = {};
  u32              num_sets                                = 0;
  ComputePipeline *pipeline                                = nullptr;

  void reset()
  {
    num_sets = 0;
    pipeline = nullptr;
  }
};

struct CommandEncoder final : gpu::CommandEncoder
{
  AllocatorImpl       allocator         = {};
  Device             *dev               = nullptr;
  ArenaPool           arg_pool          = {};
  VkCommandPool       vk_command_pool   = nullptr;
  VkCommandBuffer     vk_command_buffer = nullptr;
  Status              status            = Status::Success;
  CommandEncoderState state             = CommandEncoderState::Reset;
  RenderPassContext   render_ctx        = {};
  ComputePassContext  compute_ctx       = {};

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

  void access_image_aspect(Image &image, VkPipelineStageFlags stages,
                           VkAccessFlags access, VkImageLayout layout,
                           gpu::ImageAspects aspects, u32 aspect_index);

  void access_buffer(Buffer &buffer, VkPipelineStageFlags stages,
                     VkAccessFlags access);

  void access_image_all_aspects(Image &image, VkPipelineStageFlags stages,
                                VkAccessFlags access, VkImageLayout layout);

  void access_image_depth(Image &image, VkPipelineStageFlags stages,
                          VkAccessFlags access, VkImageLayout layout)
  {
    access_image_aspect(image, stages, access, layout, gpu::ImageAspects::Depth,
                        DEPTH_ASPECT_IDX);
  }

  void access_image_stencil(Image &image, VkPipelineStageFlags stages,
                            VkAccessFlags access, VkImageLayout layout)
  {
    access_image_aspect(image, stages, access, layout,
                        gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);
  }

  void access_compute_bindings(DescriptorSet const &set);

  void access_graphics_bindings(DescriptorSet const &set);

  void reset_context()
  {
    state = CommandEncoderState::Begin;
    render_ctx.reset();
    compute_ctx.reset();
  }

  virtual void reset_timestamp_query(gpu::TimeStampQuery query) override;

  virtual void reset_statistics_query(gpu::StatisticsQuery query) override;

  virtual void write_timestamp(gpu::TimeStampQuery query) override;

  virtual void begin_statistics(gpu::StatisticsQuery query) override;

  virtual void end_statistics(gpu::StatisticsQuery query) override;

  virtual void begin_debug_marker(Span<char const> region_name,
                                  Vec4             color) override;

  virtual void end_debug_marker() override;

  virtual void fill_buffer(gpu::Buffer dst, u64 offset, u64 size,
                           u32 data) override;

  virtual void copy_buffer(gpu::Buffer src, gpu::Buffer dst,
                           Span<gpu::BufferCopy const> copies) override;

  virtual void update_buffer(Span<u8 const> src, u64 dst_offset,
                             gpu::Buffer dst) override;

  virtual void
      clear_color_image(gpu::Image dst, gpu::Color clear_color,
                        Span<gpu::ImageSubresourceRange const> ranges) override;

  virtual void clear_depth_stencil_image(
      gpu::Image dst, gpu::DepthStencil clear_depth_stencil,
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

  virtual void begin_rendering(gpu::RenderingInfo const &info) override;

  virtual void end_rendering() override;

  virtual void bind_compute_pipeline(gpu::ComputePipeline pipeline) override;

  virtual void bind_graphics_pipeline(gpu::GraphicsPipeline pipeline) override;

  virtual void
      bind_descriptor_sets(Span<gpu::DescriptorSet const> descriptor_sets,
                           Span<u32 const> dynamic_offsets) override;

  virtual void push_constants(Span<u8 const> push_constants_data) override;

  virtual void dispatch(u32 group_count_x, u32 group_count_y,
                        u32 group_count_z) override;

  virtual void dispatch_indirect(gpu::Buffer buffer, u64 offset) override;

  virtual void set_graphics_state(gpu::GraphicsState const &state) override;

  virtual void bind_vertex_buffers(Span<gpu::Buffer const> vertex_buffers,
                                   Span<u64 const>         offsets) override;

  virtual void bind_index_buffer(gpu::Buffer index_buffer, u64 offset,
                                 gpu::IndexType index_type) override;

  virtual void draw(u32 vertex_count, u32 instance_count, u32 first_vertex_id,
                    u32 first_instance_id) override;

  virtual void draw_indexed(u32 first_index, u32 num_indices, i32 vertex_offset,
                            u32 first_instance_id, u32 num_instances) override;

  virtual void draw_indirect(gpu::Buffer buffer, u64 offset, u32 draw_count,
                             u32 stride) override;

  virtual void draw_indexed_indirect(gpu::Buffer buffer, u64 offset,
                                     u32 draw_count, u32 stride) override;
};

struct FrameContext
{
  gpu::FrameId         tail_frame                          = 0;
  gpu::FrameId         current_frame                       = 0;
  u32                  ring_index                          = 0;
  u32                  buffering                           = 0;
  CommandEncoder       encs[gpu::MAX_FRAME_BUFFERING]      = {};
  gpu::CommandEncoder *encs_impl[gpu::MAX_FRAME_BUFFERING] = {};
  VkSemaphore          acquire_s[gpu::MAX_FRAME_BUFFERING] = {};
  VkFence              submit_f[gpu::MAX_FRAME_BUFFERING]  = {};
  VkSemaphore          submit_s[gpu::MAX_FRAME_BUFFERING]  = {};
  Swapchain           *swapchain                           = nullptr;
};

struct Device final : gpu::Device
{
  AllocatorImpl      allocator       = {};
  Instance          *instance        = nullptr;
  PhysicalDevice     phy_dev         = {};
  DeviceTable        vk_table        = {};
  VmaVulkanFunctions vma_table       = {};
  VkDevice           vk_dev          = nullptr;
  u32                queue_family    = 0;
  VkQueue            vk_queue        = nullptr;
  VmaAllocator       vma_allocator   = nullptr;
  FrameContext       frame_ctx       = {};
  DescriptorHeap     descriptor_heap = {};

  void set_resource_name(Span<char const> label, void const *resource,
                         VkObjectType               type,
                         VkDebugReportObjectTypeEXT debug_type);

  void uninit_descriptor_heap(DescriptorHeap *heap);

  VkResult recreate_swapchain(Swapchain *swapchain);

  Status init_command_encoder(CommandEncoder *enc);

  void uninit_command_encoder(CommandEncoder *enc);

  Status init_frame_context(u32 buffering);

  void uninit_frame_context();

  virtual gpu::DeviceProperties get_device_properties() override;

  virtual Result<gpu::FormatProperties, Status>
      get_format_properties(gpu::Format format) override;

  virtual Result<gpu::Buffer, Status>
      create_buffer(gpu::BufferInfo const &info) override;

  virtual Result<gpu::BufferView, Status>
      create_buffer_view(gpu::BufferViewInfo const &info) override;

  virtual Result<gpu::Image, Status>
      create_image(gpu::ImageInfo const &info) override;

  virtual Result<gpu::ImageView, Status>
      create_image_view(gpu::ImageViewInfo const &info) override;

  virtual Result<gpu::Sampler, Status>
      create_sampler(gpu::SamplerInfo const &info) override;

  virtual Result<gpu::Shader, Status>
      create_shader(gpu::ShaderInfo const &info) override;

  virtual Result<gpu::DescriptorSetLayout, Status> create_descriptor_set_layout(
      gpu::DescriptorSetLayoutInfo const &info) override;

  virtual Result<gpu::DescriptorSet, Status>
      create_descriptor_set(gpu::DescriptorSetLayout layout,
                            Span<u32 const>          variable_lengths) override;

  virtual Result<gpu::PipelineCache, Status>
      create_pipeline_cache(gpu::PipelineCacheInfo const &info) override;

  virtual Result<gpu::ComputePipeline, Status>
      create_compute_pipeline(gpu::ComputePipelineInfo const &info) override;

  virtual Result<gpu::GraphicsPipeline, Status>
      create_graphics_pipeline(gpu::GraphicsPipelineInfo const &info) override;

  virtual Result<gpu::Swapchain, Status>
      create_swapchain(gpu::Surface              surface,
                       gpu::SwapchainInfo const &info) override;

  virtual Result<gpu::TimeStampQuery, Status> create_timestamp_query() override;

  virtual Result<gpu::StatisticsQuery, Status>
      create_statistics_query() override;

  virtual void uninit_buffer(gpu::Buffer buffer) override;

  virtual void uninit_buffer_view(gpu::BufferView buffer_view) override;

  virtual void uninit_image(gpu::Image image) override;

  virtual void uninit_image_view(gpu::ImageView image_view) override;

  virtual void uninit_sampler(gpu::Sampler sampler) override;

  virtual void uninit_shader(gpu::Shader shader) override;

  virtual void
      uninit_descriptor_set_layout(gpu::DescriptorSetLayout layout) override;

  virtual void uninit_descriptor_set(gpu::DescriptorSet set) override;

  virtual void uninit_pipeline_cache(gpu::PipelineCache cache) override;

  virtual void uninit_compute_pipeline(gpu::ComputePipeline pipeline) override;

  virtual void
      uninit_graphics_pipeline(gpu::GraphicsPipeline pipeline) override;

  virtual void uninit_swapchain(gpu::Swapchain swapchain) override;

  virtual void uninit_timestamp_query(gpu::TimeStampQuery query) override;

  virtual void uninit_statistics_query(gpu::StatisticsQuery query) override;

  virtual gpu::FrameContext get_frame_context() override;

  virtual Result<void *, Status> map_buffer_memory(gpu::Buffer buffer) override;

  virtual void unmap_buffer_memory(gpu::Buffer buffer) override;

  virtual Result<Void, Status>
      invalidate_mapped_buffer_memory(gpu::Buffer      buffer,
                                      gpu::MemoryRange range) override;

  virtual Result<Void, Status>
      flush_mapped_buffer_memory(gpu::Buffer      buffer,
                                 gpu::MemoryRange range) override;

  virtual Result<usize, Status>
      get_pipeline_cache_size(gpu::PipelineCache cache) override;

  virtual Result<Void, Status> get_pipeline_cache_data(gpu::PipelineCache cache,
                                                       Vec<u8> &out) override;

  virtual Result<Void, Status>
      merge_pipeline_cache(gpu::PipelineCache             dst,
                           Span<gpu::PipelineCache const> srcs) override;

  virtual void
      update_descriptor_set(gpu::DescriptorSetUpdate const &update) override;

  virtual Result<Void, Status> wait_idle() override;

  virtual Result<Void, Status> wait_queue_idle() override;

  virtual Result<Void, Status>
      get_surface_formats(gpu::Surface             surface,
                          Vec<gpu::SurfaceFormat> &formats) override;

  virtual Result<Void, Status>
      get_surface_present_modes(gpu::Surface           surface,
                                Vec<gpu::PresentMode> &modes) override;

  virtual Result<gpu::SurfaceCapabilities, Status>
      get_surface_capabilities(gpu::Surface surface) override;

  virtual Result<gpu::SwapchainState, Status>
      get_swapchain_state(gpu::Swapchain swapchain) override;

  virtual Result<Void, Status>
      invalidate_swapchain(gpu::Swapchain            swapchain,
                           gpu::SwapchainInfo const &info) override;

  virtual Result<Void, Status> begin_frame(gpu::Swapchain swapchain) override;

  virtual Result<Void, Status> submit_frame(gpu::Swapchain swapchain) override;

  virtual Result<u64, Status>
      get_timestamp_query_result(gpu::TimeStampQuery query) override;

  virtual Result<gpu::PipelineStatistics, Status>
      get_statistics_query_result(gpu::StatisticsQuery query) override;
};

}        // namespace vk
}        // namespace ash
