#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VK_NO_PROTOTYPES

#include "ashura/allocator.h"
#include "ashura/gfx.h"
#include "vk_mem_alloc.h"
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

using gfx::Status;

constexpr char const *REQUIRED_INSTANCE_EXTENSIONS[] = {
    VK_KHR_SURFACE_EXTENSION_NAME};
constexpr char const *REQUIRED_DEVICE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr char const *OPTIONAL_DEVICE_EXTENSIONS[] = {
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME};

typedef struct InstanceTable           InstanceTable;
typedef struct DeviceTable             DeviceTable;
typedef struct Buffer                  Buffer;
typedef struct BufferView              BufferView;
typedef struct Image                   Image;
typedef struct ImageView               ImageView;
typedef struct Sampler                 Sampler;
typedef struct Shader                  Shader;
typedef struct RenderPass              RenderPass;
typedef struct Framebuffer             Framebuffer;
typedef struct DescriptorSetLayout     DescriptorSetLayout;
typedef struct DescriptorHeap          DescriptorHeap;
typedef struct PipelineCache           PipelineCache;
typedef struct ComputePipeline         ComputePipeline;
typedef struct GraphicsPipeline        GraphicsPipeline;
typedef struct Fence                   Fence;
typedef struct CommandEncoder          CommandEncoder;
typedef struct Swapchain               Swapchain;
typedef struct FrameContext            FrameContext;
typedef struct Device                  Device;
typedef struct DescriptorHeapInterface DescriptorHeapInterface;
typedef struct CommandEncoderInterface CommandEncoderInterface;
typedef struct DeviceInterface         DeviceInterface;

struct InstanceTable
{
  PFN_vkCreateDebugReportCallbackEXT  CreateDebugReportCallbackEXT  = nullptr;
  PFN_vkCreateDebugUtilsMessengerEXT  CreateDebugUtilsMessengerEXT  = nullptr;
  PFN_vkCreateInstance                CreateInstance                = nullptr;
  PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT = nullptr;
  PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;
  PFN_vkDestroyInstance               DestroyInstance               = nullptr;
  PFN_vkDestroySurfaceKHR             DestroySurfaceKHR             = nullptr;
  PFN_vkEnumeratePhysicalDevices      EnumeratePhysicalDevices      = nullptr;
  PFN_vkGetInstanceProcAddr           GetInstanceProcAddr           = nullptr;

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
  PFN_vkCreateDevice                 CreateDevice                 = nullptr;
  PFN_vkCreateEvent                  CreateEvent                  = nullptr;
  PFN_vkCreateFence                  CreateFence                  = nullptr;
  PFN_vkCreateFramebuffer            CreateFramebuffer            = nullptr;
  PFN_vkCreateGraphicsPipelines      CreateGraphicsPipelines      = nullptr;
  PFN_vkCreateImage                  CreateImage                  = nullptr;
  PFN_vkCreateImageView              CreateImageView              = nullptr;
  PFN_vkCreatePipelineCache          CreatePipelineCache          = nullptr;
  PFN_vkCreatePipelineLayout         CreatePipelineLayout         = nullptr;
  PFN_vkCreateQueryPool              CreateQueryPool              = nullptr;
  PFN_vkCreateRenderPass             CreateRenderPass             = nullptr;
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
  PFN_vkDestroyFramebuffer           DestroyFramebuffer           = nullptr;
  PFN_vkDestroyImage                 DestroyImage                 = nullptr;
  PFN_vkDestroyImageView             DestroyImageView             = nullptr;
  PFN_vkDestroyPipeline              DestroyPipeline              = nullptr;
  PFN_vkDestroyPipelineCache         DestroyPipelineCache         = nullptr;
  PFN_vkDestroyPipelineLayout        DestroyPipelineLayout        = nullptr;
  PFN_vkDestroyQueryPool             DestroyQueryPool             = nullptr;
  PFN_vkDestroyRenderPass            DestroyRenderPass            = nullptr;
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
  PFN_vkCmdBeginRenderPass        CmdBeginRenderPass        = nullptr;
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
  PFN_vkCmdEndRenderPass          CmdEndRenderPass          = nullptr;
  PFN_vkCmdFillBuffer             CmdFillBuffer             = nullptr;
  PFN_vkCmdNextSubpass            CmdNextSubpass            = nullptr;
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

// NOTE: render_pass attachments MUST not be accessed in shaders within that
// render_pass NOTE: update_buffer and fill_buffer MUST be multiple of 4 for dst
// offset and dst size
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

// if is read access but with layout and access same as the transitioned one
// reader tries to read write but there's no dependency
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
  u64               refcount            = 0;
  gfx::BufferDesc   desc                = {};
  VkBuffer          vk_buffer           = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  void             *host_map            = nullptr;
  BufferState       state               = {};
};

struct BufferView
{
  u64                 refcount = 0;
  gfx::BufferViewDesc desc     = {};
  VkBufferView        vk_view  = nullptr;
};

struct Image
{
  u64               refcount            = 0;
  gfx::ImageDesc    desc                = {};
  bool              is_swapchain_image  = false;
  VkImage           vk_image            = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  ImageState        state               = {};
};

struct ImageView
{
  u64                refcount = 0;
  gfx::ImageViewDesc desc     = {};
  VkImageView        vk_view  = nullptr;
};

struct RenderPass
{
  u64                       refcount                                      = 0;
  gfx::RenderPassAttachment color_attachments[gfx::MAX_COLOR_ATTACHMENTS] = {};
  gfx::RenderPassAttachment input_attachments[gfx::MAX_INPUT_ATTACHMENTS] = {};
  gfx::RenderPassAttachment depth_stencil_attachment                      = {};
  u32                       num_color_attachments                         = 0;
  u32                       num_input_attachments                         = 0;
  VkRenderPass              vk_render_pass = nullptr;
};

struct Framebuffer
{
  u64           refcount                                      = 0;
  gfx::Extent   extent                                        = {};
  ImageView    *color_attachments[gfx::MAX_INPUT_ATTACHMENTS] = {};
  ImageView    *depth_stencil_attachment                      = nullptr;
  u32           layers                                        = 0;
  u32           num_color_attachments                         = 0;
  VkFramebuffer vk_framebuffer                                = nullptr;
};

struct Shader
{
  u64            refcount  = 0;
  VkShaderModule vk_shader = nullptr;
};

struct DescriptorSetLayout
{
  u64                         refcount     = 0;
  gfx::DescriptorBindingDesc *bindings     = nullptr;
  u32                         num_bindings = 0;
  VkDescriptorSetLayout       vk_layout    = nullptr;
};

struct PipelineCache
{
  u64             refcount = 0;
  VkPipelineCache vk_cache = nullptr;
};

struct ComputePipeline
{
  u64              refcount    = 0;
  VkPipeline       vk_pipeline = nullptr;
  VkPipelineLayout vk_layout   = nullptr;
};

struct GraphicsPipeline
{
  u64              refcount    = 0;
  VkPipeline       vk_pipeline = nullptr;
  VkPipelineLayout vk_layout   = nullptr;
};

struct Sampler
{
  u64       refcount   = 0;
  VkSampler vk_sampler = nullptr;
};

struct Fence
{
  u64     refcount = 0;
  VkFence vk_fence = nullptr;
};

struct Device
{
  u64                        refcount                 = 0;
  AllocatorImpl              allocator                = {};
  InstanceTable              vk_instance_table        = {};
  DeviceTable                vk_table                 = {};
  VmaVulkanFunctions         vma_table                = {};
  VkInstance                 vk_instance              = nullptr;
  VkPhysicalDevice           vk_phy_device            = nullptr;
  VkPhysicalDeviceProperties vk_phy_device_properties = {};
  VkDevice                   vk_device                = nullptr;
  u32                        queue_family             = 0;
  VkQueue                    vk_queue                 = nullptr;
  VmaAllocator               vma_allocator            = nullptr;
};

/// @struct DescriptorHeap
/// Descriptor heap helps with allocation of descriptor sets and checking when
/// they are in use before releasing and re-using them. Having multiple sets in
/// one group helps lighten the burden of managing separate heaps for different
/// descriptor sets belonging to an object
///
/// LAYOUT: GROUPS -> DESCRIPTOR SETS -> BINDINGS
///
/// ACCESS PATTERNS
/// ==> GET [GROUP I: SET J: DESCRIPTOR_SET]
/// ==> GET [GROUP I: SET J: BINDINGS]
/// ==> UPDATE [GROUP I: SET J: DESCRIPTOR SET] with [NEW_BINDINGS] and copy to
/// [GROUP I: SET J: BINDINGS]
///
struct DescriptorHeap
{
  u64                   refcount                    = 0;
  Device               *device                      = nullptr;
  AllocatorImpl         allocator                   = {};
  DescriptorSetLayout **set_layouts                 = nullptr;
  u32                 **binding_offsets             = nullptr;
  VkDescriptorPool     *vk_pools                    = nullptr;
  VkDescriptorSet      *vk_descriptor_sets          = nullptr;
  u64                  *last_use_frame              = nullptr;
  u32                  *released_groups             = nullptr;
  u32                  *free_groups                 = nullptr;
  u8                   *bindings                    = nullptr;
  void                 *scratch_memory              = nullptr;
  u32                   num_sets_per_group          = 0;
  u32                   num_pools                   = 0;
  u32                   num_groups_per_pool         = 0;
  u32                   num_released_groups         = 0;
  u32                   num_free_groups             = 0;
  u32                   group_binding_stride        = 0;
  u32                   vk_pools_capacity           = 0;
  u32                   vk_descriptor_sets_capacity = 0;
  u32                   last_use_frame_capacity     = 0;
  u32                   released_groups_capacity    = 0;
  u32                   free_groups_capacity        = 0;
  usize                 bindings_capacity           = 0;
  usize                 scratch_memory_size         = 0;
};

struct CommandEncoder
{
  u64               refcount                                         = 0;
  AllocatorImpl     allocator                                        = {};
  Device           *device                                           = nullptr;
  VkCommandPool     vk_command_pool                                  = nullptr;
  VkCommandBuffer   vk_command_buffer                                = nullptr;
  ComputePipeline  *bound_compute_pipeline                           = nullptr;
  GraphicsPipeline *bound_graphics_pipeline                          = nullptr;
  RenderPass       *bound_render_pass                                = nullptr;
  Framebuffer      *bound_framebuffer                                = nullptr;
  Buffer           *bound_vertex_buffers[gfx::MAX_VERTEX_ATTRIBUTES] = {};
  u32               num_bound_vertex_buffers                         = 0;
  Buffer           *bound_index_buffer                               = nullptr;
  gfx::IndexType    bound_index_type          = gfx::IndexType::Uint16;
  u64               bound_index_buffer_offset = 0;
  DescriptorHeap
        *bound_descriptor_set_heaps[gfx::MAX_PIPELINE_DESCRIPTOR_SETS]  = {};
  u32    bound_descriptor_set_groups[gfx::MAX_PIPELINE_DESCRIPTOR_SETS] = {};
  u32    bound_descriptor_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS]       = {};
  u32    num_bound_descriptor_sets                                      = 0;
  Status status = Status::Success;
};

struct FrameContext
{
  u64                      refcount                = 0;
  gfx::FrameId             trailing_frame          = 0;
  gfx::FrameId             current_frame           = 0;
  u32                      current_command_encoder = 0;
  u32                      max_frames_in_flight    = 0;
  gfx::CommandEncoderImpl *command_encoders        = nullptr;
  VkSemaphore             *acquire_semaphores      = nullptr;
  gfx::Fence              *submit_fences           = nullptr;
  VkSemaphore             *submit_semaphores       = nullptr;
};

struct Swapchain
{
  gfx::Generation    generation                             = 0;
  gfx::SwapchainDesc desc                                   = {};
  bool               is_valid                               = false;
  bool               is_optimal                             = false;
  gfx::Extent        extent                                 = {};
  Image              image_impls[gfx::MAX_SWAPCHAIN_IMAGES] = {};
  gfx::Image         images[gfx::MAX_SWAPCHAIN_IMAGES]      = {};
  VkImage            vk_images[gfx::MAX_SWAPCHAIN_IMAGES]   = {};
  u32                num_images                             = 0;
  u32                current_image                          = 0;
  VkSwapchainKHR     vk_swapchain                           = nullptr;
  VkSurfaceKHR       vk_surface                             = nullptr;
};

struct DeviceInterface
{
  static void ref(gfx::Device self);
  static void unref(gfx::Device self);
  static Result<gfx::DeviceProperties, Status>
      get_device_properties(gfx::Device self);
  static Result<gfx::FormatProperties, Status>
      get_format_properties(gfx::Device self, gfx::Format format);
  static Result<gfx::Buffer, Status> create_buffer(gfx::Device            self,
                                                   gfx::BufferDesc const &desc);
  static Result<gfx::BufferView, Status>
      create_buffer_view(gfx::Device self, gfx::BufferViewDesc const &desc);
  static Result<gfx::Image, Status> create_image(gfx::Device           self,
                                                 gfx::ImageDesc const &desc);
  static Result<gfx::ImageView, Status>
      create_image_view(gfx::Device self, gfx::ImageViewDesc const &desc);
  static Result<gfx::Sampler, Status>
      create_sampler(gfx::Device self, gfx::SamplerDesc const &desc);
  static Result<gfx::Shader, Status> create_shader(gfx::Device            self,
                                                   gfx::ShaderDesc const &desc);
  static Result<gfx::RenderPass, Status>
      create_render_pass(gfx::Device self, gfx::RenderPassDesc const &desc);
  static Result<gfx::Framebuffer, Status>
      create_framebuffer(gfx::Device self, gfx::FramebufferDesc const &desc);
  static Result<gfx::DescriptorSetLayout, Status>
      create_descriptor_set_layout(gfx::Device                         self,
                                   gfx::DescriptorSetLayoutDesc const &desc);
  static Result<gfx::DescriptorHeapImpl, Status> create_descriptor_heap(
      gfx::Device                          self,
      Span<gfx::DescriptorSetLayout const> descriptor_set_layouts,
      u32 groups_per_pool, AllocatorImpl allocator);
  static Result<gfx::PipelineCache, Status>
      create_pipeline_cache(gfx::Device                   self,
                            gfx::PipelineCacheDesc const &desc);
  static Result<gfx::ComputePipeline, Status>
      create_compute_pipeline(gfx::Device                     self,
                              gfx::ComputePipelineDesc const &desc);
  static Result<gfx::GraphicsPipeline, Status>
                                    create_graphics_pipeline(gfx::Device                      self,
                                                             gfx::GraphicsPipelineDesc const &desc);
  static Result<gfx::Fence, Status> create_fence(gfx::Device self,
                                                 bool        signaled);
  static Result<gfx::CommandEncoderImpl, Status>
      create_command_encoder(gfx::Device self, AllocatorImpl allocator);
  static Result<gfx::FrameContext, Status> create_frame_context(
      gfx::Device self, u32 max_frames_in_flight,
      Span<AllocatorImpl const> command_encoder_allocators);
  static Result<gfx::Swapchain, Status>
              create_swapchain(gfx::Device self, gfx::Surface surface,
                               gfx::SwapchainDesc const &desc);
  static void ref_buffer(gfx::Device self, gfx::Buffer buffer);
  static void ref_buffer_view(gfx::Device self, gfx::BufferView buffer_view);
  static void ref_image(gfx::Device self, gfx::Image image);
  static void ref_image_view(gfx::Device self, gfx::ImageView image_view);
  static void ref_sampler(gfx::Device self, gfx::Sampler sampler);
  static void ref_shader(gfx::Device self, gfx::Shader shader);
  static void ref_render_pass(gfx::Device self, gfx::RenderPass render_pass);
  static void ref_framebuffer(gfx::Device self, gfx::Framebuffer framebuffer);
  static void ref_descriptor_set_layout(gfx::Device              self,
                                        gfx::DescriptorSetLayout layout);
  static void ref_descriptor_heap(gfx::Device             self,
                                  gfx::DescriptorHeapImpl heap);
  static void ref_pipeline_cache(gfx::Device self, gfx::PipelineCache cache);
  static void ref_compute_pipeline(gfx::Device          self,
                                   gfx::ComputePipeline pipeline);
  static void ref_graphics_pipeline(gfx::Device           self,
                                    gfx::GraphicsPipeline pipeline);
  static void ref_fence(gfx::Device self, gfx::Fence fence);
  static void ref_command_encoder(gfx::Device             self,
                                  gfx::CommandEncoderImpl encoder);
  static void ref_frame_context(gfx::Device       self,
                                gfx::FrameContext frame_context);
  static void unref_buffer(gfx::Device self, gfx::Buffer buffer);
  static void unref_buffer_view(gfx::Device self, gfx::BufferView buffer_view);
  static void unref_image(gfx::Device self, gfx::Image image);
  static void unref_image_view(gfx::Device self, gfx::ImageView image_view);
  static void unref_sampler(gfx::Device self, gfx::Sampler sampler);
  static void unref_shader(gfx::Device self, gfx::Shader shader);
  static void unref_render_pass(gfx::Device self, gfx::RenderPass render_pass);
  static void unref_framebuffer(gfx::Device self, gfx::Framebuffer framebuffer);
  static void unref_descriptor_set_layout(gfx::Device              self,
                                          gfx::DescriptorSetLayout layout);
  static void unref_descriptor_heap(gfx::Device             self,
                                    gfx::DescriptorHeapImpl heap);
  static void unref_pipeline_cache(gfx::Device self, gfx::PipelineCache cache);
  static void unref_compute_pipeline(gfx::Device          self,
                                     gfx::ComputePipeline pipeline);
  static void unref_graphics_pipeline(gfx::Device           self,
                                      gfx::GraphicsPipeline pipeline);
  static void unref_fence(gfx::Device self, gfx::Fence fence);
  static void unref_command_encoder(gfx::Device             self,
                                    gfx::CommandEncoderImpl encoder);
  static void unref_frame_context(gfx::Device       self,
                                  gfx::FrameContext frame_context);
  static Result<void *, Status> get_buffer_memory_map(gfx::Device self,
                                                      gfx::Buffer buffer);
  static Result<Void, Status>
      invalidate_buffer_memory_map(gfx::Device self, gfx::Buffer buffer,
                                   gfx::MemoryRange ranges);
  static Result<Void, Status> flush_buffer_memory_map(gfx::Device      self,
                                                      gfx::Buffer      buffer,
                                                      gfx::MemoryRange range);
  static Result<usize, Status>
      get_pipeline_cache_size(gfx::Device self, gfx::PipelineCache cache);
  static Result<usize, Status> get_pipeline_cache_data(gfx::Device        self,
                                                       gfx::PipelineCache cache,
                                                       Span<u8>           out);
  static Result<Void, Status>
      merge_pipeline_cache(gfx::Device self, gfx::PipelineCache dst,
                           Span<gfx::PipelineCache const> srcs);
  static Result<Void, Status> wait_for_fences(gfx::Device            self,
                                              Span<gfx::Fence const> fences,
                                              bool all, u64 timeout);
  static Result<Void, Status> reset_fences(gfx::Device            self,
                                           Span<gfx::Fence const> fences);
  static Result<bool, Status> get_fence_status(gfx::Device self,
                                               gfx::Fence  fence);
  static Result<Void, Status> submit(gfx::Device         self,
                                     gfx::CommandEncoder encoder,
                                     gfx::Fence          signal_fence);
  static Result<Void, Status> wait_idle(gfx::Device self);
  static Result<Void, Status> wait_queue_idle(gfx::Device self);
  static Result<gfx::FrameInfo, Status>
      get_frame_info(gfx::Device self, gfx::FrameContext frame_context);
  static Result<u32, Status>
      get_surface_formats(gfx::Device self, gfx::Surface surface,
                          Span<gfx::SurfaceFormat> formats);
  static Result<u32, Status>
      get_surface_present_modes(gfx::Device self, gfx::Surface surface,
                                Span<gfx::PresentMode> modes);
  static Result<gfx::ImageUsage, Status> (*get_surface_usage)(
      gfx::Device self, gfx::Surface surface);
  static Result<gfx::SwapchainInfo, Status>
      get_swapchain_info(gfx::Device self, gfx::Swapchain swapchain);
  static Result<Void, Status>
      invalidate_swapchain(gfx::Device self, gfx::Swapchain swapchain,
                           gfx::SwapchainDesc const &config);
  static Result<Void, Status> begin_frame(gfx::Device       self,
                                          gfx::Swapchain    swapchain,
                                          gfx::FrameContext frame_context);
  static Result<Void, Status> submit_frame(gfx::Device       self,
                                           gfx::Swapchain    swapchain,
                                           gfx::FrameContext frame_context);
};

struct DescriptorHeapInterface
{
  static Result<u32, Status> add_group(gfx::DescriptorHeap self,
                                       gfx::FrameId        trailing_frame);
  static void sampler(gfx::DescriptorHeap self, u32 group, u32 set, u32 binding,
                      Span<gfx::SamplerBinding const> elements);
  static void combined_image_sampler(
      gfx::DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<gfx::CombinedImageSamplerBinding const> elements);
  static void sampled_image(gfx::DescriptorHeap self, u32 group, u32 set,
                            u32                                  binding,
                            Span<gfx::SampledImageBinding const> elements);
  static void storage_image(gfx::DescriptorHeap self, u32 group, u32 set,
                            u32                                  binding,
                            Span<gfx::StorageImageBinding const> elements);
  static void
      uniform_texel_buffer(gfx::DescriptorHeap self, u32 group, u32 set,
                           u32                                        binding,
                           Span<gfx::UniformTexelBufferBinding const> elements);
  static void
              storage_texel_buffer(gfx::DescriptorHeap self, u32 group, u32 set,
                                   u32                                        binding,
                                   Span<gfx::StorageTexelBufferBinding const> elements);
  static void uniform_buffer(gfx::DescriptorHeap self, u32 group, u32 set,
                             u32                                   binding,
                             Span<gfx::UniformBufferBinding const> elements);
  static void storage_buffer(gfx::DescriptorHeap self, u32 group, u32 set,
                             u32                                   binding,
                             Span<gfx::StorageBufferBinding const> elements);
  static void dynamic_uniform_buffer(
      gfx::DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<gfx::DynamicUniformBufferBinding const> elements);
  static void dynamic_storage_buffer(
      gfx::DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<gfx::DynamicStorageBufferBinding const> elements);
  static void
              input_attachment(gfx::DescriptorHeap self, u32 group, u32 set,
                               u32                                     binding,
                               Span<gfx::InputAttachmentBinding const> elements);
  static void mark_in_use(gfx::DescriptorHeap self, u32 group,
                          gfx::FrameId current_frame);
  static bool is_in_use(gfx::DescriptorHeap self, u32 group,
                        gfx::FrameId trailing_frame);
  static void release(gfx::DescriptorHeap self, u32 group);
  static gfx::DescriptorHeapStats get_stats(gfx::DescriptorHeap self);
};

struct CommandEncoderInterface
{
  static void                 begin(gfx::CommandEncoder self);
  static Result<Void, Status> end(gfx::CommandEncoder self);
  static void                 reset(gfx::CommandEncoder self);
  static void                 begin_debug_marker(gfx::CommandEncoder self,
                                                 char const *region_name, Vec4 color);
  static void                 end_debug_marker(gfx::CommandEncoder self);
  static void fill_buffer(gfx::CommandEncoder self, gfx::Buffer dst, u64 offset,
                          u64 size, u32 data);
  static void copy_buffer(gfx::CommandEncoder self, gfx::Buffer src,
                          gfx::Buffer dst, Span<gfx::BufferCopy const> copies);
  static void update_buffer(gfx::CommandEncoder self, Span<u8 const> src,
                            u64 dst_offset, gfx::Buffer dst);
  static void clear_color_image(gfx::CommandEncoder self, gfx::Image dst,
                                gfx::Color clear_color,
                                Span<gfx::ImageSubresourceRange const> ranges);
  static void
      clear_depth_stencil_image(gfx::CommandEncoder self, gfx::Image dst,
                                gfx::DepthStencil clear_depth_stencil,
                                Span<gfx::ImageSubresourceRange const> ranges);
  static void copy_image(gfx::CommandEncoder self, gfx::Image src,
                         gfx::Image dst, Span<gfx::ImageCopy const> copies);
  static void copy_buffer_to_image(gfx::CommandEncoder self, gfx::Buffer src,
                                   gfx::Image                       dst,
                                   Span<gfx::BufferImageCopy const> copies);
  static void blit_image(gfx::CommandEncoder self, gfx::Image src,
                         gfx::Image dst, Span<gfx::ImageBlit const> blits,
                         gfx::Filter filter);
  static void begin_render_pass(
      gfx::CommandEncoder self, gfx::Framebuffer framebuffer,
      gfx::RenderPass render_pass, gfx::Offset render_offset,
      gfx::Extent              render_extent,
      Span<gfx::Color const>   color_attachments_clear_values,
      gfx::DepthStencil const &depth_stencil_attachment_clear_value);
  static void end_render_pass(gfx::CommandEncoder self);
  static void bind_compute_pipeline(gfx::CommandEncoder  self,
                                    gfx::ComputePipeline pipeline);
  static void bind_graphics_pipeline(gfx::CommandEncoder   self,
                                     gfx::GraphicsPipeline pipeline);
  static void
              bind_descriptor_sets(gfx::CommandEncoder             self,
                                   Span<gfx::DescriptorHeap const> descriptor_heaps,
                                   Span<u32 const> groups, Span<u32 const> sets,
                                   Span<u32 const> dynamic_offsets);
  static void push_constants(gfx::CommandEncoder self,
                             Span<u8 const>      push_constants_data);
  static void dispatch(gfx::CommandEncoder self, u32 group_count_x,
                       u32 group_count_y, u32 group_count_z);
  static void dispatch_indirect(gfx::CommandEncoder self, gfx::Buffer buffer,
                                u64 offset);
  static void set_viewport(gfx::CommandEncoder  self,
                           gfx::Viewport const &viewport);
  static void set_scissor(gfx::CommandEncoder self, gfx::Offset scissor_offset,
                          gfx::Extent scissor_extent);
  static void set_blend_constants(gfx::CommandEncoder self,
                                  Vec4                blend_constant);
  static void set_stencil_compare_mask(gfx::CommandEncoder self,
                                       gfx::StencilFaces faces, u32 mask);
  static void set_stencil_reference(gfx::CommandEncoder self,
                                    gfx::StencilFaces faces, u32 reference);
  static void set_stencil_write_mask(gfx::CommandEncoder self,
                                     gfx::StencilFaces faces, u32 mask);
  static void bind_vertex_buffers(gfx::CommandEncoder     self,
                                  Span<gfx::Buffer const> vertex_buffers,
                                  Span<u64 const>         offsets);
  static void bind_index_buffer(gfx::CommandEncoder self,
                                gfx::Buffer index_buffer, u64 offset,
                                gfx::IndexType index_type);
  static void draw(gfx::CommandEncoder self, u32 first_index, u32 num_indices,
                   i32 vertex_offset, u32 first_instance, u32 num_instances);
  static void draw_indirect(gfx::CommandEncoder self, gfx::Buffer buffer,
                            u64 offset, u32 draw_count, u32 stride);
};

}        // namespace vk
}        // namespace ash
