#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include "ashura/allocator.h"
#include "ashura/gfx.h"
#include "stx/vec.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

constexpr char const *REQUIRED_INSTANCE_EXTENSIONS[] = {""};
constexpr char const *OPTIONAL_INSTANCE_EXTENSIONS[] = {""};
constexpr char const *REQUIRED_DEVICE_EXTENSIONS[]   = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr char const *OPTIONAL_DEVICE_EXTENSIONS[]   = {
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_EXT_MESH_SHADER_EXTENSION_NAME};
// TODO(lamarrr): specify polyfills

// only for purely trivial types
template <typename T>
struct Vec
{
  T  *data     = nullptr;
  u32 size     = 0;
  u32 capacity = 0;

  gfx::Status reserve(AllocatorImpl const &allocator, u32 target_size)
  {
    if (target_size <= capacity) [[unlikely]]
    {
      return gfx::Status::Success;
    }
    usize const target_capacity = target_size + (target_size >> 1);
    T *new_data = (T *) allocator.reallocate(data, sizeof(T) * target_capacity, alignof(T));
    if (new_data == nullptr) [[unlikely]]
    {
      return gfx::Status::OutOfHostMemory;
    }
    data     = new_data;
    capacity = target_capacity;
    return gfx::Status::Success;
  }

  gfx::Status grow_size(AllocatorImpl const &allocator, u32 growth)
  {
    gfx::Status status = reserve(allocator, size + growth);
    if (status != gfx::Status::Success) [[unlikely]]
    {
      return status;
    }
    size += growth;
    return gfx::Status::Success;
  }

  void fill(T const &element, u32 begin, u32 num)
  {
    for (u32 i = begin; i < begin + num && i < size; i++)
    {
      data[i] = element;
    }
  }

  gfx::Status push(AllocatorImpl const &allocator, T const &element)
  {
    gfx::Status status = reserve(allocator, size + 1);
    if (status != gfx::Status::Success) [[unlikely]]
    {
      return status;
    }
    data[size] = element;
    size++;

    return gfx::Status::Success;
  }

  void shrink_to_fit();
  bool is_shrink_recommended();

  void clear()
  {
    size = 0;
  }

  void deallocate(AllocatorImpl const &allocator)
  {
    allocator.deallocate(data);
    data     = nullptr;
    size     = 0;
    capacity = 0;
  }

  T *begin()
  {
    return data;
  }

  T *end()
  {
    return data + size;
  }
};

struct InstanceTable
{
  PFN_vkCreateDebugReportCallbackEXT     CreateDebugReportCallbackEXT     = nullptr;
  PFN_vkCreateDebugUtilsMessengerEXT     CreateDebugUtilsMessengerEXT     = nullptr;
  PFN_vkCreateInstance                   CreateInstance                   = nullptr;
  PFN_vkDebugReportMessageEXT            DebugReportMessageEXT            = nullptr;
  PFN_vkDestroyDebugReportCallbackEXT    DestroyDebugReportCallbackEXT    = nullptr;
  PFN_vkDestroyDebugUtilsMessengerEXT    DestroyDebugUtilsMessengerEXT    = nullptr;
  PFN_vkDestroyInstance                  DestroyInstance                  = nullptr;
  PFN_vkDestroySurfaceKHR                DestroySurfaceKHR                = nullptr;
  PFN_vkEnumeratePhysicalDeviceGroups    EnumeratePhysicalDeviceGroups    = nullptr;
  PFN_vkEnumeratePhysicalDeviceGroupsKHR EnumeratePhysicalDeviceGroupsKHR = nullptr;
  PFN_vkEnumeratePhysicalDevices         EnumeratePhysicalDevices         = nullptr;
  PFN_vkGetInstanceProcAddr              GetInstanceProcAddr              = nullptr;
  PFN_vkSubmitDebugUtilsMessageEXT       SubmitDebugUtilsMessageEXT       = nullptr;

  PFN_vkCreateDevice                           CreateDevice                           = nullptr;
  PFN_vkEnumerateDeviceExtensionProperties     EnumerateDeviceExtensionProperties     = nullptr;
  PFN_vkEnumerateDeviceLayerProperties         EnumerateDeviceLayerProperties         = nullptr;
  PFN_vkGetPhysicalDeviceFeatures              GetPhysicalDeviceFeatures              = nullptr;
  PFN_vkGetPhysicalDeviceFormatProperties      GetPhysicalDeviceFormatProperties      = nullptr;
  PFN_vkGetPhysicalDeviceImageFormatProperties GetPhysicalDeviceImageFormatProperties = nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties      GetPhysicalDeviceMemoryProperties      = nullptr;
  PFN_vkGetPhysicalDeviceProperties            GetPhysicalDeviceProperties            = nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties = nullptr;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties GetPhysicalDeviceSparseImageFormatProperties =
      nullptr;
};

// some systems have multiple vulkan implementations! dynamic loading
// VERSION 1.1 Vulkan Functions
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

  PFN_vkDebugMarkerSetObjectTagEXT  DebugMarkerSetObjectTagEXT  = nullptr;
  PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT = nullptr;

  PFN_vkCmdDebugMarkerBeginEXT  CmdDebugMarkerBeginEXT  = nullptr;
  PFN_vkCmdDebugMarkerEndEXT    CmdDebugMarkerEndEXT    = nullptr;
  PFN_vkCmdDebugMarkerInsertEXT CmdDebugMarkerInsertEXT = nullptr;
};

// TODO(lamarrr): CHECKS push constant size match check
// NOTE: renderpass attachments MUST not be accessed in shaders within that renderpass
// NOTE: update_buffer and fill_buffer MUST be multiple of 4 for dst offset and dst size
struct BufferAccess
{
  VkShaderStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags      access = VK_ACCESS_NONE;
};

struct ImageAccess
{
  VkShaderStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags      access = VK_ACCESS_NONE;
  VkImageLayout      layout = VK_IMAGE_LAYOUT_UNDEFINED;
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
  u64                 refcount       = 0;
  gfx::RenderPassDesc desc           = {};
  VkRenderPass        vk_render_pass = nullptr;
};

struct Framebuffer
{
  u64                  refcount       = 0;
  gfx::FramebufferDesc desc           = {};
  VkFramebuffer        vk_framebuffer = nullptr;
};

struct Shader
{
  u64            refcount  = 0;
  VkShaderModule vk_shader = nullptr;
};

struct DescriptorSetLayout
{
  u64                         refcount      = 0;
  gfx::DescriptorBindingDesc *binding_descs = nullptr;
  u32                         num_bindings  = 0;
  VkDescriptorSetLayout       vk_layout     = nullptr;
  // gfx::DescriptorCount count() const;
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

struct Swapchain
{
  // TODO(lamarrr): swapchain image should abort on ref and unref calls?
  // not is_weak
  Image     *images      = nullptr;
  ImageView *image_views = nullptr;
  u32        num_images  = 0;
  // framebuffer & renderpass binding?
};

struct Device
{
  u64                refcount          = 0;
  AllocatorImpl      allocator         = {};
  InstanceTable      vk_instance_table = {};
  DeviceTable        vk_table          = {};
  VmaVulkanFunctions vma_table         = {};
  VkInstance         vk_instance       = nullptr;
  VkPhysicalDevice   vk_phy_device     = nullptr;
  VkDevice           vk_device         = nullptr;
  VkQueue            vk_queue          = nullptr;
  VmaAllocator       vma_allocator     = nullptr;
  VkSwapchainKHR     vk_swapchain      = nullptr;
  Swapchain          swapchain         = {};
};

struct DescriptorPoolStats
{
  u32 num_released = 0;
  u32 num_free     = 0;
};

/// struct DescriptorHeap - // ntypes * nsets
// for each group
// can be updated independently
// must be allocated and freed together
// must
/// @group_set_strides: stride of sets within groups
/// @set_binding_strides: stride of bindings within the set
/// @vk_descriptor_sets:  multiple of nlayouts
///
/// for all sets in released indices if last used tick < trailing_frame_tick, move to free indices
// pop index from pool_free_sets if any, otherwise create new pool and add and allocate new free
// sets from that descriptor set can't be reused, destroyed or modified until its no longer in use
struct DescriptorHeap
{
  u64                  refcount            = 0;
  DescriptorSetLayout *set_layouts         = nullptr;
  u32                 *group_set_strides   = nullptr;
  u32                 *set_binding_strides = nullptr;
  VkDescriptorPool    *vk_pools            = nullptr;
  DescriptorPoolStats *pool_stats          = nullptr;
  VkDescriptorSet     *vk_descriptor_sets  = nullptr;
  u64                 *last_use_frame      = nullptr;
  u32                 *released_groups     = nullptr;
  u32                 *free_groups         = nullptr;
  void                *bindings            = 0;
  u32                  num_group_sets      = 0;
  u32                  num_pools           = 0;
  u32                  num_pool_groups     = 0;
  u32                  num_released_groups = 0;
  u32                  num_free_groups     = 0;
  u32                  bindings_stride     = 0;
  bool                 can_shrink          = false;
};

struct CommandEncoder
{
  u64                        refcount          = 0;
  AllocatorImpl              allocator         = {};
  Device                    *device            = nullptr;
  VkCommandPool              vk_command_pool   = nullptr;
  VkCommandBuffer            vk_command_buffer = nullptr;
  ComputePipeline           *compute_pipeline  = nullptr;
  GraphicsPipeline          *graphics_pipeline = nullptr;
  Framebuffer               *framebuffer       = nullptr;
  DescriptorHeap            *bound_descriptor_set_heaps[gfx::MAX_PIPELINE_DESCRIPTOR_SETS]  = {};
  u32                        bound_descriptor_set_groups[gfx::MAX_PIPELINE_DESCRIPTOR_SETS] = {};
  u32                        bound_descriptor_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS]       = {};
  u32                        num_bound_descriptor_sets                                      = 0;
  Vec<stx::UniqueFn<void()>> completion_tasks                                               = {};
  gfx::Status                status = gfx::Status::Success;
};

struct DeviceInterface
{
  static void                                       ref(gfx::Device self);
  static void                                       unref(gfx::Device self);
  static Result<gfx::DeviceInfo, gfx::Status>       get_device_info(gfx::Device self);
  static Result<gfx::FormatProperties, gfx::Status> get_format_properties(gfx::Device self,
                                                                          gfx::Format format);
  static Result<gfx::Buffer, gfx::Status>           create_buffer(gfx::Device            self,
                                                                  gfx::BufferDesc const &desc);
  static Result<gfx::BufferView, gfx::Status>       create_buffer_view(gfx::Device                self,
                                                                       gfx::BufferViewDesc const &desc);
  static Result<gfx::Image, gfx::Status> create_image(gfx::Device self, gfx::ImageDesc const &desc);
  static Result<gfx::ImageView, gfx::Status>   create_image_view(gfx::Device               self,
                                                                 gfx::ImageViewDesc const &desc);
  static Result<gfx::Sampler, gfx::Status>     create_sampler(gfx::Device             self,
                                                              gfx::SamplerDesc const &desc);
  static Result<gfx::Shader, gfx::Status>      create_shader(gfx::Device            self,
                                                             gfx::ShaderDesc const &desc);
  static Result<gfx::RenderPass, gfx::Status>  create_render_pass(gfx::Device                self,
                                                                  gfx::RenderPassDesc const &desc);
  static Result<gfx::Framebuffer, gfx::Status> create_framebuffer(gfx::Device                 self,
                                                                  gfx::FramebufferDesc const &desc);
  static Result<gfx::DescriptorSetLayout, gfx::Status>
      create_descriptor_set_layout(gfx::Device self, gfx::DescriptorSetLayoutDesc const &desc);
  static Result<gfx::DescriptorHeapImpl, gfx::Status> create_descriptor_heap(
      gfx::Device self, Span<gfx::DescriptorSetLayout const> descriptor_set_layouts, u32 pool_size);
  static Result<gfx::PipelineCache, gfx::Status>
      create_pipeline_cache(gfx::Device self, gfx::PipelineCacheDesc const &desc);
  static Result<gfx::ComputePipeline, gfx::Status>
      create_compute_pipeline(gfx::Device self, gfx::ComputePipelineDesc const &desc);
  static Result<gfx::GraphicsPipeline, gfx::Status>
      create_graphics_pipeline(gfx::Device self, gfx::GraphicsPipelineDesc const &desc);
  static Result<gfx::Fence, gfx::Status>              create_fence(gfx::Device self, bool signaled);
  static Result<gfx::CommandEncoderImpl, gfx::Status> create_command_encoder(gfx::Device self);
  static void ref_buffer(gfx::Device self, gfx::Buffer buffer);
  static void ref_buffer_view(gfx::Device self, gfx::BufferView buffer_view);
  static void ref_image(gfx::Device self, gfx::Image image);
  static void ref_image_view(gfx::Device self, gfx::ImageView image_view);
  static void ref_sampler(gfx::Device self, gfx::Sampler sampler);
  static void ref_shader(gfx::Device self, gfx::Shader shader);
  static void ref_render_pass(gfx::Device self, gfx::RenderPass render_pass);
  static void ref_framebuffer(gfx::Device self, gfx::Framebuffer framebuffer);
  static void ref_descriptor_set_layout(gfx::Device self, gfx::DescriptorSetLayout layout);
  static void ref_descriptor_heap(gfx::Device self, gfx::DescriptorHeapImpl const &heap);
  static void ref_pipeline_cache(gfx::Device self, gfx::PipelineCache cache);
  static void ref_compute_pipeline(gfx::Device self, gfx::ComputePipeline pipeline);
  static void ref_graphics_pipeline(gfx::Device self, gfx::GraphicsPipeline pipeline);
  static void ref_fence(gfx::Device self, gfx::Fence fence);
  static void ref_command_encoder(gfx::Device self, gfx::CommandEncoderImpl const &encoder);
  static void unref_buffer(gfx::Device self, gfx::Buffer buffer);
  static void unref_buffer_view(gfx::Device self, gfx::BufferView buffer_view);
  static void unref_image(gfx::Device self, gfx::Image image);
  static void unref_image_view(gfx::Device self, gfx::ImageView image_view);
  static void unref_sampler(gfx::Device self, gfx::Sampler sampler);
  static void unref_shader(gfx::Device self, gfx::Shader shader);
  static void unref_render_pass(gfx::Device self, gfx::RenderPass render_pass);
  static void unref_framebuffer(gfx::Device self, gfx::Framebuffer framebuffer);
  static void unref_descriptor_set_layout(gfx::Device self, gfx::DescriptorSetLayout layout);
  static void unref_descriptor_heap(gfx::Device self, gfx::DescriptorHeapImpl const &heap);
  static void unref_pipeline_cache(gfx::Device self, gfx::PipelineCache cache);
  static void unref_compute_pipeline(gfx::Device self, gfx::ComputePipeline pipeline);
  static void unref_graphics_pipeline(gfx::Device self, gfx::GraphicsPipeline pipeline);
  static void unref_fence(gfx::Device self, gfx::Fence fence);
  static void unref_command_encoder(gfx::Device self, gfx::CommandEncoderImpl const &encoder);
  static Result<void *, gfx::Status> get_buffer_memory_map(gfx::Device self, gfx::Buffer buffer);
  static gfx::Status invalidate_buffer_memory_map(gfx::Device self, gfx::Buffer buffer,
                                                  gfx::MemoryRange ranges);
  static gfx::Status flush_buffer_memory_map(gfx::Device self, gfx::Buffer buffer,
                                             gfx::MemoryRange range);
  static Result<usize, gfx::Status> get_pipeline_cache_size(gfx::Device        self,
                                                            gfx::PipelineCache cache);
  static Result<usize, gfx::Status> get_pipeline_cache_data(gfx::Device        self,
                                                            gfx::PipelineCache cache, Span<u8> out);
  static gfx::Status                merge_pipeline_cache(gfx::Device self, gfx::PipelineCache dst,
                                                         Span<gfx::PipelineCache const> srcs);
  static gfx::Status wait_for_fences(gfx::Device self, Span<gfx::Fence const> fences, bool all,
                                     nanoseconds timeout);
  static gfx::Status reset_fences(gfx::Device self, Span<gfx::Fence const> fences);
  static gfx::Status get_fence_status(gfx::Device self, gfx::Fence fence);
  static gfx::Status submit(gfx::Device self, gfx::CommandEncoder encoder,
                            gfx::Fence signal_fence);
  static gfx::Status wait_idle(gfx::Device self);
  static gfx::Status wait_queue_idle(gfx::Device self);
};

struct DescriptorHeapInterface
{
  static Result<u32, gfx::Status> add(gfx::DescriptorHeap self);
  static void                     update(gfx::DescriptorHeap self, u32 group, u32 set,
                                         Span<gfx::DescriptorBinding const> bindings);
  static void mark_in_use(gfx::DescriptorHeap self, u32 group, gfx::FrameId current_frame);
  static bool is_in_use(gfx::DescriptorHeap self, u32 group, gfx::FrameId trailing_frame);
  static void release(gfx::DescriptorHeap self, u32 group);
  static gfx::DescriptorHeapStats get_stats(gfx::DescriptorHeap self);
  static void                     tick(gfx::DescriptorHeap self, gfx::FrameId trailing_frame);
};

// TODO(lamarrr): min of minUniformBufferOffsetAlignment for dynamicbufferoffset
struct CommandEncoderInterface
{
  static void        begin(gfx::CommandEncoder self);
  static gfx::Status end(gfx::CommandEncoder self);
  static void        reset(gfx::CommandEncoder self);
  static void begin_debug_marker(gfx::CommandEncoder self, char const *region_name, Vec4 color);
  static void end_debug_marker(gfx::CommandEncoder self);
  static void fill_buffer(gfx::CommandEncoder self, gfx::Buffer dst, u64 offset, u64 size,
                          u32 data);
  static void copy_buffer(gfx::CommandEncoder self, gfx::Buffer src, gfx::Buffer dst,
                          Span<gfx::BufferCopy const> copies);
  static void update_buffer(gfx::CommandEncoder self, Span<u8 const> src, u64 dst_offset,
                            gfx::Buffer dst);
  static void clear_color_image(gfx::CommandEncoder self, gfx::Image dst, gfx::Color clear_color,
                                Span<gfx::ImageSubresourceRange const> ranges);
  static void clear_depth_stencil_image(gfx::CommandEncoder self, gfx::Image dst,
                                        gfx::DepthStencil                      clear_depth_stencil,
                                        Span<gfx::ImageSubresourceRange const> ranges);
  static void copy_image(gfx::CommandEncoder self, gfx::Image src, gfx::Image dst,
                         Span<gfx::ImageCopy const> copies);
  static void copy_buffer_to_image(gfx::CommandEncoder self, gfx::Buffer src, gfx::Image dst,
                                   Span<gfx::BufferImageCopy const> copies);
  static void blit_image(gfx::CommandEncoder self, gfx::Image src, gfx::Image dst,
                         Span<gfx::ImageBlit const> blits, gfx::Filter filter);
  static void
              begin_render_pass(gfx::CommandEncoder self, gfx::Framebuffer framebuffer,
                                gfx::RenderPass render_pass, IRect render_area,
                                Span<gfx::Color const>        color_attachments_clear_values,
                                Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values);
  static void end_render_pass(gfx::CommandEncoder self);
  static void bind_compute_pipeline(gfx::CommandEncoder self, gfx::ComputePipeline pipeline);
  static void bind_graphics_pipeline(gfx::CommandEncoder self, gfx::GraphicsPipeline pipeline);
  static void bind_descriptor_sets(gfx::CommandEncoder             self,
                                   Span<gfx::DescriptorHeap const> descriptor_heaps,
                                   Span<u32 const> groups, Span<u32 const> sets,
                                   Span<u32 const> dynamic_offsets);
  static void push_constants(gfx::CommandEncoder self, Span<u8 const> push_constants_data);
  static void dispatch(gfx::CommandEncoder self, u32 group_count_x, u32 group_count_y,
                       u32 group_count_z);
  static void dispatch_indirect(gfx::CommandEncoder self, gfx::Buffer buffer, u64 offset);
  static void set_viewport(gfx::CommandEncoder self, gfx::Viewport const &viewport);
  static void set_scissor(gfx::CommandEncoder self, IRect scissor);
  static void set_blend_constants(gfx::CommandEncoder self, Vec4 blend_constants);
  static void set_stencil_compare_mask(gfx::CommandEncoder self, gfx::StencilFaces faces, u32 mask);
  static void set_stencil_reference(gfx::CommandEncoder self, gfx::StencilFaces faces,
                                    u32 reference);
  static void set_stencil_write_mask(gfx::CommandEncoder self, gfx::StencilFaces faces, u32 mask);
  static void set_vertex_buffers(gfx::CommandEncoder self, Span<gfx::Buffer const> vertex_buffers,
                                 Span<u64 const> offsets);
  static void set_index_buffer(gfx::CommandEncoder self, gfx::Buffer index_buffer, u64 offset);
  static void draw(gfx::CommandEncoder self, u32 first_index, u32 num_indices, i32 vertex_offset,
                   u32 first_instance, u32 num_instances);
  static void draw_indirect(gfx::CommandEncoder self, gfx::Buffer buffer, u64 offset,
                            u32 draw_count, u32 stride);
  static void on_execution_complete(gfx::CommandEncoder self, stx::UniqueFn<void()> &&fn);
};

gfx::DeviceInterface const device_interface{
    .ref                          = DeviceInterface::ref,
    .unref                        = DeviceInterface::unref,
    .get_device_info              = DeviceInterface::get_device_info,
    .get_format_properties        = DeviceInterface::get_format_properties,
    .create_buffer                = DeviceInterface::create_buffer,
    .create_buffer_view           = DeviceInterface::create_buffer_view,
    .create_image                 = DeviceInterface::create_image,
    .create_image_view            = DeviceInterface::create_image_view,
    .create_sampler               = DeviceInterface::create_sampler,
    .create_shader                = DeviceInterface::create_shader,
    .create_render_pass           = DeviceInterface::create_render_pass,
    .create_framebuffer           = DeviceInterface::create_framebuffer,
    .create_descriptor_set_layout = DeviceInterface::create_descriptor_set_layout,
    .create_descriptor_heap       = DeviceInterface::create_descriptor_heap,
    .create_pipeline_cache        = DeviceInterface::create_pipeline_cache,
    .create_compute_pipeline      = DeviceInterface::create_compute_pipeline,
    .create_fence                 = DeviceInterface::create_fence,
    .ref_buffer                   = DeviceInterface::ref_buffer,
    .ref_buffer_view              = DeviceInterface::ref_buffer_view,
    .ref_image                    = DeviceInterface::ref_image,
    .ref_image_view               = DeviceInterface::ref_image_view,
    .ref_sampler                  = DeviceInterface::ref_sampler,
    .ref_shader                   = DeviceInterface::ref_shader,
    .ref_render_pass              = DeviceInterface::ref_render_pass,
    .ref_framebuffer              = DeviceInterface::ref_framebuffer,
    .ref_descriptor_set_layout    = DeviceInterface::ref_descriptor_set_layout,
    .ref_descriptor_heap          = DeviceInterface::ref_descriptor_heap,
    .ref_pipeline_cache           = DeviceInterface::ref_pipeline_cache,
    .ref_compute_pipeline         = DeviceInterface::ref_compute_pipeline,
    .ref_fence                    = DeviceInterface::ref_fence,
    .ref_command_encoder          = DeviceInterface::ref_command_encoder,
    .unref_buffer                 = DeviceInterface::unref_buffer,
    .unref_buffer_view            = DeviceInterface::unref_buffer_view,
    .unref_image                  = DeviceInterface::unref_image,
    .unref_image_view             = DeviceInterface::unref_image_view,
    .unref_sampler                = DeviceInterface::unref_sampler,
    .unref_shader                 = DeviceInterface::unref_shader,
    .unref_render_pass            = DeviceInterface::unref_render_pass,
    .unref_framebuffer            = DeviceInterface::unref_framebuffer,
    .unref_descriptor_set_layout  = DeviceInterface::unref_descriptor_set_layout,
    .unref_descriptor_heap        = DeviceInterface::unref_descriptor_heap,
    .unref_pipeline_cache         = DeviceInterface::unref_pipeline_cache,
    .unref_compute_pipeline       = DeviceInterface::unref_compute_pipeline,
    .unref_fence                  = DeviceInterface::unref_fence,
    .unref_command_encoder        = DeviceInterface::unref_command_encoder,
    .get_buffer_memory_map        = DeviceInterface::get_buffer_memory_map,
    .invalidate_buffer_memory_map = DeviceInterface::invalidate_buffer_memory_map,
    .flush_buffer_memory_map      = DeviceInterface::flush_buffer_memory_map,
    .get_pipeline_cache_size      = DeviceInterface::get_pipeline_cache_size,
    .get_pipeline_cache_data      = DeviceInterface::get_pipeline_cache_data,
    .merge_pipeline_cache         = DeviceInterface::merge_pipeline_cache,
    .wait_for_fences              = DeviceInterface::wait_for_fences,
    .reset_fences                 = DeviceInterface::reset_fences,
    .get_fence_status             = DeviceInterface::get_fence_status,
    .submit                       = DeviceInterface::submit,
    .wait_idle                    = DeviceInterface::wait_idle,
    .wait_queue_idle              = DeviceInterface::wait_queue_idle};

gfx::DescriptorHeapInterface const descriptor_heap_interface{
    .add         = DescriptorHeapInterface::add,
    .update      = DescriptorHeapInterface::update,
    .mark_in_use = DescriptorHeapInterface::mark_in_use,
    .is_in_use   = DescriptorHeapInterface::is_in_use,
    .release     = DescriptorHeapInterface::release,
    .get_stats   = DescriptorHeapInterface::get_stats,
    .tick        = DescriptorHeapInterface::tick};

gfx::CommandEncoderInterface const command_encoder_interface{
    .begin                     = CommandEncoderInterface::begin,
    .end                       = CommandEncoderInterface::end,
    .begin_debug_marker        = CommandEncoderInterface::begin_debug_marker,
    .end_debug_marker          = CommandEncoderInterface::end_debug_marker,
    .fill_buffer               = CommandEncoderInterface::fill_buffer,
    .copy_buffer               = CommandEncoderInterface::copy_buffer,
    .update_buffer             = CommandEncoderInterface::update_buffer,
    .clear_color_image         = CommandEncoderInterface::clear_color_image,
    .clear_depth_stencil_image = CommandEncoderInterface::clear_depth_stencil_image,
    .copy_image                = CommandEncoderInterface::copy_image,
    .copy_buffer_to_image      = CommandEncoderInterface::copy_buffer_to_image,
    .blit_image                = CommandEncoderInterface::blit_image,
    .begin_render_pass         = CommandEncoderInterface::begin_render_pass,
    .end_render_pass           = CommandEncoderInterface::end_render_pass,
    .bind_compute_pipeline     = CommandEncoderInterface::bind_compute_pipeline,
    .bind_graphics_pipeline    = CommandEncoderInterface::bind_graphics_pipeline,
    .bind_descriptor_sets      = CommandEncoderInterface::bind_descriptor_sets,
    .push_constants            = CommandEncoderInterface::push_constants,
    .dispatch                  = CommandEncoderInterface::dispatch,
    .dispatch_indirect         = CommandEncoderInterface::dispatch_indirect,
    .set_viewport              = CommandEncoderInterface::set_viewport,
    .set_scissor               = CommandEncoderInterface::set_scissor,
    .set_blend_constants       = CommandEncoderInterface::set_blend_constants,
    .set_stencil_compare_mask  = CommandEncoderInterface::set_stencil_compare_mask,
    .set_stencil_reference     = CommandEncoderInterface::set_stencil_reference,
    .set_stencil_write_mask    = CommandEncoderInterface::set_stencil_write_mask,
    .set_vertex_buffers        = CommandEncoderInterface::set_vertex_buffers,
    .set_index_buffer          = CommandEncoderInterface::set_index_buffer,
    .draw                      = CommandEncoderInterface::draw,
    .draw_indirect             = CommandEncoderInterface::draw_indirect,
    .on_execution_complete     = CommandEncoderInterface::on_execution_complete};

}        // namespace vk
}        // namespace ash
