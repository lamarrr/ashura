#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "ashura/gfx.h"
#include "stx/vec.h"
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
#include <map>

namespace ash
{
namespace vk
{

constexpr u32 DESCRIPTOR_POOL_BIN_SIZE = 1024;        // * num_entries

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

struct Buffer
{
  VkBuffer      vk_buffer     = nullptr;
  VmaAllocation vk_allocation = nullptr;
  void         *host_map      = nullptr;
};

struct Image
{
  VkImage       vk_image      = nullptr;
  VmaAllocation vk_allocation = nullptr;
};

struct Pipeline
{
  VkPipeline       pipeline = nullptr;
  VkPipelineLayout layout   = nullptr;
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

struct DescriptorSetPoolBin
{
  // batch descriptor pool free!!!
  stx::Vec<VkDescriptorSet> sets;
  VkDescriptorPool          pool = nullptr;
};

struct DeviceResources
{
  stx::SparseVec<Buffer, void *>   buffers;
  stx::SparseVec<Image, void *>    images;
  stx::SparseVec<Pipeline, void *> compute_pipelines;
  stx::SparseVec<Pipeline, void *> graphics_pipelines;
};

struct DeviceImpl : public gfx::Device
{
  static constexpr char const *REQUIRED_EXTENSIONS[] = {"VK_KHR_swapchain"};
  static constexpr char const *OPTIONAL_EXTENSIONS[] = {"VK_EXT_debug_marker"};

  VkInstance       vk_instance   = nullptr;
  DeviceTable      vk_table      = {};
  VkPhysicalDevice vk_phy_device = nullptr;
  VkDevice         vk_device     = nullptr;
  VmaAllocator     vk_allocator  = nullptr;
  DeviceResources  vk_resources;

  virtual ~DeviceImpl() override;

  virtual stx::Result<gfx::FormatProperties, gfx::Status>
                                                get_format_properties(gfx::Format format) override;
  virtual stx::Result<gfx::Buffer, gfx::Status> create_buffer(gfx::BufferDesc const &desc) override;
  virtual stx::Result<gfx::BufferView, gfx::Status>
      create_buffer_view(gfx::BufferViewDesc const &desc) override;
  virtual stx::Result<gfx::ImageView, gfx::Status>
      create_image_view(gfx::ImageViewDesc const &desc) override;
  virtual stx::Result<gfx::Sampler, gfx::Status>
      create_sampler(gfx::SamplerDesc const &desc) override;
  virtual stx::Result<gfx::Shader, gfx::Status> create_shader(gfx::ShaderDesc const &desc) override;
  virtual stx::Result<gfx::RenderPass, gfx::Status>
      create_render_pass(gfx::RenderPassDesc const &desc) override;
  virtual stx::Result<gfx::Framebuffer, gfx::Status>
      create_framebuffer(gfx::FramebufferDesc const &desc) override;
  virtual stx::Result<gfx::DescriptorSetLayout, gfx::Status>
      create_descriptor_set_layout(gfx::DescriptorSetLayoutDesc const &desc) override;
  virtual stx::Result<gfx::PipelineCache, gfx::Status>
      create_pipeline_cache(gfx::PipelineCacheDesc const &desc) override;
  virtual stx::Result<gfx::ComputePipeline, gfx::Status>
      create_compute_pipeline(gfx::ComputePipelineDesc const &desc) override;
  virtual stx::Result<gfx::GraphicsPipeline, gfx::Status>
      create_graphics_pipeline(gfx::GraphicsPipelineDesc const &desc) override;
  virtual stx::Result<gfx::Fence, gfx::Status>            create_fence(bool signaled) override;
  virtual stx::Result<gfx::CommandEncoder *, gfx::Status> create_command_encoder() override;
  // pipeline cache

  virtual void ref(gfx::Buffer buffer) override;
  virtual void ref(gfx::BufferView buffer_view) override;
  virtual void ref(gfx::Image image) override;
  virtual void ref(gfx::ImageView image_view) override;
  virtual void ref(gfx::Sampler sampler) override;
  virtual void ref(gfx::Shader shader) override;
  virtual void ref(gfx::RenderPass render_pass) override;
  virtual void ref(gfx::Framebuffer framebuffer) override;
  virtual void ref(gfx::DescriptorSetLayout descriptor_set_layout) override;
  virtual void ref(gfx::PipelineCache cache) override;
  virtual void ref(gfx::ComputePipeline pipeline) override;
  virtual void ref(gfx::GraphicsPipeline pipeline) override;
  virtual void ref(gfx::Fence fence) override;
  virtual void ref(gfx::CommandEncoder *command_encoder) override;

  virtual void unref(gfx::Buffer buffer) override;
  virtual void unref(gfx::BufferView buffer_view) override;
  virtual void unref(gfx::Image image) override;
  virtual void unref(gfx::ImageView image_view) override;
  virtual void unref(gfx::Sampler sampler) override;
  virtual void unref(gfx::Shader shader) override;
  virtual void unref(gfx::RenderPass render_pass) override;
  virtual void unref(gfx::Framebuffer framebuffer) override;
  virtual void unref(gfx::DescriptorSetLayout descriptor_set_layout) override;
  virtual void unref(gfx::PipelineCache cache) override;
  virtual void unref(gfx::ComputePipeline pipeline) override;
  virtual void unref(gfx::GraphicsPipeline pipeline) override;
  virtual void unref(gfx::Fence fence) override;
  virtual void unref(gfx::CommandEncoder *command_encoder) override;

  virtual void *get_buffer_memory_map(gfx::Buffer buffer) override;
  virtual void  invalidate_buffer_memory_map(gfx::Buffer                       buffer,
                                             stx::Span<gfx::MemoryRange const> ranges) override;
  virtual void  flush_buffer_memory_map(gfx::Buffer                       buffer,
                                        stx::Span<gfx::MemoryRange const> ranges) override;
  virtual usize get_pipeline_cache_size(gfx::PipelineCache cache) override;
  virtual void  get_pipeline_cache_data(gfx::PipelineCache cache, stx::Span<u8> out) override;
  virtual void  wait_for_fences(stx::Span<gfx::Fence const> fences, bool all, u64 timeout) override;
  virtual void  reset_fences(stx::Span<gfx::Fence const> fences) override;
  virtual gfx::FenceStatus get_fence_status(gfx::Fence fence) override;
  virtual void             submit(gfx::CommandEncoder *encoder, gfx::Fence signal_fence) override;
  virtual void             wait_idle() override;
};

// for each bind descriptor call, create a new descriptor set
// won't work for things like UI as it would require sorting by bind group
struct CommandEncoderImpl : public gfx::CommandEncoder
{
  // pool sizing depends on descriptor set layout
  using DescriptorPoolMap = std::map<VkDescriptorSetLayout, stx::Vec<DescriptorSetPoolBin>>;
  DescriptorPoolMap                              descriptor_pool_bins;
  DescriptorPoolMap::iterator                    descriptor_pool;
  stx::Vec<VkDescriptorSet>                      frame_descriptor_sets;
  stx::Vec<VkWriteDescriptorSet>                 frame_descriptor_writes;
  stx::Array<gfx::StorageImageBinding, 16>       storage_images        = {};
  stx::Array<gfx::StorageTexelBufferBinding, 16> storage_texel_buffers = {};
  stx::Array<gfx::StorageBufferBinding, 16>      storage_buffers       = {};
  gfx::Framebuffer                               framebuffer           = nullptr;
  gfx::ComputePipeline                           compute_pipeline      = nullptr;
  gfx::GraphicsPipeline                          graphics_pipeline     = nullptr;
  VkCommandPool                                  vk_command_pool       = nullptr;
  VkCommandBuffer                                vk_command_buffer     = nullptr;
  DeviceImpl                                    *device                = nullptr;

  virtual ~CommandEncoderImpl() override;
  virtual void begin() override;
  virtual void end() override;
  virtual void reset() override;
  virtual void begin_debug_marker(char const *region_name, Vec4 color) override;
  virtual void end_debug_marker() override;
  virtual stx::Result<gfx::Image, gfx::Status> create_image(gfx::ImageDesc const &desc,
                                                            gfx::Color initial_color) override;
  virtual stx::Result<gfx::Image, gfx::Status>
      create_image(gfx::ImageDesc const &desc, gfx::DepthStencil initial_depth_stencil) override;
  virtual stx::Result<gfx::Image, gfx::Status>
               create_image(gfx::ImageDesc const &desc, gfx::Buffer initial_data,
                            stx::Span<gfx::BufferImageCopy const> copies) override;
  virtual void fill_buffer(gfx::Buffer dst, u64 offset, u64 size, u32 data) override;
  virtual void copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                           stx::Span<gfx::BufferCopy const> copies) override;
  virtual void update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst) override;
  virtual void clear_color_image(gfx::Image dst, stx::Span<gfx::Color const> clear_colors,
                                 stx::Span<gfx::ImageSubresourceRange const> ranges) override;
  virtual void
               clear_depth_stencil_image(gfx::Image                                  dst,
                                         stx::Span<gfx::DepthStencil const>          clear_depth_stencils,
                                         stx::Span<gfx::ImageSubresourceRange const> ranges) override;
  virtual void copy_image(gfx::Image src, gfx::Image dst,
                          stx::Span<gfx::ImageCopy const> copies) override;
  virtual void copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                    stx::Span<gfx::BufferImageCopy const> copies) override;
  virtual void blit_image(gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageBlit const> blits,
                          gfx::Filter filter) override;
  virtual void begin_render_pass(
      gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
      stx::Span<gfx::Color const>        color_attachments_clear_values,
      stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values) override;
  virtual void end_render_pass() override;
  virtual void bind_pipeline(gfx::ComputePipeline     pipeline,
                             gfx::DescriptorSetLayout layout) override;
  virtual void bind_pipeline(gfx::GraphicsPipeline    pipeline,
                             gfx::DescriptorSetLayout layout) override;
  virtual void push_descriptors(gfx::DescriptorSetBindings const &bindings) override;
  virtual void push_constants(stx::Span<u8 const> push_constants_data) override;
  virtual void dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z) override;
  virtual void dispatch_indirect(gfx::Buffer buffer, u64 offset) override;
  virtual void set_viewport(gfx::Viewport const &viewport) override;
  virtual void set_scissor(IRect scissor) override;
  virtual void set_blend_constants(Vec4 blend_constants) override;
  virtual void set_stencil_compare_mask(gfx::StencilFaces faces, u32 mask) override;
  virtual void set_stencil_reference(gfx::StencilFaces faces, u32 reference) override;
  virtual void set_stencil_write_mask(gfx::StencilFaces faces, u32 mask) override;
  virtual void set_vertex_buffers(stx::Span<gfx::Buffer const> vertex_buffers) override;
  virtual void draw(gfx::Buffer index_buffer, u32 first_index, u32 num_indices, u32 vertex_offset,
                    u32 first_instance, u32 num_instances) override;
  virtual void draw_indirect(gfx::Buffer index_buffer, gfx::Buffer buffer, u64 offset,
                             u32 draw_count, u32 stride) override;
};

}        // namespace vk
}        // namespace ash
