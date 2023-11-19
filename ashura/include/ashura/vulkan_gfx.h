#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include "ashura/allocator.h"
#include "ashura/gfx.h"
#include "stx/vec.h"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"
#include <map>

namespace ash
{
namespace vk
{

// only for purely trivial types
template <typename T>
struct Vec
{
  T  *data     = nullptr;
  u32 count    = 0;
  u32 capacity = 0;

  gfx::Status reserve(AllocationCallbacks const &allocator, u32 target_size)
  {
    if (target_size <= capacity) [[unlikely]]
    {
      return gfx::Status::Success;
    }
    usize const target_capacity = target_size + (target_size >> 1);
    T          *new_data =
        (T *) allocator.reallocate(allocator.data, data, sizeof(T) * target_capacity, alignof(T));
    if (new_data == nullptr) [[unlikely]]
    {
      return gfx::Status::OutOfHostMemory;
    }
    data     = new_data;
    capacity = target_capacity;
    return gfx::Status::Success;
  }

  gfx::Status grow_count(AllocationCallbacks const &allocator, u32 growth)
  {
    gfx::Status status = reserve(allocator, count + growth);
    if (status != gfx::Status::Success) [[unlikely]]
    {
      return status;
    }
    count += growth;
    return gfx::Status::Success;
  }

  void fill(T const &element, u32 begin, u32 num)
  {
    for (u32 i = begin; i < begin + num && i < count; i++)
    {
      data[i] = element;
    }
  }

  gfx::Status push(AllocationCallbacks const &allocator, T const &element)
  {
    gfx::Status status = reserve(allocator, count + 1);
    if (status != gfx::Status::Success) [[unlikely]]
    {
      return status;
    }
    data[count] = element;
    count++;

    return gfx::Status::Success;
  }

  void shrink_to_fit();
  bool is_shrink_recommended();

  void clear()
  {
    count = 0;
  }

  void deallocate(AllocationCallbacks const &allocator)
  {
    allocator.deallocate(allocator.data, data);
    data     = nullptr;
    count    = 0;
    capacity = 0;
  }

  T *begin()
  {
    return data;
  }

  T *end()
  {
    return data + count;
  }
};

// some systems have multiple vulkan implementations! dynamic loading
// VERSION 1.1 Vulkan Functions
struct DeviceTable
{
#define ASH_VKDEV_FUNC(func_name) PFN_##vk##func_name func_name = nullptr

  // DEVICE OBJECT FUNCTIONS
  ASH_VKDEV_FUNC(AllocateCommandBuffers);
  ASH_VKDEV_FUNC(AllocateDescriptorSets);
  ASH_VKDEV_FUNC(AllocateMemory);
  ASH_VKDEV_FUNC(BindBufferMemory);
  ASH_VKDEV_FUNC(BindImageMemory);
  ASH_VKDEV_FUNC(CreateBuffer);
  ASH_VKDEV_FUNC(CreateBufferView);
  ASH_VKDEV_FUNC(CreateCommandPool);
  ASH_VKDEV_FUNC(CreateComputePipelines);
  ASH_VKDEV_FUNC(CreateDescriptorPool);
  ASH_VKDEV_FUNC(CreateDescriptorSetLayout);
  ASH_VKDEV_FUNC(CreateDevice);
  ASH_VKDEV_FUNC(CreateEvent);
  ASH_VKDEV_FUNC(CreateFence);
  ASH_VKDEV_FUNC(CreateFramebuffer);
  ASH_VKDEV_FUNC(CreateGraphicsPipelines);
  ASH_VKDEV_FUNC(CreateImage);
  ASH_VKDEV_FUNC(CreateImageView);
  ASH_VKDEV_FUNC(CreatePipelineCache);
  ASH_VKDEV_FUNC(CreatePipelineLayout);
  ASH_VKDEV_FUNC(CreateQueryPool);
  ASH_VKDEV_FUNC(CreateRenderPass);
  ASH_VKDEV_FUNC(CreateSampler);
  ASH_VKDEV_FUNC(CreateSemaphore);
  ASH_VKDEV_FUNC(CreateShaderModule);
  ASH_VKDEV_FUNC(DestroyBuffer);
  ASH_VKDEV_FUNC(DestroyBufferView);
  ASH_VKDEV_FUNC(DestroyCommandPool);
  ASH_VKDEV_FUNC(DestroyDescriptorPool);
  ASH_VKDEV_FUNC(DestroyDescriptorSetLayout);
  ASH_VKDEV_FUNC(DestroyDevice);
  ASH_VKDEV_FUNC(DestroyEvent);
  ASH_VKDEV_FUNC(DestroyFence);
  ASH_VKDEV_FUNC(DestroyFramebuffer);
  ASH_VKDEV_FUNC(DestroyImage);
  ASH_VKDEV_FUNC(DestroyImageView);
  ASH_VKDEV_FUNC(DestroyPipeline);
  ASH_VKDEV_FUNC(DestroyPipelineCache);
  ASH_VKDEV_FUNC(DestroyPipelineLayout);
  ASH_VKDEV_FUNC(DestroyQueryPool);
  ASH_VKDEV_FUNC(DestroyRenderPass);
  ASH_VKDEV_FUNC(DestroySampler);
  ASH_VKDEV_FUNC(DestroySemaphore);
  ASH_VKDEV_FUNC(DestroyShaderModule);
  ASH_VKDEV_FUNC(DeviceWaitIdle);
  ASH_VKDEV_FUNC(FlushMappedMemoryRanges);
  ASH_VKDEV_FUNC(FreeCommandBuffers);
  ASH_VKDEV_FUNC(FreeDescriptorSets);
  ASH_VKDEV_FUNC(FreeMemory);
  ASH_VKDEV_FUNC(GetBufferMemoryRequirements);
  ASH_VKDEV_FUNC(GetDeviceMemoryCommitment);
  ASH_VKDEV_FUNC(GetDeviceQueue);
  ASH_VKDEV_FUNC(GetEventStatus);
  ASH_VKDEV_FUNC(GetFenceStatus);
  ASH_VKDEV_FUNC(GetImageMemoryRequirements);
  ASH_VKDEV_FUNC(GetImageSubresourceLayout);
  ASH_VKDEV_FUNC(GetPipelineCacheData);
  ASH_VKDEV_FUNC(GetQueryPoolResults);
  ASH_VKDEV_FUNC(InvalidateMappedMemoryRanges);
  ASH_VKDEV_FUNC(MapMemory);
  ASH_VKDEV_FUNC(MergePipelineCaches);
  ASH_VKDEV_FUNC(ResetCommandPool);
  ASH_VKDEV_FUNC(ResetDescriptorPool);
  ASH_VKDEV_FUNC(ResetEvent);
  ASH_VKDEV_FUNC(ResetFences);
  ASH_VKDEV_FUNC(SetEvent);
  ASH_VKDEV_FUNC(UpdateDescriptorSets);
  ASH_VKDEV_FUNC(UnmapMemory);
  ASH_VKDEV_FUNC(WaitForFences);

  ASH_VKDEV_FUNC(QueueSubmit);
  ASH_VKDEV_FUNC(QueueWaitIdle);

  // COMMAND BUFFER OBJECT FUNCTIONS
  ASH_VKDEV_FUNC(BeginCommandBuffer);
  ASH_VKDEV_FUNC(CmdBeginQuery);
  ASH_VKDEV_FUNC(CmdBeginRenderPass);
  ASH_VKDEV_FUNC(CmdBindDescriptorSets);
  ASH_VKDEV_FUNC(CmdBindIndexBuffer);
  ASH_VKDEV_FUNC(CmdBindPipeline);
  ASH_VKDEV_FUNC(CmdBindVertexBuffers);
  ASH_VKDEV_FUNC(CmdBlitImage);
  ASH_VKDEV_FUNC(CmdClearAttachments);
  ASH_VKDEV_FUNC(CmdClearColorImage);
  ASH_VKDEV_FUNC(CmdClearDepthStencilImage);
  ASH_VKDEV_FUNC(CmdCopyBuffer);
  ASH_VKDEV_FUNC(CmdCopyBufferToImage);
  ASH_VKDEV_FUNC(CmdCopyImage);
  ASH_VKDEV_FUNC(CmdCopyImageToBuffer);
  ASH_VKDEV_FUNC(CmdCopyQueryPoolResults);
  ASH_VKDEV_FUNC(CmdDispatch);
  ASH_VKDEV_FUNC(CmdDispatchIndirect);
  ASH_VKDEV_FUNC(CmdDraw);
  ASH_VKDEV_FUNC(CmdDrawIndexed);
  ASH_VKDEV_FUNC(CmdDrawIndexedIndirect);
  ASH_VKDEV_FUNC(CmdDrawIndirect);
  ASH_VKDEV_FUNC(CmdEndQuery);
  ASH_VKDEV_FUNC(CmdEndRenderPass);
  ASH_VKDEV_FUNC(CmdFillBuffer);
  ASH_VKDEV_FUNC(CmdNextSubpass);
  ASH_VKDEV_FUNC(CmdPipelineBarrier);
  ASH_VKDEV_FUNC(CmdPushConstants);
  ASH_VKDEV_FUNC(CmdResetEvent);
  ASH_VKDEV_FUNC(CmdResetQueryPool);
  ASH_VKDEV_FUNC(CmdResolveImage);
  ASH_VKDEV_FUNC(CmdSetBlendConstants);
  ASH_VKDEV_FUNC(CmdSetDepthBias);
  ASH_VKDEV_FUNC(CmdSetDepthBounds);
  ASH_VKDEV_FUNC(CmdSetEvent);
  ASH_VKDEV_FUNC(CmdSetLineWidth);
  ASH_VKDEV_FUNC(CmdSetScissor);
  ASH_VKDEV_FUNC(CmdSetStencilCompareMask);
  ASH_VKDEV_FUNC(CmdSetStencilReference);
  ASH_VKDEV_FUNC(CmdSetStencilWriteMask);
  ASH_VKDEV_FUNC(CmdSetViewport);
  ASH_VKDEV_FUNC(CmdUpdateBuffer);
  ASH_VKDEV_FUNC(CmdWaitEvents);
  ASH_VKDEV_FUNC(CmdWriteTimestamp);
  ASH_VKDEV_FUNC(EndCommandBuffer);
  ASH_VKDEV_FUNC(ResetCommandBuffer);

  ASH_VKDEV_FUNC(DebugMarkerSetObjectTagEXT);
  ASH_VKDEV_FUNC(DebugMarkerSetObjectNameEXT);

  ASH_VKDEV_FUNC(CmdDebugMarkerBeginEXT);
  ASH_VKDEV_FUNC(CmdDebugMarkerEndEXT);
  ASH_VKDEV_FUNC(CmdDebugMarkerInsertEXT);

#undef ASH_VKDEV_FUNC

  VmaVulkanFunctions vma_functions = {};
};

struct BufferSyncState
{
  VkShaderStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags      access = VK_ACCESS_NONE;
};

struct ImageSyncState
{
  VkShaderStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags      access = VK_ACCESS_NONE;
  VkImageLayout      layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct Buffer
{
  u64               refcount            = 0;
  gfx::BufferDesc   desc                = {};
  VkBuffer          vk_buffer           = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  void             *host_map            = nullptr;
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
  bool              is_weak             = false;
  VkImage           vk_image            = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
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

struct DescriptorLayout
{
  u64                   refcount  = 0;
  gfx::DescriptorCount  count     = {};
  VkDescriptorSetLayout vk_layout = nullptr;
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
  stx::Array<Image, 8>     images      = {};
  stx::Array<ImageView, 8> image_views = {};
};

// warnings: can't be used as depth stencil and color attachment
// load op clear op, read write matches imageusagescope
// ssbo matches scope
// push constant size match check
// NOTE: renderpass attachments MUST not be accessed in shaders within that renderpass
// NOTE: update_buffer and fill_buffer MUST be multiple of 4 for dst offset and dst size
struct BufferScope
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlags        access = VK_ACCESS_NONE;
};

struct ImageScope
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlags        access = VK_ACCESS_NONE;
  VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct DeviceImpl final : public gfx::Device
{
  static constexpr char const *REQUIRED_EXTENSIONS[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  static constexpr char const *OPTIONAL_EXTENSIONS[] = {VK_EXT_DEBUG_MARKER_EXTENSION_NAME};

  AllocationCallbacks allocator     = {};
  VkInstance          vk_instance   = nullptr;
  DeviceTable         vk_table      = {};
  VkPhysicalDevice    vk_phy_device = nullptr;
  VkDevice            vk_device     = nullptr;
  VkQueue             vk_queue      = nullptr;
  VmaAllocator        vma_allocator = nullptr;

  virtual void *to_impl() override;
  virtual void  ref() override;
  virtual void  unref() override;
  virtual ~DeviceImpl() override;
  virtual stx::Result<gfx::FormatProperties, gfx::Status>
                                                get_format_properties(gfx::Format format) override;
  virtual stx::Result<gfx::Buffer, gfx::Status> create_buffer(gfx::BufferDesc const &desc) override;
  virtual stx::Result<gfx::BufferView, gfx::Status>
      create_buffer_view(gfx::BufferViewDesc const &desc) override;
  virtual stx::Result<gfx::Image, gfx::Status> create_image(gfx::ImageDesc const &desc,
                                                            gfx::Color            initial_color,
                                                            gfx::CommandEncoder  &encoder) override;
  virtual stx::Result<gfx::Image, gfx::Status> create_image(gfx::ImageDesc const &desc,
                                                            gfx::DepthStencil initial_depth_stencil,
                                                            gfx::CommandEncoder &encoder) override;
  virtual stx::Result<gfx::Image, gfx::Status>
      create_image(gfx::ImageDesc const &desc, gfx::Buffer initial_data,
                   stx::Span<gfx::BufferImageCopy const> copies,
                   gfx::CommandEncoder                  &encoder) override;
  virtual stx::Result<gfx::ImageView, gfx::Status>
      create_image_view(gfx::ImageViewDesc const &desc) override;
  virtual stx::Result<gfx::Sampler, gfx::Status>
      create_sampler(gfx::SamplerDesc const &desc) override;
  virtual stx::Result<gfx::Shader, gfx::Status> create_shader(gfx::ShaderDesc const &desc) override;
  virtual stx::Result<gfx::RenderPass, gfx::Status>
      create_render_pass(gfx::RenderPassDesc const &desc) override;
  virtual stx::Result<gfx::Framebuffer, gfx::Status>
      create_framebuffer(gfx::FramebufferDesc const &desc) override;
  virtual stx::Result<gfx::DescriptorLayout, gfx::Status>
      create_descriptor_layout(gfx::DescriptorLayoutDesc const &desc) override;
  virtual stx::Result<gfx::PipelineCache, gfx::Status>
      create_pipeline_cache(gfx::PipelineCacheDesc const &desc) override;
  virtual stx::Result<gfx::ComputePipeline, gfx::Status>
      create_compute_pipeline(gfx::ComputePipelineDesc const &desc) override;
  virtual stx::Result<gfx::GraphicsPipeline, gfx::Status>
      create_graphics_pipeline(gfx::GraphicsPipelineDesc const &desc) override;
  virtual stx::Result<gfx::Fence, gfx::Status>            create_fence(bool signaled) override;
  virtual stx::Result<gfx::CommandEncoder *, gfx::Status> create_command_encoder() override;
  virtual void                                            ref(gfx::Buffer buffer) override;
  virtual void                                            ref(gfx::BufferView buffer_view) override;
  virtual void                                            ref(gfx::Image image) override;
  virtual void                                            ref(gfx::ImageView image_view) override;
  virtual void                                            ref(gfx::Sampler sampler) override;
  virtual void                                            ref(gfx::Shader shader) override;
  virtual void                                            ref(gfx::RenderPass render_pass) override;
  virtual void  ref(gfx::Framebuffer framebuffer) override;
  virtual void  ref(gfx::DescriptorLayout layout) override;
  virtual void  ref(gfx::PipelineCache cache) override;
  virtual void  ref(gfx::ComputePipeline pipeline) override;
  virtual void  ref(gfx::GraphicsPipeline pipeline) override;
  virtual void  ref(gfx::Fence fence) override;
  virtual void  ref(gfx::CommandEncoder *encoder) override;
  virtual void  unref(gfx::Buffer buffer) override;
  virtual void  unref(gfx::BufferView buffer_view) override;
  virtual void  unref(gfx::Image image) override;
  virtual void  unref(gfx::ImageView image_view) override;
  virtual void  unref(gfx::Sampler sampler) override;
  virtual void  unref(gfx::Shader shader) override;
  virtual void  unref(gfx::RenderPass render_pass) override;
  virtual void  unref(gfx::Framebuffer framebuffer) override;
  virtual void  unref(gfx::DescriptorLayout layout) override;
  virtual void  unref(gfx::PipelineCache cache) override;
  virtual void  unref(gfx::ComputePipeline pipeline) override;
  virtual void  unref(gfx::GraphicsPipeline pipeline) override;
  virtual void  unref(gfx::Fence fence) override;
  virtual void  unref(gfx::CommandEncoder *encoder) override;
  virtual void *get_buffer_memory_map(gfx::Buffer buffer) override;
  virtual void  invalidate_buffer_memory_map(gfx::Buffer                       buffer,
                                             stx::Span<gfx::MemoryRange const> ranges) override;
  virtual void  flush_buffer_memory_map(gfx::Buffer                       buffer,
                                        stx::Span<gfx::MemoryRange const> ranges) override;
  virtual stx::Result<usize, gfx::Status>
      get_pipeline_cache_size(gfx::PipelineCache cache) override;
  virtual stx::Result<usize, gfx::Status> get_pipeline_cache_data(gfx::PipelineCache cache,
                                                                  stx::Span<u8>      out) override;
  virtual gfx::Status                     merge_pipeline_cache(gfx::PipelineCache                  dst,
                                                               stx::Span<gfx::PipelineCache const> srcs) override;
  virtual gfx::Status wait_for_fences(stx::Span<gfx::Fence const> fences, bool all,
                                      u64 timeout) override;
  virtual void        reset_fences(stx::Span<gfx::Fence const> fences) override;
  virtual gfx::Status get_fence_status(gfx::Fence fence) override;
  virtual void        submit(gfx::CommandEncoder &encoder, gfx::Fence signal_fence) override;
  virtual void        wait_idle() override;
  virtual void        wait_queue_idle() override;
};

struct DescriptorStorageBindings
{
  struct End
  {
    u32 buffer = 0;
    u32 image  = 0;
  };
  // how to avoid for ones that don't contain storage
  Vec<Buffer *> buffers = {};
  Vec<Image *>  images  = {};
  Vec<End>      ends    = {};        // for each descriptor
};

// for each bind descriptor call, create a new descriptor set
// won't work for things like UI as it would require sorting by bind group
// TODO(lamarrr): report malloc errors using status
struct CommandEncoderImpl final : public gfx::CommandEncoder
{
  // we need to only store storage images and buffers for descriptor sets
  // pool sizing depends on descriptor set layout
  gfx::Status                     status                           = gfx::Status::Success;
  AllocationCallbacks             allocator                        = {};
  DeviceImpl                     *device                           = nullptr;
  VkCommandPool                   vk_command_pool                  = nullptr;
  VkCommandBuffer                 vk_command_buffer                = nullptr;
  ComputePipeline                *compute_pipeline                 = nullptr;
  GraphicsPipeline               *graphics_pipeline                = nullptr;
  Framebuffer                    *framebuffer                      = nullptr;
  VkDescriptorPool                vk_descriptor_pool               = nullptr;
  gfx::DescriptorCount            descriptors_capacity             = {};
  gfx::DescriptorCount            descriptors_size_target          = {};
  Vec<VkDescriptorSetLayout>      vk_descriptor_layouts            = {};
  Vec<VkDescriptorSet>            vk_descriptors                   = {};
  Vec<VkDescriptorBufferInfo>     vk_descriptor_buffers            = {};
  Vec<VkBufferView>               vk_descriptor_texel_buffer_views = {};
  Vec<VkDescriptorImageInfo>      vk_descriptor_images             = {};
  Vec<VkWriteDescriptorSet>       vk_descriptor_writes             = {};
  DescriptorStorageBindings       descriptor_storage_bindings      = {};
  u32                             bound_descriptor                 = ~0U;
  stx::Vec<stx::UniqueFn<void()>> completion_tasks                 = {};
  virtual ~CommandEncoderImpl() override;
  virtual void        begin() override;
  virtual gfx::Status end() override;
  virtual void        reset() override;
  virtual void        begin_debug_marker(char const *region_name, Vec4 color) override;
  virtual void        end_debug_marker() override;
  virtual void        fill_buffer(gfx::Buffer dst, u64 offset, u64 size, u32 data) override;
  virtual void        copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                                  stx::Span<gfx::BufferCopy const> copies) override;
  virtual void update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst) override;
  virtual void clear_color_image(gfx::Image dst, gfx::Color clear_color,
                                 stx::Span<gfx::ImageSubresourceRange const> ranges) override;
  virtual void
               clear_depth_stencil_image(gfx::Image dst, gfx::DepthStencil clear_depth_stencil,
                                         stx::Span<gfx::ImageSubresourceRange const> ranges) override;
  virtual void copy_image(gfx::Image src, gfx::Image dst,
                          stx::Span<gfx::ImageCopy const> copies) override;
  virtual void copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                    stx::Span<gfx::BufferImageCopy const> copies) override;
  virtual void blit_image(gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageBlit const> blits,
                          gfx::Filter filter) override;
  virtual void begin_descriptor_pass() override;
  virtual u32  push_descriptors(gfx::DescriptorLayout layout, u32 count) override;
  virtual void push_bindings(u32 descriptor, gfx::PipelineBindPoint bind_point,
                             gfx::DescriptorBindings const &bindings) override;
  virtual void end_descriptor_pass() override;
  virtual void bind_descriptor(u32 index) override;
  virtual void bind_next_descriptor() override;
  virtual void begin_render_pass(
      gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
      stx::Span<gfx::Color const>        color_attachments_clear_values,
      stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values) override;
  virtual void end_render_pass() override;
  virtual void bind_pipeline(gfx::ComputePipeline pipeline) override;
  virtual void bind_pipeline(gfx::GraphicsPipeline pipeline) override;
  virtual void push_constants(stx::Span<u8 const> push_constants_data) override;
  virtual void dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z) override;
  virtual void dispatch_indirect(gfx::Buffer buffer, u64 offset) override;
  virtual void set_viewport(gfx::Viewport const &viewport) override;
  virtual void set_scissor(IRect scissor) override;
  virtual void set_blend_constants(Vec4 blend_constants) override;
  virtual void set_stencil_compare_mask(gfx::StencilFaces faces, u32 mask) override;
  virtual void set_stencil_reference(gfx::StencilFaces faces, u32 reference) override;
  virtual void set_stencil_write_mask(gfx::StencilFaces faces, u32 mask) override;
  virtual void set_vertex_buffers(stx::Span<gfx::Buffer const> vertex_buffers,
                                  stx::Span<u64 const>         offsets) override;
  virtual void set_index_buffer(gfx::Buffer index_buffer, u64 offset) override;
  virtual void draw(u32 first_index, u32 num_indices, i32 vertex_offset, u32 first_instance,
                    u32 num_instances) override;
  virtual void draw_indirect(gfx::Buffer buffer, u64 offset, u32 draw_count, u32 stride) override;
  virtual void on_execution_complete(stx::UniqueFn<void()> &&fn) override;
};

}        // namespace vk
}        // namespace ash
