#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1000000

#include "ashura/gfx/gfx.h"
#include "ashura/std/allocator.h"
#include "ashura/std/vec.h"
#include "vk_mem_alloc.h"
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

using gfx::Status;

constexpr char const *ENGINE_NAME    = "Ash";
constexpr u32         ENGINE_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);
constexpr char const *CLIENT_NAME    = "Ash Client";
constexpr u32         CLIENT_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);

constexpr u32 MAX_MEMORY_HEAP_PROPERTIES = 32;
constexpr u32 MAX_MEMORY_HEAPS           = 16;

typedef VkSampler       Sampler;
typedef VkShaderModule  Shader;
typedef VkPipelineCache PipelineCache;
typedef VkFence         Fence;
typedef VkSurfaceKHR    Surface;

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
  gfx::BufferDesc   desc                = {};
  VkBuffer          vk_buffer           = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  void             *host_map            = nullptr;
  BufferState       state               = {};
};

struct BufferView
{
  gfx::BufferViewDesc desc    = {};
  VkBufferView        vk_view = nullptr;
};

struct Image
{
  gfx::ImageDesc    desc                = {};
  bool              is_swapchain_image  = false;
  VkImage           vk_image            = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  ImageState        state               = {};
};

struct ImageView
{
  gfx::ImageViewDesc desc    = {};
  VkImageView        vk_view = nullptr;
};

struct RenderPass
{
  gfx::RenderPassAttachment color_attachments[gfx::MAX_COLOR_ATTACHMENTS] = {};
  gfx::RenderPassAttachment input_attachments[gfx::MAX_INPUT_ATTACHMENTS] = {};
  gfx::RenderPassAttachment depth_stencil_attachment                      = {};
  u32                       num_color_attachments                         = 0;
  u32                       num_input_attachments                         = 0;
  VkRenderPass              vk_render_pass = nullptr;
};

struct Framebuffer
{
  gfx::Extent   extent                                        = {};
  ImageView    *color_attachments[gfx::MAX_INPUT_ATTACHMENTS] = {};
  ImageView    *depth_stencil_attachment                      = nullptr;
  u32           layers                                        = 0;
  u32           num_color_attachments                         = 0;
  VkFramebuffer vk_framebuffer                                = nullptr;
};

struct DescriptorSetLayout
{
  gfx::DescriptorBindingDesc *bindings     = nullptr;
  u32                         num_bindings = 0;
  VkDescriptorSetLayout       vk_layout    = nullptr;
};

struct ComputePipeline
{
  VkPipeline       vk_pipeline        = nullptr;
  VkPipelineLayout vk_layout          = nullptr;
  u32              push_constant_size = 0;
};

struct GraphicsPipeline
{
  VkPipeline       vk_pipeline        = nullptr;
  VkPipelineLayout vk_layout          = nullptr;
  u32              push_constant_size = 0;
};

struct Instance
{
  AllocatorImpl            allocator   = {};
  Logger                  *logger      = {};
  InstanceTable            vk_table    = {};
  VkInstance               vk_instance = nullptr;
  VkDebugUtilsMessengerEXT vk_debug_messenger;
  bool                     validation_layer_enabled = false;
};

struct PhysicalDevice
{
  VkPhysicalDevice                 vk_physical_device = nullptr;
  VkPhysicalDeviceFeatures         features           = {};
  VkPhysicalDeviceProperties       properties         = {};
  VkPhysicalDeviceMemoryProperties memory_properties  = {};
};

struct Device
{
  AllocatorImpl      allocator       = {};
  Logger            *logger          = nullptr;
  Instance          *instance        = nullptr;
  PhysicalDevice     physical_device = {};
  DeviceTable        vk_table        = {};
  VmaVulkanFunctions vma_table       = {};
  VkDevice           vk_device       = nullptr;
  u32                queue_family    = 0;
  VkQueue            vk_queue        = nullptr;
  VmaAllocator       vma_allocator   = nullptr;
};

/// @num_allocated_groups: number of alive group allocations
/// @num_free_groups: number of released and reclaimable desciptor groups
/// @num_released_groups: number of released but non-reclaimable descriptor
/// groups. possibly still in use by the device.
struct DescriptorHeapStats
{
  u32 num_allocated = 0;
  u32 num_free      = 0;
  u32 num_released  = 0;
  u32 num_pools     = 0;
};

/// @struct DescriptorHeap
/// Descriptor heap helps with allocation of descriptor sets and checking when
/// they are in use before releasing and re-using them.
struct DescriptorHeap
{
  Device              *device                  = nullptr;
  AllocatorImpl        allocator               = {};
  Logger              *logger                  = nullptr;
  DescriptorSetLayout *set_layout              = nullptr;
  u32                 *binding_index_map       = nullptr;
  VkDescriptorPoolSize pool_sizes[11]          = {};
  VkDescriptorPool    *pools                   = nullptr;
  VkDescriptorSet     *sets                    = nullptr;
  u64                 *last_use_frame          = nullptr;
  u32                 *released                = nullptr;
  u32                 *free                    = nullptr;
  Image              **images                  = nullptr;
  Buffer             **buffers                 = nullptr;
  void                *scratch                 = nullptr;
  u32                  num_set_images          = 0;
  u32                  num_set_buffers         = 0;
  u32                  num_pool_sizes          = 0;
  u32                  num_pools               = 0;
  u32                  num_sets_per_pool       = 0;
  u32                  num_released            = 0;
  u32                  num_free                = 0;
  u32                  pools_capacity          = 0;
  u32                  sets_capacity           = 0;
  u32                  last_use_frame_capacity = 0;
  u32                  released_capacity       = 0;
  u32                  free_capacity           = 0;
  u32                  images_capacity         = 0;
  u32                  buffers_capacity        = 0;
  u32                  scratch_size            = 0;
};

enum class CommandEncoderState : u16
{
  Reset       = 0,
  Begin       = 1,
  RenderPass  = 2,
  ComputePass = 3,
  End         = 4
};

enum class RenderCommandType : u8
{
  None                  = 0,
  BindDescriptorSet     = 1,
  BindPipeline          = 2,
  PushConstants         = 3,
  SetViewport           = 6,
  SetScissor            = 7,
  SetBlendConstant      = 8,
  SetStencilCompareMask = 9,
  SetStencilReference   = 10,
  SetStencilWriteMask   = 11,
  BindVertexBuffer      = 12,
  BindIndexBuffer       = 13,
  Draw                  = 14,
  DrawIndexed           = 15,
  DrawIndirect          = 16,
  DrawIndexedIndirect   = 17
};

struct RenderCommand
{
  RenderCommandType type = RenderCommandType::None;
  union
  {
    char none_ = 0;
    Tuple<gfx::DescriptorSet[gfx::MAX_PIPELINE_DESCRIPTOR_SETS],
          u32[gfx::MAX_PIPELINE_DESCRIPTOR_SETS *
              gfx::MAX_DESCRIPTOR_DYNAMIC_BUFFERS],
          u8, u8>
                                    set;
    GraphicsPipeline               *pipeline;
    u8                              push_constant[gfx::MAX_PUSH_CONSTANT_SIZE];
    gfx::Viewport                   viewport;
    Tuple<gfx::Offset, gfx::Extent> scissor;
    Vec4                            blend_constant;
    Tuple<gfx::StencilFaces, u32>   stencil;
    Tuple<u32, Buffer *, u64>       vertex_buffer;
    Tuple<Buffer *, u64, gfx::IndexType> index_buffer;
    Tuple<u32, u32, u32, u32>            draw;
    Tuple<u32, u32, i32, u32, u32>       draw_indexed;
    Tuple<Buffer *, u64, u32, u32>       draw_indirect;
  };
};

struct RenderPassContext
{
  RenderPass        *render_pass = nullptr;
  Framebuffer       *framebuffer = nullptr;
  gfx::Offset        offset;
  gfx::Extent        extent;
  gfx::Color         color_clear_values[gfx::MAX_COLOR_ATTACHMENTS] = {};
  u32                num_color_clear_values                         = 0;
  gfx::DepthStencil  depth_stencil_clear_value;
  u32                num_depth_stencil_clear_values             = 0;
  Vec<RenderCommand> commands                                   = {};
  Buffer            *vertex_buffers[gfx::MAX_VERTEX_ATTRIBUTES] = {};
  u32                num_vertex_buffers                         = 0;
  Buffer            *index_buffer                               = nullptr;
  gfx::IndexType     index_type          = gfx::IndexType::Uint16;
  u64                index_buffer_offset = 0;
  GraphicsPipeline  *pipeline            = nullptr;
};

struct ComputePassContext
{
  gfx::DescriptorSet sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS] = {};
  u32                num_sets                                = 0;
  ComputePipeline   *pipeline                                = nullptr;
};

struct CommandEncoder
{
  AllocatorImpl       allocator         = {};
  Logger             *logger            = nullptr;
  Device             *device            = nullptr;
  VkCommandPool       vk_command_pool   = nullptr;
  VkCommandBuffer     vk_command_buffer = nullptr;
  Status              status            = Status::Success;
  CommandEncoderState state             = CommandEncoderState::Reset;
  union
  {
    char               none = 0;
    RenderPassContext  rp;
    ComputePassContext cp;
  } ctx;

  bool is_in_render_pass()
  {
    return state == CommandEncoderState::RenderPass;
  }

  bool is_in_compute_pass()
  {
    return state == CommandEncoderState::ComputePass;
  }

  bool is_recording()
  {
    return state == CommandEncoderState::Begin ||
           state == CommandEncoderState::RenderPass ||
           state == CommandEncoderState::ComputePass;
  }

  void reset_context();
  void init_rp_context();
  void uninit_rp_context();
  void init_cp_context();
  void uninit_cp_context();
};

/*
static constexpr VkQueryPipelineStatisticFlags PIPELINE_STATISTIC_QUERIES =
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
  static constexpr u32             NPIPELINE_STATISTIC_QUERIES = 7;
  static constexpr u32             NPIPELINE_TIMESTAMP_QUERIES = 2;
  u32                              max_nframes_in_flight       = 0;
  stx::Vec<VecBuffer>              vertex_buffers;
  stx::Vec<VecBuffer>              index_buffers;
  VkCommandPool                    cmd_pool = VK_NULL_HANDLE;
  stx::Vec<VkCommandBuffer>        cmd_buffers;
  stx::Vec<VkQueryPool>            pipeline_statistics_query_pools;
  stx::Vec<VkQueryPool>            pipeline_timestamp_query_pools;
  VkPhysicalDeviceMemoryProperties memory_properties;
  f32                              timestamp_period   = 1;
  u32                              queue_family_index = 0;
  VkQueue                          queue              = VK_NULL_HANDLE;
  VkDevice                         dev                = VK_NULL_HANDLE;

  void init(VkDevice adev, VkQueue aqueue, u32 aqueue_family_index,
            f32                                     atimestamp_period,
            VkPhysicalDeviceMemoryProperties const &amemory_properties,
            u32                                     amax_nframes_in_flight)
  {
    max_nframes_in_flight = amax_nframes_in_flight;
    memory_properties     = amemory_properties;
    queue_family_index    = aqueue_family_index;
    queue                 = aqueue;
    dev                   = adev;
    timestamp_period      = atimestamp_period;

    for (u32 i = 0; i < max_nframes_in_flight; i++)
    {
      VecBuffer vertex_buffer;

      vertex_buffer.init(dev, memory_properties,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

      vertex_buffers.push_inplace(vertex_buffer).unwrap();

      VecBuffer index_buffer;

      index_buffer.init(dev, memory_properties,
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

      index_buffers.push_inplace(index_buffer).unwrap();
    }


    cmd_buffers.unsafe_resize_uninitialized(max_nframes_in_flight).unwrap();


    VkQueryPoolCreateInfo pipeline_statistics_query_pool_create_info{
        .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .queryType          = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        .queryCount         = NPIPELINE_STATISTIC_QUERIES,
        .pipelineStatistics = PIPELINE_STATISTIC_QUERIES};

    VkQueryPoolCreateInfo timestamp_query_pool_create_info{
        .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .queryType          = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount         = NPIPELINE_TIMESTAMP_QUERIES,
        .pipelineStatistics = 0};

    for (u32 i = 0; i < max_nframes_in_flight; i++)
    {
      VkQueryPool pipeline_statistics_query_pool;
      ASH_VK_CHECK(
          vkCreateQueryPool(dev, &pipeline_statistics_query_pool_create_info,
                            nullptr, &pipeline_statistics_query_pool));
      pipeline_statistics_query_pools
          .push_inplace(pipeline_statistics_query_pool)
          .unwrap();

      VkQueryPool timestamp_query_pool;
      ASH_VK_CHECK(vkCreateQueryPool(dev, &timestamp_query_pool_create_info,
                                     nullptr, &timestamp_query_pool));
      pipeline_timestamp_query_pools.push_inplace(timestamp_query_pool)
          .unwrap();
    }
  }

  void destroy()
  {
    for (VkQueryPool query_pool : pipeline_statistics_query_pools)
    {
      vkDestroyQueryPool(dev, query_pool, nullptr);
    }

    for (VkQueryPool query_pool : pipeline_timestamp_query_pools)
    {
      vkDestroyQueryPool(dev, query_pool, nullptr);
    }

    vkFreeCommandBuffers(dev, cmd_pool, static_cast<u32>(max_nframes_in_flight),
                         cmd_buffers.data());

    vkDestroyCommandPool(dev, cmd_pool, nullptr);
  }

  {
    Timepoint gpu_sync_begin = Clock::now();

    ASH_VK_CHECK(
        vkWaitForFences(dev, 1, &render_fence, VK_TRUE, VULKAN_TIMEOUT));

    Timepoint gpu_sync_end = Clock::now();

    frame_stats.gpu_sync_time = gpu_sync_end - gpu_sync_begin;

    ASH_VK_CHECK(vkResetFences(dev, 1, &render_fence));

    // u64 pipeline_statistics_query_query_results[NPIPELINE_STATISTIC_QUERIES];
    // u64 pipeline_timestamp_query_results[NPIPELINE_TIMESTAMP_QUERIES];

    // if (vkGetQueryPoolResults(dev, pipeline_statistics_query_pools[frame], 0,
1,
    // sizeof(pipeline_statistics_query_query_results),
    //                           pipeline_statistics_query_query_results,
    //                           sizeof(u64),
    //                           VK_QUERY_RESULT_64_BIT) == VK_SUCCESS)
    // {
    //   frame_stats.input_assembly_vertices =
    //       pipeline_statistics_query_query_results[0];
    //   frame_stats.input_assembly_primitives =
    //       pipeline_statistics_query_query_results[1];
    //   frame_stats.vertex_shader_invocations =
    //       pipeline_statistics_query_query_results[2];
    //   frame_stats.fragment_shader_invocations =
    //       pipeline_statistics_query_query_results[3];
    //   frame_stats.compute_shader_invocations =
    //       pipeline_statistics_query_query_results[4];
    //   frame_stats.task_shader_invocations =
    //       pipeline_statistics_query_query_results[5];
    //   frame_stats.mesh_shader_invocations =
    //       pipeline_statistics_query_query_results[6];
    // }

    // if (vkGetQueryPoolResults(dev, pipeline_timestamp_query_pools[frame], 0,
    //                           NPIPELINE_TIMESTAMP_QUERIES,
    //                           sizeof(pipeline_timestamp_query_results),
    //                           pipeline_timestamp_query_results, sizeof(u64),
    //                           VK_QUERY_RESULT_64_BIT) == VK_SUCCESS)
    // {
    //   frame_stats.gpu_time = Nanoseconds{
    //       (Nanoseconds::rep)(((f64) timestamp_period) *
    //                          (f64) (pipeline_timestamp_query_results[1] -
    //                                 pipeline_timestamp_query_results[0]))};
    // }


    // vkCmdResetQueryPool(cmd_buffer, pipeline_statistics_query_pools[frame],
0,
    //                     NPIPELINE_STATISTIC_QUERIES);
    // vkCmdResetQueryPool(cmd_buffer, pipeline_timestamp_query_pools[frame], 0,
    //                     NPIPELINE_TIMESTAMP_QUERIES);

    // vkCmdBeginQuery(cmd_buffer, pipeline_statistics_query_pools[frame], 0,
0);
    // vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        // pipeline_timestamp_query_pools[frame], 0);

    u32 first_index   = 0;
    u32 vertex_offset = 0;

    // vkCmdEndQuery(cmd_buffer, pipeline_statistics_query_pools[frame], 0);
    // vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //                     pipeline_timestamp_query_pools[frame], 1);

  }

*/

inline gfx::Surface surface_to_vk(VkSurfaceKHR s)
{
  return (gfx::Surface) s;
}

struct FrameContext
{
  gfx::FrameId             tail_frame           = 0;
  gfx::FrameId             current_frame        = 0;
  u32                      ring_index           = 0;
  u32                      max_frames_in_flight = 0;
  gfx::CommandEncoderImpl *encoders             = nullptr;
  VkSemaphore             *acquire              = nullptr;
  VkFence                 *submit_f             = nullptr;
  VkSemaphore             *submit_s             = nullptr;
};

/// @is_optimal: false when vulkan returns that the surface is suboptimal or
/// the description is updated by the user
///
/// @is_out_of_date: can't present anymore
/// @is_optimal: recommended but not necessary to resize
/// @is_zero_sized: swapchain is not receiving presentation requests, because
/// the surface requested a zero sized image extent
struct Swapchain
{
  gfx::SwapchainDesc  desc            = {};
  bool                is_out_of_date  = true;
  bool                is_optimal      = false;
  bool                is_zero_sized   = false;
  gfx::SurfaceFormat  format          = {};
  gfx::ImageUsage     usage           = gfx::ImageUsage::None;
  gfx::PresentMode    present_mode    = gfx::PresentMode::Immediate;
  gfx::Extent         extent          = {};
  gfx::CompositeAlpha composite_alpha = gfx::CompositeAlpha::None;
  Image               image_impls[gfx::MAX_SWAPCHAIN_IMAGES] = {};
  gfx::Image          images[gfx::MAX_SWAPCHAIN_IMAGES]      = {};
  VkImage             vk_images[gfx::MAX_SWAPCHAIN_IMAGES]   = {};
  u32                 num_images                             = 0;
  u32                 current_image                          = 0;
  VkSwapchainKHR      vk_swapchain                           = nullptr;
  VkSurfaceKHR        vk_surface                             = nullptr;
};

struct InstanceInterface
{
  static Result<gfx::InstanceImpl, Status> create(AllocatorImpl allocator,
                                                  Logger       *logger,
                                                  bool enable_validation_layer);
  static void                              destroy(gfx::Instance self);
  static Result<gfx::DeviceImpl, Status>   create_device(
        gfx::Instance self, Span<gfx::DeviceType const> preferred_types,
        Span<gfx::Surface const> compatible_surfaces, AllocatorImpl allocator);
  static gfx::Backend get_backend(gfx::Instance self);
  static void         destroy_device(gfx::Instance self, gfx::Device device);
  static void         destroy_surface(gfx::Instance self, gfx::Surface surface);
};

struct DeviceInterface
{
  static gfx::DeviceProperties get_device_properties(gfx::Device self);
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
  static Result<gfx::DescriptorHeapImpl, Status>
      create_descriptor_heap(gfx::Device                    self,
                             gfx::DescriptorHeapDesc const &desc);
  static Result<gfx::PipelineCache, Status>
      create_pipeline_cache(gfx::Device                   self,
                            gfx::PipelineCacheDesc const &desc);
  static Result<gfx::ComputePipeline, Status>
      create_compute_pipeline(gfx::Device                     self,
                              gfx::ComputePipelineDesc const &desc);
  static Result<gfx::GraphicsPipeline, Status>
      create_graphics_pipeline(gfx::Device                      self,
                               gfx::GraphicsPipelineDesc const &desc);
  static Result<gfx::CommandEncoderImpl, Status>
      create_command_encoder(gfx::Device self, AllocatorImpl allocator);
  static Result<gfx::FrameContext, Status>
      create_frame_context(gfx::Device self, gfx::FrameContextDesc const &desc);
  static Result<gfx::Swapchain, Status>
              create_swapchain(gfx::Device self, gfx::Surface surface,
                               gfx::SwapchainDesc const &desc);
  static void destroy_buffer(gfx::Device self, gfx::Buffer buffer);
  static void destroy_buffer_view(gfx::Device     self,
                                  gfx::BufferView buffer_view);
  static void destroy_image(gfx::Device self, gfx::Image image);
  static void destroy_image_view(gfx::Device self, gfx::ImageView image_view);
  static void destroy_sampler(gfx::Device self, gfx::Sampler sampler);
  static void destroy_shader(gfx::Device self, gfx::Shader shader);
  static void destroy_render_pass(gfx::Device     self,
                                  gfx::RenderPass render_pass);
  static void destroy_framebuffer(gfx::Device      self,
                                  gfx::Framebuffer framebuffer);
  static void destroy_descriptor_set_layout(gfx::Device              self,
                                            gfx::DescriptorSetLayout layout);
  static void destroy_descriptor_heap(gfx::Device             self,
                                      gfx::DescriptorHeapImpl heap);
  static void destroy_pipeline_cache(gfx::Device        self,
                                     gfx::PipelineCache cache);
  static void destroy_compute_pipeline(gfx::Device          self,
                                       gfx::ComputePipeline pipeline);
  static void destroy_graphics_pipeline(gfx::Device           self,
                                        gfx::GraphicsPipeline pipeline);
  static void destroy_command_encoder(gfx::Device             self,
                                      gfx::CommandEncoderImpl encoder);
  static void destroy_frame_context(gfx::Device       self,
                                    gfx::FrameContext frame_context);
  static void destroy_swapchain(gfx::Device self, gfx::Swapchain swapchain);
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
  static Result<Void, Status> wait_idle(gfx::Device self);
  static Result<Void, Status> wait_queue_idle(gfx::Device self);
  static gfx::FrameInfo       get_frame_info(gfx::Device       self,
                                             gfx::FrameContext frame_context);
  static Result<u32, Status>
      get_surface_formats(gfx::Device self, gfx::Surface surface,
                          Span<gfx::SurfaceFormat> formats);
  static Result<u32, Status>
      get_surface_present_modes(gfx::Device self, gfx::Surface surface,
                                Span<gfx::PresentMode> modes);
  static Result<gfx::SurfaceCapabilities, Status>
      get_surface_capabilities(gfx::Device self, gfx::Surface surface);
  static Result<gfx::SwapchainState, Status>
      get_swapchain_state(gfx::Device self, gfx::Swapchain swapchain);
  static Result<Void, Status>
      invalidate_swapchain(gfx::Device self, gfx::Swapchain swapchain,
                           gfx::SwapchainDesc const &desc);
  static Result<Void, Status> begin_frame(gfx::Device       self,
                                          gfx::FrameContext frame_context,
                                          gfx::Swapchain    swapchain);
  static Result<Void, Status> submit_frame(gfx::Device       self,
                                           gfx::FrameContext frame_context,
                                           gfx::Swapchain    swapchain);
};

struct DescriptorHeapInterface
{
  static Result<u32, Status> allocate(gfx::DescriptorHeap self);
  static void collect(gfx::DescriptorHeap self, gfx::FrameId tail_frame);
  static void mark_in_use(gfx::DescriptorHeap self, u32 set,
                          gfx::FrameId current_frame);
  static bool is_in_use(gfx::DescriptorHeap self, u32 set,
                        gfx::FrameId tail_frame);
  static void release(gfx::DescriptorHeap self, u32 set);
  static gfx::DescriptorHeapStats get_stats(gfx::DescriptorHeap self);
  static void                     update(gfx::DescriptorHeap          self,
                                         gfx::DescriptorUpdate const &update);
};

struct CommandEncoderInterface
{
  static void begin_debug_marker(gfx::CommandEncoder self,
                                 Span<char const> region_name, Vec4 color);
  static void end_debug_marker(gfx::CommandEncoder self);
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
  static void resolve_image(gfx::CommandEncoder self, gfx::Image src,
                            gfx::Image                    dst,
                            Span<gfx::ImageResolve const> resolves);
  static void begin_render_pass(
      gfx::CommandEncoder self, gfx::Framebuffer framebuffer,
      gfx::RenderPass render_pass, gfx::Offset render_offset,
      gfx::Extent                   render_extent,
      Span<gfx::Color const>        color_attachments_clear_values,
      Span<gfx::DepthStencil const> depth_stencil_attachment_clear_value);
  static void end_render_pass(gfx::CommandEncoder self);
  static void bind_compute_pipeline(gfx::CommandEncoder  self,
                                    gfx::ComputePipeline pipeline);
  static void bind_graphics_pipeline(gfx::CommandEncoder   self,
                                     gfx::GraphicsPipeline pipeline);
  static void
              bind_descriptor_sets(gfx::CommandEncoder            self,
                                   Span<gfx::DescriptorSet const> descriptor_sets,
                                   Span<u32 const>                dynamic_offsets);
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
  static void draw(gfx::CommandEncoder self, u32 vertex_count,
                   u32 instance_count, u32 first_vertex_id,
                   u32 first_instance_id);
  static void draw_indexed(gfx::CommandEncoder self, u32 first_index,
                           u32 num_indices, i32 vertex_offset,
                           u32 first_instance_id, u32 num_instances);
  static void draw_indirect(gfx::CommandEncoder self, gfx::Buffer buffer,
                            u64 offset, u32 draw_count, u32 stride);
  static void draw_indexed_indirect(gfx::CommandEncoder self,
                                    gfx::Buffer buffer, u64 offset,
                                    u32 draw_count, u32 stride);
};

static gfx::InstanceInterface const instance_interface{
    .destroy         = InstanceInterface::destroy,
    .create_device   = InstanceInterface::create_device,
    .get_backend     = InstanceInterface::get_backend,
    .destroy_device  = InstanceInterface::destroy_device,
    .destroy_surface = InstanceInterface::destroy_surface};

static gfx::DeviceInterface const device_interface{
    .get_device_properties = DeviceInterface::get_device_properties,
    .get_format_properties = DeviceInterface::get_format_properties,
    .create_buffer         = DeviceInterface::create_buffer,
    .create_buffer_view    = DeviceInterface::create_buffer_view,
    .create_image          = DeviceInterface::create_image,
    .create_image_view     = DeviceInterface::create_image_view,
    .create_sampler        = DeviceInterface::create_sampler,
    .create_shader         = DeviceInterface::create_shader,
    .create_render_pass    = DeviceInterface::create_render_pass,
    .create_framebuffer    = DeviceInterface::create_framebuffer,
    .create_descriptor_set_layout =
        DeviceInterface::create_descriptor_set_layout,
    .create_descriptor_heap   = DeviceInterface::create_descriptor_heap,
    .create_pipeline_cache    = DeviceInterface::create_pipeline_cache,
    .create_compute_pipeline  = DeviceInterface::create_compute_pipeline,
    .create_graphics_pipeline = DeviceInterface::create_graphics_pipeline,
    .create_frame_context     = DeviceInterface::create_frame_context,
    .create_swapchain         = DeviceInterface::create_swapchain,
    .destroy_buffer           = DeviceInterface::destroy_buffer,
    .destroy_buffer_view      = DeviceInterface::destroy_buffer_view,
    .destroy_image            = DeviceInterface::destroy_image,
    .destroy_image_view       = DeviceInterface::destroy_image_view,
    .destroy_sampler          = DeviceInterface::destroy_sampler,
    .destroy_shader           = DeviceInterface::destroy_shader,
    .destroy_render_pass      = DeviceInterface::destroy_render_pass,
    .destroy_framebuffer      = DeviceInterface::destroy_framebuffer,
    .destroy_descriptor_set_layout =
        DeviceInterface::destroy_descriptor_set_layout,
    .destroy_descriptor_heap   = DeviceInterface::destroy_descriptor_heap,
    .destroy_pipeline_cache    = DeviceInterface::destroy_pipeline_cache,
    .destroy_compute_pipeline  = DeviceInterface::destroy_compute_pipeline,
    .destroy_graphics_pipeline = DeviceInterface::destroy_graphics_pipeline,
    .destroy_frame_context     = DeviceInterface::destroy_frame_context,
    .destroy_swapchain         = DeviceInterface::destroy_swapchain,
    .get_buffer_memory_map     = DeviceInterface::get_buffer_memory_map,
    .invalidate_buffer_memory_map =
        DeviceInterface::invalidate_buffer_memory_map,
    .flush_buffer_memory_map   = DeviceInterface::flush_buffer_memory_map,
    .get_pipeline_cache_size   = DeviceInterface::get_pipeline_cache_size,
    .get_pipeline_cache_data   = DeviceInterface::get_pipeline_cache_data,
    .merge_pipeline_cache      = DeviceInterface::merge_pipeline_cache,
    .wait_idle                 = DeviceInterface::wait_idle,
    .wait_queue_idle           = DeviceInterface::wait_queue_idle,
    .get_frame_info            = DeviceInterface::get_frame_info,
    .get_surface_formats       = DeviceInterface::get_surface_formats,
    .get_surface_present_modes = DeviceInterface::get_surface_present_modes,
    .get_surface_capabilities  = DeviceInterface::get_surface_capabilities,
    .get_swapchain_state       = DeviceInterface::get_swapchain_state,
    .invalidate_swapchain      = DeviceInterface::invalidate_swapchain,
    .begin_frame               = DeviceInterface::begin_frame,
    .submit_frame              = DeviceInterface::submit_frame};

static gfx::DescriptorHeapInterface const descriptor_heap_interface{
    .allocate    = DescriptorHeapInterface::allocate,
    .collect     = DescriptorHeapInterface::collect,
    .mark_in_use = DescriptorHeapInterface::mark_in_use,
    .is_in_use   = DescriptorHeapInterface::is_in_use,
    .release     = DescriptorHeapInterface::release,
    .get_stats   = DescriptorHeapInterface::get_stats,
    .update      = DescriptorHeapInterface::update};

static gfx::CommandEncoderInterface const command_encoder_interface{
    .begin_debug_marker = CommandEncoderInterface::begin_debug_marker,
    .end_debug_marker   = CommandEncoderInterface::end_debug_marker,
    .fill_buffer        = CommandEncoderInterface::fill_buffer,
    .copy_buffer        = CommandEncoderInterface::copy_buffer,
    .update_buffer      = CommandEncoderInterface::update_buffer,
    .clear_color_image  = CommandEncoderInterface::clear_color_image,
    .clear_depth_stencil_image =
        CommandEncoderInterface::clear_depth_stencil_image,
    .copy_image             = CommandEncoderInterface::copy_image,
    .copy_buffer_to_image   = CommandEncoderInterface::copy_buffer_to_image,
    .blit_image             = CommandEncoderInterface::blit_image,
    .resolve_image          = CommandEncoderInterface::resolve_image,
    .begin_render_pass      = CommandEncoderInterface::begin_render_pass,
    .end_render_pass        = CommandEncoderInterface::end_render_pass,
    .bind_compute_pipeline  = CommandEncoderInterface::bind_compute_pipeline,
    .bind_graphics_pipeline = CommandEncoderInterface::bind_graphics_pipeline,
    .bind_descriptor_sets   = CommandEncoderInterface::bind_descriptor_sets,
    .push_constants         = CommandEncoderInterface::push_constants,
    .dispatch               = CommandEncoderInterface::dispatch,
    .dispatch_indirect      = CommandEncoderInterface::dispatch_indirect,
    .set_viewport           = CommandEncoderInterface::set_viewport,
    .set_scissor            = CommandEncoderInterface::set_scissor,
    .set_blend_constants    = CommandEncoderInterface::set_blend_constants,
    .set_stencil_compare_mask =
        CommandEncoderInterface::set_stencil_compare_mask,
    .set_stencil_reference  = CommandEncoderInterface::set_stencil_reference,
    .set_stencil_write_mask = CommandEncoderInterface::set_stencil_write_mask,
    .bind_vertex_buffers    = CommandEncoderInterface::bind_vertex_buffers,
    .bind_index_buffer      = CommandEncoderInterface::bind_index_buffer,
    .draw                   = CommandEncoderInterface::draw,
    .draw_indexed           = CommandEncoderInterface::draw_indexed,
    .draw_indirect          = CommandEncoderInterface::draw_indirect,
    .draw_indexed_indirect  = CommandEncoderInterface::draw_indexed_indirect};

}        // namespace vk
}        // namespace ash
