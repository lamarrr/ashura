#include "ashura/vulkan_gfx.h"
#include "stx/span.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

#define VK_ERR(expr)                         \
  do                                         \
  {                                          \
    VkResult result = (expr);                \
    if (result != VK_SUCCESS)                \
      return stx::Err((gfx::Status) result); \
  } while (0)

VkResult DebugMarkerSetObjectTagEXT_Stub(VkDevice, const VkDebugMarkerObjectTagInfoEXT *)
{
  return VK_SUCCESS;
}

VkResult DebugMarkerSetObjectNameEXT_Stub(VkDevice, const VkDebugMarkerObjectNameInfoEXT *)
{
  return VK_SUCCESS;
}

void CmdDebugMarkerBeginEXT_Stub(VkCommandBuffer, const VkDebugMarkerMarkerInfoEXT *)
{
}

void CmdDebugMarkerEndEXT_Stub(VkCommandBuffer)
{
}

void CmdDebugMarkerInsertEXT_Stub(VkCommandBuffer, const VkDebugMarkerMarkerInfoEXT *)
{
}

bool load_device_table(VkDevice device, DeviceTable &table)
{
  bool all_loaded = true;

#define ASH_VK_LOAD(function)                                                      \
  table.function = (PFN_vk##function) vkGetDeviceProcAddr(device, "vk" #function); \
  all_loaded     = all_loaded && (table.function != nullptr)

  // DEVICE OBJECT FUNCTIONS
  ASH_VK_LOAD(AllocateCommandBuffers);
  ASH_VK_LOAD(AllocateDescriptorSets);
  ASH_VK_LOAD(AllocateMemory);
  ASH_VK_LOAD(BindBufferMemory);
  ASH_VK_LOAD(BindImageMemory);
  ASH_VK_LOAD(CreateBuffer);
  ASH_VK_LOAD(CreateBufferView);
  ASH_VK_LOAD(CreateCommandPool);
  ASH_VK_LOAD(CreateComputePipelines);
  ASH_VK_LOAD(CreateDescriptorPool);
  ASH_VK_LOAD(CreateDescriptorSetLayout);
  ASH_VK_LOAD(CreateDevice);
  ASH_VK_LOAD(CreateEvent);
  ASH_VK_LOAD(CreateFence);
  ASH_VK_LOAD(CreateFramebuffer);
  ASH_VK_LOAD(CreateGraphicsPipelines);
  ASH_VK_LOAD(CreateImage);
  ASH_VK_LOAD(CreateImageView);
  ASH_VK_LOAD(CreatePipelineCache);
  ASH_VK_LOAD(CreatePipelineLayout);
  ASH_VK_LOAD(CreateQueryPool);
  ASH_VK_LOAD(CreateRenderPass);
  ASH_VK_LOAD(CreateSampler);
  ASH_VK_LOAD(CreateSemaphore);
  ASH_VK_LOAD(CreateShaderModule);
  ASH_VK_LOAD(DestroyBuffer);
  ASH_VK_LOAD(DestroyBufferView);
  ASH_VK_LOAD(DestroyCommandPool);
  ASH_VK_LOAD(DestroyDescriptorPool);
  ASH_VK_LOAD(DestroyDescriptorSetLayout);
  ASH_VK_LOAD(DestroyDevice);
  ASH_VK_LOAD(DestroyEvent);
  ASH_VK_LOAD(DestroyFence);
  ASH_VK_LOAD(DestroyFramebuffer);
  ASH_VK_LOAD(DestroyImage);
  ASH_VK_LOAD(DestroyImageView);
  ASH_VK_LOAD(DestroyPipeline);
  ASH_VK_LOAD(DestroyPipelineCache);
  ASH_VK_LOAD(DestroyPipelineLayout);
  ASH_VK_LOAD(DestroyQueryPool);
  ASH_VK_LOAD(DestroyRenderPass);
  ASH_VK_LOAD(DestroySampler);
  ASH_VK_LOAD(DestroySemaphore);
  ASH_VK_LOAD(DestroyShaderModule);
  ASH_VK_LOAD(DeviceWaitIdle);
  ASH_VK_LOAD(FlushMappedMemoryRanges);
  ASH_VK_LOAD(FreeCommandBuffers);
  ASH_VK_LOAD(FreeDescriptorSets);
  ASH_VK_LOAD(FreeMemory);
  ASH_VK_LOAD(GetBufferMemoryRequirements);
  ASH_VK_LOAD(GetDeviceMemoryCommitment);
  ASH_VK_LOAD(GetDeviceQueue);
  ASH_VK_LOAD(GetEventStatus);
  ASH_VK_LOAD(GetFenceStatus);
  ASH_VK_LOAD(GetImageMemoryRequirements);
  ASH_VK_LOAD(GetImageSubresourceLayout);
  ASH_VK_LOAD(GetPipelineCacheData);
  ASH_VK_LOAD(GetQueryPoolResults);
  ASH_VK_LOAD(InvalidateMappedMemoryRanges);
  ASH_VK_LOAD(MapMemory);
  ASH_VK_LOAD(MergePipelineCaches);
  ASH_VK_LOAD(ResetCommandPool);
  ASH_VK_LOAD(ResetDescriptorPool);
  ASH_VK_LOAD(ResetEvent);
  ASH_VK_LOAD(ResetFences);
  ASH_VK_LOAD(SetEvent);
  ASH_VK_LOAD(UpdateDescriptorSets);
  ASH_VK_LOAD(UnmapMemory);
  ASH_VK_LOAD(WaitForFences);

  ASH_VK_LOAD(QueueSubmit);
  ASH_VK_LOAD(QueueWaitIdle);

  // COMMAND BUFFER OBJECT FUNCTIONS
  ASH_VK_LOAD(BeginCommandBuffer);
  ASH_VK_LOAD(CmdBeginQuery);
  ASH_VK_LOAD(CmdBeginRenderPass);
  ASH_VK_LOAD(CmdBindDescriptorSets);
  ASH_VK_LOAD(CmdBindIndexBuffer);
  ASH_VK_LOAD(CmdBindPipeline);
  ASH_VK_LOAD(CmdBindVertexBuffers);
  ASH_VK_LOAD(CmdBlitImage);
  ASH_VK_LOAD(CmdClearAttachments);
  ASH_VK_LOAD(CmdClearColorImage);
  ASH_VK_LOAD(CmdClearDepthStencilImage);
  ASH_VK_LOAD(CmdCopyBuffer);
  ASH_VK_LOAD(CmdCopyBufferToImage);
  ASH_VK_LOAD(CmdCopyImage);
  ASH_VK_LOAD(CmdCopyImageToBuffer);
  ASH_VK_LOAD(CmdCopyQueryPoolResults);
  ASH_VK_LOAD(CmdDispatch);
  ASH_VK_LOAD(CmdDispatchIndirect);
  ASH_VK_LOAD(CmdDraw);
  ASH_VK_LOAD(CmdDrawIndexed);
  ASH_VK_LOAD(CmdDrawIndexedIndirect);
  ASH_VK_LOAD(CmdDrawIndirect);
  ASH_VK_LOAD(CmdEndQuery);
  ASH_VK_LOAD(CmdEndRenderPass);
  ASH_VK_LOAD(CmdFillBuffer);
  ASH_VK_LOAD(CmdNextSubpass);
  ASH_VK_LOAD(CmdPipelineBarrier);
  ASH_VK_LOAD(CmdPushConstants);
  ASH_VK_LOAD(CmdResetEvent);
  ASH_VK_LOAD(CmdResetQueryPool);
  ASH_VK_LOAD(CmdResolveImage);
  ASH_VK_LOAD(CmdSetBlendConstants);
  ASH_VK_LOAD(CmdSetDepthBias);
  ASH_VK_LOAD(CmdSetDepthBounds);
  ASH_VK_LOAD(CmdSetEvent);
  ASH_VK_LOAD(CmdSetLineWidth);
  ASH_VK_LOAD(CmdSetScissor);
  ASH_VK_LOAD(CmdSetStencilCompareMask);
  ASH_VK_LOAD(CmdSetStencilReference);
  ASH_VK_LOAD(CmdSetStencilWriteMask);
  ASH_VK_LOAD(CmdSetViewport);
  ASH_VK_LOAD(CmdUpdateBuffer);
  ASH_VK_LOAD(CmdWaitEvents);
  ASH_VK_LOAD(CmdWriteTimestamp);
  ASH_VK_LOAD(EndCommandBuffer);
  ASH_VK_LOAD(ResetCommandBuffer);
#undef ASH_VK_LOAD

#define ASH_VKEXT_LOAD(function)                                                   \
  table.function = (PFN_vk##function) vkGetDeviceProcAddr(device, "vk" #function); \
  table.function = (table.function != nullptr) ? table.function : function##_Stub;

  ASH_VKEXT_LOAD(DebugMarkerSetObjectTagEXT);
  ASH_VKEXT_LOAD(DebugMarkerSetObjectNameEXT);

  ASH_VKEXT_LOAD(CmdDebugMarkerBeginEXT);
  ASH_VKEXT_LOAD(CmdDebugMarkerEndEXT);
  ASH_VKEXT_LOAD(CmdDebugMarkerInsertEXT);

#undef ASH_VKEXT_LOAD

#define ASH_VMA_SET(function) table.vma_functions.vk##function = table.function

  ASH_VMA_SET(AllocateMemory);
  ASH_VMA_SET(FreeMemory);
  ASH_VMA_SET(UnmapMemory);
  ASH_VMA_SET(FlushMappedMemoryRanges);
  ASH_VMA_SET(InvalidateMappedMemoryRanges);
  ASH_VMA_SET(BindBufferMemory);
  ASH_VMA_SET(BindImageMemory);
  ASH_VMA_SET(GetBufferMemoryRequirements);
  ASH_VMA_SET(GetImageMemoryRequirements);
  ASH_VMA_SET(CreateBuffer);
  ASH_VMA_SET(DestroyBuffer);
  ASH_VMA_SET(CreateImage);
  ASH_VMA_SET(DestroyImage);
  ASH_VMA_SET(CmdCopyBuffer);

#undef ASH_VMA_SET

  return all_loaded;
}

constexpr VkBufferUsageFlags to_vkUsage(gfx::BufferUsage usage)
{
  VkBufferUsageFlags out = (VkBufferUsageFlags) 0;
  if (has_bits(usage, gfx::BufferUsage::TransferSrc))
  {
    out |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::TransferDst))
  {
    out |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::IndirectCommand))
  {
    out |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::ComputeShaderUniform))
  {
    out |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::ComputeShaderUniformTexel))
  {
    out |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::ComputeShaderStorage))
  {
    out |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::ComputeShaderStorageTexel))
  {
    out |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::IndexBuffer))
  {
    out |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::VertexBuffer))
  {
    out |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::VertexShaderUniform))
  {
    out |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  if (has_bits(usage, gfx::BufferUsage::FragmentShaderUniform))
  {
    out |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }
  return out;
}

constexpr VkImageUsageFlags to_vkUsage(gfx::ImageUsage usage)
{
  VkImageUsageFlags out = (VkImageUsageFlags) 0;
  if (has_bits(usage, gfx::ImageUsage::TransferSrc))
  {
    out |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::TransferDst))
  {
    out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::ComputeShaderSampled))
  {
    out |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::ComputeShaderStorage))
  {
    out |= VK_IMAGE_USAGE_STORAGE_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::VertexShaderSampled))
  {
    out |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::FragmentShaderSampled))
  {
    out |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::InputAttachment))
  {
    out |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::ReadColorAttachment))
  {
    out |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::WriteColorAttachment))
  {
    out |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::ReadDepthStencilAttachment))
  {
    out |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  if (has_bits(usage, gfx::ImageUsage::WriteDepthStencilAttachment))
  {
    out |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }
  return out;
}

inline void sync_acquire_buffer(DeviceTable const &table, VkCommandBuffer command_buffer,
                                VkBuffer buffer, gfx::BufferUsage usage, BufferScope const &scope)
{
  if (has_any_bit(usage, gfx::BufferUsage::TransferSrc | gfx::BufferUsage::TransferDst))
  {
    VkAccessFlags src_access = VK_ACCESS_NONE;
    if (has_bits(usage, gfx::BufferUsage::TransferSrc))
    {
      src_access |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (has_bits(usage, gfx::BufferUsage::TransferDst))
    {
      src_access |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = src_access,
                                  .dstAccessMask       = scope.access,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};

    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, scope.stages,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_bits(usage, gfx::BufferUsage::IndirectCommand))
  {
    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                  .dstAccessMask       = scope.access,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, scope.stages,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_any_bit(usage, gfx::BufferUsage::ComputeShaderUniform |
                             gfx::BufferUsage::ComputeShaderUniformTexel |
                             gfx::BufferUsage::ComputeShaderStorage |
                             gfx::BufferUsage::ComputeShaderStorageTexel))
  {
    VkAccessFlags src_access = VK_ACCESS_SHADER_READ_BIT;

    if (has_any_bit(usage, gfx::BufferUsage::ComputeShaderStorage |
                               gfx::BufferUsage::ComputeShaderStorageTexel))
    {
      src_access |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = src_access,
                                  .dstAccessMask       = scope.access,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};

    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, scope.stages,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_bits(usage, gfx::BufferUsage::IndexBuffer))
  {
    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = VK_ACCESS_INDEX_READ_BIT,
                                  .dstAccessMask       = scope.access,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, scope.stages,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_bits(usage, gfx::BufferUsage::VertexBuffer))
  {
    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                                  .dstAccessMask       = scope.access,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, scope.stages,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_any_bit(usage,
                  gfx::BufferUsage::VertexShaderUniform | gfx::BufferUsage::FragmentShaderUniform))
  {
    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_NONE;
    if (has_bits(usage, gfx::BufferUsage::VertexShaderUniform))
    {
      src_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (has_bits(usage, gfx::BufferUsage::FragmentShaderUniform))
    {
      src_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = VK_ACCESS_SHADER_READ_BIT,
                                  .dstAccessMask       = scope.access,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, src_stages, scope.stages, VK_DEPENDENCY_BY_REGION_BIT,
                             0, nullptr, 1, &barrier, 0, nullptr);
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the
// resource

inline void sync_release_buffer(DeviceTable const &table, VkCommandBuffer command_buffer,
                                VkBuffer buffer, gfx::BufferUsage usage, BufferScope const &scope)
{
  if (has_bits(usage, gfx::BufferUsage::TransferSrc))
  {
    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = scope.access,
                                  .dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};

    table.CmdPipelineBarrier(command_buffer, scope.stages, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_bits(usage, gfx::BufferUsage::IndirectCommand))
  {
    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = scope.access,
                                  .dstAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, scope.stages, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_any_bit(usage, gfx::BufferUsage::ComputeShaderUniform |
                             gfx::BufferUsage::ComputeShaderUniformTexel))
  {
    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = scope.access,
                                  .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, scope.stages, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_any_bit(usage, gfx::BufferUsage::IndexBuffer | gfx::BufferUsage::VertexBuffer))
  {
    VkAccessFlags dst_access = VK_ACCESS_NONE;
    if (has_bits(usage, gfx::BufferUsage::IndexBuffer))
    {
      dst_access |= VK_ACCESS_INDEX_READ_BIT;
    }

    if (has_bits(usage, gfx::BufferUsage::VertexBuffer))
    {
      dst_access |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }

    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = scope.access,
                                  .dstAccessMask       = dst_access,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, scope.stages, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);
  }

  if (has_any_bit(usage,
                  gfx::BufferUsage::VertexShaderUniform | gfx::BufferUsage::FragmentShaderUniform))
  {
    VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_NONE;

    if (has_bits(usage, gfx::BufferUsage::VertexShaderUniform))
    {
      dst_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (has_bits(usage, gfx::BufferUsage::FragmentShaderUniform))
    {
      dst_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    VkBufferMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                  .pNext               = nullptr,
                                  .srcAccessMask       = scope.access,
                                  .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
                                  .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                  .buffer              = buffer,
                                  .offset              = 0,
                                  .size                = VK_WHOLE_SIZE};
    table.CmdPipelineBarrier(command_buffer, scope.stages, dst_stages, VK_DEPENDENCY_BY_REGION_BIT,
                             0, nullptr, 1, &barrier, 0, nullptr);
  }
}

constexpr BufferScope transfer_buffer_scope(gfx::BufferUsage usage)
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;

  if (has_bits(usage, gfx::BufferUsage::TransferSrc))
  {
    access |= VK_ACCESS_TRANSFER_READ_BIT;
  }

  if (has_bits(usage, gfx::BufferUsage::TransferDst))
  {
    access |= VK_ACCESS_TRANSFER_WRITE_BIT;
  }

  return BufferScope{.stages = stages, .access = access};
}

constexpr BufferScope compute_storage_buffer_scope()
{
  return BufferScope{.stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                     .access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT};
}

// we need a single barrier for stages where the layout is different from the base layout
// (readonlyoptimal) i.e. storage, transfer
//
// Q(lamarrr): by inserting multiple barriers, will they all execute???
// only one of them will ever get executed because the previous barriers would have drained the work
// they had only executes if there is still work left to be drained???? but there will be multiple
// works to be drained once a work completes and there are multiple barriers waiting on it, will all
// the barriers perform the wait
//, no because this will implicitly depend on any op that performs read, writes, or layout
//transitions
//
// Q(lamarrr): if there's no task that has the required access and stages, will the barrier still
// execute?
//
//
// images and buffers must have a first command that operate on them? to transition them and provide
// access to other commands?
//
// what if no previous operation meets the barrier's requirements? then it will continue executing
// the requesting command
//
// no previous cmd means contents are undefined
//
// for transfer post-stages, we can omit the barriers, we only need to give barriers to readers
//
//
// Requirements:
// - layout merging:
//      - used as storage and sampled in the same command
//      - sampled and input attachment  in the same command
//      - transfer src and transfer dst in the same command
//
//
// release only upon init
//
inline void sync_acquire_image(DeviceTable const &table, VkCommandBuffer command_buffer,
                               VkImage image, gfx::ImageUsage usage, ImageScope const &scope,
                               VkImageAspectFlags aspects)
{
  if (has_any_bit(usage, gfx::ImageUsage::TransferSrc | gfx::ImageUsage::TransferDst))
  {
    VkAccessFlags src_access = VK_ACCESS_NONE;
    VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (has_bits(usage, gfx::ImageUsage::TransferSrc | gfx::ImageUsage::TransferDst))
    {
      old_layout = VK_IMAGE_LAYOUT_GENERAL;
      src_access = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (has_bits(usage, gfx::ImageUsage::TransferSrc))
    {
      old_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      src_access = VK_ACCESS_TRANSFER_READ_BIT;
    }
    else if (has_bits(usage, gfx::ImageUsage::TransferDst))
    {
      old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      src_access = VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = src_access,
        .dstAccessMask       = scope.access,
        .oldLayout           = old_layout,
        .newLayout           = scope.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = VkImageSubresourceRange{.aspectMask     = aspects,
                                                       .baseMipLevel   = 0,
                                                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                                                       .baseArrayLayer = 0,
                                                       .layerCount     = VK_REMAINING_ARRAY_LAYERS}};

    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, scope.stages,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
  }

  // if scope has compute shader write then it will always be transitioned to general
  if (has_bits(usage,
               gfx::ImageUsage::ComputeShaderStorage | gfx::ImageUsage::ComputeShaderSampled))
  {
    VkImageLayout old_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAccessFlags src_access = VK_ACCESS_SHADER_READ_BIT;

    if (has_bits(usage, gfx::ImageUsage::ComputeShaderStorage))
    {
      old_layout = VK_IMAGE_LAYOUT_GENERAL;
      src_access |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = src_access,
        .dstAccessMask       = scope.access,
        .oldLayout           = old_layout,
        .newLayout           = scope.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = VkImageSubresourceRange{.aspectMask     = aspects,
                                                       .baseMipLevel   = 0,
                                                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                                                       .baseArrayLayer = 0,
                                                       .layerCount     = VK_REMAINING_ARRAY_LAYERS}};

    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, scope.stages,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
  }

  if (has_any_bit(usage, gfx::ImageUsage::VertexShaderSampled |
                             gfx::ImageUsage::FragmentShaderSampled |
                             gfx::ImageUsage::InputAttachment))
  {
    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_NONE;

    if (has_bits(usage, gfx::ImageUsage::VertexShaderSampled))
    {
      src_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (has_any_bit(usage,
                    gfx::ImageUsage::FragmentShaderSampled | gfx::ImageUsage::InputAttachment))
    {
      src_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = VK_ACCESS_SHADER_READ_BIT,
        .dstAccessMask       = scope.access,
        .oldLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .newLayout           = scope.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = VkImageSubresourceRange{.aspectMask     = aspects,
                                                       .baseMipLevel   = 0,
                                                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                                                       .baseArrayLayer = 0,
                                                       .layerCount     = VK_REMAINING_ARRAY_LAYERS}};

    table.CmdPipelineBarrier(command_buffer, src_stages, scope.stages, VK_DEPENDENCY_BY_REGION_BIT,
                             0, nullptr, 0, nullptr, 1, &barrier);
  }

  if (has_any_bit(usage,
                  gfx::ImageUsage::ReadColorAttachment | gfx::ImageUsage::WriteColorAttachment))
  {
    VkAccessFlags src_access = VK_ACCESS_NONE;

    if (has_bits(usage, gfx::ImageUsage::ReadColorAttachment))
    {
      src_access |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }

    if (has_bits(usage, gfx::ImageUsage::WriteColorAttachment))
    {
      src_access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = src_access,
        .dstAccessMask       = scope.access,
        .oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout           = scope.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = VkImageSubresourceRange{.aspectMask     = aspects,
                                                       .baseMipLevel   = 0,
                                                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                                                       .baseArrayLayer = 0,
                                                       .layerCount     = VK_REMAINING_ARRAY_LAYERS}};

    table.CmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             scope.stages, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                             &barrier);
  }

  if (has_any_bit(usage, gfx::ImageUsage::ReadDepthStencilAttachment |
                             gfx::ImageUsage::WriteDepthStencilAttachment))
  {
    VkImageLayout        old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAccessFlags        src_access = VK_ACCESS_NONE;
    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_NONE;

    if (has_bits(usage, gfx::ImageUsage::ReadDepthStencilAttachment))
    {
      old_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      src_access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
      src_stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    if (has_bits(usage, gfx::ImageUsage::WriteDepthStencilAttachment))
    {
      old_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      src_access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      src_stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }

    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = src_access,
        .dstAccessMask       = scope.access,
        .oldLayout           = old_layout,
        .newLayout           = scope.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = VkImageSubresourceRange{.aspectMask     = aspects,
                                                       .baseMipLevel   = 0,
                                                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                                                       .baseArrayLayer = 0,
                                                       .layerCount     = VK_REMAINING_ARRAY_LAYERS}};

    table.CmdPipelineBarrier(command_buffer, src_stages, scope.stages, VK_DEPENDENCY_BY_REGION_BIT,
                             0, nullptr, 0, nullptr, 1, &barrier);
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the
// resource
inline void sync_release_image(DeviceTable const &table, VkCommandBuffer command_buffer,
                               VkImage image, gfx::ImageUsage usage, ImageScope const &scope,
                               VkImageAspectFlags aspects)
{
  // only shader-sampled images can run parallel to other command views
  // only transitioned to Shader read only if it is not used as storage at the same stage
  //
  // for all non-shader-read-only-optimal usages, an acquire must be performed
  //
  if (has_bits(usage, gfx::ImageUsage::ComputeShaderSampled) &&
      !has_bits(usage, gfx::ImageUsage::ComputeShaderStorage))
  {
    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = scope.access,
        .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout           = scope.layout,
        .newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = VkImageSubresourceRange{.aspectMask     = aspects,
                                                       .baseMipLevel   = 0,
                                                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                                                       .baseArrayLayer = 0,
                                                       .layerCount     = VK_REMAINING_ARRAY_LAYERS}};

    table.CmdPipelineBarrier(command_buffer, scope.stages, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);
  }

  if (has_any_bit(usage, gfx::ImageUsage::VertexShaderSampled |
                             gfx::ImageUsage::FragmentShaderSampled |
                             gfx::ImageUsage::InputAttachment))
  {
    VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_NONE;

    if (has_bits(usage, gfx::ImageUsage::VertexShaderSampled))
    {
      dst_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (has_any_bit(usage,
                    gfx::ImageUsage::FragmentShaderSampled | gfx::ImageUsage::InputAttachment))
    {
      dst_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    VkImageMemoryBarrier barrier{
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = VK_ACCESS_SHADER_READ_BIT,
        .dstAccessMask       = scope.access,
        .oldLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .newLayout           = scope.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = VkImageSubresourceRange{.aspectMask     = aspects,
                                                       .baseMipLevel   = 0,
                                                       .levelCount     = VK_REMAINING_MIP_LEVELS,
                                                       .baseArrayLayer = 0,
                                                       .layerCount     = VK_REMAINING_ARRAY_LAYERS}};

    table.CmdPipelineBarrier(command_buffer, scope.stages, dst_stages, VK_DEPENDENCY_BY_REGION_BIT,
                             0, nullptr, 0, nullptr, 1, &barrier);
  }
}

// apply to both src and dst since they require layout transitions
constexpr ImageScope transfer_image_scope(gfx::ImageUsage usage)
{
  VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;

  if (has_bits(usage, gfx::ImageUsage::TransferSrc | gfx::ImageUsage::TransferDst))
  {
    access = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    layout = VK_IMAGE_LAYOUT_GENERAL;
  }
  else if (has_bits(usage, gfx::ImageUsage::TransferSrc))
  {
    access = VK_ACCESS_TRANSFER_READ_BIT;
    layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  }
  else if (has_bits(usage, gfx::ImageUsage::TransferDst))
  {
    access = VK_ACCESS_TRANSFER_WRITE_BIT;
    layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope compute_storage_image_scope()
{
  return ImageScope{.stages = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    .access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                    .layout = VK_IMAGE_LAYOUT_GENERAL};
}

constexpr ImageScope color_attachment_image_scope(gfx::ImageUsage usage)
{
  VkAccessFlags access = VK_ACCESS_NONE;

  if (has_bits(usage, gfx::ImageUsage::ReadColorAttachment))
  {
    access |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  }

  if (has_bits(usage, gfx::ImageUsage::WriteColorAttachment))
  {
    access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }

  return ImageScope{.stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .access = access,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
}

constexpr ImageScope depth_stencil_attachment_image_scope(gfx::ImageUsage usage)
{
  VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlags        access = VK_ACCESS_NONE;

  if (has_bits(usage, gfx::ImageUsage::ReadDepthStencilAttachment))
  {
    stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  }

  if (has_bits(usage, gfx::ImageUsage::WriteDepthStencilAttachment))
  {
    stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr gfx::ImageUsage color_attachment_image_usage(gfx::RenderPassAttachment const &attachment)
{
  gfx::ImageUsage usage = gfx::ImageUsage::None;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store)
  {
    usage |= gfx::ImageUsage::WriteColorAttachment;
  }

  if (attachment.load_op == gfx::LoadOp::Load)
  {
    usage |= gfx::ImageUsage::ReadColorAttachment;
  }

  return usage;
}

constexpr gfx::ImageUsage
    depth_stencil_attachment_image_usage(gfx::RenderPassAttachment const &attachment)
{
  gfx::ImageUsage usage = gfx::ImageUsage::None;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store || attachment.store_op == gfx::StoreOp::DontCare ||
      attachment.stencil_load_op == gfx::LoadOp::Clear ||
      attachment.stencil_load_op == gfx::LoadOp::DontCare ||
      attachment.stencil_store_op == gfx::StoreOp::Store ||
      attachment.stencil_store_op == gfx::StoreOp::DontCare)
  {
    usage |= gfx::ImageUsage::WriteDepthStencilAttachment;
  }

  if (attachment.load_op == gfx::LoadOp::Load || attachment.stencil_load_op == gfx::LoadOp::Load)
  {
    usage |= gfx::ImageUsage::ReadDepthStencilAttachment;
  }

  return usage;
}

stx::Result<gfx::FormatProperties, gfx::Status>
    DeviceImpl::get_format_properties(gfx::Format format)
{
  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(vk_phy_device, (VkFormat) format, &props);
  return stx::Ok(gfx::FormatProperties{
      .linear_tiling_features  = (gfx::FormatFeatures) props.linearTilingFeatures,
      .optimal_tiling_features = (gfx::FormatFeatures) props.optimalTilingFeatures,
      .buffer_features         = (gfx::FormatFeatures) props.bufferFeatures});
}

stx::Result<gfx::Buffer, gfx::Status> DeviceImpl::create_buffer(gfx::BufferDesc const &desc)
{
  VkBufferCreateInfo      create_info{.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                      .pNext                 = nullptr,
                                      .flags                 = 0,
                                      .size                  = desc.size,
                                      .usage                 = to_vkUsage(desc.usage),
                                      .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
                                      .queueFamilyIndexCount = 1,
                                      .pQueueFamilyIndices   = nullptr};
  VmaAllocationCreateInfo alloc_create_info{.flags = 0,
                                            .usage = VMA_MEMORY_USAGE_UNKNOWN,
                                            .requiredFlags =
                                                (VkMemoryPropertyFlags) desc.properties,
                                            .preferredFlags = 0,
                                            .memoryTypeBits = 0,
                                            .pool           = nullptr,
                                            .pUserData      = nullptr,
                                            .priority       = 0};
  VmaAllocationInfo       vma_allocation_info;
  VmaAllocation           vma_allocation;
  VkBuffer                vk_buffer;
  VK_ERR(vmaCreateBuffer(vma_allocator, &create_info, &alloc_create_info, &vk_buffer,
                         &vma_allocation, &vma_allocation_info));

  void *host_map = nullptr;
  if (has_bits(desc.properties, gfx::MemoryProperties::HostVisible))
  {
    VK_ERR(vk_table.MapMemory(vk_device, vma_allocation_info.deviceMemory, 0, VK_WHOLE_SIZE, 0,
                              &host_map));
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
        .object      = (u64) vk_buffer,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  Buffer *buffer = (Buffer *) allocator.allocate(allocator.data, sizeof(Buffer), alignof(Buffer));
  if (buffer == nullptr)
  {
    vmaDestroyBuffer(vma_allocator, vk_buffer, vma_allocation);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (buffer) Buffer{.refcount            = 1,
                      .desc                = desc,
                      .vk_buffer           = vk_buffer,
                      .vma_allocation      = vma_allocation,
                      .vma_allocation_info = vma_allocation_info,
                      .host_map            = host_map};

  return stx::Ok((gfx::Buffer) buffer);
}

stx::Result<gfx::BufferView, gfx::Status>
    DeviceImpl::create_buffer_view(gfx::BufferViewDesc const &desc)
{
  VkBufferViewCreateInfo create_info{.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                                     .pNext  = nullptr,
                                     .flags  = 0,
                                     .buffer = ((Buffer *) desc.buffer)->vk_buffer,
                                     .format = (VkFormat) desc.format,
                                     .offset = desc.offset,
                                     .range  = desc.size};

  VkBufferView vk_view;

  VK_ERR(vk_table.CreateBufferView(vk_device, &create_info, nullptr, &vk_view));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
        .object      = (u64) vk_view,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  BufferView *view =
      (BufferView *) allocator.allocate(allocator.data, sizeof(BufferView), alignof(BufferView));

  if (view == nullptr)
  {
    vk_table.DestroyBufferView(vk_device, vk_view, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (view) BufferView{.refcount = 1, .desc = desc, .vk_view = vk_view};

  return stx::Ok((gfx::BufferView) view);
}

#define CREATE_IMAGE_PRELUDE()                                                                    \
  gfx::ImageDesc desc{old_desc};                                                                  \
  desc.usage |= gfx::ImageUsage::TransferDst;                                                     \
                                                                                                  \
  VkImageCreateInfo create_info{.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,     \
                                .pNext                 = nullptr,                                 \
                                .flags                 = 0,                                       \
                                .imageType             = (VkImageType) desc.type,                 \
                                .format                = (VkFormat) desc.format,                  \
                                .extent                = VkExtent3D{.width  = desc.extent.width,  \
                                                                    .height = desc.extent.height, \
                                                                    .depth  = desc.extent.depth},  \
                                .mipLevels             = desc.mip_levels,                         \
                                .arrayLayers           = desc.array_layers,                       \
                                .samples               = VK_SAMPLE_COUNT_1_BIT,                   \
                                .tiling                = VK_IMAGE_TILING_OPTIMAL,                 \
                                .usage                 = to_vkUsage(desc.usage),                  \
                                .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,               \
                                .queueFamilyIndexCount = 0,                                       \
                                .pQueueFamilyIndices   = nullptr,                                 \
                                .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};                      \
                                                                                                  \
  VkImage           vk_image;                                                                     \
  VmaAllocation     vma_allocation;                                                               \
  VmaAllocationInfo vma_allocation_info;                                                          \
  VK_ERR(vmaCreateImage(vma_allocator, &create_info, nullptr, &vk_image, &vma_allocation,         \
                        &vma_allocation_info));                                                   \
                                                                                                  \
  if (desc.label != nullptr)                                                                      \
  {                                                                                               \
    VkDebugMarkerObjectNameInfoEXT debug_info{                                                    \
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,                       \
        .pNext       = nullptr,                                                                   \
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,                                     \
        .object      = (u64) vk_image,                                                            \
        .pObjectName = desc.label};                                                               \
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);                                 \
  }                                                                                               \
                                                                                                  \
  ImageScope scope = transfer_image_scope(desc.usage);                                            \
                                                                                                  \
  VkImageSubresourceRange range{.aspectMask     = (VkImageAspectFlags) desc.aspects,              \
                                .baseMipLevel   = 0,                                              \
                                .levelCount     = VK_REMAINING_MIP_LEVELS,                        \
                                .baseArrayLayer = 0,                                              \
                                .layerCount     = VK_REMAINING_ARRAY_LAYERS};                         \
                                                                                                  \
  VkImageMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     \
                               .pNext               = nullptr,                                    \
                               .srcAccessMask       = VK_ACCESS_NONE,                             \
                               .dstAccessMask       = scope.access,                               \
                               .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,                  \
                               .newLayout           = scope.layout,                               \
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,                    \
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,                    \
                               .image               = vk_image,                                   \
                               .subresourceRange    = range};                                        \
                                                                                                  \
  Image *image = (Image *) allocator.allocate(allocator.data, sizeof(Image), alignof(Image));     \
                                                                                                  \
  if (image == nullptr)                                                                           \
  {                                                                                               \
    vmaDestroyImage(vma_allocator, vk_image, vma_allocation);                                     \
    return stx::Err(gfx::Status::OutOfHostMemory);                                                \
  }                                                                                               \
                                                                                                  \
  new (image) Image{.refcount            = 1,                                                     \
                    .desc                = desc,                                                  \
                    .is_weak             = false,                                                 \
                    .vk_image            = vk_image,                                              \
                    .vma_allocation      = vma_allocation,                                        \
                    .vma_allocation_info = vma_allocation_info};                                  \
                                                                                                  \
  VkCommandBuffer vk_command_buffer =                                                             \
      ((CommandEncoderImpl *) command_encoder.to_impl())->vk_command_buffer;                      \
  vk_table.CmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, scope.access, \
                              VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

stx::Result<gfx::Image, gfx::Status> DeviceImpl::create_image(gfx::ImageDesc const &old_desc,
                                                              gfx::Color            initial_color,
                                                              gfx::CommandEncoder  &command_encoder)
{
  CREATE_IMAGE_PRELUDE();

  vk_table.CmdClearColorImage(vk_command_buffer, vk_image, scope.layout,
                              (VkClearColorValue *) &initial_color, 1, &range);

  sync_release_image(vk_table, vk_command_buffer, vk_image, desc.usage, scope,
                     (VkImageAspectFlags) desc.aspects);

  return stx::Ok((gfx::Image) image);
}

stx::Result<gfx::Image, gfx::Status>
    DeviceImpl::create_image(gfx::ImageDesc const &old_desc,
                             gfx::DepthStencil     initial_depth_stencil,
                             gfx::CommandEncoder  &command_encoder)
{
  CREATE_IMAGE_PRELUDE();

  vk_table.CmdClearDepthStencilImage(vk_command_buffer, vk_image, scope.layout,
                                     (VkClearDepthStencilValue *) &initial_depth_stencil, 1,
                                     &range);

  sync_release_image(vk_table, vk_command_buffer, vk_image, desc.usage, scope,
                     (VkImageAspectFlags) desc.aspects);

  return stx::Ok((gfx::Image) image);
}

stx::Result<gfx::Image, gfx::Status>
    DeviceImpl::create_image(gfx::ImageDesc const &old_desc, gfx::Buffer initial_data,
                             stx::Span<gfx::BufferImageCopy const> copies,
                             gfx::CommandEncoder                  &command_encoder)
{
  CREATE_IMAGE_PRELUDE();

  VkBufferImageCopy *vk_copies = (VkBufferImageCopy *) allocator.allocate(
      allocator.data, sizeof(VkBufferImageCopy) * (u32) copies.size(), alignof(VkBufferImageCopy));

  if (vk_copies == nullptr)
  {
    vmaDestroyImage(vma_allocator, vk_image, vma_allocation);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  for (u32 i = 0; i < (u32) copies.size(); i++)
  {
    gfx::BufferImageCopy const &copy = copies.data()[i];
    vk_copies[i]                     = VkBufferImageCopy{
                            .bufferOffset      = copy.buffer_offset,
                            .bufferRowLength   = copy.buffer_row_length,
                            .bufferImageHeight = copy.buffer_image_height,
                            .imageSubresource =
            VkImageSubresourceLayers{.aspectMask = (VkImageAspectFlags) copy.image_layers.aspects,
                                                         .mipLevel   = copy.image_layers.mip_level,
                                                         .baseArrayLayer = copy.image_layers.first_array_layer,
                                                         .layerCount     = copy.image_layers.num_array_layers},
                            .imageOffset = VkOffset3D{(i32) copy.image_area.offset.x, (i32) copy.image_area.offset.y,
                                  (i32) copy.image_area.offset.z},
                            .imageExtent = VkExtent3D{copy.image_area.extent.width, copy.image_area.extent.height,
                                  copy.image_area.extent.depth}};
  }

  vk_table.CmdCopyBufferToImage(vk_command_buffer, ((Buffer *) initial_data)->vk_buffer, vk_image,
                                scope.layout, (u32) copies.size(), vk_copies);

  allocator.deallocate(allocator.data, vk_copies);

  sync_release_image(vk_table, vk_command_buffer, vk_image, desc.usage, scope,
                     (VkImageAspectFlags) desc.aspects);

  return stx::Ok((gfx::Image) image);
}

stx::Result<gfx::ImageView, gfx::Status>
    DeviceImpl::create_image_view(gfx::ImageViewDesc const &desc)
{
  VkImageViewCreateInfo create_info{
      .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = 0,
      .image            = ((Image *) desc.image)->vk_image,
      .viewType         = (VkImageViewType) desc.view_type,
      .format           = (VkFormat) desc.view_format,
      .components       = VkComponentMapping{.r = (VkComponentSwizzle) desc.mapping.r,
                                             .g = (VkComponentSwizzle) desc.mapping.g,
                                             .b = (VkComponentSwizzle) desc.mapping.b,
                                             .a = (VkComponentSwizzle) desc.mapping.a},
      .subresourceRange = VkImageSubresourceRange{.aspectMask   = (VkImageAspectFlags) desc.aspects,
                                                  .baseMipLevel = desc.first_mip_level,
                                                  .levelCount   = desc.num_mip_levels,
                                                  .baseArrayLayer = desc.first_array_layer,
                                                  .layerCount     = desc.num_array_layers}};

  VkImageView vk_view;
  VK_ERR(vk_table.CreateImageView(vk_device, &create_info, nullptr, &vk_view));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
        .object      = (u64) vk_view,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  ImageView *view =
      (ImageView *) allocator.allocate(allocator.data, sizeof(ImageView), alignof(ImageView));
  if (view == nullptr)
  {
    vk_table.DestroyImageView(vk_device, vk_view, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (view) ImageView{.refcount = 1, .desc = desc, .vk_view = vk_view};

  return stx::Ok((gfx::ImageView) view);
}

stx::Result<gfx::Sampler, gfx::Status> DeviceImpl::create_sampler(gfx::SamplerDesc const &desc)
{
  VkSamplerCreateInfo create_info{.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                                  .pNext            = nullptr,
                                  .magFilter        = (VkFilter) desc.mag_filter,
                                  .minFilter        = (VkFilter) desc.min_filter,
                                  .mipmapMode       = (VkSamplerMipmapMode) desc.mip_map_mode,
                                  .addressModeU     = (VkSamplerAddressMode) desc.address_mode_u,
                                  .addressModeV     = (VkSamplerAddressMode) desc.address_mode_v,
                                  .addressModeW     = (VkSamplerAddressMode) desc.address_mode_w,
                                  .mipLodBias       = desc.mip_lod_bias,
                                  .anisotropyEnable = (VkBool32) desc.anisotropy_enable,
                                  .maxAnisotropy    = desc.max_anisotropy,
                                  .compareEnable    = (VkBool32) desc.compare_enable,
                                  .compareOp        = (VkCompareOp) desc.compare_op,
                                  .minLod           = desc.min_lod,
                                  .maxLod           = desc.max_lod,
                                  .borderColor      = (VkBorderColor) desc.border_color,
                                  .unnormalizedCoordinates =
                                      (VkBool32) desc.unnormalized_coordinates};

  VkSampler vk_sampler;
  VK_ERR(vk_table.CreateSampler(vk_device, &create_info, nullptr, &vk_sampler));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,
        .object      = (u64) vk_sampler,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  Sampler *sampler =
      (Sampler *) allocator.allocate(allocator.data, sizeof(Sampler), alignof(Sampler));
  if (sampler == nullptr)
  {
    vk_table.DestroySampler(vk_device, vk_sampler, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (sampler) Sampler{.refcount = 1, .vk_sampler = vk_sampler};

  return stx::Ok((gfx::Sampler) sampler);
}

stx::Result<gfx::Shader, gfx::Status> DeviceImpl::create_shader(gfx::ShaderDesc const &desc)
{
  VkShaderModuleCreateInfo create_info{.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                       .pNext    = nullptr,
                                       .flags    = 0,
                                       .codeSize = desc.spirv_code.size_bytes(),
                                       .pCode    = desc.spirv_code.data()};

  VkShaderModule vk_shader;
  VK_ERR(vk_table.CreateShaderModule(vk_device, &create_info, nullptr, &vk_shader));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
        .object      = (u64) vk_shader,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  Shader *shader = (Shader *) allocator.allocate(allocator.data, sizeof(Shader), alignof(Shader));
  if (shader == nullptr)
  {
    vk_table.DestroyShaderModule(vk_device, vk_shader, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (shader) Shader{.refcount = 1, .vk_shader = vk_shader};

  return stx::Ok((gfx::Shader) shader);
}

stx::Result<gfx::RenderPass, gfx::Status>
    DeviceImpl::create_render_pass(gfx::RenderPassDesc const &desc)
{
  VkAttachmentDescription vk_attachments[gfx::MAX_COLOR_ATTACHMENTS * 2 + 1];
  VkAttachmentReference   vk_color_attachments[gfx::MAX_COLOR_ATTACHMENTS];
  VkAttachmentReference   vk_input_attachments[gfx::MAX_COLOR_ATTACHMENTS];
  VkAttachmentReference   vk_depth_stencil_attachment;
  u32                     num_attachments       = 0;
  u32                     num_color_attachments = (u32) desc.color_attachments.size();
  u32                     num_input_attachments = (u32) desc.input_attachments.size();
  u32                     num_depth_stencil_attachments =
      desc.depth_stencil_attachment.format == gfx::Format::Undefined ? 0 : 1;
  for (u32 icolor_attachment = 0; icolor_attachment < (u32) desc.color_attachments.size();
       num_attachments++, icolor_attachment++)
  {
    gfx::RenderPassAttachment const &attachment =
        desc.color_attachments.get_unsafe(icolor_attachment);
    vk_attachments[num_attachments] =
        VkAttachmentDescription{.flags          = 0,
                                .format         = (VkFormat) attachment.format,
                                .samples        = VK_SAMPLE_COUNT_1_BIT,
                                .loadOp         = (VkAttachmentLoadOp) attachment.load_op,
                                .storeOp        = (VkAttachmentStoreOp) attachment.store_op,
                                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    vk_color_attachments[icolor_attachment] = VkAttachmentReference{
        .attachment = num_attachments, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  }

  for (u32 iinput_attachment = 0; iinput_attachment < (u32) desc.input_attachments.size();
       num_attachments++, iinput_attachment++)
  {
    gfx::RenderPassAttachment const &attachment =
        desc.input_attachments.get_unsafe(iinput_attachment);
    vk_attachments[num_attachments] =
        VkAttachmentDescription{.flags          = 0,
                                .format         = (VkFormat) attachment.format,
                                .samples        = VK_SAMPLE_COUNT_1_BIT,
                                .loadOp         = (VkAttachmentLoadOp) attachment.load_op,
                                .storeOp        = (VkAttachmentStoreOp) attachment.store_op,
                                .stencilLoadOp  = (VkAttachmentLoadOp) attachment.stencil_load_op,
                                .stencilStoreOp = (VkAttachmentStoreOp) attachment.stencil_store_op,
                                .initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    vk_input_attachments[iinput_attachment] = VkAttachmentReference{
        .attachment = num_attachments, .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }

  if (num_depth_stencil_attachments != 0)
  {
    vk_attachments[num_attachments] = VkAttachmentDescription{
        .flags          = 0,
        .format         = (VkFormat) desc.depth_stencil_attachment.format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = (VkAttachmentLoadOp) desc.depth_stencil_attachment.load_op,
        .storeOp        = (VkAttachmentStoreOp) desc.depth_stencil_attachment.store_op,
        .stencilLoadOp  = (VkAttachmentLoadOp) desc.depth_stencil_attachment.stencil_load_op,
        .stencilStoreOp = (VkAttachmentStoreOp) desc.depth_stencil_attachment.stencil_store_op,
        .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    vk_depth_stencil_attachment = VkAttachmentReference{
        .attachment = num_attachments, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    num_attachments++;
  }

  VkSubpassDescription vk_subpass{.flags                   = 0,
                                  .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  .inputAttachmentCount    = num_input_attachments,
                                  .pInputAttachments       = vk_input_attachments,
                                  .colorAttachmentCount    = num_color_attachments,
                                  .pColorAttachments       = vk_color_attachments,
                                  .pResolveAttachments     = nullptr,
                                  .pDepthStencilAttachment = num_depth_stencil_attachments == 0 ?
                                                                 nullptr :
                                                                 &vk_depth_stencil_attachment,
                                  .preserveAttachmentCount = 0,
                                  .pPreserveAttachments    = nullptr};

  VkRenderPassCreateInfo create_info{.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                     .pNext           = nullptr,
                                     .flags           = 0,
                                     .attachmentCount = num_attachments,
                                     .pAttachments    = vk_attachments,
                                     .subpassCount    = 1,
                                     .pSubpasses      = &vk_subpass,
                                     .dependencyCount = 0,
                                     .pDependencies   = nullptr};
  VkRenderPass           vk_render_pass;
  VK_ERR(vk_table.CreateRenderPass(vk_device, &create_info, nullptr, &vk_render_pass));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
        .object      = (u64) vk_render_pass,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  RenderPass *render_pass =
      (RenderPass *) allocator.allocate(allocator.data, sizeof(RenderPass), alignof(RenderPass));
  if (render_pass == nullptr)
  {
    vk_table.DestroyRenderPass(vk_device, vk_render_pass, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (render_pass) RenderPass{.refcount = 1, .desc = desc, .vk_render_pass = vk_render_pass};

  return stx::Ok((gfx::RenderPass) render_pass);
}

stx::Result<gfx::Framebuffer, gfx::Status>
    DeviceImpl::create_framebuffer(gfx::FramebufferDesc const &desc)
{
  u32         num_color_attachments         = (u32) desc.color_attachments.size();
  u32         num_depth_stencil_attachments = desc.depth_stencil_attachment == nullptr ? 0 : 1;
  VkImageView vk_attachments[gfx::MAX_COLOR_ATTACHMENTS + 1];
  u32         num_attachments = 0;
  // TODO(lamarrr): fill, does the index slot association make sense preesently???
  VkFramebufferCreateInfo create_info{.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                      .pNext           = nullptr,
                                      .flags           = 0,
                                      .attachmentCount = num_attachments,
                                      .pAttachments    = vk_attachments,
                                      .width           = desc.extent.width,
                                      .height          = desc.extent.height,
                                      .layers          = desc.layers};

  VkFramebuffer vk_framebuffer;

  VK_ERR(vk_table.CreateFramebuffer(vk_device, &create_info, nullptr, &vk_framebuffer));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,
        .object      = (u64) vk_framebuffer,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  Framebuffer *framebuffer =
      (Framebuffer *) allocator.allocate(allocator.data, sizeof(Framebuffer), alignof(Framebuffer));
  if (framebuffer == nullptr)
  {
    vk_table.DestroyFramebuffer(vk_device, vk_framebuffer, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (framebuffer) Framebuffer{.refcount = 1, .desc = desc, .vk_framebuffer = vk_framebuffer};

  return stx::Ok((gfx::Framebuffer) framebuffer);
}

stx::Result<gfx::DescriptorSetLayout, gfx::Status>
    DeviceImpl::create_descriptor_set_layout(gfx::DescriptorSetLayoutDesc const &desc)
{
  u32                           num_bindings = (u32) desc.bindings.size();
  VkDescriptorSetLayoutBinding *vk_bindings  = (VkDescriptorSetLayoutBinding *) allocator.allocate(
      allocator.data, sizeof(VkDescriptorSetLayoutBinding) * num_bindings,
      alignof(VkDescriptorSetLayoutBinding));

  if (vk_bindings == nullptr)
  {
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  gfx::DescriptorBindingCount binding_count;

  for (usize i = 0; i < num_bindings; i++)
  {
    gfx::DescriptorBindingDesc const &binding = desc.bindings.data()[i];
    vk_bindings[i] = VkDescriptorSetLayoutBinding{.binding        = binding.binding,
                                                  .descriptorType = (VkDescriptorType) binding.type,
                                                  .descriptorCount    = binding.count,
                                                  .stageFlags         = VK_SHADER_STAGE_ALL,
                                                  .pImmutableSamplers = nullptr};

    binding_count.counts[(u32) binding.type] += binding.count;
  }

  VkDescriptorSetLayoutCreateInfo create_info{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext        = nullptr,
      .flags        = 0,
      .bindingCount = num_bindings,
      .pBindings    = vk_bindings};

  VkDescriptorSetLayout vk_layout;
  VkResult              result =
      vk_table.CreateDescriptorSetLayout(vk_device, &create_info, nullptr, &vk_layout);

  allocator.deallocate(allocator.data, vk_bindings);

  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
        .object      = (u64) vk_layout,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  DescriptorSetLayout *layout = (DescriptorSetLayout *) allocator.allocate(
      allocator.data, sizeof(DescriptorSetLayout), alignof(DescriptorSetLayout));
  if (layout == nullptr)
  {
    vk_table.DestroyDescriptorSetLayout(vk_device, vk_layout, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (layout)
      DescriptorSetLayout{.refcount = 1, .binding_count = binding_count, .vk_layout = vk_layout};

  return stx::Ok((gfx::DescriptorSetLayout) layout);
}

stx::Result<gfx::PipelineCache, gfx::Status>
    DeviceImpl::create_pipeline_cache(gfx::PipelineCacheDesc const &desc)
{
  VkPipelineCacheCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
                                        .pNext = nullptr,
                                        .flags = 0,
                                        .initialDataSize = desc.initial_data.size_bytes(),
                                        .pInitialData    = desc.initial_data.data()};

  VkPipelineCache vk_cache;
  VK_ERR(vk_table.CreatePipelineCache(vk_device, &create_info, nullptr, &vk_cache));

  PipelineCache *cache = (PipelineCache *) allocator.allocate(allocator.data, sizeof(PipelineCache),
                                                              alignof(PipelineCache));
  if (cache == nullptr)
  {
    vk_table.DestroyPipelineCache(vk_device, vk_cache, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (cache) PipelineCache{.refcount = 1, .vk_cache = vk_cache};

  return stx::Ok((gfx::PipelineCache) cache);
}

stx::Result<gfx::ComputePipeline, gfx::Status>
    DeviceImpl::create_compute_pipeline(gfx::ComputePipelineDesc const &desc)
{
  VkSpecializationInfo vk_specialization{
      .mapEntryCount = desc.compute_shader.specialization_constants.size(),
      .pMapEntries =
          (VkSpecializationMapEntry const *) desc.compute_shader.specialization_constants.data(),
      .dataSize = desc.compute_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.compute_shader.specialization_constants_data.data()};

  VkPipelineShaderStageCreateInfo vk_stage{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext  = nullptr,
      .flags  = 0,
      .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = ((Shader *) desc.compute_shader.shader)->vk_shader,
      .pName =
          desc.compute_shader.entry_point == nullptr ? "main" : desc.compute_shader.entry_point,
      .pSpecializationInfo = &vk_specialization};

  VkPushConstantRange push_constant_range{
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT, .offset = 0, .size = desc.push_constant_size};

  VkPipelineLayoutCreateInfo layout_create_info{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0,
      .setLayoutCount         = 1,
      .pSetLayouts            = &((DescriptorSetLayout *) desc.descriptor_set_layout)->vk_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges    = &push_constant_range};

  VkPipelineLayout vk_layout;
  VK_ERR(vk_table.CreatePipelineLayout(vk_device, &layout_create_info, nullptr, &vk_layout));

  VkComputePipelineCreateInfo create_info{.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                          .pNext  = nullptr,
                                          .flags  = 0,
                                          .stage  = vk_stage,
                                          .layout = vk_layout,
                                          .basePipelineHandle = nullptr,
                                          .basePipelineIndex  = 0};

  VkPipeline vk_pipeline;
  VkResult   result = vk_table.CreateComputePipelines(
      vk_device, desc.cache == nullptr ? nullptr : ((PipelineCache *) desc.cache)->vk_cache, 1,
      &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyPipelineLayout(vk_device, vk_layout, nullptr);
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
        .object      = (u64) vk_pipeline,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  ComputePipeline *pipeline = (ComputePipeline *) allocator.allocate(
      allocator.data, sizeof(ComputePipeline), alignof(ComputePipeline));
  if (pipeline == nullptr)
  {
    vk_table.DestroyPipelineLayout(vk_device, vk_layout, nullptr);
    vk_table.DestroyPipeline(vk_device, vk_pipeline, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (pipeline) ComputePipeline{.refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

  return stx::Ok((gfx::ComputePipeline) pipeline);
}

stx::Result<gfx::GraphicsPipeline, gfx::Status>
    DeviceImpl::create_graphics_pipeline(gfx::GraphicsPipelineDesc const &desc)
{
  VkSpecializationInfo vk_vs_specialization{
      .mapEntryCount = desc.vertex_shader.specialization_constants.size(),
      .pMapEntries =
          (VkSpecializationMapEntry const *) desc.vertex_shader.specialization_constants.data(),
      .dataSize = desc.vertex_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.vertex_shader.specialization_constants_data.data()};

  VkSpecializationInfo vk_fs_specialization{
      .mapEntryCount = desc.fragment_shader.specialization_constants.size(),
      .pMapEntries =
          (VkSpecializationMapEntry const *) desc.fragment_shader.specialization_constants.data(),
      .dataSize = desc.fragment_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.fragment_shader.specialization_constants_data.data()};

  VkPipelineShaderStageCreateInfo vk_stages[2] = {
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_VERTEX_BIT,
       .module = ((Shader *) desc.vertex_shader.shader)->vk_shader,
       .pName = desc.vertex_shader.entry_point == nullptr ? "main" : desc.vertex_shader.entry_point,
       .pSpecializationInfo = &vk_vs_specialization},
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = ((Shader *) desc.fragment_shader.shader)->vk_shader,
       .pName =
           desc.fragment_shader.entry_point == nullptr ? "main" : desc.fragment_shader.entry_point,
       .pSpecializationInfo = &vk_fs_specialization}};

  VkPushConstantRange push_constant_range{
      .stageFlags = VK_SHADER_STAGE_ALL, .offset = 0, .size = desc.push_constant_size};

  VkPipelineLayoutCreateInfo layout_create_info{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0,
      .setLayoutCount         = 1,
      .pSetLayouts            = &((DescriptorSetLayout *) desc.descriptor_set_layout)->vk_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges    = &push_constant_range};

  VkPipelineLayout vk_layout;
  VK_ERR(vk_table.CreatePipelineLayout(vk_device, &layout_create_info, nullptr, &vk_layout));

  VkVertexInputBindingDescription input_bindings[gfx::MAX_VERTEX_ATTRIBUTES];
  u32                             num_input_bindings = 0;
  for (; num_input_bindings < desc.vertex_input_bindings.size(); num_input_bindings++)
  {
    gfx::VertexInputBinding const &binding =
        desc.vertex_input_bindings.get_unsafe(num_input_bindings);
    input_bindings[num_input_bindings] = VkVertexInputBindingDescription{
        .binding   = binding.binding,
        .stride    = binding.stride,
        .inputRate = (VkVertexInputRate) binding.input_rate,
    };
  }

  VkVertexInputAttributeDescription attributes[gfx::MAX_VERTEX_ATTRIBUTES];
  u32                               num_attributes = 0;
  for (; num_attributes < desc.vertex_attributes.size(); num_attributes++)
  {
    gfx::VertexAttribute const &attribute = desc.vertex_attributes.get_unsafe(num_attributes);
    attributes[num_attributes] =
        VkVertexInputAttributeDescription{.location = attribute.location,
                                          .binding  = attribute.binding,
                                          .format   = (VkFormat) attribute.format,
                                          .offset   = attribute.offset};
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_state{
      .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext                           = nullptr,
      .flags                           = 0,
      .vertexBindingDescriptionCount   = num_input_bindings,
      .pVertexBindingDescriptions      = input_bindings,
      .vertexAttributeDescriptionCount = num_attributes,
      .pVertexAttributeDescriptions    = attributes};

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0,
      .topology               = (VkPrimitiveTopology) desc.primitive_topology,
      .primitiveRestartEnable = false};

  VkPipelineViewportStateCreateInfo viewport_state{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0,
      .viewportCount = 1,
      .pViewports    = nullptr,
      .scissorCount  = 1,
      .pScissors     = nullptr};

  VkPipelineRasterizationStateCreateInfo rasterization_state{
      .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext                   = nullptr,
      .flags                   = 0,
      .depthClampEnable        = desc.rasterization_state.depth_clamp_enable,
      .rasterizerDiscardEnable = false,
      .polygonMode             = (VkPolygonMode) desc.rasterization_state.polygon_mode,
      .cullMode                = (VkCullModeFlags) desc.rasterization_state.cull_mode,
      .frontFace               = (VkFrontFace) desc.rasterization_state.front_face,
      .depthBiasEnable         = desc.rasterization_state.depth_bias_enable,
      .depthBiasConstantFactor = desc.rasterization_state.depth_bias_constant_factor,
      .depthBiasClamp          = desc.rasterization_state.depth_bias_clamp,
      .depthBiasSlopeFactor    = desc.rasterization_state.depth_bias_slope_factor,
      .lineWidth               = 1.0f};

  VkPipelineMultisampleStateCreateInfo multisample_state{
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext                 = nullptr,
      .flags                 = 0,
      .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable   = false,
      .minSampleShading      = 1,
      .pSampleMask           = nullptr,
      .alphaToCoverageEnable = false,
      .alphaToOneEnable      = false};

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext                 = nullptr,
      .flags                 = 0,
      .depthTestEnable       = desc.depth_stencil_state.depth_test_enable,
      .depthWriteEnable      = desc.depth_stencil_state.depth_write_enable,
      .depthCompareOp        = (VkCompareOp) desc.depth_stencil_state.depth_compare_op,
      .depthBoundsTestEnable = desc.depth_stencil_state.depth_bounds_test_enable,
      .stencilTestEnable     = desc.depth_stencil_state.stencil_test_enable,
      .front =
          VkStencilOpState{
              .failOp      = (VkStencilOp) desc.depth_stencil_state.front_stencil.fail_op,
              .passOp      = (VkStencilOp) desc.depth_stencil_state.front_stencil.pass_op,
              .depthFailOp = (VkStencilOp) desc.depth_stencil_state.front_stencil.depth_fail_op,
              .compareOp   = (VkCompareOp) desc.depth_stencil_state.front_stencil.compare_op,
              .compareMask = desc.depth_stencil_state.front_stencil.compare_mask,
              .writeMask   = desc.depth_stencil_state.front_stencil.write_mask,
              .reference   = desc.depth_stencil_state.front_stencil.reference},
      .back =
          VkStencilOpState{
              .failOp      = (VkStencilOp) desc.depth_stencil_state.back_stencil.fail_op,
              .passOp      = (VkStencilOp) desc.depth_stencil_state.back_stencil.pass_op,
              .depthFailOp = (VkStencilOp) desc.depth_stencil_state.back_stencil.depth_fail_op,
              .compareOp   = (VkCompareOp) desc.depth_stencil_state.back_stencil.compare_op,
              .compareMask = desc.depth_stencil_state.back_stencil.compare_mask,
              .writeMask   = desc.depth_stencil_state.back_stencil.write_mask,
              .reference   = desc.depth_stencil_state.back_stencil.reference},
      .minDepthBounds = desc.depth_stencil_state.min_depth_bounds,
      .maxDepthBounds = desc.depth_stencil_state.max_depth_bounds};

  VkPipelineColorBlendAttachmentState attachment_states[gfx::MAX_COLOR_ATTACHMENTS];
  u32                                 num_color_attachments = 0;

  for (; num_color_attachments < (u32) desc.color_blend_state.attachments.size();
       num_color_attachments++)
  {
    gfx::PipelineColorBlendAttachmentState const &state =
        desc.color_blend_state.attachments.get_unsafe(num_color_attachments);
    attachment_states[num_color_attachments] = VkPipelineColorBlendAttachmentState{
        .blendEnable         = state.blend_enable,
        .srcColorBlendFactor = (VkBlendFactor) state.src_color_blend_factor,
        .dstColorBlendFactor = (VkBlendFactor) state.dst_color_blend_factor,
        .colorBlendOp        = (VkBlendOp) state.color_blend_op,
        .srcAlphaBlendFactor = (VkBlendFactor) state.src_alpha_blend_factor,
        .dstAlphaBlendFactor = (VkBlendFactor) state.dst_alpha_blend_factor,
        .alphaBlendOp        = (VkBlendOp) state.alpha_blend_op,
        .colorWriteMask      = (VkColorComponentFlags) state.color_write_mask};
  }

  VkPipelineColorBlendStateCreateInfo color_blend_state{
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0,
      .logicOpEnable   = desc.color_blend_state.logic_op_enable,
      .logicOp         = (VkLogicOp) desc.color_blend_state.logic_op,
      .attachmentCount = num_color_attachments,
      .pAttachments    = attachment_states,
      .blendConstants  = {
          desc.color_blend_state.blend_constants.x, desc.color_blend_state.blend_constants.y,
          desc.color_blend_state.blend_constants.z, desc.color_blend_state.blend_constants.w}};

  constexpr u32  num_dynamic_states                 = 6;
  VkDynamicState dynamic_states[num_dynamic_states] = {
      VK_DYNAMIC_STATE_VIEWPORT,          VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_BLEND_CONSTANTS,   VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK};

  VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext             = nullptr,
      .flags             = 0,
      .dynamicStateCount = num_dynamic_states,
      .pDynamicStates    = dynamic_states};

  VkGraphicsPipelineCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                           .pNext = nullptr,
                                           .flags = 0,
                                           .stageCount          = 2,
                                           .pStages             = vk_stages,
                                           .pVertexInputState   = &vertex_input_state,
                                           .pInputAssemblyState = &input_assembly_state,
                                           .pTessellationState  = nullptr,
                                           .pViewportState      = &viewport_state,
                                           .pRasterizationState = &rasterization_state,
                                           .pMultisampleState   = &multisample_state,
                                           .pDepthStencilState  = &depth_stencil_state,
                                           .pColorBlendState    = &color_blend_state,
                                           .pDynamicState       = &dynamic_state,
                                           .layout              = vk_layout,
                                           .renderPass =
                                               ((RenderPass *) desc.render_pass)->vk_render_pass,
                                           .subpass            = 0,
                                           .basePipelineHandle = nullptr,
                                           .basePipelineIndex  = 0};

  VkPipeline vk_pipeline;
  VkResult   result = vk_table.CreateGraphicsPipelines(
      vk_device, desc.cache == nullptr ? nullptr : ((PipelineCache *) desc.cache)->vk_cache, 1,
      &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyPipelineLayout(vk_device, vk_layout, nullptr);
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
        .object      = (u64) vk_pipeline,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  GraphicsPipeline *pipeline = (GraphicsPipeline *) allocator.allocate(
      allocator.data, sizeof(GraphicsPipeline), alignof(GraphicsPipeline));
  if (pipeline == nullptr)
  {
    vk_table.DestroyPipelineLayout(vk_device, vk_layout, nullptr);
    vk_table.DestroyPipeline(vk_device, vk_pipeline, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (pipeline)
      GraphicsPipeline{.refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

  return stx::Ok((gfx::GraphicsPipeline) pipeline);
}

stx::Result<gfx::CommandEncoder *, gfx::Status> DeviceImpl::create_command_encoder()
{
}

void *DeviceImpl::get_buffer_memory_map(gfx::Buffer buffer)
{
  return ((Buffer *) buffer)->host_map;
}

void DeviceImpl::invalidate_buffer_memory_map(gfx::Buffer                       buffer,
                                              stx::Span<gfx::MemoryRange const> ranges)
{
  Buffer *buffer_impl = ((Buffer *) buffer);

  if (buffer_impl->host_map != nullptr)
  {
    for (gfx::MemoryRange const &range : ranges)
    {
      VkMappedMemoryRange vk_range{.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                   .pNext  = nullptr,
                                   .memory = buffer_impl->vma_allocation_info.deviceMemory,
                                   .offset = range.offset,
                                   .size   = range.size};
      vk_table.InvalidateMappedMemoryRanges(vk_device, 1, &vk_range);
    }
  }
}

void DeviceImpl::flush_buffer_memory_map(gfx::Buffer                       buffer,
                                         stx::Span<gfx::MemoryRange const> ranges)
{
  Buffer *buffer_impl = ((Buffer *) buffer);

  if (buffer_impl->host_map != nullptr)
  {
    for (gfx::MemoryRange const &range : ranges)
    {
      VkMappedMemoryRange vk_range{.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                   .pNext  = nullptr,
                                   .memory = buffer_impl->vma_allocation_info.deviceMemory,
                                   .offset = range.offset,
                                   .size   = range.size};
      vk_table.FlushMappedMemoryRanges(vk_device, 1, &vk_range);
    }
  }
}

usize DeviceImpl::get_pipeline_cache_size(gfx::PipelineCache cache)
{
  usize size;
  ASH_CHECK(vk_table.GetPipelineCacheData(vk_device, ((PipelineCache *) cache)->vk_cache, &size,
                                          nullptr) == VK_SUCCESS);
}

usize DeviceImpl::get_pipeline_cache_data(gfx::PipelineCache cache, stx::Span<u8> out)
{
  usize size = out.size_bytes();
  ASH_CHECK(vk_table.GetPipelineCacheData(vk_device, ((PipelineCache *) cache)->vk_cache, &size,
                                          out.data()) == VK_SUCCESS);
  return size;
}

gfx::Status DeviceImpl::wait_for_fences(stx::Span<gfx::Fence const> fences, bool all, u64 timeout)
{
  u32      num_fences = (u32) fences.size();
  VkFence *vk_fences  = (VkFence *) allocator.allocate(allocator.data, sizeof(VkFence) * num_fences,
                                                       alignof(VkFence));
  ASH_CHECK(vk_fences != nullptr);
  for (u32 i = 0; i < num_fences; i++)
  {
    vk_fences[i] = ((Fence *) fences.data()[i])->vk_fence;
  }

  VkResult result = vk_table.WaitForFences(vk_device, num_fences, vk_fences, all, timeout);

  allocator.deallocate(allocator.data, vk_fences);

  return (gfx::Status) result;
}

void DeviceImpl::reset_fences(stx::Span<gfx::Fence const> fences)
{
  for (gfx::Fence fence : fences)
  {
    vk_table.ResetFences(vk_device, 1, &((Fence *) fence)->vk_fence);
  }
}

gfx::Status DeviceImpl::get_fence_status(gfx::Fence fence)
{
  return (gfx::Status) vk_table.GetFenceStatus(vk_device, ((Fence *) fence)->vk_fence);
}

void DeviceImpl::submit(gfx::CommandEncoder *encoder, gfx::Fence signal_fence)
{
}

void DeviceImpl::wait_idle()
{
  vk_table.DeviceWaitIdle(vk_device);
  // todo(lamarrr):check
}

void DeviceImpl::wait_queue_idle()
{
  vk_table.QueueWaitIdle(vk_queue);
  // todo(lamarrr):check
}


void CommandEncoderImpl::begin()
{
  VkCommandBufferBeginInfo info{.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                .pNext            = nullptr,
                                .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                .pInheritanceInfo = nullptr};
  device->vk_table.BeginCommandBuffer(vk_command_buffer, &info);        //TODO: CHECK
}

void CommandEncoderImpl::end()
{
  device->vk_table.EndCommandBuffer(vk_command_buffer);
}

void CommandEncoderImpl::reset()
{
  // reset state - pools
  device->vk_table.ResetCommandPool(device->vk_device, vk_command_pool, 0);
  // FLAG must have been set whilst creating to signify that batch deletion
}

void CommandEncoderImpl::begin_debug_marker(char const *region_name, Vec4 color)
{
  VkDebugMarkerMarkerInfoEXT info{.sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
                                  .pNext       = nullptr,
                                  .pMarkerName = region_name,
                                  .color       = {color.x, color.y, color.z, color.w}};
  device->vk_table.CmdDebugMarkerBeginEXT(vk_command_buffer, &info);
}

void CommandEncoderImpl::end_debug_marker()
{
  device->vk_table.CmdDebugMarkerEndEXT(vk_command_buffer);
}

void CommandEncoderImpl::fill_buffer(gfx::Buffer dst, u64 offset, u64 size, u32 data)
{
  Buffer     *dst_impl = (Buffer *) dst;
  BufferScope scope    = transfer_buffer_scope(dst_impl->desc.usage);
  sync_acquire_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);
  device->vk_table.CmdFillBuffer(vk_command_buffer, dst_impl->vk_buffer, offset, size, data);
  sync_release_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);
}

void CommandEncoderImpl::copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                                     stx::Span<gfx::BufferCopy const> copies)
{
  Buffer     *src_impl = (Buffer *) src;
  Buffer     *dst_impl = (Buffer *) dst;
  BufferScope scope    = transfer_buffer_scope(dst_impl->desc.usage);

  sync_acquire_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);

  u32           num_copies = (u32) copies.size();
  VkBufferCopy *vk_copies  = (VkBufferCopy *) allocator.allocate(
      allocator.data, sizeof(VkBufferCopy) * num_copies, alignof(VkBufferCopy));
  ASH_CHECK(vk_copies != nullptr);

  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferCopy const &copy = copies.data()[i];
    vk_copies[i] =
        VkBufferCopy{.srcOffset = copy.src_offset, .dstOffset = copy.dst_offset, .size = copy.size};
  }

  device->vk_table.CmdCopyBuffer(vk_command_buffer, src_impl->vk_buffer, dst_impl->vk_buffer,
                                 num_copies, vk_copies);

  sync_release_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);

  allocator.deallocate(allocator.data, vk_copies);
}

void CommandEncoderImpl::update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst)
{
  Buffer     *dst_impl = (Buffer *) dst;
  BufferScope scope    = transfer_buffer_scope(dst_impl->desc.usage);

  sync_acquire_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);

  device->vk_table.CmdUpdateBuffer(vk_command_buffer, dst_impl->vk_buffer, dst_offset,
                                   (u64) src.size(), src.data());

  sync_release_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);
}

void CommandEncoderImpl::clear_color_image(gfx::Image dst, gfx::Color clear_color,
                                           stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  Image     *dst_impl = (Image *) dst;
  ImageScope scope    = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  u32                      num_ranges = (u32) ranges.size();
  VkImageSubresourceRange *vk_ranges  = (VkImageSubresourceRange *) allocator.allocate(
      allocator.data, sizeof(VkImageSubresourceRange) * num_ranges,
      alignof(VkImageSubresourceRange));

  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges.data()[i];
    vk_ranges[i] = VkImageSubresourceRange{.aspectMask     = (VkImageAspectFlags) range.aspects,
                                           .baseMipLevel   = range.first_mip_level,
                                           .levelCount     = range.num_mip_levels,
                                           .baseArrayLayer = range.first_array_layer,
                                           .layerCount     = range.num_array_layers};
  }

  device->vk_table.CmdClearColorImage(vk_command_buffer, dst_impl->vk_image, scope.layout,
                                      (VkClearColorValue *) &clear_color, num_ranges, vk_ranges);

  sync_release_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  allocator.deallocate(allocator.data, vk_ranges);
}

void CommandEncoderImpl::clear_depth_stencil_image(
    gfx::Image dst, gfx::DepthStencil clear_depth_stencil,
    stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  Image     *dst_impl = (Image *) dst;
  ImageScope scope    = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  u32                      num_ranges = (u32) ranges.size();
  VkImageSubresourceRange *vk_ranges  = (VkImageSubresourceRange *) allocator.allocate(
      allocator.data, sizeof(VkImageSubresourceRange) * num_ranges,
      alignof(VkImageSubresourceRange));

  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges.data()[i];
    vk_ranges[i] = VkImageSubresourceRange{.aspectMask     = (VkImageAspectFlags) range.aspects,
                                           .baseMipLevel   = range.first_mip_level,
                                           .levelCount     = range.num_mip_levels,
                                           .baseArrayLayer = range.first_array_layer,
                                           .layerCount     = range.num_array_layers};
  }

  device->vk_table.CmdClearDepthStencilImage(vk_command_buffer, dst_impl->vk_image, scope.layout,
                                             (VkClearDepthStencilValue *) &clear_depth_stencil,
                                             num_ranges, vk_ranges);

  sync_release_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  allocator.deallocate(allocator.data, vk_ranges);
}

void CommandEncoderImpl::copy_image(gfx::Image src, gfx::Image dst,
                                    stx::Span<gfx::ImageCopy const> copies)
{
  Image     *src_impl  = (Image *) src;
  Image     *dst_impl  = (Image *) dst;
  ImageScope src_scope = transfer_image_scope(src_impl->desc.usage);
  ImageScope dst_scope = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, src_impl->vk_image, src_impl->desc.usage,
                     src_scope, (VkImageAspectFlags) dst_impl->desc.aspects);
  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     dst_scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  u32          num_copies = (u32) copies.size();
  VkImageCopy *vk_copies  = (VkImageCopy *) allocator.allocate(
      allocator.data, sizeof(VkImageCopy) * num_copies, alignof(VkImageCopy));

  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::ImageCopy const &copy = copies.data()[i];
    vk_copies[i]               = VkImageCopy{
                      .srcSubresource =
            VkImageSubresourceLayers{.aspectMask     = (VkImageAspectFlags) copy.src_layers.aspects,
                                                   .mipLevel       = copy.src_layers.mip_level,
                                                   .baseArrayLayer = copy.src_layers.first_array_layer,
                                                   .layerCount     = copy.src_layers.num_array_layers},
                      .srcOffset =
            VkOffset3D{copy.src_area.offset.x, copy.src_area.offset.y, copy.src_area.offset.z},
                      .dstSubresource =
            VkImageSubresourceLayers{.aspectMask     = (VkImageAspectFlags) copy.dst_layers.aspects,
                                                   .mipLevel       = copy.dst_layers.mip_level,
                                                   .baseArrayLayer = copy.dst_layers.first_array_layer,
                                                   .layerCount     = copy.dst_layers.num_array_layers},
                      .dstOffset = VkOffset3D{copy.dst_offset.x, copy.dst_offset.y, copy.dst_offset.z},
                      .extent    = VkExtent3D{copy.src_area.extent.width, copy.src_area.extent.height,
                             copy.src_area.extent.depth}};
  }

  device->vk_table.CmdCopyImage(vk_command_buffer, src_impl->vk_image, src_scope.layout,
                                src_impl->vk_image, dst_scope.layout, num_copies, vk_copies);

  sync_release_image(device->vk_table, vk_command_buffer, src_impl->vk_image, src_impl->desc.usage,
                     src_scope, (VkImageAspectFlags) src_impl->desc.aspects);
  sync_release_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     dst_scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  allocator.deallocate(allocator.data, vk_copies);
}

void CommandEncoderImpl::copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                              stx::Span<gfx::BufferImageCopy const> copies)
{
  Image     *dst_impl  = (Image *) dst;
  ImageScope dst_scope = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     dst_scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  u32                num_copies = (u32) copies.size();
  VkBufferImageCopy *vk_copies  = (VkBufferImageCopy *) allocator.allocate(
      allocator.data, sizeof(VkBufferImageCopy) * num_copies, alignof(VkBufferImageCopy));

  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferImageCopy const &copy = copies.data()[i];
    vk_copies[i]                     = VkBufferImageCopy{
                            .bufferOffset      = copy.buffer_offset,
                            .bufferRowLength   = copy.buffer_row_length,
                            .bufferImageHeight = copy.buffer_image_height,
                            .imageSubresource =
            VkImageSubresourceLayers{.aspectMask = (VkImageAspectFlags) copy.image_layers.aspects,
                                                         .mipLevel   = copy.image_layers.mip_level,
                                                         .baseArrayLayer = copy.image_layers.first_array_layer,
                                                         .layerCount     = copy.image_layers.num_array_layers},
                            .imageOffset = VkOffset3D{copy.image_area.offset.x, copy.image_area.offset.y,
                                  copy.image_area.offset.z},
                            .imageExtent = VkExtent3D{copy.image_area.extent.width, copy.image_area.extent.height,
                                  copy.image_area.extent.depth}};
  }

  device->vk_table.CmdCopyBufferToImage(vk_command_buffer, ((Buffer *) src)->vk_buffer,
                                        dst_impl->vk_image, dst_scope.layout, num_copies,
                                        vk_copies);

  sync_release_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     dst_scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  allocator.deallocate(allocator.data, vk_copies);
}

void CommandEncoderImpl::blit_image(gfx::Image src, gfx::Image dst,
                                    stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter)
{
  Image     *src_impl  = (Image *) src;
  Image     *dst_impl  = (Image *) dst;
  ImageScope src_scope = transfer_image_scope(src_impl->desc.usage);
  ImageScope dst_scope = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, src_impl->vk_image, src_impl->desc.usage,
                     src_scope, (VkImageAspectFlags) dst_impl->desc.aspects);
  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     dst_scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  u32          num_blits = (u32) blits.size();
  VkImageBlit *vk_blits  = (VkImageBlit *) allocator.allocate(
      allocator.data, sizeof(VkImageBlit) * num_blits, alignof(VkImageBlit));

  for (u32 i = 0; i < num_blits; i++)
  {
    gfx::ImageBlit const &blit = blits.data()[i];
    vk_blits[i]                = VkImageBlit{
                       .srcSubresource =
            VkImageSubresourceLayers{.aspectMask     = (VkImageAspectFlags) blit.src_layers.aspects,
                                                    .mipLevel       = blit.src_layers.mip_level,
                                                    .baseArrayLayer = blit.src_layers.first_array_layer,
                                                    .layerCount     = blit.src_layers.num_array_layers},
                       .srcOffsets = {VkOffset3D{(i32) blit.src_offsets[0].x, (i32) blit.src_offsets[0].y,
                                  (i32) blit.src_offsets[0].z},
                                      VkOffset3D{(i32) blit.src_offsets[1].x, (i32) blit.src_offsets[1].y,
                                  (i32) blit.src_offsets[1].z}},
                       .dstSubresource =
            VkImageSubresourceLayers{.aspectMask     = (VkImageAspectFlags) blit.dst_layers.aspects,
                                                    .mipLevel       = blit.dst_layers.mip_level,
                                                    .baseArrayLayer = blit.dst_layers.first_array_layer,
                                                    .layerCount     = blit.dst_layers.num_array_layers},
                       .dstOffsets = {VkOffset3D{(i32) blit.dst_offsets[0].x, (i32) blit.dst_offsets[0].y,
                                  (i32) blit.dst_offsets[0].z},
                                      VkOffset3D{(i32) blit.dst_offsets[1].x, (i32) blit.dst_offsets[1].y,
                                  (i32) blit.dst_offsets[1].z}}};
  }

  device->vk_table.CmdBlitImage(vk_command_buffer, src_impl->vk_image, src_scope.layout,
                                src_impl->vk_image, dst_scope.layout, num_blits, vk_blits,
                                (VkFilter) filter);

  sync_release_image(device->vk_table, vk_command_buffer, src_impl->vk_image, src_impl->desc.usage,
                     src_scope, (VkImageAspectFlags) src_impl->desc.aspects);
  sync_release_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     dst_scope, (VkImageAspectFlags) dst_impl->desc.aspects);

  allocator.deallocate(allocator.data, vk_blits);
}

void CommandEncoderImpl::begin_render_pass(
    gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
    stx::Span<gfx::Color const>        color_attachments_clear_values,
    stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values)
{
  this->framebuffer = framebuffer;
  this->render_pass = render_pass;

  for (gfx::ImageView view : graph->framebuffers[framebuffer].desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }

    gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
    ImageScope                scope = color_attachment_image_scope(image_resource.desc.scope);
    acquire_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                  scope.layout, image_resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  {
    gfx::ImageView view = graph->framebuffers[framebuffer].desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
      ImageScope scope = depth_stencil_attachment_image_scope(image_resource.desc.scope);
      acquire_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                    scope.layout, image_resource.desc.aspects, tmp_image_barriers);
      driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
    }
  }

  driver->cmd_begin_render_pass(
      rhi, graph->framebuffers[framebuffer].handle, graph->render_passes[render_pass].handle,
      render_area, color_attachments_clear_values, depth_stencil_attachments_clear_values);
}

void CommandEncoderImpl::end_render_pass()
{
  driver->cmd_end_render_pass(rhi);

  for (gfx::ImageView view : graph->framebuffers[framebuffer].desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }

    gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
    ImageScope                scope = color_attachment_image_scope(image_resource.desc.scope);
    release_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                  scope.layout, image_resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  {
    gfx::ImageView view = graph->framebuffers[framebuffer].desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
      ImageScope scope = depth_stencil_attachment_image_scope(image_resource.desc.scope);
      release_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                    scope.layout, image_resource.desc.aspects, tmp_image_barriers);
      driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
    }
  }
}

void CommandBuffer::bind_pipeline(gfx::ComputePipeline pipeline)
{
  this->compute_pipeline  = pipeline;
  this->graphics_pipeline = nullptr;

  driver->bind_pipeline(pipeline);
}

void CommandBuffer::bind_pipeline(gfx::GraphicsPipeline pipeline);

void CommandBuffer::dispatch(gfx::ComputePipeline pipeline, u32 group_count_x, u32 group_count_y,
                             u32 group_count_z, gfx::DescriptorSetBindings const &bindings,
                             stx::Span<u8 const> push_constants_data)
{
  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    acquire_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  driver->cmd_dispatch(rhi, pipeline, group_count_x, group_count_y, group_count_z, bindings,
                       push_constants_data);

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    release_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }
}

void CommandBuffer::dispatch_indirect(gfx::ComputePipeline pipeline, gfx::Buffer buffer, u64 offset,
                                      gfx::DescriptorSetBindings const &bindings,
                                      stx::Span<u8 const>               push_constants_data)
{
  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    acquire_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  driver->cmd_dispatch_indirect(rhi, graph->to_rhi(pipeline), graph->to_rhi(buffer), offset,
                                bindings, push_constants_data);

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    release_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }
}

void CommandBuffer::draw(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                         stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer,
                         u32 first_index, u32 num_indices, u32 vertex_offset, u32 first_instance,
                         u32 num_instances, gfx::DescriptorSetBindings const &bindings,
                         stx::Span<u8 const> push_constants_data)
{
  driver->cmd_draw(rhi, pipeline, state, vertex_buffers, index_buffer, first_index, num_indices,
                   vertex_offset, first_instance, num_instances, bindings, push_constants_data);
}

void CommandBuffer::draw_indirect(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                                  stx::Span<gfx::Buffer const> vertex_buffers,
                                  gfx::Buffer index_buffer, gfx::Buffer buffer, u64 offset,
                                  u32 draw_count, u32 stride,
                                  gfx::DescriptorSetBindings const &bindings,
                                  stx::Span<u8 const>               push_constants_data)
{
  driver->cmd_draw_indirect(rhi, pipeline, state, vertex_buffers, index_buffer, buffer, offset,
                            draw_count, stride, bindings, push_constants_data);
}

void CommandBuffer::on_execution_complete_fn(stx::UniqueFn<void()> &&fn)
{
  completion_tasks.push(std::move(fn)).unwrap();
}

}        // namespace vk
}        // namespace ash
