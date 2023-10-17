#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "ashura/gfx.h"
#include "ashura/rhi.h"
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace ash
{

namespace rhi
{

// some systems have multiple vulkan implementations! dynamic loading
// VERSION 1.1 Vulkan Functions
struct VulkanDeviceTable
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

#undef ASH_VKDEV_FUNC

  VmaVulkanFunctions vma_functions = {};
};

// TODO(lamarrr): expose multi-device here
struct VulkanDriver : public Driver
{
  static constexpr char const *REQUIRED_EXTENSIONS[] = {"VK_KHR_swapchain"};

  VkInstance               instance  = VK_NULL_HANDLE;
  VulkanDeviceTable const *table     = nullptr;
  VkDevice                 device    = VK_NULL_HANDLE;
  VmaAllocator             allocator = nullptr;
  virtual ~VulkanDriver() override;
  virtual gfx::FormatProperties get_format_properties(gfx::Format format) override;
  virtual gfx::Buffer           create(gfx::BufferDesc const &desc) override;
  virtual gfx::BufferView       create(gfx::BufferViewDesc const &desc) override;
  virtual gfx::Image            create(gfx::ImageDesc const &desc) override;
  virtual gfx::ImageView        create(gfx::ImageViewDesc const &desc) override;
  virtual gfx::RenderPass       create(gfx::RenderPassDesc const &desc) override;
  virtual gfx::Framebuffer      create(gfx::FramebufferDesc const &desc) override;
  virtual gfx::Sampler          create(gfx::SamplerDesc const &sampler) override;
  virtual gfx::DescriptorSetLayout
                               create(gfx::DescriptorSetDesc const &descriptor_set_layout) override;
  virtual gfx::Shader          create_shader(stx::Span<u32 const> shader) override;
  virtual gfx::ComputePipeline create(gfx::ComputePipelineDesc const &desc) override;
  virtual gfx::GraphicsPipeline create(gfx::GraphicsPipelineDesc const &desc) override;
  virtual gfx::CommandBuffer    create_command_buffer() override;
  virtual void                  release(gfx::Buffer buffer) override;
  virtual void                  release(gfx::BufferView buffer_view) override;
  virtual void                  release(gfx::Image image) override;
  virtual void                  release(gfx::ImageView image_view) override;
  virtual void                  release(gfx::RenderPass render_pass) override;
  virtual void                  release(gfx::Framebuffer framebuffer) override;
  virtual void                  release(gfx::Sampler sampler) override;
  virtual void                  release(gfx::DescriptorSetLayout descriptor_set_layout) override;
  virtual void                  release(gfx::Shader shader) override;
  virtual void                  release(gfx::ComputePipeline pipeline) override;
  virtual void                  release(gfx::GraphicsPipeline pipeline) override;
  virtual void                  release(gfx::CommandBuffer command_buffer) override;
  virtual void cmd_fill_buffer(gfx::CommandBuffer command_buffer, gfx::Buffer buffer, u64 offset,
                               u64 size, u32 data) override;
  virtual void cmd_copy_buffer(gfx::CommandBuffer command_buffer, gfx::Buffer src, gfx::Buffer dst,
                               stx::Span<gfx::BufferCopy const> copies) override;
  virtual void cmd_update_buffer(gfx::CommandBuffer command_buffer, stx::Span<u8 const> src,
                                 u64 dst_offset, gfx::Buffer dst) override;
  virtual void cmd_copy_image(gfx::CommandBuffer command_buffer, gfx::Image src, gfx::Image dst,
                              stx::Span<gfx::ImageCopy const> copies) override;
  virtual void cmd_copy_buffer_to_image(gfx::CommandBuffer command_buffer, gfx::Buffer src,
                                        gfx::Image                            dst,
                                        stx::Span<gfx::BufferImageCopy const> copies) override;
  virtual void cmd_blit_image(gfx::CommandBuffer command_buffer, gfx::Image src, gfx::Image dst,
                              stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter) override;
  virtual void cmd_begin_render_pass(gfx::CommandBuffer              command_buffer,
                                     gfx::RenderPassBeginInfo const &info) override;
  virtual void cmd_end_render_pass(gfx::CommandBuffer command_buffer) override;
  virtual void cmd_bind_pipeline(gfx::CommandBuffer   command_buffer,
                                 gfx::ComputePipeline pipeline) override;
  virtual void cmd_bind_pipeline(gfx::CommandBuffer    command_buffer,
                                 gfx::GraphicsPipeline pipeline) override;
  virtual void cmd_bind_vertex_buffers(gfx::CommandBuffer command_buffer, u32 first_binding,
                                       stx::Span<gfx::Buffer const> vertex_buffers,
                                       stx::Span<u64 const>         offsets) override;
  virtual void cmd_bind_index_buffer(gfx::CommandBuffer command_buffer, gfx::Buffer index_buffer,
                                     u64 offset) override;
  virtual void cmd_push_descriptor_set(gfx::CommandBuffer command_buffer, u32 set,
                                       gfx::DescriptorSetBindings const &bindings) override;
  virtual void cmd_set_scissor(gfx::CommandBuffer command_buffer, IRect scissor) override;
  virtual void cmd_set_viewport(gfx::CommandBuffer   command_buffer,
                                gfx::Viewport const &viewport) override;
  virtual void cmd_set_blend_constants(gfx::CommandBuffer command_buffer, f32 r, f32 g, f32 b,
                                       f32 a) override;
  virtual void cmd_set_stencil_compare_mask(gfx::CommandBuffer command_buffer,
                                            gfx::StencilFaces faces, u32 compare_mask) override;
  virtual void cmd_set_stencil_reference(gfx::CommandBuffer command_buffer, gfx::StencilFaces faces,
                                         u32 reference) override;
  virtual void cmd_set_stencil_write_mask(gfx::CommandBuffer command_buffer,
                                          gfx::StencilFaces faces, u32 write_mask) override;
  virtual void cmd_dispatch(gfx::CommandBuffer command_buffer, u32 group_count_x, u32 group_count_y,
                            u32 group_count_z) override;
  virtual void cmd_dispatch_indirect(gfx::CommandBuffer command_buffer, gfx::Buffer buffer,
                                     u64 offset) override;
  virtual void cmd_draw(gfx::CommandBuffer command_buffer, u32 first_vertex, u32 vertex_count,
                        u32 instance_count, u32 first_instance_id) override;
  virtual void cmd_draw_indexed(gfx::CommandBuffer command_buffer, u32 first_index, u32 index_count,
                                u32 instance_count, i32 vertex_offset,
                                u32 first_instance_id) override;
  virtual void cmd_draw_indexed_indirect(gfx::CommandBuffer command_buffer, gfx::Buffer buffer,
                                         u64 offset, u32 draw_count, u32 stride) override;
  virtual void cmd_insert_barriers(
      gfx::CommandBuffer                             command_buffer,
      stx::Span<gfx::QueueBufferMemoryBarrier const> buffer_memory_barriers,
      stx::Span<gfx::QueueImageMemoryBarrier const>  image_memory_barriers) override;
};

}        // namespace rhi
}        // namespace ash
