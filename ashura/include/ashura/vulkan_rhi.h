#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "ashura/gfx.h"
#include "ashura/rhi.h"
#include "stx/vec.h"
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"
#include <map>

namespace ash
{

namespace rhi
{




constexpr u32 DESCRIPTOR_POOL_BIN_SIZE = 1024;        // * num_entries

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


struct VulkanDevice : public Device
{
  static constexpr char const *REQUIRED_EXTENSIONS[] = {"VK_KHR_swapchain"};

  VkInstance               instance  = VK_NULL_HANDLE;
  VulkanDeviceTable const *table     = nullptr;
  VkDevice                 device    = VK_NULL_HANDLE;
  VmaAllocator             allocator = nullptr;
  virtual ~VulkanDevice() override;
};

struct ComputePipeline
{
  VkPipeline pipeline = nullptr;
};

struct GraphicsPipeline
{
  VkPipeline pipeline = nullptr;
  // for each bind descriptor call, create a new descriptor set
  // won't work for things like UI as it would require sorting by bind group
};

struct DescriptorSetPoolBin
{
  // batch descriptor pool free!!!
  stx::Vec<VkDescriptorSet> sets;
  VkDescriptorPool          pool = nullptr;
};

struct DescriptorSetLayout
{
  VkDescriptorSetLayout layout     = nullptr;
  u32                   sizing[11] = {};
};

struct CommandBuffer
{
  // pool sizing depends on descriptor set layout
  using DescriptorPoolMap = std::map<VkDescriptorSetLayout, stx::Vec<DescriptorSetPoolBin>>;
  DescriptorPoolMap                              descriptor_pool_bins;
  DescriptorPoolMap::iterator                    descriptor_pool;
  stx::Vec<VkDescriptorSet>                      descriptor_sets;
  stx::Vec<VkWriteDescriptorSet>                 descriptor_writes;
  stx::Array<VkBufferMemoryBarrier, 16>          tmp_buffer_barriers   = {};
  stx::Array<VkImageMemoryBarrier, 16>           tmp_image_barriers    = {};
  stx::Array<gfx::StorageImageBinding, 16>       storage_images        = {};
  stx::Array<gfx::StorageTexelBufferBinding, 16> storage_texel_buffers = {};
  stx::Array<gfx::StorageBufferBinding, 16>      storage_buffers       = {};
  gfx::Framebuffer                               framebuffer           = nullptr;
  gfx::ComputePipeline                           compute_pipeline      = nullptr;
  gfx::GraphicsPipeline                          graphics_pipeline     = nullptr;

  void begin();
  void end();
  void reset();
};

}        // namespace rhi
}        // namespace ash
