/// SPDX-License-Identifier: MIT
#include "ashura/gpu/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "vulkan/vulkan.h"
#include <stdlib.h>

namespace ash
{
namespace vk
{

#define BUFFER_FROM_VIEW(buffer_view) \
  ((Buffer *) (((BufferView *) (buffer_view))->desc.buffer))
#define IMAGE_FROM_VIEW(image_view) \
  ((Image *) (((ImageView *) (image_view))->desc.image))

VkResult DebugMarkerSetObjectTagEXT_Stub(VkDevice,
                                         const VkDebugMarkerObjectTagInfoEXT *)
{
  return VK_SUCCESS;
}

VkResult
    DebugMarkerSetObjectNameEXT_Stub(VkDevice,
                                     const VkDebugMarkerObjectNameInfoEXT *)
{
  return VK_SUCCESS;
}

void CmdDebugMarkerBeginEXT_Stub(VkCommandBuffer,
                                 const VkDebugMarkerMarkerInfoEXT *)
{
}

void CmdDebugMarkerEndEXT_Stub(VkCommandBuffer)
{
}

void CmdDebugMarkerInsertEXT_Stub(VkCommandBuffer,
                                  const VkDebugMarkerMarkerInfoEXT *)
{
}

VkResult SetDebugUtilsObjectNameEXT_Stub(VkDevice,
                                         const VkDebugUtilsObjectNameInfoEXT *)
{
  return VK_SUCCESS;
}

bool load_instance_table(VkInstance                instance,
                         PFN_vkGetInstanceProcAddr GetInstanceProcAddr,
                         InstanceTable &vk_table, bool validation_enabled)
{
  bool all_loaded = true;

#define LOAD_VK(function)                                               \
  vk_table.function =                                                   \
      (PFN_vk##function) GetInstanceProcAddr(instance, "vk" #function); \
  all_loaded = all_loaded && (vk_table.function != nullptr)

  LOAD_VK(CreateInstance);
  LOAD_VK(DestroyInstance);
  LOAD_VK(DestroySurfaceKHR);
  LOAD_VK(EnumeratePhysicalDevices);
  LOAD_VK(GetInstanceProcAddr);
  LOAD_VK(GetDeviceProcAddr);
  LOAD_VK(CreateDevice);
  LOAD_VK(EnumerateDeviceExtensionProperties);
  LOAD_VK(EnumerateDeviceLayerProperties);
  LOAD_VK(GetPhysicalDeviceFeatures);
  LOAD_VK(GetPhysicalDeviceFormatProperties);
  LOAD_VK(GetPhysicalDeviceImageFormatProperties);
  LOAD_VK(GetPhysicalDeviceMemoryProperties);
  LOAD_VK(GetPhysicalDeviceProperties);
  LOAD_VK(GetPhysicalDeviceQueueFamilyProperties);
  LOAD_VK(GetPhysicalDeviceSparseImageFormatProperties);

  LOAD_VK(GetPhysicalDeviceSurfaceSupportKHR);
  LOAD_VK(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  LOAD_VK(GetPhysicalDeviceSurfaceFormatsKHR);
  LOAD_VK(GetPhysicalDeviceSurfacePresentModesKHR);

  if (validation_enabled)
  {
    LOAD_VK(CreateDebugUtilsMessengerEXT);
    LOAD_VK(DestroyDebugUtilsMessengerEXT);
    LOAD_VK(SetDebugUtilsObjectNameEXT);
  }
  else
  {
    vk_table.SetDebugUtilsObjectNameEXT = SetDebugUtilsObjectNameEXT_Stub;
  }

#undef LOAD_VK

  return all_loaded;
}

bool to_c_str(Span<char const> str, Span<char> out)
{
  if (out.size() == 0)
  {
    return false;
  }
  usize cut_size = min(str.size(), out.size() - 1);
  mem::copy(str.slice(0, cut_size), out.data());
  out[cut_size] = 0;
  return true;
}

bool load_device_table(VkDevice dev, InstanceTable const &instance_table,
                       DeviceTable &vk_table, bool debug_marker_enabled)
{
  mem::zero(&vk_table, 1);
  bool all_loaded = true;

#define LOAD_VK(function)                                                  \
  vk_table.function = (PFN_vk##function) instance_table.GetDeviceProcAddr( \
      dev, "vk" #function);                                                \
  all_loaded = all_loaded && (vk_table.function != nullptr)

  // DEVICE OBJECT FUNCTIONS
  LOAD_VK(AllocateCommandBuffers);
  LOAD_VK(AllocateDescriptorSets);
  LOAD_VK(AllocateMemory);
  LOAD_VK(BindBufferMemory);
  LOAD_VK(BindImageMemory);
  LOAD_VK(CreateBuffer);
  LOAD_VK(CreateBufferView);
  LOAD_VK(CreateCommandPool);
  LOAD_VK(CreateComputePipelines);
  LOAD_VK(CreateDescriptorPool);
  LOAD_VK(CreateDescriptorSetLayout);
  LOAD_VK(CreateEvent);
  LOAD_VK(CreateFence);
  LOAD_VK(CreateGraphicsPipelines);
  LOAD_VK(CreateImage);
  LOAD_VK(CreateImageView);
  LOAD_VK(CreatePipelineCache);
  LOAD_VK(CreatePipelineLayout);
  LOAD_VK(CreateQueryPool);
  LOAD_VK(CreateSampler);
  LOAD_VK(CreateSemaphore);
  LOAD_VK(CreateShaderModule);
  LOAD_VK(DestroyBuffer);
  LOAD_VK(DestroyBufferView);
  LOAD_VK(DestroyCommandPool);
  LOAD_VK(DestroyDescriptorPool);
  LOAD_VK(DestroyDescriptorSetLayout);
  LOAD_VK(DestroyDevice);
  LOAD_VK(DestroyEvent);
  LOAD_VK(DestroyFence);
  LOAD_VK(DestroyImage);
  LOAD_VK(DestroyImageView);
  LOAD_VK(DestroyPipeline);
  LOAD_VK(DestroyPipelineCache);
  LOAD_VK(DestroyPipelineLayout);
  LOAD_VK(DestroyQueryPool);
  LOAD_VK(DestroySampler);
  LOAD_VK(DestroySemaphore);
  LOAD_VK(DestroyShaderModule);
  LOAD_VK(DeviceWaitIdle);
  LOAD_VK(FlushMappedMemoryRanges);
  LOAD_VK(FreeCommandBuffers);
  LOAD_VK(FreeDescriptorSets);
  LOAD_VK(FreeMemory);
  LOAD_VK(GetBufferMemoryRequirements);
  LOAD_VK(GetDeviceMemoryCommitment);
  LOAD_VK(GetDeviceQueue);
  LOAD_VK(GetEventStatus);
  LOAD_VK(GetFenceStatus);
  LOAD_VK(GetImageMemoryRequirements);
  LOAD_VK(GetImageSubresourceLayout);
  LOAD_VK(GetPipelineCacheData);
  LOAD_VK(GetQueryPoolResults);
  LOAD_VK(InvalidateMappedMemoryRanges);
  LOAD_VK(MapMemory);
  LOAD_VK(MergePipelineCaches);
  LOAD_VK(ResetCommandPool);
  LOAD_VK(ResetDescriptorPool);
  LOAD_VK(ResetEvent);
  LOAD_VK(ResetFences);
  LOAD_VK(SetEvent);
  LOAD_VK(UpdateDescriptorSets);
  LOAD_VK(UnmapMemory);
  LOAD_VK(WaitForFences);

  LOAD_VK(QueueSubmit);
  LOAD_VK(QueueWaitIdle);

  // COMMAND BUFFER OBJECT FUNCTIONS
  LOAD_VK(BeginCommandBuffer);
  LOAD_VK(CmdBeginQuery);
  LOAD_VK(CmdBindDescriptorSets);
  LOAD_VK(CmdBindIndexBuffer);
  LOAD_VK(CmdBindPipeline);
  LOAD_VK(CmdBindVertexBuffers);
  LOAD_VK(CmdBlitImage);
  LOAD_VK(CmdClearAttachments);
  LOAD_VK(CmdClearColorImage);
  LOAD_VK(CmdClearDepthStencilImage);
  LOAD_VK(CmdCopyBuffer);
  LOAD_VK(CmdCopyBufferToImage);
  LOAD_VK(CmdCopyImage);
  LOAD_VK(CmdCopyImageToBuffer);
  LOAD_VK(CmdCopyQueryPoolResults);
  LOAD_VK(CmdDispatch);
  LOAD_VK(CmdDispatchIndirect);
  LOAD_VK(CmdDraw);
  LOAD_VK(CmdDrawIndexed);
  LOAD_VK(CmdDrawIndexedIndirect);
  LOAD_VK(CmdDrawIndirect);
  LOAD_VK(CmdEndQuery);
  LOAD_VK(CmdFillBuffer);
  LOAD_VK(CmdPipelineBarrier);
  LOAD_VK(CmdPushConstants);
  LOAD_VK(CmdResetEvent);
  LOAD_VK(CmdResetQueryPool);
  LOAD_VK(CmdResolveImage);
  LOAD_VK(CmdSetBlendConstants);
  LOAD_VK(CmdSetDepthBias);
  LOAD_VK(CmdSetDepthBounds);
  LOAD_VK(CmdSetEvent);
  LOAD_VK(CmdSetLineWidth);
  LOAD_VK(CmdSetScissor);
  LOAD_VK(CmdSetStencilCompareMask);
  LOAD_VK(CmdSetStencilReference);
  LOAD_VK(CmdSetStencilWriteMask);
  LOAD_VK(CmdSetViewport);
  LOAD_VK(CmdUpdateBuffer);
  LOAD_VK(CmdWaitEvents);
  LOAD_VK(CmdWriteTimestamp);
  LOAD_VK(EndCommandBuffer);
  LOAD_VK(ResetCommandBuffer);

  LOAD_VK(CmdSetStencilOpEXT);
  LOAD_VK(CmdSetStencilTestEnableEXT);
  LOAD_VK(CmdSetCullModeEXT);
  LOAD_VK(CmdSetFrontFaceEXT);
  LOAD_VK(CmdSetPrimitiveTopologyEXT);
  LOAD_VK(CmdSetDepthBoundsTestEnableEXT);
  LOAD_VK(CmdSetDepthCompareOpEXT);
  LOAD_VK(CmdSetDepthTestEnableEXT);
  LOAD_VK(CmdSetDepthWriteEnableEXT);

  LOAD_VK(CmdBeginRenderingKHR);
  LOAD_VK(CmdEndRenderingKHR);

  LOAD_VK(CreateSwapchainKHR);
  LOAD_VK(DestroySwapchainKHR);
  LOAD_VK(GetSwapchainImagesKHR);
  LOAD_VK(AcquireNextImageKHR);
  LOAD_VK(QueuePresentKHR);

  if (debug_marker_enabled)
  {
    LOAD_VK(DebugMarkerSetObjectTagEXT);
    LOAD_VK(DebugMarkerSetObjectNameEXT);
    LOAD_VK(CmdDebugMarkerBeginEXT);
    LOAD_VK(CmdDebugMarkerEndEXT);
    LOAD_VK(CmdDebugMarkerInsertEXT);
  }
  else
  {
#define STUB_VK(function) vk_table.function = function##_Stub
    STUB_VK(DebugMarkerSetObjectTagEXT);
    STUB_VK(DebugMarkerSetObjectNameEXT);
    STUB_VK(CmdDebugMarkerBeginEXT);
    STUB_VK(CmdDebugMarkerEndEXT);
    STUB_VK(CmdDebugMarkerInsertEXT);
#undef STUB_VK
  }

#undef LOAD_VK

  return all_loaded;
}

void load_vma_table(InstanceTable const &instance_table,
                    DeviceTable const &vk_table, VmaVulkanFunctions &vma_table)
{
  mem::zero(&vma_table, 1);
#define SET_VMA_INST(function) vma_table.vk##function = instance_table.function
  SET_VMA_INST(GetInstanceProcAddr);
  SET_VMA_INST(GetDeviceProcAddr);
  SET_VMA_INST(GetPhysicalDeviceProperties);
  SET_VMA_INST(GetPhysicalDeviceMemoryProperties);
#undef SET_VMA_INST

#define SET_VMA_DEV(function) vma_table.vk##function = vk_table.function
  SET_VMA_DEV(AllocateMemory);
  SET_VMA_DEV(FreeMemory);
  SET_VMA_DEV(UnmapMemory);
  SET_VMA_DEV(FlushMappedMemoryRanges);
  SET_VMA_DEV(InvalidateMappedMemoryRanges);
  SET_VMA_DEV(BindBufferMemory);
  SET_VMA_DEV(BindImageMemory);
  SET_VMA_DEV(GetBufferMemoryRequirements);
  SET_VMA_DEV(GetImageMemoryRequirements);
  SET_VMA_DEV(CreateBuffer);
  SET_VMA_DEV(DestroyBuffer);
  SET_VMA_DEV(CreateImage);
  SET_VMA_DEV(DestroyImage);
  SET_VMA_DEV(CmdCopyBuffer);
#undef SET_VMA_DEV
}

static VkBool32 VKAPI_ATTR VKAPI_CALL
    debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                   VkDebugUtilsMessageTypeFlagsEXT             message_type,
                   VkDebugUtilsMessengerCallbackDataEXT const *data, void *)
{
  LogLevels level = LogLevels::Trace;
  if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
  {
    level = LogLevels::Debug;
  }
  else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
  {
    level = LogLevels::Warning;
  }
  else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
  {
    level = LogLevels::Info;
  }
  else
  {
    level = LogLevels::Trace;
  }

  logger->log(
      level, "[Type: ", string_VkDebugUtilsMessageTypeFlagsEXT(message_type),
      ", Id: ", data->messageIdNumber, ", Name: ", data->pMessageIdName, "] ",
      data->pMessage == nullptr ? "(empty message)" : data->pMessage);
  if (data->objectCount != 0)
  {
    logger->log(level, "Objects Involved:");
    for (u32 i = 0; i < data->objectCount; i++)
    {
      logger->log(level,
                  "[Type: ", string_VkObjectType(data->pObjects[i].objectType),
                  "] ",
                  data->pObjects[i].pObjectName == nullptr ?
                      "(unnamed)" :
                      data->pObjects[i].pObjectName);
    }
  }

  return VK_FALSE;
}

constexpr bool has_read_access(VkAccessFlags access)
{
  return has_any_bit(
      access,
      (VkAccessFlags) (VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
                       VK_ACCESS_INDEX_READ_BIT |
                       VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
                       VK_ACCESS_UNIFORM_READ_BIT |
                       VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
                       VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                       VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT |
                       VK_ACCESS_MEMORY_READ_BIT |
                       VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT |
                       VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT |
                       VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT |
                       VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR |
                       VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT |
                       VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR |
                       VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV));
}

constexpr bool has_write_access(VkAccessFlags access)
{
  return has_any_bit(
      access,
      (VkAccessFlags) (VK_ACCESS_SHADER_WRITE_BIT |
                       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                       VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT |
                       VK_ACCESS_MEMORY_WRITE_BIT |
                       VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT |
                       VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT |
                       VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
                       VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV |
                       VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV));
}

inline bool sync_buffer_state(BufferState &state, BufferAccess request,
                              VkBufferMemoryBarrier &barrier,
                              VkPipelineStageFlags  &src_stages,
                              VkPipelineStageFlags  &dst_stages)
{
  bool const has_write = has_write_access(request.access);
  bool const has_read  = has_read_access(request.access);

  switch (state.sequence)
  {
      // no sync needed, no accessor before this
    case AccessSequence::None:
    {
      if (has_write)
      {
        state.sequence = AccessSequence::Write;
        state.access[0] =
            BufferAccess{.stages = request.stages, .access = request.access};
        return false;
      }

      if (has_read)
      {
        state.sequence = AccessSequence::Reads;
        state.access[0] =
            BufferAccess{.stages = request.stages, .access = request.access};
        return false;
      }

      return false;
    }
    case AccessSequence::Reads:
    {
      if (has_write)
      {
        // wait till done reading before modifying
        // reset access sequence since all stages following this write need to
        // wait on this write
        state.sequence                    = AccessSequence::Write;
        BufferAccess const previous_reads = state.access[0];
        state.access[0] =
            BufferAccess{.stages = request.stages, .access = request.access};
        state.access[1]       = BufferAccess{};
        src_stages            = previous_reads.stages;
        barrier.srcAccessMask = previous_reads.access;
        dst_stages            = request.stages;
        barrier.dstAccessMask = request.access;
        return true;
      }

      if (has_read)
      {
        // combine all subsequent reads, so the next writer knows to wait on all
        // combined reads to complete
        state.sequence                    = AccessSequence::Reads;
        BufferAccess const previous_reads = state.access[0];
        state.access[0] =
            BufferAccess{.stages = previous_reads.stages | request.stages,
                         .access = previous_reads.access | request.access};
        return false;
      }

      return false;
    }
    case AccessSequence::Write:
    {
      if (has_write)
      {
        // wait till done writing before modifying
        // remove previous write since this access already waits on another
        // access to complete and the next access will have to wait on this
        // access
        state.sequence                    = AccessSequence::Write;
        BufferAccess const previous_write = state.access[0];
        state.access[0] =
            BufferAccess{.stages = request.stages, .access = request.access};
        state.access[1]       = BufferAccess{};
        src_stages            = previous_write.stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = previous_write.access;
        barrier.dstAccessMask = request.access;
        return true;
      }

      if (has_read)
      {
        // wait till all write stages are done
        state.sequence = AccessSequence::ReadAfterWrite;
        state.access[1] =
            BufferAccess{.stages = request.stages, .access = request.access};
        src_stages            = state.access[0].stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = state.access[0].access;
        barrier.dstAccessMask = request.access;
        return true;
      }

      return false;
    }
    case AccessSequence::ReadAfterWrite:
    {
      if (has_write)
      {
        // wait for all reading stages only
        // stages can be reset and point only to the latest write stage, since
        // they all need to wait for this write anyway.
        state.sequence                    = AccessSequence::Write;
        BufferAccess const previous_reads = state.access[1];
        state.access[0] =
            BufferAccess{.stages = request.stages, .access = request.access};
        state.access[1]       = BufferAccess{};
        src_stages            = previous_reads.stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = previous_reads.access;
        barrier.dstAccessMask = request.access;
        return true;
      }

      if (has_read)
      {
        // wait for all write stages to be done
        // no need to wait on other reads since we are only performing a read
        // mask all subsequent reads so next writer knows to wait on all reads
        // to complete

        // if stage and access intersects previous barrier, no need to add new
        // one

        if (has_any_bit(state.access[1].stages, request.stages) &&
            has_any_bit(state.access[1].access, request.access))
        {
          return false;
        }

        state.sequence = AccessSequence::ReadAfterWrite;
        state.access[1].stages |= request.stages;
        state.access[1].access |= request.access;
        src_stages            = state.access[0].stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = state.access[0].access;
        barrier.dstAccessMask = request.access;
        return true;
      }

      return false;
    }
    default:
      return false;
  }
}

// layout transitions are considered write operations even if only a read
// happens so multiple ones can't happen at the same time
//
// we'll kind of be waiting on a barrier operation which doesn't make sense cos
// the barrier might have already taken care of us even when they both only
// perform reads
//
// if their scopes don't line-up, they won't observe the effects same
inline bool sync_image_state(ImageState &state, ImageAccess request,
                             VkImageMemoryBarrier &barrier,
                             VkPipelineStageFlags &src_stages,
                             VkPipelineStageFlags &dst_stages)
{
  VkImageLayout const current_layout = state.access[0].layout;
  bool const needs_layout_transition = current_layout != request.layout;
  bool const has_write =
      has_write_access(request.access) || needs_layout_transition;
  bool const has_read = has_read_access(request.access);
  barrier.oldLayout   = current_layout;
  barrier.newLayout   = request.layout;

  switch (state.sequence)
  {
      // no sync needed, no accessor before this
    case AccessSequence::None:
    {
      if (has_write)
      {
        state.sequence  = AccessSequence::Write;
        state.access[0] = ImageAccess{.stages = request.stages,
                                      .access = request.access,
                                      .layout = request.layout};

        if (needs_layout_transition)
        {
          src_stages            = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
          dst_stages            = request.stages;
          barrier.srcAccessMask = VK_ACCESS_NONE;
          barrier.dstAccessMask = request.access;
          return true;
        }

        return false;
      }

      if (has_read)
      {
        state.sequence  = AccessSequence::Reads;
        state.access[0] = ImageAccess{.stages = request.stages,
                                      .access = request.access,
                                      .layout = request.layout};
        return false;
      }

      return false;
    }
    case AccessSequence::Reads:
    {
      if (has_write)
      {
        // wait till done reading before modifying
        // reset access sequence since all stages following this write need to
        // wait on this write
        state.sequence                   = AccessSequence::Write;
        ImageAccess const previous_reads = state.access[0];
        state.access[0]                  = ImageAccess{.stages = request.stages,
                                                       .access = request.access,
                                                       .layout = request.layout};
        state.access[1]                  = ImageAccess{};
        src_stages                       = previous_reads.stages;
        dst_stages                       = request.stages;
        barrier.srcAccessMask            = previous_reads.access;
        barrier.dstAccessMask            = request.access;
        return true;
      }

      if (has_read)
      {
        // combine all subsequent reads, so the next writer knows to wait on all
        // combined reads to complete
        state.sequence                   = AccessSequence::Reads;
        ImageAccess const previous_reads = state.access[0];
        state.access[0] =
            ImageAccess{.stages = previous_reads.stages | request.stages,
                        .access = previous_reads.access | request.access,
                        .layout = request.layout};
        return false;
      }

      return false;
    }
    case AccessSequence::Write:
    {
      if (has_write)
      {
        // wait till done writing before modifying
        // remove previous write since this access already waits on another
        // access to complete and the next access will have to wait on this
        // access
        state.sequence                   = AccessSequence::Write;
        ImageAccess const previous_write = state.access[0];
        state.access[0]                  = ImageAccess{.stages = request.stages,
                                                       .access = request.access,
                                                       .layout = request.layout};
        state.access[1]                  = ImageAccess{};
        src_stages                       = previous_write.stages;
        dst_stages                       = request.stages;
        barrier.srcAccessMask            = previous_write.access;
        barrier.dstAccessMask            = request.access;
        return true;
      }

      if (has_read)
      {
        // wait till all write stages are done
        state.sequence        = AccessSequence::ReadAfterWrite;
        state.access[1]       = ImageAccess{.stages = request.stages,
                                            .access = request.access,
                                            .layout = request.layout};
        src_stages            = state.access[0].stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = state.access[0].access;
        barrier.dstAccessMask = request.access;
        return true;
      }

      return false;
    }
    case AccessSequence::ReadAfterWrite:
    {
      if (has_write)
      {
        // wait for all reading stages only
        // stages can be reset and point only to the latest write stage, since
        // they all need to wait for this write anyway.
        state.sequence                   = AccessSequence::Write;
        ImageAccess const previous_reads = state.access[1];
        state.access[0]                  = ImageAccess{.stages = request.stages,
                                                       .access = request.access,
                                                       .layout = request.layout};
        state.access[1]                  = ImageAccess{};
        src_stages                       = previous_reads.stages;
        dst_stages                       = request.stages;
        barrier.srcAccessMask            = previous_reads.access;
        barrier.dstAccessMask            = request.access;
        return true;
      }

      if (has_read)
      {
        // wait for all write stages to be done
        // no need to wait on other reads since we are only performing a read
        // mask all subsequent reads so next writer knows to wait on all reads
        // to complete
        //
        // if stage and access intersects previous barrier, no need to add new
        // one as we'll observe the effect
        state.sequence = AccessSequence::ReadAfterWrite;

        if (has_any_bit(state.access[1].stages, request.stages) &&
            has_any_bit(state.access[1].access, request.access))
        {
          return false;
        }

        state.access[1].stages |= request.stages;
        state.access[1].access |= request.access;
        src_stages            = state.access[0].stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = state.access[0].access;
        barrier.dstAccessMask = request.access;
        return true;
      }

      return false;
    }
    default:
      return false;
  }
}

inline void access_buffer(CommandEncoder const &enc, Buffer &buffer,
                          VkPipelineStageFlags stages, VkAccessFlags access)
{
  VkBufferMemoryBarrier barrier;
  VkPipelineStageFlags  src_stages;
  VkPipelineStageFlags  dst_stages;
  if (sync_buffer_state(buffer.state,
                        BufferAccess{.stages = stages, .access = access},
                        barrier, src_stages, dst_stages))
  {
    barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer              = buffer.vk_buffer;
    barrier.offset              = 0;
    barrier.size                = VK_WHOLE_SIZE;
    enc.dev->vk_table.CmdPipelineBarrier(enc.vk_command_buffer, src_stages,
                                         dst_stages, 0, 0, nullptr, 1, &barrier,
                                         0, nullptr);
  }
}

inline void access_image_aspect(CommandEncoder const &enc, Image &image,
                                VkPipelineStageFlags stages,
                                VkAccessFlags access, VkImageLayout layout,
                                gpu::ImageAspects aspects, u32 aspect_index)
{
  VkImageMemoryBarrier barrier;
  VkPipelineStageFlags src_stages;
  VkPipelineStageFlags dst_stages;
  if (sync_image_state(
          image.states[aspect_index],
          ImageAccess{.stages = stages, .access = access, .layout = layout},
          barrier, src_stages, dst_stages))
  {
    barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = image.vk_image;
    barrier.subresourceRange.aspectMask     = (VkImageAspectFlags) aspects;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
    enc.dev->vk_table.CmdPipelineBarrier(enc.vk_command_buffer, src_stages,
                                         dst_stages, 0, 0, nullptr, 0, nullptr,
                                         1, &barrier);
  }
}

inline void access_image_all_aspects(CommandEncoder const &enc, Image &image,
                                     VkPipelineStageFlags stages,
                                     VkAccessFlags access, VkImageLayout layout)
{
  if (has_bits(image.desc.aspects,
               gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil))
  {
    access_image_aspect(enc, image, stages, access, layout,
                        gpu::ImageAspects::Depth, DEPTH_ASPECT_IDX);
    access_image_aspect(enc, image, stages, access, layout,
                        gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);
  }
  else
  {
    access_image_aspect(enc, image, stages, access, layout, image.desc.aspects,
                        0);
  }
}

inline void access_image_depth(CommandEncoder const &enc, Image &image,
                               VkPipelineStageFlags stages,
                               VkAccessFlags access, VkImageLayout layout)
{
  access_image_aspect(enc, image, stages, access, layout,
                      gpu::ImageAspects::Depth, DEPTH_ASPECT_IDX);
}

inline void access_image_stencil(CommandEncoder const &enc, Image &image,
                                 VkPipelineStageFlags stages,
                                 VkAccessFlags access, VkImageLayout layout)
{
  access_image_aspect(enc, image, stages, access, layout,
                      gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);
}

inline void access_compute_bindings(CommandEncoder const &enc,
                                    DescriptorSet const  &set)
{
  for (u32 ibinding = 0; ibinding < set.num_bindings; ibinding++)
  {
    DescriptorBinding const &binding = set.bindings[ibinding];
    switch (binding.type)
    {
      case gpu::DescriptorType::CombinedImageSampler:
      case gpu::DescriptorType::SampledImage:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image_all_aspects(enc, *binding.images[i],
                                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     VK_ACCESS_SHADER_READ_BIT,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

      case gpu::DescriptorType::StorageImage:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image_all_aspects(
                enc, *binding.images[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL);
          }
        }
        break;

      case gpu::DescriptorType::UniformBuffer:
      case gpu::DescriptorType::DynamicUniformBuffer:
      case gpu::DescriptorType::UniformTexelBuffer:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.buffers[i] != nullptr)
          {
            access_buffer(enc, *binding.buffers[i],
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

      case gpu::DescriptorType::StorageBuffer:
      case gpu::DescriptorType::DynamicStorageBuffer:
      case gpu::DescriptorType::StorageTexelBuffer:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.buffers[i] != nullptr)
          {
            access_buffer(
                enc, *binding.buffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
          }
        }
        break;

      case gpu::DescriptorType::InputAttachment:
        break;

      default:
        UNREACHABLE();
    }
  }
}

inline void access_graphics_bindings(CommandEncoder const &enc,
                                     DescriptorSet const  &set)
{
  for (u32 ibinding = 0; ibinding < set.num_bindings; ibinding++)
  {
    DescriptorBinding const &binding = set.bindings[ibinding];
    switch (binding.type)
    {
      case gpu::DescriptorType::CombinedImageSampler:
      case gpu::DescriptorType::SampledImage:
      case gpu::DescriptorType::InputAttachment:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image_all_aspects(enc, *binding.images[i],
                                     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     VK_ACCESS_SHADER_READ_BIT,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

      case gpu::DescriptorType::UniformTexelBuffer:
      case gpu::DescriptorType::UniformBuffer:
      case gpu::DescriptorType::DynamicUniformBuffer:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.buffers[i] != nullptr)
          {
            access_buffer(enc, *binding.buffers[i],
                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        // only readonly storage images are supported
      case gpu::DescriptorType::StorageImage:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image_all_aspects(enc, *binding.images[i],
                                     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     VK_ACCESS_SHADER_READ_BIT,
                                     VK_IMAGE_LAYOUT_GENERAL);
          }
        }
        break;

        // only readonly storage buffers are supported
      case gpu::DescriptorType::StorageTexelBuffer:
      case gpu::DescriptorType::StorageBuffer:
      case gpu::DescriptorType::DynamicStorageBuffer:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.buffers[i] != nullptr)
          {
            access_buffer(enc, *binding.buffers[i],
                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

      case gpu::DescriptorType::Sampler:
        break;
      default:
        UNREACHABLE();
    }
  }
}

inline void validate_render_pass_compatible(gpu::CommandEncoder   self_,
                                            gpu::GraphicsPipeline pipeline_)
{
  CommandEncoder          *self     = (CommandEncoder *) self_;
  RenderPassContext const &ctx      = self->render_ctx;
  GraphicsPipeline        *pipeline = (GraphicsPipeline *) pipeline_;

  CHECK(pipeline->num_colors == ctx.num_color_attachments);
  CHECK(pipeline->num_depths == ctx.num_depth_attachments);
  CHECK(pipeline->num_stencils == ctx.num_stencil_attachments);

  for (u32 i = 0; i < pipeline->num_colors; i++)
  {
    if (pipeline->colors[i] != gpu::Format::Undefined)
    {
      CHECK(ctx.color_attachments[i].view != nullptr);
      CHECK(pipeline->colors[i] ==
            IMAGE_FROM_VIEW(ctx.color_attachments[i].view)->desc.format);
    }
  }

  for (u32 i = 0; i < pipeline->num_depths; i++)
  {
    if (pipeline->depth[i] != gpu::Format::Undefined)
    {
      CHECK(ctx.depth_attachment[i].view != nullptr);
      CHECK(pipeline->depth[i] ==
            IMAGE_FROM_VIEW(ctx.depth_attachment[i].view)->desc.format);
    }
  }

  for (u32 i = 0; i < pipeline->num_stencils; i++)
  {
    if (pipeline->stencil[i] != gpu::Format::Undefined)
    {
      CHECK(ctx.stencil_attachment[i].view != nullptr);
      CHECK(pipeline->stencil[i] ==
            IMAGE_FROM_VIEW(ctx.stencil_attachment[i].view)->desc.format);
    }
  }
}

inline bool is_image_view_type_compatible(gpu::ImageType     image_type,
                                          gpu::ImageViewType view_type)
{
  switch (view_type)
  {
    case gpu::ImageViewType::Type1D:
    case gpu::ImageViewType::Type1DArray:
      return image_type == gpu::ImageType::Type1D;
    case gpu::ImageViewType::Type2D:
    case gpu::ImageViewType::Type2DArray:
      return image_type == gpu::ImageType::Type2D ||
             image_type == gpu::ImageType::Type3D;
    case gpu::ImageViewType::TypeCube:
    case gpu::ImageViewType::TypeCubeArray:
      return image_type == gpu::ImageType::Type2D;
    case gpu::ImageViewType::Type3D:
      return image_type == gpu::ImageType::Type3D;
    default:
      return false;
  }
}

inline u64 index_type_size(gpu::IndexType type)
{
  switch (type)
  {
    case gpu::IndexType::Uint16:
      return 2;
    case gpu::IndexType::Uint32:
      return 4;
    default:
      UNREACHABLE();
  }
}

inline bool is_valid_buffer_access(u64 size, u64 access_offset, u64 access_size,
                                   u64 offset_alignment = 1)
{
  access_size =
      (access_size == gpu::WHOLE_SIZE) ? (size - access_offset) : access_size;
  return (access_size > 0) && (access_offset < size) &&
         ((access_offset + access_size) <= size) &&
         mem::is_aligned(offset_alignment, access_offset);
}

inline bool is_valid_image_access(gpu::ImageAspects aspects, u32 num_levels,
                                  u32               num_layers,
                                  gpu::ImageAspects access_aspects,
                                  u32 access_level, u32 num_access_levels,
                                  u32 access_layer, u32 num_access_layers)
{
  num_access_levels = num_access_levels == gpu::REMAINING_MIP_LEVELS ?
                          (num_levels - access_level) :
                          num_access_levels;
  num_access_layers = num_access_layers == gpu::REMAINING_ARRAY_LAYERS ?
                          (num_access_layers - access_layer) :
                          num_access_layers;
  return num_access_levels > 0 && num_access_layers > 0 &&
         access_level < num_levels && access_layer < num_layers &&
         (access_level + num_access_levels) <= num_levels &&
         (access_layer + num_access_layers) <= num_layers &&
         has_bits(aspects, access_aspects) &&
         access_aspects != gpu::ImageAspects::None;
}

Result<gpu::InstanceImpl, Status> create_instance(AllocatorImpl allocator,
                                                  bool enable_validation)
{
  u32      num_exts;
  VkResult result =
      vkEnumerateInstanceExtensionProperties(nullptr, &num_exts, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkExtensionProperties *exts;
  if (!allocator.nalloc(num_exts, &exts))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer exts_{[&] { allocator.ndealloc(exts, num_exts); }};

  {
    u32 num_read_exts = num_exts;
    result =
        vkEnumerateInstanceExtensionProperties(nullptr, &num_read_exts, exts);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    CHECK(num_read_exts == num_exts);
  }

  u32 num_layers;
  result = vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkLayerProperties *layers;

  if (!allocator.nalloc(num_layers, &layers))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer layers_{[&] { allocator.ndealloc(layers, num_layers); }};

  {
    u32 num_read_layers = num_layers;
    result = vkEnumerateInstanceLayerProperties(&num_read_layers, layers);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    CHECK(num_read_layers == num_layers);
  }

  logger->trace("Available Extensions:");

  for (VkExtensionProperties const &ext : Span{exts, num_exts})
  {
    logger->trace(ext.extensionName, "\t\t(spec version ",
                  VK_API_VERSION_MAJOR(ext.specVersion), ".",
                  VK_API_VERSION_MINOR(ext.specVersion), ".",
                  VK_API_VERSION_PATCH(ext.specVersion), " variant ",
                  VK_API_VERSION_VARIANT(ext.specVersion), ")");
  }

  logger->trace("Available Layers:");

  for (VkLayerProperties const &layer : Span{layers, num_layers})
  {
    logger->trace(layer.layerName, "\t\t(spec version ",
                  VK_API_VERSION_MAJOR(layer.specVersion), ".",
                  VK_API_VERSION_MINOR(layer.specVersion), ".",
                  VK_API_VERSION_PATCH(layer.specVersion), " variant ",
                  VK_API_VERSION_VARIANT(layer.specVersion),
                  ", implementation version: ", layer.implementationVersion,
                  ")");
  }

  char const *load_exts[16];
  u32         num_load_exts = 0;

  constexpr char const            *OPTIONAL_EXTS[]  = {"VK_KHR_surface",
                                                       "VK_KHR_android_surface",
                                                       "VK_MVK_ios_surface",
                                                       "VK_MVK_macos_surface",
                                                       "VK_EXT_metal_surface",
                                                       "VK_NN_vi_surface",
                                                       "VK_KHR_wayland_surface",
                                                       "VK_KHR_win32_surface",
                                                       "VK_KHR_xcb_surface",
                                                       "VK_KHR_xlib_surface",
                                                       "VK_KHR_portability_enumeration"};
  Bits<u64, sizeof(OPTIONAL_EXTS)> has_optional_ext = {};

  bool has_debug_utils_ext = false;

  for (u32 i = 0; i < num_exts; i++)
  {
    for (u32 iopt = 0; iopt < size(OPTIONAL_EXTS); iopt++)
    {
      if (strcmp(OPTIONAL_EXTS[iopt], exts[i].extensionName) == 0)
      {
        load_exts[num_load_exts++] = OPTIONAL_EXTS[iopt];
        set_bit(span(has_optional_ext), iopt);
      }
    }

    if (strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, exts[i].extensionName) == 0)
    {
      has_debug_utils_ext = true;
    }
  }

  if (enable_validation)
  {
    if (has_debug_utils_ext)
    {
      load_exts[num_load_exts++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    else
    {
      logger->warn("Required Vulkan "
                   "Extension: " VK_EXT_DEBUG_UTILS_EXTENSION_NAME
                   " is not supported on device");
    }
  }

  char const *load_layers[16];
  u32         num_load_layers      = 0;
  bool        has_validation_layer = false;

  for (u32 i = 0; i < num_layers; i++)
  {
    if (strcmp("VK_LAYER_KHRONOS_validation", layers[i].layerName) == 0)
    {
      has_validation_layer = true;
    }
  }

  if (enable_validation)
  {
    if (has_validation_layer)
    {
      load_layers[num_load_layers++] = "VK_LAYER_KHRONOS_validation";
    }
    else
    {
      logger->warn("Required Layer: VK_LAYER_KHRONOS_validation is "
                   "not supported");
    }
  }

  bool const validation_enabled =
      enable_validation && has_debug_utils_ext && has_validation_layer;

  // setup before vkInstance to allow debug reporter report
  // messages through the pointer to it
  Instance *instance;
  if (!allocator.nalloc(1, &instance))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer instance_{[&] {
    if (instance != nullptr)
    {
      allocator.ndealloc(instance, 1);
    }
  }};

  new (instance) Instance{.allocator          = allocator,
                          .vk_table           = {},
                          .vk_instance        = nullptr,
                          .vk_debug_messenger = nullptr,
                          .validation_enabled = validation_enabled};

  VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pNext = nullptr,
                             .pApplicationName   = CLIENT_NAME,
                             .applicationVersion = CLIENT_VERSION,
                             .pEngineName        = ENGINE_NAME,
                             .engineVersion      = ENGINE_VERSION,
                             .apiVersion         = VK_API_VERSION_1_1};

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debug_callback,
      .pUserData       = instance};

  // .pNext helps to debug issues with vkDestroyInstance and vkCreateInstance
  // i.e. (before and after the debug messenger is installed)
  VkInstanceCreateInfo create_info{
      .sType                 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext                 = enable_validation ? &debug_create_info : nullptr,
      .flags                 = get_bit(span(has_optional_ext), 10) ?
                                   VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR :
                                   ((VkInstanceCreateFlags) 0),
      .pApplicationInfo      = &app_info,
      .enabledLayerCount     = num_load_layers,
      .ppEnabledLayerNames   = load_layers,
      .enabledExtensionCount = num_load_exts,
      .ppEnabledExtensionNames = load_exts};

  VkInstance vk_instance;

  result = vkCreateInstance(&create_info, nullptr, &vk_instance);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_instance_{[&] {
    if (vk_instance != nullptr)
    {
      vkDestroyInstance(vk_instance, nullptr);
    }
  }};

  InstanceTable vk_table;

  CHECK(load_instance_table(vk_instance, vkGetInstanceProcAddr, vk_table,
                            validation_enabled));

  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;

  if (validation_enabled)
  {
    result = vk_table.CreateDebugUtilsMessengerEXT(
        vk_instance, &debug_create_info, nullptr, &vk_debug_messenger);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
  }

  instance->vk_table           = vk_table;
  instance->vk_instance        = vk_instance;
  instance->vk_debug_messenger = vk_debug_messenger;

  vk_instance     = nullptr;
  Instance *iself = instance;
  instance        = nullptr;

  return Ok{gpu::InstanceImpl{.self      = (gpu::Instance) iself,
                              .interface = &instance_interface}};
}
}        // namespace vk

namespace gpu
{

Result<InstanceImpl, Status> create_vulkan_instance(AllocatorImpl allocator,
                                                    bool enable_validation)
{
  return vk::create_instance(allocator, enable_validation);
}

}        // namespace gpu

namespace vk
{

void InstanceInterface::uninit(gpu::Instance instance_)
{
  Instance *const instance = (Instance *) instance_;

  if (instance == nullptr)
  {
    return;
  }
  if (instance->validation_enabled)
  {
    instance->vk_table.DestroyDebugUtilsMessengerEXT(
        instance->vk_instance, instance->vk_debug_messenger, nullptr);
  }
  instance->vk_table.DestroyInstance(instance->vk_instance, nullptr);
  instance->allocator.ndealloc(instance, 1);
}

void check_device_limits(VkPhysicalDeviceLimits limits)
{
  CHECK(limits.maxImageDimension1D >= gpu::MAX_IMAGE_EXTENT_1D);
  CHECK(limits.maxImageDimension2D >= gpu::MAX_IMAGE_EXTENT_2D);
  CHECK(limits.maxImageDimension3D >= gpu::MAX_IMAGE_EXTENT_3D);
  CHECK(limits.maxImageDimensionCube >= gpu::MAX_IMAGE_EXTENT_CUBE);
  CHECK(limits.maxImageArrayLayers >= gpu::MAX_IMAGE_ARRAY_LAYERS);
  CHECK(limits.maxViewportDimensions[0] >= gpu::MAX_VIEWPORT_EXTENT);
  CHECK(limits.maxViewportDimensions[1] >= gpu::MAX_VIEWPORT_EXTENT);
  CHECK(limits.maxFramebufferWidth >= gpu::MAX_FRAMEBUFFER_EXTENT);
  CHECK(limits.maxFramebufferHeight >= gpu::MAX_FRAMEBUFFER_EXTENT);
  CHECK(limits.maxFramebufferLayers >= gpu::MAX_FRAMEBUFFER_LAYERS);
  CHECK(limits.maxVertexInputAttributes >= gpu::MAX_VERTEX_ATTRIBUTES);
  CHECK(limits.maxVertexInputBindings >= gpu::MAX_VERTEX_ATTRIBUTES);
  CHECK(limits.maxPushConstantsSize >= gpu::MAX_PUSH_CONSTANTS_SIZE);
  CHECK(limits.maxBoundDescriptorSets >= gpu::MAX_PIPELINE_DESCRIPTOR_SETS);
  CHECK(limits.maxPerStageDescriptorInputAttachments >=
        gpu::MAX_PIPELINE_INPUT_ATTACHMENTS);
  CHECK(limits.maxUniformBufferRange >= gpu::MAX_UNIFORM_BUFFER_RANGE);
  CHECK(limits.maxColorAttachments >= gpu::MAX_PIPELINE_COLOR_ATTACHMENTS);
  CHECK(limits.maxSamplerAnisotropy >= gpu::MAX_SAMPLER_ANISOTROPY);
}

void check_device_features(VkPhysicalDeviceFeatures feat)
{
  CHECK(feat.samplerAnisotropy == VK_TRUE);
  CHECK(feat.shaderUniformBufferArrayDynamicIndexing == VK_TRUE);
  CHECK(feat.shaderSampledImageArrayDynamicIndexing == VK_TRUE);
  CHECK(feat.shaderStorageBufferArrayDynamicIndexing == VK_TRUE);
  CHECK(feat.shaderStorageImageArrayDynamicIndexing == VK_TRUE);
  CHECK(feat.multiDrawIndirect == VK_TRUE);
  CHECK(feat.drawIndirectFirstInstance == VK_TRUE);
  CHECK(feat.imageCubeArray == VK_TRUE);
}

void set_resource_name(Device *dev, Span<char const> label,
                       void const *resource, VkObjectType type,
                       VkDebugReportObjectTypeEXT debug_type)
{
  char buff[256];
  to_c_str(label, span(buff));
  VkDebugUtilsObjectNameInfoEXT name_info{
      .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext        = nullptr,
      .objectType   = type,
      .objectHandle = (u64) resource,
      .pObjectName  = buff};
  dev->instance->vk_table.SetDebugUtilsObjectNameEXT(dev->vk_dev, &name_info);
  VkDebugMarkerObjectNameInfoEXT debug_info{
      .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      .pNext       = nullptr,
      .objectType  = debug_type,
      .object      = (u64) resource,
      .pObjectName = buff};
  dev->vk_table.DebugMarkerSetObjectNameEXT(dev->vk_dev, &debug_info);
}

Status create_command_encoder(Device *dev, CommandEncoder *enc)
{
  VkCommandPoolCreateInfo command_pool_create_info{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = dev->queue_family};

  VkCommandPool vk_command_pool;
  VkResult      result = dev->vk_table.CreateCommandPool(
      dev->vk_dev, &command_pool_create_info, nullptr, &vk_command_pool);

  if (result != VK_SUCCESS)
  {
    return (Status) result;
  }

  VkCommandBufferAllocateInfo allocate_info{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext              = nullptr,
      .commandPool        = vk_command_pool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  VkCommandBuffer vk_command_buffer;
  result = dev->vk_table.AllocateCommandBuffers(dev->vk_dev, &allocate_info,
                                                &vk_command_buffer);

  if (result != VK_SUCCESS)
  {
    dev->vk_table.DestroyCommandPool(dev->vk_dev, vk_command_pool, nullptr);
    return (Status) result;
  }

  set_resource_name(dev, "Frame Command Buffer"_span, vk_command_buffer,
                    VK_OBJECT_TYPE_COMMAND_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT);

  new (enc) CommandEncoder{
      .allocator         = dev->allocator,
      .dev               = dev,
      .arg_pool          = ArenaPool{dev->allocator},
      .vk_command_pool   = vk_command_pool,
      .vk_command_buffer = vk_command_buffer,
      .status            = Status::Success,
      .state             = CommandEncoderState::Reset,
      .render_ctx = RenderPassContext{.command_pool = ArenaPool{dev->allocator},
                                      .arg_pool = ArenaPool{dev->allocator}},
      .compute_ctx = {}};

  enc->render_ctx.commands =
      Vec<Command>{enc->render_ctx.command_pool.to_allocator()};

  return Status::Success;
}

void uninit_command_encoder(Device *dev, CommandEncoder *enc)
{
  enc->render_ctx.commands.reset();
  dev->vk_table.DestroyCommandPool(dev->vk_dev, enc->vk_command_pool, nullptr);
}

Status create_frame_context(Device *dev, u32 buffering)
{
  FrameContext &ctx = dev->frame_ctx;
  ctx.tail_frame    = 0;
  ctx.current_frame = 0;
  ctx.ring_index    = 0;
  ctx.tail_frame    = 0;
  u32 num_encs      = 0;
  u32 num_acquire_s = 0;
  u32 num_submit_f  = 0;
  u32 num_submit_s  = 0;

  defer encs_{[&] {
    for (u32 i = num_encs; i-- > 0;)
    {
      uninit_command_encoder(dev, ctx.encs + i);
    }
  }};

  defer acquire_s_{[&] {
    for (u32 i = num_acquire_s; i-- > 0;)
    {
      dev->vk_table.DestroySemaphore(dev->vk_dev, ctx.acquire_s[i], nullptr);
    }
  }};

  defer submit_f_{[&] {
    for (u32 i = num_submit_f; i-- > 0;)
    {
      dev->vk_table.DestroyFence(dev->vk_dev, ctx.submit_f[i], nullptr);
    }
  }};

  defer submit_s_{[&] {
    for (u32 i = num_submit_s; i-- > 0;)
    {
      dev->vk_table.DestroySemaphore(dev->vk_dev, ctx.submit_s[i], nullptr);
    }
  }};

  for (; num_encs < buffering; num_encs++)
  {
    Status status = create_command_encoder(dev, ctx.encs + num_encs);
    if (status != Status::Success)
    {
      return status;
    }
  }

  VkSemaphoreCreateInfo sem_info{.sType =
                                     VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0};

  for (; num_acquire_s < buffering; num_acquire_s++)
  {
    VkResult result = dev->vk_table.CreateSemaphore(
        dev->vk_dev, &sem_info, nullptr, ctx.acquire_s + num_acquire_s);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }
    set_resource_name(dev, "Frame Acquire Semaphore"_span,
                      ctx.acquire_s[num_acquire_s], VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
  }

  VkFenceCreateInfo fence_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                               .pNext = nullptr,
                               .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  for (; num_submit_f < buffering; num_submit_f++)
  {
    VkResult result = dev->vk_table.CreateFence(
        dev->vk_dev, &fence_info, nullptr, ctx.submit_f + num_submit_f);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }

    set_resource_name(dev, "Frame Submit Fence"_span,
                      ctx.submit_f[num_submit_f], VK_OBJECT_TYPE_FENCE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);
  }

  for (; num_submit_s < buffering; num_submit_s++)
  {
    VkResult result = dev->vk_table.CreateSemaphore(
        dev->vk_dev, &sem_info, nullptr, ctx.submit_s + num_submit_s);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }
    set_resource_name(dev, "Frame Submit Semaphore"_span,
                      ctx.submit_s[num_submit_s], VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
  }

  num_encs      = 0;
  num_acquire_s = 0;
  num_submit_f  = 0;
  num_submit_s  = 0;

  // self-referential
  for (u32 i = 0; i < buffering; i++)
  {
    ctx.encs_impl[i] =
        gpu::CommandEncoderImpl{.self = (gpu::CommandEncoder)(ctx.encs + i),
                                .interface = &command_encoder_interface};
  }

  ctx.buffering = buffering;

  return Status::Success;
}

void uninit_frame_context(Device *dev)
{
  FrameContext &ctx = dev->frame_ctx;
  for (u32 i = ctx.buffering; i-- > 0;)
  {
    uninit_command_encoder(dev, ctx.encs + i);
  }
  for (u32 i = ctx.buffering; i-- > 0;)
  {
    dev->vk_table.DestroySemaphore(dev->vk_dev, ctx.acquire_s[i], nullptr);
  }
  for (u32 i = ctx.buffering; i-- > 0;)
  {
    dev->vk_table.DestroyFence(dev->vk_dev, ctx.submit_f[i], nullptr);
  }
  for (u32 i = ctx.buffering; i-- > 0;)
  {
    dev->vk_table.DestroySemaphore(dev->vk_dev, ctx.submit_s[i], nullptr);
  }
}

void uninit_descriptor_heap(Device *self, DescriptorHeap *heap)
{
  for (u32 i = heap->num_pools; i-- > 0;)
  {
    self->vk_table.DestroyDescriptorPool(self->vk_dev, heap->pools[i].vk_pool,
                                         nullptr);
  }
  heap->allocator.ndealloc(heap->pools, heap->num_pools);
  heap->allocator.dealloc(MAX_STANDARD_ALIGNMENT, heap->scratch,
                          heap->scratch_size);
}

Result<gpu::DeviceImpl, Status> InstanceInterface::create_device(
    gpu::Instance self_, AllocatorImpl allocator,
    Span<gpu::DeviceType const> preferred_types,
    Span<gpu::Surface const> compatible_surfaces, u32 buffering)
{
  Instance *const self               = (Instance *) self_;
  u32 const       num_surfaces       = compatible_surfaces.size32();
  constexpr u32   MAX_QUEUE_FAMILIES = 16;

  CHECK(buffering > 0);
  CHECK(buffering <= gpu::MAX_FRAME_BUFFERING);

  u32      num_devs;
  VkResult result = self->vk_table.EnumeratePhysicalDevices(self->vk_instance,
                                                            &num_devs, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  if (num_devs == 0)
  {
    return Err{Status::DeviceLost};
  }

  VkPhysicalDevice *vk_phy_devs;

  if (!self->allocator.nalloc(num_devs, &vk_phy_devs))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_phy_devs_{[&] { self->allocator.ndealloc(vk_phy_devs, num_devs); }};

  {
    u32 num_read_devs = num_devs;
    result            = self->vk_table.EnumeratePhysicalDevices(
        self->vk_instance, &num_read_devs, vk_phy_devs);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    CHECK(num_read_devs == num_devs);
  }

  PhysicalDevice *physical_devs;
  if (!self->allocator.nalloc(num_devs, &physical_devs))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer physical_devs_{
      [&] { self->allocator.ndealloc(physical_devs, num_devs); }};

  for (u32 i = 0; i < num_devs; i++)
  {
    PhysicalDevice  &dev    = physical_devs[i];
    VkPhysicalDevice vk_dev = vk_phy_devs[i];
    dev.vk_phy_dev          = vk_dev;
    self->vk_table.GetPhysicalDeviceFeatures(vk_dev, &dev.vk_features);
    self->vk_table.GetPhysicalDeviceMemoryProperties(vk_dev,
                                                     &dev.vk_memory_properties);
    self->vk_table.GetPhysicalDeviceProperties(vk_dev, &dev.vk_properties);
  }

  logger->trace("Available Devices:");
  for (u32 i = 0; i < num_devs; i++)
  {
    PhysicalDevice const             &dev        = physical_devs[i];
    VkPhysicalDeviceProperties const &properties = dev.vk_properties;
    logger->trace("[Device: ", i, "] ",
                  string_VkPhysicalDeviceType(properties.deviceType), " ",
                  properties.deviceName, " Vulkan API version ",
                  VK_API_VERSION_MAJOR(properties.apiVersion), ".",
                  VK_API_VERSION_MINOR(properties.apiVersion), ".",
                  VK_API_VERSION_PATCH(properties.apiVersion), " Variant ",
                  VK_API_VERSION_VARIANT(properties.apiVersion),
                  ", Driver "
                  "Version: ",
                  properties.driverVersion,
                  ", "
                  "Vendor ID: ",
                  properties.vendorID, ", Device ID: ", properties.deviceID);

    u32 num_queue_families;
    self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
        dev.vk_phy_dev, &num_queue_families, nullptr);

    CHECK(num_queue_families <= MAX_QUEUE_FAMILIES);

    VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

    {
      u32 num_read_queue_families = num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          dev.vk_phy_dev, &num_queue_families, queue_family_properties);
      CHECK(num_read_queue_families == num_queue_families);
    }

    for (u32 i = 0; i < num_queue_families; i++)
    {
      logger->trace("\t\tQueue Family: ", i,
                    ", Count: ", queue_family_properties[i].queueCount,
                    ", Flags: ",
                    string_VkQueueFlags(queue_family_properties[i].queueFlags));
    }
  }

  u32 selected_dev_idx      = num_devs;
  u32 selected_queue_family = VK_QUEUE_FAMILY_IGNORED;

  for (usize i = 0; i < preferred_types.size32(); i++)
  {
    for (u32 idev = 0; idev < num_devs && selected_dev_idx == num_devs; idev++)
    {
      PhysicalDevice const &dev = physical_devs[idev];

      u32 num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          dev.vk_phy_dev, &num_queue_families, nullptr);

      CHECK(num_queue_families <= MAX_QUEUE_FAMILIES);

      VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

      {
        u32 num_read_queue_families = num_queue_families;
        self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
            dev.vk_phy_dev, &num_queue_families, queue_family_properties);
        CHECK(num_read_queue_families == num_queue_families);
      }

      if (((VkPhysicalDeviceType) preferred_types[i]) ==
          dev.vk_properties.deviceType)
      {
        for (u32 iqueue_family = 0;
             iqueue_family < num_queue_families && selected_dev_idx == num_devs;
             iqueue_family++)
        {
          if (has_bits(queue_family_properties[iqueue_family].queueFlags,
                       (VkQueueFlags) (VK_QUEUE_COMPUTE_BIT |
                                       VK_QUEUE_GRAPHICS_BIT |
                                       VK_QUEUE_TRANSFER_BIT)))
          {
            u32 num_supported_surfaces = 0;
            for (u32 isurface = 0; isurface < num_surfaces; isurface++)
            {
              VkBool32 supported;
              self->vk_table.GetPhysicalDeviceSurfaceSupportKHR(
                  dev.vk_phy_dev, iqueue_family,
                  (Surface) compatible_surfaces[isurface], &supported);
              if (supported == VK_TRUE)
              {
                num_supported_surfaces++;
              }
            }

            if (num_supported_surfaces == num_surfaces)
            {
              selected_dev_idx      = idev;
              selected_queue_family = iqueue_family;
              break;
            }
          }
        }
      }
    }
  }

  if (selected_dev_idx == num_devs)
  {
    logger->trace("No Suitable Device Found");
    return Err{Status::DeviceLost};
  }

  PhysicalDevice selected_dev = physical_devs[selected_dev_idx];

  check_device_limits(selected_dev.vk_properties.limits);
  check_device_features(selected_dev.vk_features);

  logger->trace("Selected Device ", selected_dev_idx);

  u32 num_exts;
  result = self->vk_table.EnumerateDeviceExtensionProperties(
      selected_dev.vk_phy_dev, nullptr, &num_exts, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkExtensionProperties *exts;

  if (!self->allocator.nalloc(num_exts, &exts))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer exts_{[&] { self->allocator.ndealloc(exts, num_exts); }};

  {
    u32 num_read_exts = num_exts;
    result            = self->vk_table.EnumerateDeviceExtensionProperties(
        selected_dev.vk_phy_dev, nullptr, &num_read_exts, exts);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
    CHECK(num_exts == num_read_exts);
  }

  u32 num_layers;
  result = self->vk_table.EnumerateDeviceLayerProperties(
      selected_dev.vk_phy_dev, &num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkLayerProperties *layers;

  if (!self->allocator.nalloc(num_layers, &layers))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer layers_{[&] { self->allocator.ndealloc(layers, num_layers); }};

  {
    u32 num_read_layers = num_layers;
    result              = self->vk_table.EnumerateDeviceLayerProperties(
        selected_dev.vk_phy_dev, &num_read_layers, layers);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
    CHECK(num_read_layers == num_layers);
  }

  logger->trace("Available Extensions:");

  for (u32 i = 0; i < num_exts; i++)
  {
    VkExtensionProperties const &ext = exts[i];
    logger->trace("\t\t", ext.extensionName,
                  " (spec version: ", VK_API_VERSION_MAJOR(ext.specVersion),
                  ".", VK_API_VERSION_MINOR(ext.specVersion), ".",
                  VK_API_VERSION_PATCH(ext.specVersion), " variant ",
                  VK_API_VERSION_VARIANT(ext.specVersion), ")");
  }

  logger->trace("Available Layers:");

  for (u32 i = 0; i < num_layers; i++)
  {
    VkLayerProperties const &layer = layers[i];

    logger->trace("\t\t", layer.layerName,
                  " (spec version: ", VK_API_VERSION_MAJOR(layer.specVersion),
                  ".", VK_API_VERSION_MINOR(layer.specVersion), ".",
                  VK_API_VERSION_PATCH(layer.specVersion), " variant ",
                  VK_API_VERSION_VARIANT(layer.specVersion),
                  ", "
                  "implementation version: ",
                  layer.implementationVersion, ")");
  }

  constexpr char const *required_exts[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
      VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
      VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
      VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
      VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
      VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME};
  bool required_ext_found[size(required_exts)] = {};
  bool has_debug_marker_ext                    = false;
  bool has_portability_ext                     = false;

  for (u32 i = 0; i < num_exts; i++)
  {
    for (u32 ireq = 0; ireq < size(required_exts); ireq++)
    {
      if (strcmp(required_exts[ireq], exts[i].extensionName) == 0)
      {
        required_ext_found[ireq] = true;
      }
    }
    if (strcmp(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, exts[i].extensionName) == 0)
    {
      has_debug_marker_ext = true;
    }
    else if (strcmp("VK_KHR_portability_subset", exts[i].extensionName) == 0)
    {
      has_portability_ext = true;
    }
  }

  char const *load_exts[16];
  u32         num_load_exts = 0;

  // optional, stubbed
  if (has_debug_marker_ext)
  {
    load_exts[num_load_exts] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
    num_load_exts++;
  }

  if (has_portability_ext)
  {
    load_exts[num_load_exts++] = "VK_KHR_portability_subset";
  }

  // required
  for (u32 i = 0; i < size(required_exts); i++)
  {
    if (!required_ext_found[i])
    {
      logger->trace("Required Extension: ", required_exts[i], " Not Present");
      return Err{Status::ExtensionNotPresent};
    }

    load_exts[num_load_exts] = required_exts[i];
    num_load_exts++;
  }

  bool has_validation_layer = false;

  for (u32 i = 0; i < num_layers; i++)
  {
    if (strcmp(layers[i].layerName, "VK_LAYER_KHRONOS_validation") == 0)
    {
      has_validation_layer = true;
      break;
    }
  }

  char const *load_layers[8];
  u32         num_load_layers = 0;

  // optional
  if (self->vk_debug_messenger != nullptr && has_validation_layer)
  {
    load_layers[num_load_layers++] = "VK_LAYER_KHRONOS_validation";
  }

  f32 const queue_priority = 1.0F;

  VkDeviceQueueCreateInfo queue_create_info{
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = 0,
      .queueFamilyIndex = selected_queue_family,
      .queueCount       = 1,
      .pQueuePriorities = &queue_priority};

  VkPhysicalDeviceFeatures features{
      .robustBufferAccess         = VK_FALSE,
      .fullDrawIndexUint32        = VK_FALSE,
      .imageCubeArray             = VK_TRUE,
      .independentBlend           = VK_FALSE,
      .geometryShader             = VK_FALSE,
      .tessellationShader         = VK_FALSE,
      .sampleRateShading          = VK_FALSE,
      .dualSrcBlend               = VK_FALSE,
      .logicOp                    = VK_FALSE,
      .multiDrawIndirect          = VK_TRUE,
      .drawIndirectFirstInstance  = VK_TRUE,
      .depthClamp                 = VK_FALSE,
      .depthBiasClamp             = VK_FALSE,
      .fillModeNonSolid           = selected_dev.vk_features.fillModeNonSolid,
      .depthBounds                = VK_FALSE,
      .wideLines                  = VK_FALSE,
      .largePoints                = VK_FALSE,
      .alphaToOne                 = VK_FALSE,
      .multiViewport              = VK_FALSE,
      .samplerAnisotropy          = VK_TRUE,
      .textureCompressionETC2     = VK_FALSE,
      .textureCompressionASTC_LDR = VK_FALSE,
      .textureCompressionBC       = VK_FALSE,
      .occlusionQueryPrecise      = VK_FALSE,
      .pipelineStatisticsQuery    = VK_FALSE,
      .vertexPipelineStoresAndAtomics          = VK_FALSE,
      .fragmentStoresAndAtomics                = VK_FALSE,
      .shaderTessellationAndGeometryPointSize  = VK_FALSE,
      .shaderImageGatherExtended               = VK_FALSE,
      .shaderStorageImageExtendedFormats       = VK_FALSE,
      .shaderStorageImageMultisample           = VK_FALSE,
      .shaderStorageImageReadWithoutFormat     = VK_FALSE,
      .shaderStorageImageWriteWithoutFormat    = VK_FALSE,
      .shaderUniformBufferArrayDynamicIndexing = VK_TRUE,
      .shaderSampledImageArrayDynamicIndexing  = VK_TRUE,
      .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
      .shaderStorageImageArrayDynamicIndexing  = VK_TRUE,
      .shaderClipDistance       = selected_dev.vk_features.shaderClipDistance,
      .shaderCullDistance       = selected_dev.vk_features.shaderCullDistance,
      .shaderFloat64            = selected_dev.vk_features.shaderFloat64,
      .shaderInt64              = selected_dev.vk_features.shaderInt64,
      .shaderInt16              = selected_dev.vk_features.shaderInt16,
      .shaderResourceResidency  = VK_FALSE,
      .shaderResourceMinLod     = VK_FALSE,
      .sparseBinding            = VK_FALSE,
      .sparseResidencyBuffer    = VK_FALSE,
      .sparseResidencyImage2D   = VK_FALSE,
      .sparseResidencyImage3D   = VK_FALSE,
      .sparseResidency2Samples  = VK_FALSE,
      .sparseResidency4Samples  = VK_FALSE,
      .sparseResidency8Samples  = VK_FALSE,
      .sparseResidency16Samples = VK_FALSE,
      .sparseResidencyAliased   = VK_FALSE,
      .variableMultisampleRate  = VK_FALSE,
      .inheritedQueries         = VK_FALSE};

  VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures
      separate_depth_stencil_layout_feature{
          .sType =
              VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR,
          .pNext                       = nullptr,
          .separateDepthStencilLayouts = VK_TRUE};

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state_features{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .pNext                = &separate_depth_stencil_layout_feature,
      .extendedDynamicState = VK_TRUE};

  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
      .pNext = &extended_dynamic_state_features,
      .dynamicRendering = VK_TRUE};

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
      .pNext                                     = &dynamic_rendering_features,
      .shaderInputAttachmentArrayDynamicIndexing = VK_TRUE,
      .shaderUniformTexelBufferArrayDynamicIndexing       = VK_TRUE,
      .shaderStorageTexelBufferArrayDynamicIndexing       = VK_TRUE,
      .shaderUniformBufferArrayNonUniformIndexing         = VK_TRUE,
      .shaderSampledImageArrayNonUniformIndexing          = VK_TRUE,
      .shaderStorageBufferArrayNonUniformIndexing         = VK_TRUE,
      .shaderStorageImageArrayNonUniformIndexing          = VK_TRUE,
      .shaderInputAttachmentArrayNonUniformIndexing       = VK_TRUE,
      .shaderUniformTexelBufferArrayNonUniformIndexing    = VK_TRUE,
      .shaderStorageTexelBufferArrayNonUniformIndexing    = VK_TRUE,
      .descriptorBindingUniformBufferUpdateAfterBind      = VK_TRUE,
      .descriptorBindingSampledImageUpdateAfterBind       = VK_TRUE,
      .descriptorBindingStorageImageUpdateAfterBind       = VK_TRUE,
      .descriptorBindingStorageBufferUpdateAfterBind      = VK_TRUE,
      .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
      .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
      .descriptorBindingUpdateUnusedWhilePending          = VK_TRUE,
      .descriptorBindingPartiallyBound                    = VK_TRUE,
      .descriptorBindingVariableDescriptorCount           = VK_TRUE,
      .runtimeDescriptorArray                             = VK_TRUE};

  VkDeviceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                 .pNext = &descriptor_indexing_features,
                                 .flags = 0,
                                 .queueCreateInfoCount    = 1,
                                 .pQueueCreateInfos       = &queue_create_info,
                                 .enabledLayerCount       = num_load_layers,
                                 .ppEnabledLayerNames     = load_layers,
                                 .enabledExtensionCount   = num_load_exts,
                                 .ppEnabledExtensionNames = load_exts,
                                 .pEnabledFeatures        = &features};

  VkDevice vk_dev;
  result = self->vk_table.CreateDevice(selected_dev.vk_phy_dev, &create_info,
                                       nullptr, &vk_dev);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  DeviceTable        vk_table;
  VmaVulkanFunctions vma_table;
  CHECK(load_device_table(vk_dev, self->vk_table, vk_table,
                          has_debug_marker_ext));

  load_vma_table(self->vk_table, vk_table, vma_table);

  defer vk_dev_{[&] {
    if (vk_dev != nullptr)
    {
      vk_table.DestroyDevice(vk_dev, nullptr);
    }
  }};

  VkQueue vk_queue;
  vk_table.GetDeviceQueue(vk_dev, selected_queue_family, 0, &vk_queue);

  VmaAllocatorCreateInfo vma_create_info{
      .flags          = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
      .physicalDevice = selected_dev.vk_phy_dev,
      .device         = vk_dev,
      .preferredLargeHeapBlockSize    = 0,
      .pAllocationCallbacks           = nullptr,
      .pDeviceMemoryCallbacks         = nullptr,
      .pHeapSizeLimit                 = nullptr,
      .pVulkanFunctions               = &vma_table,
      .instance                       = self->vk_instance,
      .vulkanApiVersion               = VK_API_VERSION_1_0,
      .pTypeExternalMemoryHandleTypes = nullptr};

  VmaAllocator vma_allocator;
  result = vmaCreateAllocator(&vma_create_info, &vma_allocator);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vma_allocator_{[&] {
    if (vma_allocator != nullptr)
    {
      vmaDestroyAllocator(vma_allocator);
    }
  }};

  Device *dev;

  if (!self->allocator.nalloc(1, &dev))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (dev) Device{.allocator     = allocator,
                   .instance      = self,
                   .phy_dev       = selected_dev,
                   .vk_table      = vk_table,
                   .vma_table     = vma_table,
                   .vk_dev        = vk_dev,
                   .queue_family  = selected_queue_family,
                   .vk_queue      = vk_queue,
                   .vma_allocator = vma_allocator,
                   .frame_ctx     = FrameContext{.buffering = 0},
                   .descriptor_heap =
                       DescriptorHeap{.allocator = allocator,
                                      .pools     = nullptr,
                                      .pool_size = gpu::MAX_BINDING_DESCRIPTORS,
                                      .scratch   = nullptr,
                                      .num_pools = 0,
                                      .scratch_size = 0}};

  defer dev_{[&] {
    if (dev != nullptr)
    {
      uninit_device((gpu::Instance) self, (gpu::Device) dev);
    }
  }};

  Status status = create_frame_context(dev, buffering);

  if (status != Status::Success)
  {
    return Err{status};
  }

  Device *out   = dev;
  vma_allocator = nullptr;
  vk_dev        = nullptr;
  dev           = nullptr;

  return Ok{gpu::DeviceImpl{.self      = (gpu::Device) out,
                            .interface = &device_interface}};
}

gpu::Backend InstanceInterface::get_backend(gpu::Instance)
{
  return gpu::Backend::Vulkan;
}

void InstanceInterface::uninit_device(gpu::Instance instance_,
                                      gpu::Device   device_)
{
  Instance *const instance = (Instance *) instance_;
  Device *const   dev      = (Device *) device_;

  if (dev == nullptr)
  {
    return;
  }

  uninit_frame_context(dev);
  uninit_descriptor_heap(dev, &dev->descriptor_heap);
  vmaDestroyAllocator(dev->vma_allocator);
  dev->vk_table.DestroyDevice(dev->vk_dev, nullptr);
  instance->allocator.ndealloc(dev, 1);
}

void InstanceInterface::uninit_surface(gpu::Instance self_,
                                       gpu::Surface  surface)
{
  Instance *const self = (Instance *) self_;
  self->vk_table.DestroySurfaceKHR(self->vk_instance, (Surface) surface,
                                   nullptr);
}

gpu::DeviceProperties DeviceInterface::get_device_properties(gpu::Device self_)
{
  Device *const                     self          = (Device *) self_;
  VkPhysicalDeviceFeatures const   &vk_features   = self->phy_dev.vk_features;
  VkPhysicalDeviceProperties const &vk_properties = self->phy_dev.vk_properties;

  bool has_uma = false;
  for (u32 i = 0; i < self->phy_dev.vk_memory_properties.memoryTypeCount; i++)
  {
    if (has_bits(
            self->phy_dev.vk_memory_properties.memoryTypes[i].propertyFlags,
            (VkMemoryPropertyFlags) (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)))
    {
      has_uma = true;
      break;
    }
  }

  gpu::DeviceProperties properties{
      .api_version    = vk_properties.apiVersion,
      .driver_version = vk_properties.driverVersion,
      .vendor_id      = vk_properties.vendorID,
      .device_id      = vk_properties.deviceID,
      .device_name =
          Span{vk_properties.deviceName, strlen(vk_properties.deviceName)},
      .type                    = (gpu::DeviceType) vk_properties.deviceType,
      .has_unified_memory      = has_uma,
      .has_non_solid_fill_mode = (vk_features.fillModeNonSolid == VK_TRUE),
      .texel_buffer_offset_alignment =
          vk_properties.limits.minTexelBufferOffsetAlignment,
      .uniform_buffer_offset_alignment =
          vk_properties.limits.minUniformBufferOffsetAlignment,
      .storage_buffer_offset_alignment =
          vk_properties.limits.minStorageBufferOffsetAlignment,
      .timestamp_period = vk_properties.limits.timestampPeriod,
      .max_compute_work_group_invocations =
          vk_properties.limits.maxComputeWorkGroupInvocations,
      .max_compute_shared_memory_size =
          vk_properties.limits.maxComputeSharedMemorySize};

  mem::copy(vk_properties.limits.maxComputeWorkGroupCount,
            properties.max_compute_work_group_count, 3);
  mem::copy(vk_properties.limits.maxComputeWorkGroupSize,
            properties.max_compute_work_group_size, 3);

  return properties;
}

Result<gpu::FormatProperties, Status>
    DeviceInterface::get_format_properties(gpu::Device self_,
                                           gpu::Format format)
{
  Device *const      self = (Device *) self_;
  VkFormatProperties props;
  self->instance->vk_table.GetPhysicalDeviceFormatProperties(
      self->phy_dev.vk_phy_dev, (VkFormat) format, &props);
  return Ok(gpu::FormatProperties{
      .linear_tiling_features =
          (gpu::FormatFeatures) props.linearTilingFeatures,
      .optimal_tiling_features =
          (gpu::FormatFeatures) props.optimalTilingFeatures,
      .buffer_features = (gpu::FormatFeatures) props.bufferFeatures});
}

Result<gpu::Buffer, Status>
    DeviceInterface::create_buffer(gpu::Device            self_,
                                   gpu::BufferDesc const &desc)
{
  Device *const self = (Device *) self_;

  CHECK(desc.size != 0);
  CHECK(desc.usage != gpu::BufferUsage::None);

  VkBufferCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size  = desc.size,
                                 .usage = (VkBufferUsageFlags) desc.usage,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 1,
                                 .pQueueFamilyIndices   = nullptr};
  VmaAllocationCreateInfo alloc_create_info{
      .flags =
          desc.host_mapped ?
              (VmaAllocationCreateFlags) (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                          VMA_ALLOCATION_CREATE_MAPPED_BIT) :
              (VmaAllocationCreateFlags) 0,
      .usage          = VMA_MEMORY_USAGE_AUTO,
      .requiredFlags  = 0,
      .preferredFlags = 0,
      .memoryTypeBits = 0,
      .pool           = nullptr,
      .pUserData      = nullptr,
      .priority       = 0};
  VmaAllocation vma_allocation;
  VkBuffer      vk_buffer;
  VkResult      result =
      vmaCreateBuffer(self->vma_allocator, &create_info, &alloc_create_info,
                      &vk_buffer, &vma_allocation, nullptr);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_buffer, VK_OBJECT_TYPE_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);

  Buffer *buffer;
  if (!self->allocator.nalloc(1, &buffer))
  {
    vmaDestroyBuffer(self->vma_allocator, vk_buffer, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  new (buffer) Buffer{
      .desc = desc, .vk_buffer = vk_buffer, .vma_allocation = vma_allocation};

  return Ok{(gpu::Buffer) buffer};
}

Result<gpu::BufferView, Status>
    DeviceInterface::create_buffer_view(gpu::Device                self_,
                                        gpu::BufferViewDesc const &desc)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) desc.buffer;

  CHECK(buffer != nullptr);
  CHECK(has_any_bit(buffer->desc.usage,
                    gpu::BufferUsage::UniformTexelBuffer |
                        gpu::BufferUsage::StorageTexelBuffer));
  CHECK(desc.format != gpu::Format::Undefined);
  CHECK(is_valid_buffer_access(buffer->desc.size, desc.offset, desc.size));

  u64 const view_size = desc.size == gpu::WHOLE_SIZE ?
                            (buffer->desc.size - desc.offset) :
                            desc.size;

  VkBufferViewCreateInfo create_info{
      .sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
      .pNext  = nullptr,
      .flags  = 0,
      .buffer = buffer->vk_buffer,
      .format = (VkFormat) desc.format,
      .offset = desc.offset,
      .range  = desc.size};

  VkBufferView vk_view;

  VkResult result = self->vk_table.CreateBufferView(self->vk_dev, &create_info,
                                                    nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_view, VK_OBJECT_TYPE_BUFFER_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);

  BufferView *view;

  if (!self->allocator.nalloc(1, &view))
  {
    self->vk_table.DestroyBufferView(self->vk_dev, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) BufferView{.desc = desc, .vk_view = vk_view};

  view->desc.size = view_size;

  return Ok{(gpu::BufferView) view};
}

Result<gpu::Image, Status>
    DeviceInterface::create_image(gpu::Device self_, gpu::ImageDesc const &desc)
{
  Device *const self = (Device *) self_;

  CHECK(desc.format != gpu::Format::Undefined);
  CHECK(desc.usage != gpu::ImageUsage::None);
  CHECK(desc.aspects != gpu::ImageAspects::None);
  CHECK(desc.sample_count != gpu::SampleCount::None);
  CHECK(desc.extent.x != 0);
  CHECK(desc.extent.y != 0);
  CHECK(desc.extent.z != 0);
  CHECK(desc.mip_levels > 0);
  CHECK(desc.mip_levels <= num_mip_levels(desc.extent));
  CHECK(desc.array_layers > 0);
  CHECK(desc.array_layers <= gpu::MAX_IMAGE_ARRAY_LAYERS);

  switch (desc.type)
  {
    case gpu::ImageType::Type1D:
      CHECK(desc.extent.x <= gpu::MAX_IMAGE_EXTENT_1D);
      CHECK(desc.extent.y == 1);
      CHECK(desc.extent.z == 1);
      break;

    case gpu::ImageType::Type2D:
      CHECK(desc.extent.x <= gpu::MAX_IMAGE_EXTENT_2D);
      CHECK(desc.extent.y <= gpu::MAX_IMAGE_EXTENT_2D);
      CHECK(desc.extent.z == 1);
      break;

    case gpu::ImageType::Type3D:
      CHECK(desc.extent.x <= gpu::MAX_IMAGE_EXTENT_3D);
      CHECK(desc.extent.y <= gpu::MAX_IMAGE_EXTENT_3D);
      CHECK(desc.extent.z <= gpu::MAX_IMAGE_EXTENT_3D);
      break;

    default:
      break;
  }

  VkImageCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = 0,
                                .imageType = (VkImageType) desc.type,
                                .format    = (VkFormat) desc.format,
                                .extent    = VkExtent3D{.width  = desc.extent.x,
                                                        .height = desc.extent.y,
                                                        .depth  = desc.extent.z},
                                .mipLevels = desc.mip_levels,
                                .arrayLayers = desc.array_layers,
                                .samples =
                                    (VkSampleCountFlagBits) desc.sample_count,
                                .tiling      = VK_IMAGE_TILING_OPTIMAL,
                                .usage       = (VkImageUsageFlags) desc.usage,
                                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                .queueFamilyIndexCount = 0,
                                .pQueueFamilyIndices   = nullptr,
                                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};
  VmaAllocationCreateInfo vma_allocation_create_info{
      .flags          = 0,
      .usage          = VMA_MEMORY_USAGE_AUTO,
      .requiredFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .preferredFlags = 0,
      .memoryTypeBits = 0,
      .pool           = nullptr,
      .pUserData      = nullptr,
      .priority       = 0};
  VkImage           vk_image;
  VmaAllocation     vma_allocation;
  VmaAllocationInfo vma_allocation_info;

  VkResult result = vmaCreateImage(self->vma_allocator, &create_info,
                                   &vma_allocation_create_info, &vk_image,
                                   &vma_allocation, &vma_allocation_info);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_image, VK_OBJECT_TYPE_IMAGE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);

  Image *image;

  if (!self->allocator.nalloc(1, &image))
  {
    vmaDestroyImage(self->vma_allocator, vk_image, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  // separate states for depth and stencil image aspects
  u32 const num_aspects =
      has_bits(desc.aspects,
               gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil) ?
          2 :
          1;

  new (image) Image{.desc                = desc,
                    .is_swapchain_image  = false,
                    .vk_image            = vk_image,
                    .vma_allocation      = vma_allocation,
                    .vma_allocation_info = vma_allocation_info,
                    .states              = {},
                    .num_aspects         = num_aspects};

  return Ok{(gpu::Image) image};
}

Result<gpu::ImageView, Status>
    DeviceInterface::create_image_view(gpu::Device               self_,
                                       gpu::ImageViewDesc const &desc)
{
  Device *const self      = (Device *) self_;
  Image *const  src_image = (Image *) desc.image;

  CHECK(desc.image != nullptr);
  CHECK(desc.view_format != gpu::Format::Undefined);
  CHECK(is_image_view_type_compatible(src_image->desc.type, desc.view_type));
  CHECK(is_valid_image_access(
      src_image->desc.aspects, src_image->desc.mip_levels,
      src_image->desc.array_layers, desc.aspects, desc.first_mip_level,
      desc.num_mip_levels, desc.first_array_layer, desc.num_array_layers));

  u32 const mip_levels =
      desc.num_mip_levels == gpu::REMAINING_MIP_LEVELS ?
          (src_image->desc.mip_levels - desc.first_mip_level) :
          desc.num_mip_levels;
  u32 const array_layers =
      desc.num_array_layers == gpu::REMAINING_ARRAY_LAYERS ?
          (src_image->desc.array_layers - desc.first_array_layer) :
          desc.num_array_layers;

  VkImageViewCreateInfo create_info{
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0,
      .image    = src_image->vk_image,
      .viewType = (VkImageViewType) desc.view_type,
      .format   = (VkFormat) desc.view_format,
      .components =
          VkComponentMapping{.r = (VkComponentSwizzle) desc.mapping.r,
                             .g = (VkComponentSwizzle) desc.mapping.g,
                             .b = (VkComponentSwizzle) desc.mapping.b,
                             .a = (VkComponentSwizzle) desc.mapping.a},
      .subresourceRange = VkImageSubresourceRange{
          .aspectMask     = (VkImageAspectFlags) desc.aspects,
          .baseMipLevel   = desc.first_mip_level,
          .levelCount     = desc.num_mip_levels,
          .baseArrayLayer = desc.first_array_layer,
          .layerCount     = desc.num_array_layers}};

  VkImageView vk_view;
  VkResult result = self->vk_table.CreateImageView(self->vk_dev, &create_info,
                                                   nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_view, VK_OBJECT_TYPE_IMAGE_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);

  ImageView *view;
  if (!self->allocator.nalloc(1, &view))
  {
    self->vk_table.DestroyImageView(self->vk_dev, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) ImageView{.desc = desc, .vk_view = vk_view};

  view->desc.num_mip_levels   = mip_levels;
  view->desc.num_array_layers = array_layers;

  return Ok{(gpu::ImageView) view};
}

Result<gpu::Sampler, Status>
    DeviceInterface::create_sampler(gpu::Device             self_,
                                    gpu::SamplerDesc const &desc)
{
  Device *const self = (Device *) self_;
  CHECK(!(desc.anisotropy_enable &&
          (desc.max_anisotropy > gpu::MAX_SAMPLER_ANISOTROPY)));
  CHECK(!(desc.anisotropy_enable && (desc.max_anisotropy < 1.0)));

  VkSamplerCreateInfo create_info{
      .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext                   = nullptr,
      .flags                   = 0,
      .magFilter               = (VkFilter) desc.mag_filter,
      .minFilter               = (VkFilter) desc.min_filter,
      .mipmapMode              = (VkSamplerMipmapMode) desc.mip_map_mode,
      .addressModeU            = (VkSamplerAddressMode) desc.address_mode_u,
      .addressModeV            = (VkSamplerAddressMode) desc.address_mode_v,
      .addressModeW            = (VkSamplerAddressMode) desc.address_mode_w,
      .mipLodBias              = desc.mip_lod_bias,
      .anisotropyEnable        = (VkBool32) desc.anisotropy_enable,
      .maxAnisotropy           = desc.max_anisotropy,
      .compareEnable           = (VkBool32) desc.compare_enable,
      .compareOp               = (VkCompareOp) desc.compare_op,
      .minLod                  = desc.min_lod,
      .maxLod                  = desc.max_lod,
      .borderColor             = (VkBorderColor) desc.border_color,
      .unnormalizedCoordinates = (VkBool32) desc.unnormalized_coordinates};

  VkSampler vk_sampler;
  VkResult  result = self->vk_table.CreateSampler(self->vk_dev, &create_info,
                                                  nullptr, &vk_sampler);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_sampler, VK_OBJECT_TYPE_SAMPLER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT);

  return Ok{(gpu::Sampler) vk_sampler};
}

Result<gpu::Shader, Status>
    DeviceInterface::create_shader(gpu::Device            self_,
                                   gpu::ShaderDesc const &desc)
{
  Device *const self = (Device *) self_;

  CHECK(desc.spirv_code.size_bytes() > 0);

  VkShaderModuleCreateInfo create_info{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0,
      .codeSize = desc.spirv_code.size_bytes(),
      .pCode    = desc.spirv_code.data()};

  VkShaderModule vk_shader;
  VkResult       result = self->vk_table.CreateShaderModule(
      self->vk_dev, &create_info, nullptr, &vk_shader);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_shader, VK_OBJECT_TYPE_SHADER_MODULE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT);

  return Ok{(gpu::Shader) vk_shader};
}

Result<gpu::DescriptorSetLayout, Status>
    DeviceInterface::create_descriptor_set_layout(
        gpu::Device self_, gpu::DescriptorSetLayoutDesc const &desc)
{
  Device *const self                         = (Device *) self_;
  u32 const     num_bindings                 = desc.bindings.size32();
  u32           num_descriptors              = 0;
  u32           num_variable_length          = 0;
  u32           sizing[NUM_DESCRIPTOR_TYPES] = {};

  for (gpu::DescriptorBindingDesc const &desc : desc.bindings)
  {
    num_descriptors += desc.count;
    sizing[(u32) desc.type] += desc.count;
    num_variable_length += desc.is_variable_length ? 1 : 0;
  }

  u32 num_dynamic_storage_buffers =
      sizing[(u32) gpu::DescriptorType::DynamicStorageBuffer];
  u32 num_dynamic_uniform_buffers =
      sizing[(u32) gpu::DescriptorType::DynamicUniformBuffer];

  CHECK(num_bindings > 0);
  CHECK(num_bindings <= gpu::MAX_DESCRIPTOR_SET_BINDINGS);
  CHECK(num_dynamic_storage_buffers <=
        gpu::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS);
  CHECK(num_dynamic_uniform_buffers <=
        gpu::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS);
  CHECK(num_descriptors <= gpu::MAX_DESCRIPTOR_SET_DESCRIPTORS);
  CHECK(num_variable_length <= 1);
  CHECK(!(num_variable_length > 0 && (num_dynamic_storage_buffers > 0 ||
                                      num_dynamic_uniform_buffers > 0)));

  for (u32 i = 0; i < num_bindings; i++)
  {
    CHECK(desc.bindings[i].count > 0);
    CHECK(desc.bindings[i].count <= gpu::MAX_BINDING_DESCRIPTORS);
    CHECK(!(desc.bindings[i].is_variable_length &&
            (i != (desc.bindings.size() - 1))));
  }

  VkDescriptorSetLayoutBinding vk_bindings[gpu::MAX_DESCRIPTOR_SET_BINDINGS];
  VkDescriptorBindingFlagsEXT
      vk_binding_flags[gpu::MAX_DESCRIPTOR_SET_BINDINGS];

  for (u32 i = 0; i < num_bindings; i++)
  {
    gpu::DescriptorBindingDesc const &binding = desc.bindings[i];
    VkShaderStageFlags                stage_flags =
        (VkShaderStageFlags) (binding.type ==
                                      gpu::DescriptorType::InputAttachment ?
                                  VK_SHADER_STAGE_FRAGMENT_BIT :
                                  VK_SHADER_STAGE_ALL);
    vk_bindings[i] = VkDescriptorSetLayoutBinding{
        .binding            = i,
        .descriptorType     = (VkDescriptorType) binding.type,
        .descriptorCount    = binding.count,
        .stageFlags         = stage_flags,
        .pImmutableSamplers = nullptr};

    VkDescriptorBindingFlagsEXT const vk_flags =
        VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT |
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        (binding.is_variable_length ?
             VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT :
             0);
    vk_binding_flags[i] = vk_flags;
  }

  VkDescriptorSetLayoutBindingFlagsCreateInfoEXT vk_binding_flags_create_info{
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
      .pNext         = nullptr,
      .bindingCount  = desc.bindings.size32(),
      .pBindingFlags = vk_binding_flags};

  VkDescriptorSetLayoutCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = &vk_binding_flags_create_info,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
      .bindingCount = num_bindings,
      .pBindings    = vk_bindings};

  VkDescriptorSetLayout vk_layout;
  VkResult              result = self->vk_table.CreateDescriptorSetLayout(
      self->vk_dev, &create_info, nullptr, &vk_layout);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_layout_{[&] {
    if (vk_layout != nullptr)
    {
      self->vk_table.DestroyDescriptorSetLayout(self->vk_dev, vk_layout,
                                                nullptr);
    }
  }};

  set_resource_name(self, desc.label, vk_layout,
                    VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);

  DescriptorSetLayout *layout;
  if (!self->allocator.nalloc(1, &layout))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (layout) DescriptorSetLayout{.vk_layout           = vk_layout,
                                   .num_bindings        = num_bindings,
                                   .num_variable_length = num_variable_length};

  mem::copy(desc.bindings, layout->bindings);
  mem::copy(sizing, layout->sizing, NUM_DESCRIPTOR_TYPES);
  vk_layout = nullptr;

  return Ok{(gpu::DescriptorSetLayout) layout};
}

Result<gpu::DescriptorSet, Status>
    DeviceInterface::create_descriptor_set(gpu::Device              self_,
                                           gpu::DescriptorSetLayout layout_,
                                           Span<u32 const> variable_lengths)
{
  Device *const              self   = (Device *) self_;
  DescriptorSetLayout *const layout = (DescriptorSetLayout *) layout_;
  DescriptorHeap            &heap   = self->descriptor_heap;
  CHECK(variable_lengths.size() == layout->num_variable_length);

  {
    u32 vla_idx = 0;
    for (u32 i = 0; i < layout->num_bindings; i++)
    {
      if (layout->bindings[i].is_variable_length)
      {
        CHECK(variable_lengths[vla_idx] <= layout->bindings[i].count);
        vla_idx++;
      }
    }
  }

  u32 descriptor_usage[NUM_DESCRIPTOR_TYPES]           = {};
  u32 bindings_sizes[gpu::MAX_DESCRIPTOR_SET_BINDINGS] = {};

  {
    u32 vla_idx = 0;
    for (u32 i = 0; i < layout->num_bindings; i++)
    {
      gpu::DescriptorBindingDesc const &desc  = layout->bindings[i];
      u32                               count = 0;
      if (!desc.is_variable_length)
      {
        count = desc.count;
      }
      else
      {
        count = variable_lengths[vla_idx];
        vla_idx++;
      }
      descriptor_usage[(u32) desc.type] += count;
      bindings_sizes[i] = count;
    }
  }

  u32 ipool = 0;
  for (; ipool < heap.num_pools; ipool++)
  {
    bool fits = false;
    for (u32 i = 0; i < NUM_DESCRIPTOR_TYPES; i++)
    {
      fits = fits || descriptor_usage[i] <= heap.pools[ipool].avail[i];
    }
    if (fits)
    {
      break;
    }
  }

  if (ipool >= heap.num_pools)
  {
    VkDescriptorPoolSize size[NUM_DESCRIPTOR_TYPES];
    for (u32 i = 0; i < NUM_DESCRIPTOR_TYPES; i++)
    {
      size[i] = VkDescriptorPoolSize{.type            = (VkDescriptorType) i,
                                     .descriptorCount = heap.pool_size};
    }

    VkDescriptorPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
                 VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets       = heap.pool_size * NUM_DESCRIPTOR_TYPES,
        .poolSizeCount = NUM_DESCRIPTOR_TYPES,
        .pPoolSizes    = size};

    VkDescriptorPool vk_pool;
    VkResult         result = self->vk_table.CreateDescriptorPool(
        self->vk_dev, &create_info, nullptr, &vk_pool);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    defer vk_pool_{[&] {
      self->vk_table.DestroyDescriptorPool(self->vk_dev, vk_pool, nullptr);
    }};

    if (!heap.allocator.nrealloc(heap.num_pools, heap.num_pools + 1,
                                 &heap.pools))
    {
      return Err{Status::OutOfHostMemory};
    }

    DescriptorPool *pool = heap.pools + heap.num_pools;

    fill(pool->avail, heap.pool_size);
    pool->vk_pool = vk_pool;

    heap.num_pools++;
    vk_pool = nullptr;
  }

  DescriptorBinding bindings[gpu::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  u32               num_bindings                               = 0;

  defer sync_resources_{[&] {
    for (u32 i = num_bindings; i-- > 0;)
    {
      if (bindings[i].sync_resources != nullptr)
      {
        heap.allocator.ndealloc(bindings[i].sync_resources, bindings[i].count);
      }
    }
  }};

  for (; num_bindings < layout->num_bindings; num_bindings++)
  {
    gpu::DescriptorBindingDesc const &desc = layout->bindings[num_bindings];
    void                            **sync_resources = nullptr;
    u32                               count = bindings_sizes[num_bindings];

    switch (desc.type)
    {
      case gpu::DescriptorType::CombinedImageSampler:
      case gpu::DescriptorType::SampledImage:
      case gpu::DescriptorType::StorageImage:
      case gpu::DescriptorType::UniformTexelBuffer:
      case gpu::DescriptorType::StorageTexelBuffer:
      case gpu::DescriptorType::UniformBuffer:
      case gpu::DescriptorType::StorageBuffer:
      case gpu::DescriptorType::DynamicUniformBuffer:
      case gpu::DescriptorType::DynamicStorageBuffer:
      case gpu::DescriptorType::InputAttachment:
        if (!heap.allocator.nalloc_zeroed(count, &sync_resources))
        {
          return Err{Status::OutOfHostMemory};
        }
        break;
      default:
        sync_resources = nullptr;
        break;
    }

    bindings[num_bindings] =
        DescriptorBinding{.sync_resources     = sync_resources,
                          .count              = count,
                          .type               = desc.type,
                          .is_variable_length = desc.is_variable_length,
                          .max_count          = desc.count};
  }

  VkDescriptorSetVariableDescriptorCountAllocateInfoEXT var_alloc_info{
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
      .pNext              = nullptr,
      .descriptorSetCount = variable_lengths.size32(),
      .pDescriptorCounts  = variable_lengths.data()};

  VkDescriptorSetAllocateInfo alloc_info{
      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext              = &var_alloc_info,
      .descriptorPool     = heap.pools[ipool].vk_pool,
      .descriptorSetCount = 1,
      .pSetLayouts        = &layout->vk_layout};

  VkDescriptorSet vk_set;
  VkResult        result =
      self->vk_table.AllocateDescriptorSets(self->vk_dev, &alloc_info, &vk_set);

  // must not have these errors
  CHECK(result != VK_ERROR_OUT_OF_POOL_MEMORY &&
        result != VK_ERROR_FRAGMENTED_POOL);

  for (u32 i = 0; i < NUM_DESCRIPTOR_TYPES; i++)
  {
    heap.pools[ipool].avail[i] -= descriptor_usage[i];
  }

  DescriptorSet *set;

  if (!heap.allocator.nalloc(1, &set))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (set) DescriptorSet{
      .vk_set = vk_set, .num_bindings = num_bindings, .pool = ipool};

  mem::copy(bindings, set->bindings, num_bindings);
  num_bindings = 0;

  return Ok{(gpu::DescriptorSet) set};
}

Result<gpu::PipelineCache, Status>
    DeviceInterface::create_pipeline_cache(gpu::Device                   self_,
                                           gpu::PipelineCacheDesc const &desc)
{
  Device *const             self = (Device *) self_;
  VkPipelineCacheCreateInfo create_info{
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0,
      .initialDataSize = desc.initial_data.size_bytes(),
      .pInitialData    = desc.initial_data.data()};

  VkPipelineCache vk_cache;
  VkResult        result = self->vk_table.CreatePipelineCache(
      self->vk_dev, &create_info, nullptr, &vk_cache);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_cache, VK_OBJECT_TYPE_PIPELINE_CACHE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT);

  return Ok{(gpu::PipelineCache) vk_cache};
}

Result<gpu::ComputePipeline, Status> DeviceInterface::create_compute_pipeline(
    gpu::Device self_, gpu::ComputePipelineDesc const &desc)
{
  Device *const self                = (Device *) self_;
  u32 const     num_descriptor_sets = desc.descriptor_set_layouts.size32();

  CHECK(num_descriptor_sets <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS);
  CHECK(desc.push_constants_size <= gpu::MAX_PUSH_CONSTANTS_SIZE);
  CHECK(mem::is_aligned(4U, desc.push_constants_size));
  CHECK(desc.compute_shader.entry_point.size() > 0 &&
        desc.compute_shader.entry_point.size() < 256);
  CHECK(desc.compute_shader.shader != nullptr);

  VkDescriptorSetLayout
      vk_descriptor_set_layouts[gpu::MAX_PIPELINE_DESCRIPTOR_SETS];
  for (u32 i = 0; i < num_descriptor_sets; i++)
  {
    vk_descriptor_set_layouts[i] =
        ((DescriptorSetLayout *) desc.descriptor_set_layouts[i])->vk_layout;
  }

  VkSpecializationInfo vk_specialization{
      .mapEntryCount = desc.compute_shader.specialization_constants.size32(),
      .pMapEntries   = (VkSpecializationMapEntry const *)
                         desc.compute_shader.specialization_constants.data(),
      .dataSize =
          desc.compute_shader.specialization_constants_data.size_bytes(),
      .pData = desc.compute_shader.specialization_constants_data.data()};

  char entry_point[256];
  to_c_str(desc.compute_shader.entry_point, span(entry_point));

  VkPipelineShaderStageCreateInfo vk_stage{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext  = nullptr,
      .flags  = 0,
      .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = (Shader) desc.compute_shader.shader,
      .pName  = entry_point,
      .pSpecializationInfo = &vk_specialization};

  VkPushConstantRange push_constants_range{.stageFlags =
                                               VK_SHADER_STAGE_COMPUTE_BIT,
                                           .offset = 0,
                                           .size   = desc.push_constants_size};

  VkPipelineLayoutCreateInfo layout_create_info{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0,
      .setLayoutCount         = num_descriptor_sets,
      .pSetLayouts            = vk_descriptor_set_layouts,
      .pushConstantRangeCount = desc.push_constants_size == 0 ? 0U : 1U,
      .pPushConstantRanges =
          desc.push_constants_size == 0 ? nullptr : &push_constants_range};

  VkPipelineLayout vk_layout;
  VkResult         result = self->vk_table.CreatePipelineLayout(
      self->vk_dev, &layout_create_info, nullptr, &vk_layout);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkComputePipelineCreateInfo create_info{
      .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .pNext              = nullptr,
      .flags              = 0,
      .stage              = vk_stage,
      .layout             = vk_layout,
      .basePipelineHandle = nullptr,
      .basePipelineIndex  = 0};

  VkPipeline vk_pipeline;
  result = self->vk_table.CreateComputePipelines(
      self->vk_dev,
      desc.cache == nullptr ? nullptr : (PipelineCache) desc.cache, 1,
      &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_dev, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_pipeline, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
  set_resource_name(self, desc.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);

  ComputePipeline *pipeline;
  if (!self->allocator.nalloc(1, &pipeline))
  {
    self->vk_table.DestroyPipelineLayout(self->vk_dev, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_dev, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
      ComputePipeline{.vk_pipeline         = vk_pipeline,
                      .vk_layout           = vk_layout,
                      .push_constants_size = desc.push_constants_size,
                      .num_sets = desc.descriptor_set_layouts.size32()};

  return Ok{(gpu::ComputePipeline) pipeline};
}

Result<gpu::GraphicsPipeline, Status> DeviceInterface::create_graphics_pipeline(
    gpu::Device self_, gpu::GraphicsPipelineDesc const &desc)
{
  Device *const self                = (Device *) self_;
  u32 const     num_descriptor_sets = desc.descriptor_set_layouts.size32();
  u32 const     num_input_bindings  = desc.vertex_input_bindings.size32();
  u32 const     num_attributes      = desc.vertex_attributes.size32();
  u32 const     num_blend_color_attachments =
      desc.color_blend_state.attachments.size32();
  u32 const num_colors   = desc.color_formats.size32();
  u32 const num_depths   = desc.depth_format.size32();
  u32 const num_stencils = desc.stencil_format.size32();

  CHECK(!(desc.rasterization_state.polygon_mode != gpu::PolygonMode::Fill &&
          !self->phy_dev.vk_features.fillModeNonSolid));
  CHECK(num_descriptor_sets <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS);
  CHECK(desc.push_constants_size <= gpu::MAX_PUSH_CONSTANTS_SIZE);
  CHECK(mem::is_aligned(4U, desc.push_constants_size));
  CHECK(desc.vertex_shader.entry_point.size() > 0 &&
        desc.vertex_shader.entry_point.size() <= 255);
  CHECK(desc.fragment_shader.entry_point.size() > 0 &&
        desc.fragment_shader.entry_point.size() <= 255);
  CHECK(num_attributes <= gpu::MAX_VERTEX_ATTRIBUTES);
  CHECK(num_colors <= gpu::MAX_PIPELINE_COLOR_ATTACHMENTS);
  CHECK(num_depths <= 1);
  CHECK(num_stencils <= 1);

  VkDescriptorSetLayout
      vk_descriptor_set_layouts[gpu::MAX_PIPELINE_DESCRIPTOR_SETS];
  for (u32 i = 0; i < num_descriptor_sets; i++)
  {
    vk_descriptor_set_layouts[i] =
        ((DescriptorSetLayout *) desc.descriptor_set_layouts[i])->vk_layout;
  }

  VkSpecializationInfo vk_vs_specialization{
      .mapEntryCount = desc.vertex_shader.specialization_constants.size32(),
      .pMapEntries   = (VkSpecializationMapEntry const *)
                         desc.vertex_shader.specialization_constants.data(),
      .dataSize = desc.vertex_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.vertex_shader.specialization_constants_data.data()};

  VkSpecializationInfo vk_fs_specialization{
      .mapEntryCount = desc.fragment_shader.specialization_constants.size32(),
      .pMapEntries   = (VkSpecializationMapEntry const *)
                         desc.fragment_shader.specialization_constants.data(),
      .dataSize =
          desc.fragment_shader.specialization_constants_data.size_bytes(),
      .pData = desc.fragment_shader.specialization_constants_data.data()};

  char vs_entry_point[256];
  char fs_entry_point[256];
  to_c_str(desc.vertex_shader.entry_point, span(vs_entry_point));
  to_c_str(desc.fragment_shader.entry_point, span(fs_entry_point));

  VkPipelineShaderStageCreateInfo vk_stages[2] = {
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_VERTEX_BIT,
       .module = (Shader) desc.vertex_shader.shader,
       .pName  = vs_entry_point,
       .pSpecializationInfo = &vk_vs_specialization},
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = (Shader) desc.fragment_shader.shader,
       .pName  = fs_entry_point,
       .pSpecializationInfo = &vk_fs_specialization}};

  VkPushConstantRange push_constants_range{.stageFlags = VK_SHADER_STAGE_ALL,
                                           .offset     = 0,
                                           .size = desc.push_constants_size};

  VkPipelineLayoutCreateInfo layout_create_info{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0,
      .setLayoutCount         = num_descriptor_sets,
      .pSetLayouts            = vk_descriptor_set_layouts,
      .pushConstantRangeCount = desc.push_constants_size == 0 ? 0U : 1U,
      .pPushConstantRanges =
          desc.push_constants_size == 0 ? nullptr : &push_constants_range};

  VkPipelineLayout vk_layout;

  VkResult result = self->vk_table.CreatePipelineLayout(
      self->vk_dev, &layout_create_info, nullptr, &vk_layout);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkVertexInputBindingDescription input_bindings[gpu::MAX_VERTEX_ATTRIBUTES];
  for (u32 ibinding = 0; ibinding < num_input_bindings; ibinding++)
  {
    gpu::VertexInputBinding const &binding =
        desc.vertex_input_bindings[ibinding];
    input_bindings[ibinding] = VkVertexInputBindingDescription{
        .binding   = binding.binding,
        .stride    = binding.stride,
        .inputRate = (VkVertexInputRate) binding.input_rate};
  }

  VkVertexInputAttributeDescription attributes[gpu::MAX_VERTEX_ATTRIBUTES];
  for (u32 iattribute = 0; iattribute < num_attributes; iattribute++)
  {
    gpu::VertexAttribute const &attribute = desc.vertex_attributes[iattribute];
    attributes[iattribute] =
        VkVertexInputAttributeDescription{.location = attribute.location,
                                          .binding  = attribute.binding,
                                          .format = (VkFormat) attribute.format,
                                          .offset = attribute.offset};
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount   = num_input_bindings,
      .pVertexBindingDescriptions      = input_bindings,
      .vertexAttributeDescriptionCount = num_attributes,
      .pVertexAttributeDescriptions    = attributes};

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
      .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0,
      .topology = (VkPrimitiveTopology) desc.primitive_topology,
      .primitiveRestartEnable = VK_FALSE};

  VkViewport viewport{
      .x = 0, .y = 0, .width = 0, .height = 0, .minDepth = 0, .maxDepth = 1};
  VkRect2D scissor{.offset = {0, 0}, .extent = {0, 0}};

  VkPipelineViewportStateCreateInfo viewport_state{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0,
      .viewportCount = 1,
      .pViewports    = &viewport,
      .scissorCount  = 1,
      .pScissors     = &scissor};

  VkPipelineRasterizationStateCreateInfo rasterization_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthClampEnable =
          (VkBool32) desc.rasterization_state.depth_clamp_enable,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode     = (VkPolygonMode) desc.rasterization_state.polygon_mode,
      .cullMode        = (VkCullModeFlags) desc.rasterization_state.cull_mode,
      .frontFace       = (VkFrontFace) desc.rasterization_state.front_face,
      .depthBiasEnable = (VkBool32) desc.rasterization_state.depth_bias_enable,
      .depthBiasConstantFactor =
          desc.rasterization_state.depth_bias_constant_factor,
      .depthBiasClamp       = desc.rasterization_state.depth_bias_clamp,
      .depthBiasSlopeFactor = desc.rasterization_state.depth_bias_slope_factor,
      .lineWidth            = 1.0F};

  VkPipelineMultisampleStateCreateInfo multisample_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable   = (VkBool32) false,
      .minSampleShading      = 1,
      .pSampleMask           = nullptr,
      .alphaToCoverageEnable = (VkBool32) false,
      .alphaToOneEnable      = (VkBool32) false};

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthTestEnable = (VkBool32) desc.depth_stencil_state.depth_test_enable,
      .depthWriteEnable =
          (VkBool32) desc.depth_stencil_state.depth_write_enable,
      .depthCompareOp = (VkCompareOp) desc.depth_stencil_state.depth_compare_op,
      .depthBoundsTestEnable =
          (VkBool32) desc.depth_stencil_state.depth_bounds_test_enable,
      .stencilTestEnable =
          (VkBool32) desc.depth_stencil_state.stencil_test_enable,
      .front =
          VkStencilOpState{
              .failOp =
                  (VkStencilOp) desc.depth_stencil_state.front_stencil.fail_op,
              .passOp =
                  (VkStencilOp) desc.depth_stencil_state.front_stencil.pass_op,
              .depthFailOp = (VkStencilOp) desc.depth_stencil_state
                                 .front_stencil.depth_fail_op,
              .compareOp = (VkCompareOp) desc.depth_stencil_state.front_stencil
                               .compare_op,
              .compareMask =
                  desc.depth_stencil_state.front_stencil.compare_mask,
              .writeMask = desc.depth_stencil_state.front_stencil.write_mask,
              .reference = desc.depth_stencil_state.front_stencil.reference},
      .back =
          VkStencilOpState{
              .failOp =
                  (VkStencilOp) desc.depth_stencil_state.back_stencil.fail_op,
              .passOp =
                  (VkStencilOp) desc.depth_stencil_state.back_stencil.pass_op,
              .depthFailOp = (VkStencilOp) desc.depth_stencil_state.back_stencil
                                 .depth_fail_op,
              .compareOp = (VkCompareOp)
                               desc.depth_stencil_state.back_stencil.compare_op,
              .compareMask = desc.depth_stencil_state.back_stencil.compare_mask,
              .writeMask   = desc.depth_stencil_state.back_stencil.write_mask,
              .reference   = desc.depth_stencil_state.back_stencil.reference},
      .minDepthBounds = desc.depth_stencil_state.min_depth_bounds,
      .maxDepthBounds = desc.depth_stencil_state.max_depth_bounds};

  VkPipelineColorBlendAttachmentState
      attachment_states[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS];

  for (u32 i = 0; i < num_blend_color_attachments; i++)
  {
    gpu::ColorBlendAttachmentState const &state =
        desc.color_blend_state.attachments[i];
    attachment_states[i] = VkPipelineColorBlendAttachmentState{
        .blendEnable         = (VkBool32) state.blend_enable,
        .srcColorBlendFactor = (VkBlendFactor) state.src_color_blend_factor,
        .dstColorBlendFactor = (VkBlendFactor) state.dst_color_blend_factor,
        .colorBlendOp        = (VkBlendOp) state.color_blend_op,
        .srcAlphaBlendFactor = (VkBlendFactor) state.src_alpha_blend_factor,
        .dstAlphaBlendFactor = (VkBlendFactor) state.dst_alpha_blend_factor,
        .alphaBlendOp        = (VkBlendOp) state.alpha_blend_op,
        .colorWriteMask      = (VkColorComponentFlags) state.color_write_mask};
  }

  VkPipelineColorBlendStateCreateInfo color_blend_state{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0,
      .logicOpEnable = (VkBool32) desc.color_blend_state.logic_op_enable,
      .logicOp       = (VkLogicOp) desc.color_blend_state.logic_op,
      .attachmentCount = num_blend_color_attachments,
      .pAttachments    = attachment_states,
      .blendConstants  = {desc.color_blend_state.blend_constant.x,
                          desc.color_blend_state.blend_constant.y,
                          desc.color_blend_state.blend_constant.z,
                          desc.color_blend_state.blend_constant.w}};

  constexpr VkDynamicState dynamic_states[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_BLEND_CONSTANTS,
      VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
      VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE,
      VK_DYNAMIC_STATE_CULL_MODE_EXT,
      VK_DYNAMIC_STATE_FRONT_FACE_EXT,
      VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,
      VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
      VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,
      VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT,
      VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT,
      VK_DYNAMIC_STATE_STENCIL_OP_EXT};

  VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext             = nullptr,
      .flags             = 0,
      .dynamicStateCount = (u32) size(dynamic_states),
      .pDynamicStates    = dynamic_states};

  VkFormat color_formats[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS];

  for (u32 i = 0; i < desc.color_formats.size(); i++)
  {
    color_formats[i] = (VkFormat) desc.color_formats[i];
  }

  VkFormat depth_format =
      (VkFormat) ((num_depths == 0) ? gpu::Format::Undefined :
                                      desc.depth_format[0]);
  VkFormat stencil_format =
      (VkFormat) ((num_stencils == 0) ? gpu::Format::Undefined :
                                        desc.stencil_format[0]);

  VkPipelineRenderingCreateInfoKHR rendering_info{
      .sType    = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
      .pNext    = nullptr,
      .viewMask = 0,
      .colorAttachmentCount    = desc.color_formats.size32(),
      .pColorAttachmentFormats = color_formats,
      .depthAttachmentFormat   = depth_format,
      .stencilAttachmentFormat = stencil_format};

  VkGraphicsPipelineCreateInfo create_info{
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext               = &rendering_info,
      .flags               = 0,
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
      .renderPass          = nullptr,
      .subpass             = 0,
      .basePipelineHandle  = nullptr,
      .basePipelineIndex   = 0};

  VkPipeline vk_pipeline;
  result = self->vk_table.CreateGraphicsPipelines(
      self->vk_dev,
      desc.cache == nullptr ? nullptr : (PipelineCache) desc.cache, 1,
      &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_dev, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_pipeline, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
  set_resource_name(self, desc.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);

  GraphicsPipeline *pipeline;
  if (!self->allocator.nalloc(1, &pipeline))
  {
    self->vk_table.DestroyPipelineLayout(self->vk_dev, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_dev, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
      GraphicsPipeline{.vk_pipeline         = vk_pipeline,
                       .vk_layout           = vk_layout,
                       .push_constants_size = desc.push_constants_size,
                       .num_sets     = desc.descriptor_set_layouts.size32(),
                       .num_colors   = num_colors,
                       .num_depths   = num_depths,
                       .num_stencils = num_stencils};

  mem::copy(desc.color_formats, pipeline->colors);
  mem::copy(desc.depth_format, pipeline->depth);
  mem::copy(desc.stencil_format, pipeline->stencil);

  return Ok{(gpu::GraphicsPipeline) pipeline};
}

/// old swapchain will be retired and destroyed irregardless of whether new
/// swapchain recreation fails.
inline VkResult recreate_swapchain(Device *self, Swapchain *swapchain)
{
  CHECK(swapchain->desc.preferred_extent.x > 0);
  CHECK(swapchain->desc.preferred_extent.y > 0);
  CHECK(swapchain->desc.preferred_buffering <= gpu::MAX_SWAPCHAIN_IMAGES);

  VkSurfaceCapabilitiesKHR surface_capabilities;
  VkResult                 result =
      self->instance->vk_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(
          self->phy_dev.vk_phy_dev, swapchain->vk_surface,
          &surface_capabilities);

  if (result != VK_SUCCESS)
  {
    return result;
  }

  if (surface_capabilities.currentExtent.width == 0 ||
      surface_capabilities.currentExtent.height == 0)
  {
    swapchain->is_zero_sized = true;
    return VK_SUCCESS;
  }

  CHECK(has_bits(surface_capabilities.supportedUsageFlags,
                 (VkImageUsageFlags) swapchain->desc.usage));
  CHECK(has_bits(surface_capabilities.supportedCompositeAlpha,
                 (VkImageUsageFlags) swapchain->desc.composite_alpha));

  // take ownership of internal data for re-use/release
  VkSwapchainKHR old_vk_swapchain = swapchain->vk_swapchain;
  defer          old_vk_swapchain_{[&] {
    if (old_vk_swapchain != nullptr)
    {
      self->vk_table.DestroySwapchainKHR(self->vk_dev, old_vk_swapchain,
                                                  nullptr);
    }
  }};

  swapchain->is_out_of_date  = true;
  swapchain->is_optimal      = false;
  swapchain->is_zero_sized   = false;
  swapchain->format          = gpu::SurfaceFormat{};
  swapchain->usage           = gpu::ImageUsage::None;
  swapchain->present_mode    = gpu::PresentMode::Immediate;
  swapchain->extent          = gpu::Extent{};
  swapchain->composite_alpha = gpu::CompositeAlpha::None;
  swapchain->num_images      = 0;
  swapchain->current_image   = 0;
  swapchain->vk_swapchain    = nullptr;

  VkExtent2D vk_extent;

  if (surface_capabilities.currentExtent.width == 0xFFFFFFFFU &&
      surface_capabilities.currentExtent.height == 0xFFFFFFFFU)
  {
    vk_extent.width  = clamp(swapchain->desc.preferred_extent.x,
                             surface_capabilities.minImageExtent.width,
                             surface_capabilities.maxImageExtent.width);
    vk_extent.height = clamp(swapchain->desc.preferred_extent.y,
                             surface_capabilities.minImageExtent.height,
                             surface_capabilities.maxImageExtent.height);
  }
  else
  {
    vk_extent = surface_capabilities.currentExtent;
  }

  u32 min_image_count = 0;

  if (surface_capabilities.maxImageCount != 0)
  {
    min_image_count = clamp(swapchain->desc.preferred_buffering,
                            surface_capabilities.minImageCount,
                            surface_capabilities.maxImageCount);
  }
  else
  {
    min_image_count = max(min_image_count, surface_capabilities.minImageCount);
  }

  VkSwapchainCreateInfoKHR create_info{
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext            = nullptr,
      .flags            = 0,
      .surface          = swapchain->vk_surface,
      .minImageCount    = min_image_count,
      .imageFormat      = (VkFormat) swapchain->desc.format.format,
      .imageColorSpace  = (VkColorSpaceKHR) swapchain->desc.format.color_space,
      .imageExtent      = vk_extent,
      .imageArrayLayers = 1,
      .imageUsage       = (VkImageUsageFlags) swapchain->desc.usage,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices   = nullptr,
      .preTransform          = surface_capabilities.currentTransform,
      .compositeAlpha =
          (VkCompositeAlphaFlagBitsKHR) swapchain->desc.composite_alpha,
      .presentMode  = (VkPresentModeKHR) swapchain->desc.present_mode,
      .clipped      = VK_TRUE,
      .oldSwapchain = old_vk_swapchain};

  VkSwapchainKHR new_vk_swapchain;

  result = self->vk_table.CreateSwapchainKHR(self->vk_dev, &create_info,
                                             nullptr, &new_vk_swapchain);

  CHECK(result == VK_SUCCESS);

  defer new_vk_swapchain_{[&] {
    if (new_vk_swapchain != nullptr)
    {
      self->vk_table.DestroySwapchainKHR(self->vk_dev, new_vk_swapchain,
                                         nullptr);
    }
  }};

  u32 num_images;
  result = self->vk_table.GetSwapchainImagesKHR(self->vk_dev, new_vk_swapchain,
                                                &num_images, nullptr);

  CHECK(result == VK_SUCCESS);

  CHECK(num_images <= gpu::MAX_SWAPCHAIN_IMAGES);

  result = self->vk_table.GetSwapchainImagesKHR(
      self->vk_dev, new_vk_swapchain, &num_images, swapchain->vk_images);

  CHECK(result == VK_SUCCESS);

  for (u32 i = 0; i < num_images; i++)
  {
    swapchain->image_impls[i] = Image{
        .desc                = gpu::ImageDesc{.type         = gpu::ImageType::Type2D,
                                              .format       = swapchain->desc.format.format,
                                              .usage        = swapchain->desc.usage,
                                              .aspects      = gpu::ImageAspects::Color,
                                              .extent       = gpu::Extent3D{vk_extent.width,
                                                       vk_extent.height, 1},
                                              .mip_levels   = 1,
                                              .array_layers = 1},
        .is_swapchain_image  = true,
        .vk_image            = swapchain->vk_images[i],
        .vma_allocation      = nullptr,
        .vma_allocation_info = {},
        .states              = {},
        .num_aspects         = 1};
    swapchain->images[i] = (gpu::Image)(swapchain->image_impls + i);
  }

  set_resource_name(self, swapchain->desc.label, new_vk_swapchain,
                    VK_OBJECT_TYPE_SWAPCHAIN_KHR,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT);
  for (u32 i = 0; i < num_images; i++)
  {
    set_resource_name(self, swapchain->desc.label, swapchain->vk_images[i],
                      VK_OBJECT_TYPE_IMAGE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);
  }

  swapchain->is_out_of_date  = false;
  swapchain->is_optimal      = true;
  swapchain->is_zero_sized   = false;
  swapchain->format          = swapchain->desc.format;
  swapchain->usage           = swapchain->desc.usage;
  swapchain->present_mode    = swapchain->desc.present_mode;
  swapchain->extent.x        = vk_extent.width;
  swapchain->extent.y        = vk_extent.height;
  swapchain->composite_alpha = swapchain->desc.composite_alpha;
  swapchain->num_images      = num_images;
  swapchain->current_image   = 0;
  swapchain->vk_swapchain    = new_vk_swapchain;
  new_vk_swapchain           = nullptr;

  return VK_SUCCESS;
}

Result<gpu::Swapchain, Status>
    DeviceInterface::create_swapchain(gpu::Device self_, gpu::Surface surface,
                                      gpu::SwapchainDesc const &desc)
{
  Device *const self = (Device *) self_;

  CHECK(desc.preferred_extent.x > 0);
  CHECK(desc.preferred_extent.y > 0);

  Swapchain *swapchain;
  if (!self->allocator.nalloc(1, &swapchain))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (swapchain) Swapchain{.desc            = desc,
                            .is_out_of_date  = true,
                            .is_optimal      = false,
                            .is_zero_sized   = false,
                            .format          = {},
                            .usage           = {},
                            .present_mode    = gpu::PresentMode::Immediate,
                            .extent          = {},
                            .composite_alpha = gpu::CompositeAlpha::None,
                            .image_impls     = {},
                            .images          = {},
                            .vk_images       = {},
                            .num_images      = 0,
                            .current_image   = 0,
                            .vk_swapchain    = nullptr,
                            .vk_surface      = (VkSurfaceKHR) surface};

  return Ok{(gpu::Swapchain) swapchain};
}

Result<gpu::TimeStampQuery, Status>
    DeviceInterface::create_timestamp_query(gpu::Device self_)
{
  Device *const self = (Device *) self_;

  VkQueryPoolCreateInfo create_info{
      .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
      .pNext              = nullptr,
      .flags              = 0,
      .queryType          = VK_QUERY_TYPE_TIMESTAMP,
      .queryCount         = 7,
      .pipelineStatistics = 0};
  VkQueryPool vk_pool;
  VkResult result = self->vk_table.CreateQueryPool(self->vk_dev, &create_info,
                                                   nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{(gpu::TimeStampQuery) vk_pool};
}

Result<gpu::StatisticsQuery, Status>
    DeviceInterface::create_statistics_query(gpu::Device self_)
{
  Device *const self = (Device *) self_;

  if (self->phy_dev.vk_features.pipelineStatisticsQuery != VK_TRUE)
  {
    return Err{Status::FeatureNotPresent};
  }

  constexpr VkQueryPipelineStatisticFlags QUERY_STATS =
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

  VkQueryPoolCreateInfo create_info{
      .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
      .pNext              = nullptr,
      .flags              = 0,
      .queryType          = VK_QUERY_TYPE_PIPELINE_STATISTICS,
      .queryCount         = 1,
      .pipelineStatistics = QUERY_STATS};

  VkQueryPool vk_pool;
  VkResult result = self->vk_table.CreateQueryPool(self->vk_dev, &create_info,
                                                   nullptr, &vk_pool);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{(gpu::StatisticsQuery) vk_pool};
}

void DeviceInterface::uninit_buffer(gpu::Device self_, gpu::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  if (buffer == nullptr)
  {
    return;
  }

  vmaDestroyBuffer(self->vma_allocator, buffer->vk_buffer,
                   buffer->vma_allocation);
  self->allocator.ndealloc(buffer, 1);
}

void DeviceInterface::uninit_buffer_view(gpu::Device     self_,
                                         gpu::BufferView buffer_view_)
{
  Device *const     self        = (Device *) self_;
  BufferView *const buffer_view = (BufferView *) buffer_view_;

  if (buffer_view == nullptr)
  {
    return;
  }

  self->vk_table.DestroyBufferView(self->vk_dev, buffer_view->vk_view, nullptr);
  self->allocator.ndealloc(buffer_view, 1);
}

void DeviceInterface::uninit_image(gpu::Device self_, gpu::Image image_)
{
  Device *const self  = (Device *) self_;
  Image *const  image = (Image *) image_;

  if (image == nullptr)
  {
    return;
  }

  CHECK(!image->is_swapchain_image);

  vmaDestroyImage(self->vma_allocator, image->vk_image, image->vma_allocation);
  self->allocator.ndealloc(image, 1);
}

void DeviceInterface::uninit_image_view(gpu::Device    self_,
                                        gpu::ImageView image_view_)
{
  Device *const    self       = (Device *) self_;
  ImageView *const image_view = (ImageView *) image_view_;

  if (image_view == nullptr)
  {
    return;
  }

  self->vk_table.DestroyImageView(self->vk_dev, image_view->vk_view, nullptr);
  self->allocator.ndealloc(image_view, 1);
}

void DeviceInterface::uninit_sampler(gpu::Device self_, gpu::Sampler sampler_)
{
  Device *const self = (Device *) self_;

  self->vk_table.DestroySampler(self->vk_dev, (Sampler) sampler_, nullptr);
}

void DeviceInterface::uninit_shader(gpu::Device self_, gpu::Shader shader_)
{
  Device *const self = (Device *) self_;

  self->vk_table.DestroyShaderModule(self->vk_dev, (Shader) shader_, nullptr);
}

void DeviceInterface::uninit_descriptor_set_layout(
    gpu::Device self_, gpu::DescriptorSetLayout layout_)
{
  Device *const              self   = (Device *) self_;
  DescriptorSetLayout *const layout = (DescriptorSetLayout *) layout_;

  if (layout == nullptr)
  {
    return;
  }

  self->vk_table.DestroyDescriptorSetLayout(self->vk_dev, layout->vk_layout,
                                            nullptr);
  self->allocator.ndealloc(layout, 1);
}

void DeviceInterface::uninit_descriptor_set(gpu::Device        self_,
                                            gpu::DescriptorSet set_)
{
  Device *const        self = (Device *) self_;
  DescriptorSet *const set  = (DescriptorSet *) set_;
  DescriptorHeap      &heap = self->descriptor_heap;

  if (set == nullptr)
  {
    return;
  }

  DescriptorPool *pool   = heap.pools + set->pool;
  VkResult        result = self->vk_table.FreeDescriptorSets(
      self->vk_dev, pool->vk_pool, 1, &set->vk_set);

  CHECK(result == VK_SUCCESS);

  for (u32 i = 0; i < set->num_bindings; i++)
  {
    pool->avail[(u32) set->bindings[i].type] += set->bindings[i].count;
  }

  for (u32 i = set->num_bindings; i-- > 0;)
  {
    if (set->bindings[i].sync_resources != nullptr)
    {
      heap.allocator.ndealloc(set->bindings[i].sync_resources,
                              set->num_bindings);
    }
  }
  heap.allocator.ndealloc(set, 1);
}

void DeviceInterface::uninit_pipeline_cache(gpu::Device        self_,
                                            gpu::PipelineCache cache_)
{
  Device *const self = (Device *) self_;
  self->vk_table.DestroyPipelineCache(self->vk_dev, (PipelineCache) cache_,
                                      nullptr);
}

void DeviceInterface::uninit_compute_pipeline(gpu::Device          self_,
                                              gpu::ComputePipeline pipeline_)
{
  Device *const          self     = (Device *) self_;
  ComputePipeline *const pipeline = (ComputePipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  self->vk_table.DestroyPipeline(self->vk_dev, pipeline->vk_pipeline, nullptr);
  self->vk_table.DestroyPipelineLayout(self->vk_dev, pipeline->vk_layout,
                                       nullptr);
  self->allocator.ndealloc(pipeline, 1);
}

void DeviceInterface::uninit_graphics_pipeline(gpu::Device           self_,
                                               gpu::GraphicsPipeline pipeline_)
{
  Device *const           self     = (Device *) self_;
  GraphicsPipeline *const pipeline = (GraphicsPipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  self->vk_table.DestroyPipeline(self->vk_dev, pipeline->vk_pipeline, nullptr);
  self->vk_table.DestroyPipelineLayout(self->vk_dev, pipeline->vk_layout,
                                       nullptr);
  self->allocator.ndealloc(pipeline, 1);
}

void DeviceInterface::uninit_swapchain(gpu::Device    self_,
                                       gpu::Swapchain swapchain_)
{
  Device *const    self      = (Device *) self_;
  Swapchain *const swapchain = (Swapchain *) swapchain_;

  if (swapchain == nullptr)
  {
    return;
  }

  self->vk_table.DestroySwapchainKHR(self->vk_dev, swapchain->vk_swapchain,
                                     nullptr);
  self->allocator.ndealloc(swapchain, 1);
}

void DeviceInterface::uninit_timestamp_query(gpu::Device         self_,
                                             gpu::TimeStampQuery query_)
{
  Device *const     self    = (Device *) self_;
  VkQueryPool const vk_pool = (VkQueryPool) query_;

  self->vk_table.DestroyQueryPool(self->vk_dev, vk_pool, nullptr);
}

void DeviceInterface::uninit_statistics_query(gpu::Device          self_,
                                              gpu::StatisticsQuery query_)
{
  Device *const     self    = (Device *) self_;
  VkQueryPool const vk_pool = (VkQueryPool) query_;

  self->vk_table.DestroyQueryPool(self->vk_dev, vk_pool, nullptr);
}

gpu::FrameContext DeviceInterface::get_frame_context(gpu::Device self_)
{
  FrameContext const &ctx = ((Device *) self_)->frame_ctx;

  return gpu::FrameContext{.buffering  = ctx.buffering,
                           .tail       = ctx.tail_frame,
                           .current    = ctx.current_frame,
                           .encoders   = Span{ctx.encs_impl, ctx.buffering},
                           .ring_index = ctx.ring_index};
}

Result<void *, Status> DeviceInterface::map_buffer_memory(gpu::Device self_,
                                                          gpu::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;
  CHECK(buffer->desc.host_mapped);

  void    *map;
  VkResult result =
      vmaMapMemory(self->vma_allocator, buffer->vma_allocation, &map);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{(void *) map};
}

void DeviceInterface::unmap_buffer_memory(gpu::Device self_,
                                          gpu::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  CHECK(buffer->desc.host_mapped);

  vmaUnmapMemory(self->vma_allocator, buffer->vma_allocation);
}

Result<Void, Status> DeviceInterface::invalidate_mapped_buffer_memory(
    gpu::Device self_, gpu::Buffer buffer_, gpu::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  CHECK(buffer->desc.host_mapped);
  CHECK(range.offset < buffer->desc.size);
  CHECK(range.size == gpu::WHOLE_SIZE ||
        (range.offset + range.size) <= buffer->desc.size);

  VkResult result = vmaInvalidateAllocation(
      self->vma_allocator, buffer->vma_allocation, range.offset, range.size);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::flush_mapped_buffer_memory(
    gpu::Device self_, gpu::Buffer buffer_, gpu::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  CHECK(buffer->desc.host_mapped);
  CHECK(range.offset < buffer->desc.size);
  CHECK(range.size == gpu::WHOLE_SIZE ||
        (range.offset + range.size) <= buffer->desc.size);

  VkResult result = vmaFlushAllocation(
      self->vma_allocator, buffer->vma_allocation, range.offset, range.size);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<usize, Status>
    DeviceInterface::get_pipeline_cache_size(gpu::Device        self_,
                                             gpu::PipelineCache cache)
{
  Device *const self = (Device *) self_;
  usize         size;

  VkResult result = self->vk_table.GetPipelineCacheData(
      self->vk_dev, (PipelineCache) cache, &size, nullptr);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{(usize) size};
}

Result<usize, Status> DeviceInterface::get_pipeline_cache_data(
    gpu::Device self_, gpu::PipelineCache cache, Span<u8> out)
{
  Device *const self = (Device *) self_;
  usize         size = out.size_bytes();

  VkResult result = self->vk_table.GetPipelineCacheData(
      self->vk_dev, (PipelineCache) cache, &size, out.data());
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{(usize) size};
}

Result<Void, Status>
    DeviceInterface::merge_pipeline_cache(gpu::Device                    self_,
                                          gpu::PipelineCache             dst,
                                          Span<gpu::PipelineCache const> srcs)
{
  Device *const self     = (Device *) self_;
  u32 const     num_srcs = srcs.size32();

  CHECK(num_srcs > 0);

  VkResult result = self->vk_table.MergePipelineCaches(
      self->vk_dev, (PipelineCache) dst, num_srcs,
      (PipelineCache *) srcs.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

void DeviceInterface::update_descriptor_set(
    gpu::Device self_, gpu::DescriptorSetUpdate const &update)
{
  Device *const         self = (Device *) self_;
  DescriptorHeap *const heap = &self->descriptor_heap;
  DescriptorSet *const  set  = (DescriptorSet *) update.set;
  u64 const             ubo_offset_alignment =
      self->phy_dev.vk_properties.limits.minUniformBufferOffsetAlignment;
  u64 const ssbo_offset_alignment =
      self->phy_dev.vk_properties.limits.minStorageBufferOffsetAlignment;

  CHECK(update.binding < set->num_bindings);
  DescriptorBinding &binding = set->bindings[update.binding];
  CHECK(update.element < binding.count);
  usize info_size = 0;
  u32   count     = 0;

  switch (binding.type)
  {
    case gpu::DescriptorType::DynamicStorageBuffer:
    case gpu::DescriptorType::StorageBuffer:
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        gpu::BufferBinding const &b      = update.buffers[i];
        Buffer                   *buffer = (Buffer *) b.buffer;
        if (buffer != nullptr)
        {
          CHECK(has_bits(buffer->desc.usage, gpu::BufferUsage::StorageBuffer));
          CHECK(is_valid_buffer_access(buffer->desc.size, b.offset, b.size,
                                       ubo_offset_alignment));
        }
      }
      break;

    case gpu::DescriptorType::DynamicUniformBuffer:
    case gpu::DescriptorType::UniformBuffer:
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        gpu::BufferBinding const &b      = update.buffers[i];
        Buffer                   *buffer = (Buffer *) b.buffer;
        if (buffer != nullptr)
        {
          CHECK(has_bits(buffer->desc.usage, gpu::BufferUsage::UniformBuffer));
          CHECK(is_valid_buffer_access(buffer->desc.size, b.offset, b.size,
                                       ssbo_offset_alignment));
        }
      }
      break;

    case gpu::DescriptorType::Sampler:
      break;

    case gpu::DescriptorType::SampledImage:
    case gpu::DescriptorType::CombinedImageSampler:
    case gpu::DescriptorType::InputAttachment:
    {
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        if (view != nullptr)
        {
          Image *image = (Image *) view->desc.image;
          CHECK(has_bits(image->desc.usage, gpu::ImageUsage::Sampled));
        }
      }
    }
    break;

    case gpu::DescriptorType::StorageImage:
    {
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        if (view != nullptr)
        {
          Image *image = (Image *) view->desc.image;
          CHECK(has_bits(image->desc.usage, gpu::ImageUsage::Storage));
        }
      }
    }
    break;

    case gpu::DescriptorType::StorageTexelBuffer:
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView *view = (BufferView *) update.texel_buffers[i];
        if (view != nullptr)
        {
          Buffer *buffer = (Buffer *) view->desc.buffer;
          CHECK(has_bits(buffer->desc.usage,
                         gpu::BufferUsage::StorageTexelBuffer));
        }
      }
      break;

    case gpu::DescriptorType::UniformTexelBuffer:
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView *view = (BufferView *) update.texel_buffers[i];
        if (view != nullptr)
        {
          Buffer *buffer = (Buffer *) view->desc.buffer;
          CHECK(has_bits(buffer->desc.usage,
                         gpu::BufferUsage::UniformTexelBuffer));
        }
      }
      break;

    default:
      UNREACHABLE();
  }

  switch (binding.type)
  {
    case gpu::DescriptorType::DynamicStorageBuffer:
    case gpu::DescriptorType::DynamicUniformBuffer:
    case gpu::DescriptorType::StorageBuffer:
    case gpu::DescriptorType::UniformBuffer:
      CHECK((update.element + update.buffers.size()) <= binding.count);
      info_size = sizeof(VkDescriptorBufferInfo) * update.buffers.size();
      count     = update.buffers.size32();
      break;

    case gpu::DescriptorType::StorageTexelBuffer:
    case gpu::DescriptorType::UniformTexelBuffer:
      CHECK((update.element + update.texel_buffers.size()) <= binding.count);
      info_size = sizeof(VkBufferView) * update.texel_buffers.size();
      count     = update.texel_buffers.size32();
      break;

    case gpu::DescriptorType::SampledImage:
    case gpu::DescriptorType::CombinedImageSampler:
    case gpu::DescriptorType::StorageImage:
    case gpu::DescriptorType::InputAttachment:
    case gpu::DescriptorType::Sampler:
      CHECK((update.element + update.images.size()) <= binding.count);
      info_size = sizeof(VkDescriptorImageInfo) * update.images.size();
      count     = update.images.size32();
      break;

    default:
      break;
  }

  if (count == 0)
  {
    return;
  }

  if (heap->scratch_size < info_size)
  {
    CHECK(heap->allocator.realloc(MAX_STANDARD_ALIGNMENT, heap->scratch_size,
                                  info_size, &heap->scratch));
    heap->scratch_size = info_size;
  }

  VkDescriptorImageInfo  *pImageInfo       = nullptr;
  VkDescriptorBufferInfo *pBufferInfo      = nullptr;
  VkBufferView           *pTexelBufferView = nullptr;

  switch (binding.type)
  {
    case gpu::DescriptorType::DynamicStorageBuffer:
    case gpu::DescriptorType::DynamicUniformBuffer:
    case gpu::DescriptorType::StorageBuffer:
    case gpu::DescriptorType::UniformBuffer:
    {
      pBufferInfo = (VkDescriptorBufferInfo *) heap->scratch;
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        gpu::BufferBinding const &b      = update.buffers[i];
        Buffer                   *buffer = (Buffer *) b.buffer;
        pBufferInfo[i]                   = VkDescriptorBufferInfo{
                              .buffer = (buffer == nullptr) ? nullptr : buffer->vk_buffer,
                              .offset = b.offset,
                              .range  = b.size};
      }
    }
    break;

    case gpu::DescriptorType::Sampler:
    {
      pImageInfo = (VkDescriptorImageInfo *) heap->scratch;
      for (u32 i = 0; i < update.images.size(); i++)
      {
        pImageInfo[i] =
            VkDescriptorImageInfo{.sampler = (Sampler) update.images[i].sampler,
                                  .imageView   = nullptr,
                                  .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED};
      }
    }
    break;

    case gpu::DescriptorType::SampledImage:
    {
      pImageInfo = (VkDescriptorImageInfo *) heap->scratch;
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        pImageInfo[i]   = VkDescriptorImageInfo{
              .sampler     = nullptr,
              .imageView   = (view == nullptr) ? nullptr : view->vk_view,
              .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      }
    }
    break;

    case gpu::DescriptorType::CombinedImageSampler:
    {
      pImageInfo = (VkDescriptorImageInfo *) heap->scratch;
      for (u32 i = 0; i < update.images.size(); i++)
      {
        gpu::ImageBinding const &b    = update.images[i];
        ImageView               *view = (ImageView *) b.image_view;
        pImageInfo[i]                 = VkDescriptorImageInfo{
                            .sampler     = (Sampler) b.sampler,
                            .imageView   = (view == nullptr) ? nullptr : view->vk_view,
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      }
    }
    break;

    case gpu::DescriptorType::StorageImage:
    {
      pImageInfo = (VkDescriptorImageInfo *) heap->scratch;
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        pImageInfo[i]   = VkDescriptorImageInfo{
              .sampler     = nullptr,
              .imageView   = (view == nullptr) ? nullptr : view->vk_view,
              .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
      }
    }
    break;

    case gpu::DescriptorType::InputAttachment:
    {
      pImageInfo = (VkDescriptorImageInfo *) heap->scratch;
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        pImageInfo[i]   = VkDescriptorImageInfo{
              .sampler     = nullptr,
              .imageView   = (view == nullptr) ? nullptr : view->vk_view,
              .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      }
    }
    break;

    case gpu::DescriptorType::StorageTexelBuffer:
    case gpu::DescriptorType::UniformTexelBuffer:
    {
      pTexelBufferView = (VkBufferView *) heap->scratch;
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView *view    = (BufferView *) update.texel_buffers[i];
        pTexelBufferView[i] = (view == nullptr) ? nullptr : view->vk_view;
      }
    }
    break;

    default:
      UNREACHABLE();
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = set->vk_set,
                                .dstBinding      = update.binding,
                                .dstArrayElement = update.element,
                                .descriptorCount = count,
                                .descriptorType =
                                    (VkDescriptorType) binding.type,
                                .pImageInfo       = pImageInfo,
                                .pBufferInfo      = pBufferInfo,
                                .pTexelBufferView = pTexelBufferView};

  self->vk_table.UpdateDescriptorSets(self->vk_dev, 1, &vk_write, 0, nullptr);

  switch (binding.type)
  {
    case gpu::DescriptorType::DynamicStorageBuffer:
    case gpu::DescriptorType::DynamicUniformBuffer:
    case gpu::DescriptorType::StorageBuffer:
    case gpu::DescriptorType::UniformBuffer:
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        binding.buffers[update.element + i] =
            (Buffer *) update.buffers[i].buffer;
      }
      break;

    case gpu::DescriptorType::StorageTexelBuffer:
    case gpu::DescriptorType::UniformTexelBuffer:
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView *view = (BufferView *) update.texel_buffers[i];
        binding.buffers[update.element + i] =
            (view == nullptr) ? nullptr : (Buffer *) view->desc.buffer;
      }
      break;

    case gpu::DescriptorType::Sampler:
      break;
    case gpu::DescriptorType::SampledImage:
    case gpu::DescriptorType::CombinedImageSampler:
    case gpu::DescriptorType::StorageImage:
    case gpu::DescriptorType::InputAttachment:
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        binding.images[update.element + i] =
            (view == nullptr) ? nullptr : (Image *) view->desc.image;
      }
      break;

    default:
      UNREACHABLE();
  }
}

Result<Void, Status> DeviceInterface::wait_idle(gpu::Device self_)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.DeviceWaitIdle(self->vk_dev);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::wait_queue_idle(gpu::Device self_)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.QueueWaitIdle(self->vk_queue);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<u32, Status> DeviceInterface::get_surface_formats(
    gpu::Device self_, gpu::Surface surface_, Span<gpu::SurfaceFormat> formats)
{
  Device *const      self    = (Device *) self_;
  VkSurfaceKHR const surface = (VkSurfaceKHR) surface_;

  u32      num_supported;
  VkResult result = self->instance->vk_table.GetPhysicalDeviceSurfaceFormatsKHR(
      self->phy_dev.vk_phy_dev, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkSurfaceFormatKHR *vk_formats;
  if (!self->allocator.nalloc(num_supported, &vk_formats))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_formats_{
      [&] { self->allocator.ndealloc(vk_formats, num_supported); }};

  {
    u32 num_read = num_supported;
    result       = self->instance->vk_table.GetPhysicalDeviceSurfaceFormatsKHR(
        self->phy_dev.vk_phy_dev, surface, &num_supported, vk_formats);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      return Err{(Status) result};
    }

    CHECK(num_read == num_supported && result != VK_INCOMPLETE);
  }

  u32 num_copies = min(num_supported, formats.size32());

  for (u32 i = 0; i < num_copies; i++)
  {
    formats[i].format      = (gpu::Format) vk_formats[i].format;
    formats[i].color_space = (gpu::ColorSpace) vk_formats[i].colorSpace;
  }

  return Ok{(u32) num_supported};
}

Result<u32, Status> DeviceInterface::get_surface_present_modes(
    gpu::Device self_, gpu::Surface surface_, Span<gpu::PresentMode> modes)
{
  Device *const      self    = (Device *) self_;
  VkSurfaceKHR const surface = (VkSurfaceKHR) surface_;

  u32      num_supported;
  VkResult result =
      self->instance->vk_table.GetPhysicalDeviceSurfacePresentModesKHR(
          self->phy_dev.vk_phy_dev, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkPresentModeKHR *vk_present_modes;
  if (!self->allocator.nalloc(num_supported, &vk_present_modes))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_present_modes_{
      [&] { self->allocator.ndealloc(vk_present_modes, num_supported); }};

  {
    u32 num_read = num_supported;
    result = self->instance->vk_table.GetPhysicalDeviceSurfacePresentModesKHR(
        self->phy_dev.vk_phy_dev, surface, &num_supported, vk_present_modes);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      return Err{(Status) result};
    }

    CHECK(num_read == num_supported && result != VK_INCOMPLETE);
  }

  u32 num_copies = min(num_supported, modes.size32());

  for (u32 i = 0; i < num_copies; i++)
  {
    modes[i] = (gpu::PresentMode) vk_present_modes[i];
  }

  return Ok{(u32) num_supported};
}

Result<gpu::SurfaceCapabilities, Status>
    DeviceInterface::get_surface_capabilities(gpu::Device  self_,
                                              gpu::Surface surface_)
{
  Device *const            self    = (Device *) self_;
  VkSurfaceKHR const       surface = (VkSurfaceKHR) surface_;
  VkSurfaceCapabilitiesKHR capabilities;
  VkResult                 result =
      self->instance->vk_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(
          self->phy_dev.vk_phy_dev, surface, &capabilities);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{gpu::SurfaceCapabilities{
      .image_usage = (gpu::ImageUsage) capabilities.supportedUsageFlags,
      .composite_alpha =
          (gpu::CompositeAlpha) capabilities.supportedCompositeAlpha}};
}

Result<gpu::SwapchainState, Status>
    DeviceInterface::get_swapchain_state(gpu::Device, gpu::Swapchain swapchain_)
{
  Swapchain *const swapchain = (Swapchain *) swapchain_;

  gpu::SwapchainState state{.extent = swapchain->extent,
                            .format = swapchain->desc.format,
                            .images =
                                Span{swapchain->images, swapchain->num_images}};

  if (swapchain->is_zero_sized)
  {
    state.current_image = None;
  }
  else
  {
    state.current_image = Some{swapchain->current_image};
  }
  return Ok{state};
}

Result<Void, Status>
    DeviceInterface::invalidate_swapchain(gpu::Device               self_,
                                          gpu::Swapchain            swapchain_,
                                          gpu::SwapchainDesc const &desc)
{
  (void) self_;
  CHECK(desc.preferred_extent.x > 0);
  CHECK(desc.preferred_extent.y > 0);
  Swapchain *const swapchain = (Swapchain *) swapchain_;
  swapchain->is_optimal      = false;
  swapchain->desc            = desc;
  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::begin_frame(gpu::Device    self_,
                                                  gpu::Swapchain swapchain_)
{
  Device *const    self         = (Device *) self_;
  FrameContext    &ctx          = self->frame_ctx;
  Swapchain *const swapchain    = (Swapchain *) swapchain_;
  VkFence          submit_fence = ctx.submit_f[ctx.ring_index];
  CommandEncoder  &enc          = ctx.encs[ctx.ring_index];

  CHECK(!enc.is_recording());

  VkResult result = self->vk_table.WaitForFences(self->vk_dev, 1, &submit_fence,
                                                 VK_TRUE, U64_MAX);

  CHECK(result == VK_SUCCESS);

  result = self->vk_table.ResetFences(self->vk_dev, 1, &submit_fence);

  CHECK(result == VK_SUCCESS);

  if (swapchain->is_out_of_date || !swapchain->is_optimal ||
      swapchain->vk_swapchain == nullptr)
  {
    // await all pending submitted operations on the device possibly using
    // the swapchain, to avoid destroying whilst in use
    result = self->vk_table.DeviceWaitIdle(self->vk_dev);
    CHECK(result == VK_SUCCESS);

    result = recreate_swapchain(self, swapchain);
    CHECK(result == VK_SUCCESS);
  }

  if (!swapchain->is_zero_sized)
  {
    u32 next_image;
    result = self->vk_table.AcquireNextImageKHR(
        self->vk_dev, swapchain->vk_swapchain, U64_MAX,
        ctx.acquire_s[ctx.ring_index], nullptr, &next_image);

    if (result == VK_SUBOPTIMAL_KHR)
    {
      swapchain->is_optimal = false;
    }
    else
    {
      CHECK(result == VK_SUCCESS);
    }

    swapchain->current_image = next_image;
  }

  self->vk_table.ResetCommandBuffer(enc.vk_command_buffer, 0);

  enc.reset_context();

  VkCommandBufferBeginInfo info{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext            = nullptr,
      .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr};
  result = self->vk_table.BeginCommandBuffer(enc.vk_command_buffer, &info);
  CHECK(result == VK_SUCCESS);

  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::submit_frame(gpu::Device    self_,
                                                   gpu::Swapchain swapchain_)
{
  Device *const         self              = (Device *) self_;
  FrameContext         &ctx               = self->frame_ctx;
  Swapchain *const      swapchain         = (Swapchain *) swapchain_;
  VkFence const         submit_fence      = ctx.submit_f[ctx.ring_index];
  CommandEncoder       &enc               = ctx.encs[ctx.ring_index];
  VkCommandBuffer const command_buffer    = enc.vk_command_buffer;
  VkSemaphore const     submit_semaphore  = ctx.submit_s[ctx.ring_index];
  VkSemaphore const     acquire_semaphore = ctx.acquire_s[ctx.ring_index];
  bool const            was_acquired      = !swapchain->is_zero_sized;
  bool const            can_present =
      !(swapchain->is_out_of_date || swapchain->is_zero_sized);

  CHECK(enc.is_recording());

  if (was_acquired)
  {
    access_image_all_aspects(enc,
                             swapchain->image_impls[swapchain->current_image],
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             VK_ACCESS_NONE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }

  VkResult result = self->vk_table.EndCommandBuffer(command_buffer);
  CHECK(result == VK_SUCCESS);
  CHECK(enc.status == gpu::Status::Success);

  VkPipelineStageFlags const wait_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  VkSubmitInfo submit_info{
      .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext                = nullptr,
      .waitSemaphoreCount   = was_acquired ? 1U : 0U,
      .pWaitSemaphores      = was_acquired ? &acquire_semaphore : nullptr,
      .pWaitDstStageMask    = was_acquired ? &wait_stages : nullptr,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &command_buffer,
      .signalSemaphoreCount = can_present ? 1U : 0U,
      .pSignalSemaphores    = can_present ? &submit_semaphore : nullptr};

  result =
      self->vk_table.QueueSubmit(self->vk_queue, 1, &submit_info, submit_fence);

  enc.state = CommandEncoderState::End;

  CHECK(result == VK_SUCCESS);

  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands could have been executed.
  ctx.current_frame++;
  ctx.tail_frame =
      max(ctx.current_frame, (gpu::FrameId) ctx.buffering) - ctx.buffering;
  ctx.ring_index = (ctx.ring_index + 1) % ctx.buffering;

  if (can_present)
  {
    VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                  .pNext = nullptr,
                                  .waitSemaphoreCount = 1,
                                  .pWaitSemaphores    = &submit_semaphore,
                                  .swapchainCount     = 1,
                                  .pSwapchains   = &swapchain->vk_swapchain,
                                  .pImageIndices = &swapchain->current_image,
                                  .pResults      = nullptr};
    result = self->vk_table.QueuePresentKHR(self->vk_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      swapchain->is_out_of_date = true;
    }
    else if (result == VK_SUBOPTIMAL_KHR)
    {
      swapchain->is_optimal = false;
    }
    else
    {
      CHECK(result == VK_SUCCESS);
    }
  }

  return Ok{Void{}};
}

Result<u64, Status>
    DeviceInterface::get_timestamp_query_result(gpu::Device         self_,
                                                gpu::TimeStampQuery query_)
{
  Device *const     self    = (Device *) self_;
  VkQueryPool const vk_pool = (VkQueryPool) query_;

  u64      timestamp;
  VkResult result = self->vk_table.GetQueryPoolResults(
      self->vk_dev, vk_pool, 0, 1, sizeof(u64), &timestamp, sizeof(u64),
      VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{timestamp};
}

Result<gpu::PipelineStatistics, Status>
    DeviceInterface::get_statistics_query_result(gpu::Device          self_,
                                                 gpu::StatisticsQuery query_)
{
  Device *const self = (Device *) self_;

  if (self->phy_dev.vk_features.pipelineStatisticsQuery != VK_TRUE)
  {
    return Err{Status::FeatureNotPresent};
  }

  VkQueryPool const vk_pool = (VkQueryPool) query_;

  gpu::PipelineStatistics stats;
  VkResult                result = self->vk_table.GetQueryPoolResults(
      self->vk_dev, vk_pool, 0, 1, sizeof(gpu::PipelineStatistics), &stats,
      sizeof(u64), VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{stats};
}

#define ENCODE_PRELUDE()                                 \
  CommandEncoder *const self = (CommandEncoder *) self_; \
  CHECK(self->is_recording());                           \
  if (self->status != Status::Success)                   \
  {                                                      \
    return;                                              \
  }                                                      \
  defer pool_reclaim_                                    \
  {                                                      \
    [&] { self->arg_pool.reclaim(); }                    \
  }

void CommandEncoderInterface::reset_timestamp_query(gpu::CommandEncoder self_,
                                                    gpu::TimeStampQuery query_)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  CHECK(!self->is_in_pass());

  self->dev->vk_table.CmdResetQueryPool(self->vk_command_buffer, vk_pool, 0, 1);
}

void CommandEncoderInterface::reset_statistics_query(
    gpu::CommandEncoder self_, gpu::StatisticsQuery query_)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  CHECK(!self->is_in_pass());

  self->dev->vk_table.CmdResetQueryPool(self->vk_command_buffer, vk_pool, 0, 1);
}

void CommandEncoderInterface::write_timestamp(gpu::CommandEncoder self_,
                                              gpu::TimeStampQuery query_)
{
  ENCODE_PRELUDE();
  CHECK(!self->is_in_pass());
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  self->dev->vk_table.CmdWriteTimestamp(self->vk_command_buffer,
                                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                        vk_pool, 0);
}

void CommandEncoderInterface::begin_statistics(gpu::CommandEncoder  self_,
                                               gpu::StatisticsQuery query_)
{
  ENCODE_PRELUDE();
  CHECK(!self->is_in_pass());
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  self->dev->vk_table.CmdBeginQuery(self->vk_command_buffer, vk_pool, 0, 0);
}

void CommandEncoderInterface::end_statistics(gpu::CommandEncoder  self_,
                                             gpu::StatisticsQuery query_)
{
  ENCODE_PRELUDE();
  CHECK(!self->is_in_pass());
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  self->dev->vk_table.CmdEndQuery(self->vk_command_buffer, vk_pool, 0);
}

void CommandEncoderInterface::begin_debug_marker(gpu::CommandEncoder self_,
                                                 Span<char const> region_name,
                                                 Vec4             color)
{
  ENCODE_PRELUDE();
  CHECK(!self->is_in_pass());
  CHECK(region_name.size() < 256);
  char region_name_cstr[256];
  to_c_str(region_name, span(region_name_cstr));

  VkDebugMarkerMarkerInfoEXT info{
      .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
      .pNext       = nullptr,
      .pMarkerName = region_name_cstr,
      .color       = {color.x, color.y, color.z, color.w}};
  self->dev->vk_table.CmdDebugMarkerBeginEXT(self->vk_command_buffer, &info);
}

void CommandEncoderInterface::end_debug_marker(gpu::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  CHECK(!self->is_in_pass());
  self->dev->vk_table.CmdDebugMarkerEndEXT(self->vk_command_buffer);
}

void CommandEncoderInterface::fill_buffer(gpu::CommandEncoder self_,
                                          gpu::Buffer dst_, u64 offset,
                                          u64 size, u32 data)
{
  ENCODE_PRELUDE();
  Buffer *const dst = (Buffer *) dst_;

  CHECK(!self->is_in_pass());
  CHECK(has_bits(dst->desc.usage, gpu::BufferUsage::TransferDst));
  CHECK(is_valid_buffer_access(dst->desc.size, offset, size, 4));
  CHECK(mem::is_aligned<u64>(4, size));

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);
  self->dev->vk_table.CmdFillBuffer(self->vk_command_buffer, dst->vk_buffer,
                                    offset, size, data);
}

void CommandEncoderInterface::copy_buffer(gpu::CommandEncoder self_,
                                          gpu::Buffer src_, gpu::Buffer dst_,
                                          Span<gpu::BufferCopy const> copies)
{
  ENCODE_PRELUDE();
  Buffer *const src        = (Buffer *) src_;
  Buffer *const dst        = (Buffer *) dst_;
  u32 const     num_copies = copies.size32();

  CHECK(!self->is_in_pass());
  CHECK(has_bits(src->desc.usage, gpu::BufferUsage::TransferSrc));
  CHECK(has_bits(dst->desc.usage, gpu::BufferUsage::TransferDst));
  CHECK(num_copies > 0);
  for (gpu::BufferCopy const &copy : copies)
  {
    CHECK(is_valid_buffer_access(src->desc.size, copy.src_offset, copy.size));
    CHECK(is_valid_buffer_access(dst->desc.size, copy.dst_offset, copy.size));
  }

  VkBufferCopy *vk_copies;

  if (!self->arg_pool.nalloc(num_copies, &vk_copies))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferCopy const &copy = copies[i];
    vk_copies[i]                = VkBufferCopy{.srcOffset = copy.src_offset,
                                               .dstOffset = copy.dst_offset,
                                               .size      = copy.size};
  }

  access_buffer(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT);
  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  self->dev->vk_table.CmdCopyBuffer(self->vk_command_buffer, src->vk_buffer,
                                    dst->vk_buffer, num_copies, vk_copies);
}

void CommandEncoderInterface::update_buffer(gpu::CommandEncoder self_,
                                            Span<u8 const> src, u64 dst_offset,
                                            gpu::Buffer dst_)
{
  ENCODE_PRELUDE();
  Buffer *const dst       = (Buffer *) dst_;
  u64 const     copy_size = src.size_bytes();

  CHECK(!self->is_in_pass());
  CHECK(has_bits(dst->desc.usage, gpu::BufferUsage::TransferDst));
  CHECK(is_valid_buffer_access(dst->desc.size, dst_offset, copy_size, 4));
  CHECK(mem::is_aligned<u64>(4, copy_size));
  CHECK(copy_size <= gpu::MAX_UPDATE_BUFFER_SIZE);

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  self->dev->vk_table.CmdUpdateBuffer(self->vk_command_buffer, dst->vk_buffer,
                                      dst_offset, (u64) src.size(), src.data());
}

void CommandEncoderInterface::clear_color_image(
    gpu::CommandEncoder self_, gpu::Image dst_, gpu::Color clear_color,
    Span<gpu::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  Image *const dst        = (Image *) dst_;
  u32 const    num_ranges = ranges.size32();

  static_assert(sizeof(gpu::Color) == sizeof(VkClearColorValue));
  CHECK(!self->is_in_pass());
  CHECK(has_bits(dst->desc.usage, gpu::ImageUsage::TransferDst));
  CHECK(num_ranges > 0);
  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const &range = ranges[i];
    CHECK(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        range.aspects, range.first_mip_level, range.num_mip_levels,
        range.first_array_layer, range.num_array_layers));
  }

  VkImageSubresourceRange *vk_ranges;
  if (!self->arg_pool.nalloc(num_ranges, &vk_ranges))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const &range = ranges[i];
    vk_ranges[i]                            = VkImageSubresourceRange{
                                   .aspectMask     = (VkImageAspectFlags) range.aspects,
                                   .baseMipLevel   = range.first_mip_level,
                                   .levelCount     = range.num_mip_levels,
                                   .baseArrayLayer = range.first_array_layer,
                                   .layerCount     = range.num_array_layers};
  }

  access_image_all_aspects(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkClearColorValue vk_clear_color;
  memcpy(&vk_clear_color, &clear_color, sizeof(VkClearColorValue));

  self->dev->vk_table.CmdClearColorImage(self->vk_command_buffer, dst->vk_image,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         &vk_clear_color, num_ranges,
                                         vk_ranges);
}

void CommandEncoderInterface::clear_depth_stencil_image(
    gpu::CommandEncoder self_, gpu::Image dst_,
    gpu::DepthStencil                      clear_depth_stencil,
    Span<gpu::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  Image *const dst        = (Image *) dst_;
  u32 const    num_ranges = ranges.size32();

  static_assert(sizeof(gpu::DepthStencil) == sizeof(VkClearDepthStencilValue));
  CHECK(!self->is_in_pass());
  CHECK(num_ranges > 0);
  CHECK(has_bits(dst->desc.usage, gpu::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const &range = ranges[i];
    CHECK(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        range.aspects, range.first_mip_level, range.num_mip_levels,
        range.first_array_layer, range.num_array_layers));
  }

  VkImageSubresourceRange *vk_ranges;
  if (!self->arg_pool.nalloc(num_ranges, &vk_ranges))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const &range = ranges[i];
    vk_ranges[i]                            = VkImageSubresourceRange{
                                   .aspectMask     = (VkImageAspectFlags) range.aspects,
                                   .baseMipLevel   = range.first_mip_level,
                                   .levelCount     = range.num_mip_levels,
                                   .baseArrayLayer = range.first_array_layer,
                                   .layerCount     = range.num_array_layers};
  }

  access_image_all_aspects(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkClearDepthStencilValue vk_clear_depth_stencil;
  memcpy(&vk_clear_depth_stencil, &clear_depth_stencil,
         sizeof(gpu::DepthStencil));

  self->dev->vk_table.CmdClearDepthStencilImage(
      self->vk_command_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_clear_depth_stencil, num_ranges,
      vk_ranges);
}

void CommandEncoderInterface::copy_image(gpu::CommandEncoder self_,
                                         gpu::Image src_, gpu::Image dst_,
                                         Span<gpu::ImageCopy const> copies)
{
  ENCODE_PRELUDE();
  Image *const src        = (Image *) src_;
  Image *const dst        = (Image *) dst_;
  u32 const    num_copies = copies.size32();

  CHECK(!self->is_in_pass());
  CHECK(num_copies > 0);
  CHECK(has_bits(src->desc.usage, gpu::ImageUsage::TransferSrc));
  CHECK(has_bits(dst->desc.usage, gpu::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::ImageCopy const &copy = copies[i];

    CHECK(is_valid_image_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        copy.src_layers.aspects, copy.src_layers.mip_level, 1,
        copy.src_layers.first_array_layer, copy.src_layers.num_array_layers));
    CHECK(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        copy.dst_layers.aspects, copy.dst_layers.mip_level, 1,
        copy.dst_layers.first_array_layer, copy.dst_layers.num_array_layers));

    gpu::Extent3D src_extent =
        mip_down(src->desc.extent, copy.src_layers.mip_level);
    gpu::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.dst_layers.mip_level);
    CHECK(copy.extent.x > 0);
    CHECK(copy.extent.y > 0);
    CHECK(copy.extent.z > 0);
    CHECK(copy.src_offset.x <= src_extent.x);
    CHECK(copy.src_offset.y <= src_extent.y);
    CHECK(copy.src_offset.z <= src_extent.z);
    CHECK((copy.src_offset.x + copy.extent.x) <= src_extent.x);
    CHECK((copy.src_offset.y + copy.extent.y) <= src_extent.y);
    CHECK((copy.src_offset.z + copy.extent.z) <= src_extent.z);
    CHECK(copy.dst_offset.x <= dst_extent.x);
    CHECK(copy.dst_offset.y <= dst_extent.y);
    CHECK(copy.dst_offset.z <= dst_extent.z);
    CHECK((copy.dst_offset.x + copy.extent.x) <= dst_extent.x);
    CHECK((copy.dst_offset.y + copy.extent.y) <= dst_extent.y);
    CHECK((copy.dst_offset.z + copy.extent.z) <= dst_extent.z);
  }

  VkImageCopy *vk_copies;
  if (!self->arg_pool.nalloc(num_copies, &vk_copies))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::ImageCopy const    &copy = copies[i];
    VkImageSubresourceLayers src_subresource{
        .aspectMask     = (VkImageAspectFlags) copy.src_layers.aspects,
        .mipLevel       = copy.src_layers.mip_level,
        .baseArrayLayer = copy.src_layers.first_array_layer,
        .layerCount     = copy.src_layers.num_array_layers};
    VkOffset3D src_offset{(i32) copy.src_offset.x, (i32) copy.src_offset.y,
                          (i32) copy.src_offset.z};
    VkImageSubresourceLayers dst_subresource{
        .aspectMask     = (VkImageAspectFlags) copy.dst_layers.aspects,
        .mipLevel       = copy.dst_layers.mip_level,
        .baseArrayLayer = copy.dst_layers.first_array_layer,
        .layerCount     = copy.dst_layers.num_array_layers};
    VkOffset3D dst_offset{(i32) copy.dst_offset.x, (i32) copy.dst_offset.y,
                          (i32) copy.dst_offset.z};
    VkExtent3D extent{copy.extent.x, copy.extent.y, copy.extent.z};

    vk_copies[i] = VkImageCopy{.srcSubresource = src_subresource,
                               .srcOffset      = src_offset,
                               .dstSubresource = dst_subresource,
                               .dstOffset      = dst_offset,
                               .extent         = extent};
  }

  access_image_all_aspects(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image_all_aspects(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  self->dev->vk_table.CmdCopyImage(
      self->vk_command_buffer, src->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoderInterface::copy_buffer_to_image(
    gpu::CommandEncoder self_, gpu::Buffer src_, gpu::Image dst_,
    Span<gpu::BufferImageCopy const> copies)
{
  ENCODE_PRELUDE();
  Buffer *const src        = (Buffer *) src_;
  Image *const  dst        = (Image *) dst_;
  u32 const     num_copies = copies.size32();

  CHECK(!self->is_in_pass());
  CHECK(num_copies > 0);
  CHECK(has_bits(src->desc.usage, gpu::BufferUsage::TransferSrc));
  CHECK(has_bits(dst->desc.usage, gpu::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferImageCopy const &copy = copies[i];
    CHECK(is_valid_buffer_access(src->desc.size, copy.buffer_offset,
                                 gpu::WHOLE_SIZE));

    CHECK(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        copy.image_layers.aspects, copy.image_layers.mip_level, 1,
        copy.image_layers.first_array_layer,
        copy.image_layers.num_array_layers));

    CHECK(copy.image_extent.x > 0);
    CHECK(copy.image_extent.y > 0);
    CHECK(copy.image_extent.z > 0);
    gpu::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.image_layers.mip_level);
    CHECK(copy.image_extent.x <= dst_extent.x);
    CHECK(copy.image_extent.y <= dst_extent.y);
    CHECK(copy.image_extent.z <= dst_extent.z);
    CHECK((copy.image_offset.x + copy.image_extent.x) <= dst_extent.x);
    CHECK((copy.image_offset.y + copy.image_extent.y) <= dst_extent.y);
    CHECK((copy.image_offset.z + copy.image_extent.z) <= dst_extent.z);
  }

  VkBufferImageCopy *vk_copies;
  if (!self->arg_pool.nalloc(num_copies, &vk_copies))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferImageCopy const &copy = copies[i];
    VkImageSubresourceLayers    image_subresource{
           .aspectMask     = (VkImageAspectFlags) copy.image_layers.aspects,
           .mipLevel       = copy.image_layers.mip_level,
           .baseArrayLayer = copy.image_layers.first_array_layer,
           .layerCount     = copy.image_layers.num_array_layers};
    vk_copies[i] = VkBufferImageCopy{
        .bufferOffset      = copy.buffer_offset,
        .bufferRowLength   = copy.buffer_row_length,
        .bufferImageHeight = copy.buffer_image_height,
        .imageSubresource  = image_subresource,
        .imageOffset =
            VkOffset3D{(i32) copy.image_offset.x, (i32) copy.image_offset.y,
                       (i32) copy.image_offset.z},
        .imageExtent = VkExtent3D{copy.image_extent.x, copy.image_extent.y,
                                  copy.image_extent.z}};
  }

  access_buffer(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT);
  access_image_all_aspects(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  self->dev->vk_table.CmdCopyBufferToImage(
      self->vk_command_buffer, src->vk_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoderInterface::blit_image(gpu::CommandEncoder self_,
                                         gpu::Image src_, gpu::Image dst_,
                                         Span<gpu::ImageBlit const> blits,
                                         gpu::Filter                filter)
{
  ENCODE_PRELUDE();
  Image *const src       = (Image *) src_;
  Image *const dst       = (Image *) dst_;
  u32 const    num_blits = blits.size32();

  CHECK(!self->is_in_pass());
  CHECK(num_blits > 0);
  CHECK(has_bits(src->desc.usage, gpu::ImageUsage::TransferSrc));
  CHECK(has_bits(dst->desc.usage, gpu::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_blits; i++)
  {
    gpu::ImageBlit const &blit = blits[i];

    CHECK(is_valid_image_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        blit.src_layers.aspects, blit.src_layers.mip_level, 1,
        blit.src_layers.first_array_layer, blit.src_layers.num_array_layers));

    CHECK(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        blit.dst_layers.aspects, blit.dst_layers.mip_level, 1,
        blit.dst_layers.first_array_layer, blit.dst_layers.num_array_layers));

    gpu::Extent3D src_extent =
        mip_down(src->desc.extent, blit.src_layers.mip_level);
    gpu::Extent3D dst_extent =
        mip_down(dst->desc.extent, blit.dst_layers.mip_level);
    CHECK(blit.src_offsets[0].x <= src_extent.x);
    CHECK(blit.src_offsets[0].y <= src_extent.y);
    CHECK(blit.src_offsets[0].z <= src_extent.z);
    CHECK(blit.src_offsets[1].x <= src_extent.x);
    CHECK(blit.src_offsets[1].y <= src_extent.y);
    CHECK(blit.src_offsets[1].z <= src_extent.z);
    CHECK(blit.dst_offsets[0].x <= dst_extent.x);
    CHECK(blit.dst_offsets[0].y <= dst_extent.y);
    CHECK(blit.dst_offsets[0].z <= dst_extent.z);
    CHECK(blit.dst_offsets[1].x <= dst_extent.x);
    CHECK(blit.dst_offsets[1].y <= dst_extent.y);
    CHECK(blit.dst_offsets[1].z <= dst_extent.z);
    CHECK(!((src->desc.type == gpu::ImageType::Type1D) &&
            (blit.src_offsets[0].y != 0 || blit.src_offsets[1].y != 1)));
    CHECK(!((src->desc.type == gpu::ImageType::Type1D ||
             src->desc.type == gpu::ImageType::Type2D) &&
            (blit.src_offsets[0].z != 0 || blit.src_offsets[1].z != 1)));
    CHECK(!((dst->desc.type == gpu::ImageType::Type1D) &&
            (blit.dst_offsets[0].y != 0 || blit.dst_offsets[1].y != 1)));
    CHECK(!((dst->desc.type == gpu::ImageType::Type1D ||
             dst->desc.type == gpu::ImageType::Type2D) &&
            (blit.src_offsets[0].z != 0 || blit.dst_offsets[1].z != 1)));
  }

  VkImageBlit *vk_blits;
  if (!self->arg_pool.nalloc(num_blits, &vk_blits))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_blits; i++)
  {
    gpu::ImageBlit const    &blit = blits[i];
    VkImageSubresourceLayers src_subresource{
        .aspectMask     = (VkImageAspectFlags) blit.src_layers.aspects,
        .mipLevel       = blit.src_layers.mip_level,
        .baseArrayLayer = blit.src_layers.first_array_layer,
        .layerCount     = blit.src_layers.num_array_layers};
    VkImageSubresourceLayers dst_subresource{
        .aspectMask     = (VkImageAspectFlags) blit.dst_layers.aspects,
        .mipLevel       = blit.dst_layers.mip_level,
        .baseArrayLayer = blit.dst_layers.first_array_layer,
        .layerCount     = blit.dst_layers.num_array_layers};
    vk_blits[i] = VkImageBlit{
        .srcSubresource = src_subresource,
        .srcOffsets     = {VkOffset3D{(i32) blit.src_offsets[0].x,
                                  (i32) blit.src_offsets[0].y,
                                  (i32) blit.src_offsets[0].z},
                           VkOffset3D{(i32) blit.src_offsets[1].x,
                                  (i32) blit.src_offsets[1].y,
                                  (i32) blit.src_offsets[1].z}},
        .dstSubresource = dst_subresource,
        .dstOffsets     = {
            VkOffset3D{(i32) blit.dst_offsets[0].x, (i32) blit.dst_offsets[0].y,
                       (i32) blit.dst_offsets[0].z},
            VkOffset3D{(i32) blit.dst_offsets[1].x, (i32) blit.dst_offsets[1].y,
                       (i32) blit.dst_offsets[1].z}}};
  }

  access_image_all_aspects(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image_all_aspects(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  self->dev->vk_table.CmdBlitImage(self->vk_command_buffer, src->vk_image,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   dst->vk_image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   num_blits, vk_blits, (VkFilter) filter);
}

void CommandEncoderInterface::resolve_image(
    gpu::CommandEncoder self_, gpu::Image src_, gpu::Image dst_,
    Span<gpu::ImageResolve const> resolves)
{
  ENCODE_PRELUDE();
  Image *const src          = (Image *) src_;
  Image *const dst          = (Image *) dst_;
  u32 const    num_resolves = resolves.size32();

  CHECK(!self->is_in_pass());
  CHECK(num_resolves > 0);
  CHECK(has_bits(src->desc.usage, gpu::ImageUsage::TransferSrc));
  CHECK(has_bits(dst->desc.usage, gpu::ImageUsage::TransferDst));
  CHECK(has_bits(dst->desc.sample_count, gpu::SampleCount::Count1));

  for (u32 i = 0; i < num_resolves; i++)
  {
    gpu::ImageResolve const &resolve = resolves[i];

    CHECK(is_valid_image_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        resolve.src_layers.aspects, resolve.src_layers.mip_level, 1,
        resolve.src_layers.first_array_layer,
        resolve.src_layers.num_array_layers));
    CHECK(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        resolve.dst_layers.aspects, resolve.dst_layers.mip_level, 1,
        resolve.dst_layers.first_array_layer,
        resolve.dst_layers.num_array_layers));

    gpu::Extent3D src_extent =
        mip_down(src->desc.extent, resolve.src_layers.mip_level);
    gpu::Extent3D dst_extent =
        mip_down(dst->desc.extent, resolve.dst_layers.mip_level);
    CHECK(resolve.extent.x > 0);
    CHECK(resolve.extent.y > 0);
    CHECK(resolve.extent.z > 0);
    CHECK(resolve.src_offset.x <= src_extent.x);
    CHECK(resolve.src_offset.y <= src_extent.y);
    CHECK(resolve.src_offset.z <= src_extent.z);
    CHECK((resolve.src_offset.x + resolve.extent.x) <= src_extent.x);
    CHECK((resolve.src_offset.y + resolve.extent.y) <= src_extent.y);
    CHECK((resolve.src_offset.z + resolve.extent.z) <= src_extent.z);
    CHECK(resolve.dst_offset.x <= dst_extent.x);
    CHECK(resolve.dst_offset.y <= dst_extent.y);
    CHECK(resolve.dst_offset.z <= dst_extent.z);
    CHECK((resolve.dst_offset.x + resolve.extent.x) <= dst_extent.x);
    CHECK((resolve.dst_offset.y + resolve.extent.y) <= dst_extent.y);
    CHECK((resolve.dst_offset.z + resolve.extent.z) <= dst_extent.z);
  }

  VkImageResolve *vk_resolves;
  if (!self->arg_pool.nalloc<VkImageResolve>(num_resolves, &vk_resolves))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_resolves; i++)
  {
    gpu::ImageResolve const &resolve = resolves[i];
    VkImageSubresourceLayers src_subresource{
        .aspectMask     = (VkImageAspectFlags) resolve.src_layers.aspects,
        .mipLevel       = resolve.src_layers.mip_level,
        .baseArrayLayer = resolve.src_layers.first_array_layer,
        .layerCount     = resolve.src_layers.num_array_layers};
    VkOffset3D               src_offset{(i32) resolve.src_offset.x,
                          (i32) resolve.src_offset.y,
                          (i32) resolve.src_offset.z};
    VkImageSubresourceLayers dst_subresource{
        .aspectMask     = (VkImageAspectFlags) resolve.dst_layers.aspects,
        .mipLevel       = resolve.dst_layers.mip_level,
        .baseArrayLayer = resolve.dst_layers.first_array_layer,
        .layerCount     = resolve.dst_layers.num_array_layers};
    VkOffset3D dst_offset{(i32) resolve.dst_offset.x,
                          (i32) resolve.dst_offset.y,
                          (i32) resolve.dst_offset.z};
    VkExtent3D extent{resolve.extent.x, resolve.extent.y, resolve.extent.z};

    vk_resolves[i] = VkImageResolve{.srcSubresource = src_subresource,
                                    .srcOffset      = src_offset,
                                    .dstSubresource = dst_subresource,
                                    .dstOffset      = dst_offset,
                                    .extent         = extent};
  }

  access_image_all_aspects(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image_all_aspects(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  self->dev->vk_table.CmdResolveImage(
      self->vk_command_buffer, src->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_resolves, vk_resolves);
}

void CommandEncoderInterface::begin_compute_pass(gpu::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  CHECK(!self->is_in_pass());

  self->state = CommandEncoderState::ComputePass;
}

void CommandEncoderInterface::end_compute_pass(gpu::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  CHECK(self->is_in_compute_pass());

  self->reset_context();
}

void validate_attachment(gpu::RenderingAttachment const &info,
                         gpu::ImageAspects aspects, gpu::ImageUsage usage)
{
  CHECK(
      !(info.resolve_mode != gpu::ResolveModes::None && info.view == nullptr));
  CHECK(!(info.resolve_mode != gpu::ResolveModes::None &&
          info.resolve == nullptr));
  if (info.view != nullptr)
  {
    CHECK(has_bits(IMAGE_FROM_VIEW(info.view)->desc.aspects, aspects));
    CHECK(has_bits(IMAGE_FROM_VIEW(info.view)->desc.usage, usage));
    CHECK(!(info.resolve_mode != gpu::ResolveModes::None &&
            IMAGE_FROM_VIEW(info.view)->desc.sample_count ==
                gpu::SampleCount::Count1));
  }
  if (info.resolve != nullptr)
  {
    CHECK(has_bits(IMAGE_FROM_VIEW(info.resolve)->desc.aspects, aspects));
    CHECK(has_bits(IMAGE_FROM_VIEW(info.resolve)->desc.usage, usage));
    CHECK(IMAGE_FROM_VIEW(info.resolve)->desc.sample_count ==
          gpu::SampleCount::Count1);
  }
}

void CommandEncoderInterface::begin_rendering(gpu::CommandEncoder       self_,
                                              gpu::RenderingInfo const &info)
{
  ENCODE_PRELUDE();
  u32 const num_color_attachments   = info.color_attachments.size32();
  u32 const num_depth_attachments   = info.depth_attachment.size32();
  u32 const num_stencil_attachments = info.stencil_attachment.size32();

  CHECK(!self->is_in_pass());
  CHECK(num_color_attachments <= gpu::MAX_PIPELINE_COLOR_ATTACHMENTS);
  CHECK(num_depth_attachments <= 1);
  CHECK(num_stencil_attachments <= 1);
  CHECK(info.render_area.extent.x > 0);
  CHECK(info.render_area.extent.y > 0);
  CHECK(info.num_layers > 0);

  for (gpu::RenderingAttachment const &attachment : info.color_attachments)
  {
    validate_attachment(attachment, gpu::ImageAspects::Color,
                        gpu::ImageUsage::ColorAttachment);
  }

  for (gpu::RenderingAttachment const &attachment : info.depth_attachment)
  {
    validate_attachment(attachment, gpu::ImageAspects::Depth,
                        gpu::ImageUsage::DepthStencilAttachment);
  }

  for (gpu::RenderingAttachment const &attachment : info.stencil_attachment)
  {
    validate_attachment(attachment, gpu::ImageAspects::Stencil,
                        gpu::ImageUsage::DepthStencilAttachment);
  }

  self->reset_context();
  mem::copy(info.color_attachments, self->render_ctx.color_attachments);
  mem::copy(info.depth_attachment, self->render_ctx.depth_attachment);
  mem::copy(info.stencil_attachment, self->render_ctx.stencil_attachment);
  self->state                              = CommandEncoderState::RenderPass;
  self->render_ctx.render_area             = info.render_area;
  self->render_ctx.num_layers              = info.num_layers;
  self->render_ctx.num_color_attachments   = num_color_attachments;
  self->render_ctx.num_depth_attachments   = num_depth_attachments;
  self->render_ctx.num_stencil_attachments = num_stencil_attachments;
}

constexpr VkAccessFlags
    color_attachment_access(gpu::RenderingAttachment const &attachment)
{
  VkAccessFlags access = VK_ACCESS_NONE;

  if (attachment.load_op == gpu::LoadOp::Clear ||
      attachment.load_op == gpu::LoadOp::DontCare ||
      attachment.store_op == gpu::StoreOp::Store)
  {
    access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }

  if (attachment.load_op == gpu::LoadOp::Load)
  {
    access |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  }

  return access;
}

constexpr VkAccessFlags
    depth_attachment_access(gpu::RenderingAttachment const &attachment)
{
  VkAccessFlags access = VK_ACCESS_NONE;

  if (attachment.load_op == gpu::LoadOp::Clear ||
      attachment.load_op == gpu::LoadOp::DontCare ||
      attachment.store_op == gpu::StoreOp::Store ||
      attachment.store_op == gpu::StoreOp::DontCare)
  {
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  }

  if (attachment.load_op == gpu::LoadOp::Load)
  {
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  }

  return access;
}

constexpr VkAccessFlags
    stencil_attachment_access(gpu::RenderingAttachment const &attachment)
{
  return depth_attachment_access(attachment);
}

void CommandEncoderInterface::end_rendering(gpu::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;
  DeviceTable const *t   = &self->dev->vk_table;

  CHECK(self->is_in_render_pass());

  for (Command const &cmd : self->render_ctx.commands)
  {
    switch (cmd.type)
    {
      case CommandType::BindDescriptorSets:
      {
        for (u32 i = 0; i < cmd.set.v1; i++)
        {
          access_graphics_bindings(*self, *cmd.set.v0[i]);
        }
      }
      break;
      case CommandType::BindVertexBuffer:
      {
        access_buffer(*self, *cmd.vertex_buffer.v1,
                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
      }
      break;
      case CommandType::BindIndexBuffer:
      {
        access_buffer(*self, *cmd.index_buffer.v0,
                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_INDEX_READ_BIT);
      }
      break;
      case CommandType::DrawIndirect:
      case CommandType::DrawIndexedIndirect:
      {
        access_buffer(*self, *cmd.draw_indirect.v0,
                      VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                      VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
      }
      break;
      default:
        break;
    }
  }

  {
    VkRenderingAttachmentInfoKHR
        vk_color_attachments[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS];
    VkRenderingAttachmentInfoKHR vk_depth_attachment[1];
    VkRenderingAttachmentInfoKHR vk_stencil_attachment[1];

    constexpr VkPipelineStageFlags RESOLVE_STAGE =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    constexpr VkAccessFlags RESOLVE_SRC_ACCESS =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    constexpr VkAccessFlags RESOLVE_DST_ACCESS =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    constexpr VkImageLayout RESOLVE_COLOR_LAYOUT =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    constexpr VkImageLayout RESOLVE_DEPTH_LAYOUT =
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
    constexpr VkImageLayout RESOLVE_STENCIL_LAYOUT =
        VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;

    for (u32 i = 0; i < ctx.num_color_attachments; i++)
    {
      gpu::RenderingAttachment const &attachment = ctx.color_attachments[i];
      VkAccessFlags        access     = color_attachment_access(attachment);
      VkImageView          vk_view    = nullptr;
      VkImageView          vk_resolve = nullptr;
      VkPipelineStageFlags stages =
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkClearValue  clear_value;
      memcpy(&clear_value, &attachment.clear, sizeof(VkClearValue));

      if (attachment.resolve_mode != gpu::ResolveModes::None)
      {
        access |= RESOLVE_SRC_ACCESS;
        stages |= RESOLVE_STAGE;
      }

      if (attachment.view != nullptr)
      {
        ImageView *view    = (ImageView *) attachment.view;
        ImageView *resolve = (ImageView *) attachment.resolve;
        vk_view            = view->vk_view;
        if (attachment.resolve_mode != gpu::ResolveModes::None)
        {
          vk_resolve = resolve->vk_view;
          access_image_aspect(*self, *IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                              RESOLVE_DST_ACCESS, RESOLVE_COLOR_LAYOUT,
                              gpu::ImageAspects::Color, COLOR_ASPECT_IDX);
        }

        access_image_aspect(*self, *IMAGE_FROM_VIEW(view), stages, access,
                            layout, gpu::ImageAspects::Color, COLOR_ASPECT_IDX);
      }

      vk_color_attachments[i] = VkRenderingAttachmentInfoKHR{
          .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
          .pNext              = nullptr,
          .imageView          = vk_view,
          .imageLayout        = layout,
          .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
          .resolveImageView   = vk_resolve,
          .resolveImageLayout = RESOLVE_COLOR_LAYOUT,
          .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
          .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
          .clearValue         = clear_value};
    }

    for (u32 i = 0; i < ctx.num_depth_attachments; i++)
    {
      gpu::RenderingAttachment const &attachment = ctx.depth_attachment[i];
      VkAccessFlags        access     = depth_attachment_access(attachment);
      VkImageView          vk_view    = nullptr;
      VkImageView          vk_resolve = nullptr;
      VkImageLayout        layout     = has_write_access(access) ?
                                            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR :
                                            VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
      VkPipelineStageFlags stages     = 0;
      if (has_read_access(access))
      {
        stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
      }
      if (has_write_access(access))
      {
        stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      }

      if (attachment.resolve_mode != gpu::ResolveModes::None)
      {
        access |= RESOLVE_SRC_ACCESS;
        stages |= RESOLVE_STAGE;
      }

      VkClearValue clear_value;

      memcpy(&clear_value, &attachment.clear, sizeof(VkClearValue));

      if (attachment.view != nullptr)
      {
        ImageView *view    = (ImageView *) attachment.view;
        ImageView *resolve = (ImageView *) attachment.resolve;
        vk_view            = view->vk_view;
        if (attachment.resolve_mode != gpu::ResolveModes::None)
        {
          vk_resolve = resolve->vk_view;
          access_image_aspect(*self, *IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                              RESOLVE_DST_ACCESS, RESOLVE_DEPTH_LAYOUT,
                              gpu::ImageAspects::Depth, DEPTH_ASPECT_IDX);
        }

        access_image_aspect(*self, *IMAGE_FROM_VIEW(view), stages, access,
                            layout, gpu::ImageAspects::Depth, DEPTH_ASPECT_IDX);
      }

      vk_depth_attachment[i] = VkRenderingAttachmentInfoKHR{
          .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
          .pNext              = nullptr,
          .imageView          = vk_view,
          .imageLayout        = layout,
          .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
          .resolveImageView   = vk_resolve,
          .resolveImageLayout = RESOLVE_DEPTH_LAYOUT,
          .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
          .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
          .clearValue         = clear_value};
    }

    for (u32 i = 0; i < ctx.num_stencil_attachments; i++)
    {
      gpu::RenderingAttachment const &attachment = ctx.stencil_attachment[i];
      VkAccessFlags access     = stencil_attachment_access(attachment);
      VkImageView   vk_view    = nullptr;
      VkImageView   vk_resolve = nullptr;
      VkImageLayout layout =
          has_write_access(access) ?
              VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR :
              VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
      VkPipelineStageFlags stages = 0;
      if (has_read_access(access))
      {
        stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
      }
      if (has_write_access(access))
      {
        stages |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      }

      if (attachment.resolve_mode != gpu::ResolveModes::None)
      {
        access |= RESOLVE_SRC_ACCESS;
        stages |= RESOLVE_STAGE;
      }

      VkClearValue clear_value;

      memcpy(&clear_value, &attachment.clear, sizeof(VkClearValue));

      if (attachment.view != nullptr)
      {
        ImageView *view    = (ImageView *) attachment.view;
        ImageView *resolve = (ImageView *) attachment.resolve;
        vk_view            = view->vk_view;
        if (attachment.resolve_mode != gpu::ResolveModes::None)
        {
          vk_resolve = resolve->vk_view;
          access_image_aspect(*self, *IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                              RESOLVE_DST_ACCESS, RESOLVE_STENCIL_LAYOUT,
                              gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);
        }

        access_image_aspect(*self, *IMAGE_FROM_VIEW(view), stages, access,
                            layout, gpu::ImageAspects::Stencil,
                            STENCIL_ASPECT_IDX);
      }

      vk_stencil_attachment[i] = VkRenderingAttachmentInfoKHR{
          .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
          .pNext              = nullptr,
          .imageView          = vk_view,
          .imageLayout        = layout,
          .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
          .resolveImageView   = vk_resolve,
          .resolveImageLayout = RESOLVE_STENCIL_LAYOUT,
          .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
          .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
          .clearValue         = clear_value};
    }

    VkRenderingInfoKHR begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .renderArea =
            VkRect2D{.offset = VkOffset2D{.x = (i32) ctx.render_area.offset.x,
                                          .y = (i32) ctx.render_area.offset.y},
                     .extent = VkExtent2D{.width  = ctx.render_area.extent.x,
                                          .height = ctx.render_area.extent.y}},
        .layerCount           = ctx.num_layers,
        .viewMask             = 0,
        .colorAttachmentCount = ctx.num_color_attachments,
        .pColorAttachments    = vk_color_attachments,
        .pDepthAttachment =
            (ctx.num_depth_attachments == 0) ? nullptr : vk_depth_attachment,
        .pStencilAttachment = (ctx.num_stencil_attachments == 0) ?
                                  nullptr :
                                  vk_stencil_attachment};

    t->CmdBeginRenderingKHR(self->vk_command_buffer, &begin_info);
  }

  GraphicsPipeline *pipeline = nullptr;

  for (Command const &cmd : ctx.commands)
  {
    switch (cmd.type)
    {
      case CommandType::BindDescriptorSets:
      {
        VkDescriptorSet vk_sets[gpu::MAX_PIPELINE_DESCRIPTOR_SETS];
        for (u32 i = 0; i < cmd.set.v1; i++)
        {
          vk_sets[i] = cmd.set.v0[i]->vk_set;
        }

        t->CmdBindDescriptorSets(self->vk_command_buffer,
                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 pipeline->vk_layout, 0, cmd.set.v1, vk_sets,
                                 cmd.set.v3, cmd.set.v2);
      }
      break;
      case CommandType::BindPipeline:
      {
        pipeline = cmd.pipeline;
        t->CmdBindPipeline(self->vk_command_buffer,
                           VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipeline->vk_pipeline);
      }
      break;
      case CommandType::PushConstants:
      {
        t->CmdPushConstants(
            self->vk_command_buffer, pipeline->vk_layout, VK_SHADER_STAGE_ALL,
            0, pipeline->push_constants_size, cmd.push_constant.v0);
      }
      break;
      case CommandType::SetGraphicsState:
      {
        gpu::GraphicsState const &s = cmd.state;

        VkRect2D vk_scissor{
            .offset =
                VkOffset2D{(i32) s.scissor.offset.x, (i32) s.scissor.offset.y},
            .extent = VkExtent2D{s.scissor.extent.x, s.scissor.extent.y}};
        t->CmdSetScissor(self->vk_command_buffer, 0, 1, &vk_scissor);

        VkViewport vk_viewport{.x        = s.viewport.offset.x,
                               .y        = s.viewport.offset.y,
                               .width    = s.viewport.extent.x,
                               .height   = s.viewport.extent.y,
                               .minDepth = s.viewport.min_depth,
                               .maxDepth = s.viewport.max_depth};
        t->CmdSetViewport(self->vk_command_buffer, 0, 1, &vk_viewport);

        f32 vk_constant[4] = {s.blend_constant.x, s.blend_constant.y,
                              s.blend_constant.z, s.blend_constant.w};
        t->CmdSetBlendConstants(self->vk_command_buffer, vk_constant);

        t->CmdSetStencilTestEnableEXT(self->vk_command_buffer,
                                      s.stencil_test_enable);

        t->CmdSetStencilReference(self->vk_command_buffer,
                                  VK_STENCIL_FACE_FRONT_BIT,
                                  s.front_face_stencil.reference);
        t->CmdSetStencilCompareMask(self->vk_command_buffer,
                                    VK_STENCIL_FACE_FRONT_BIT,
                                    s.front_face_stencil.compare_mask);
        t->CmdSetStencilWriteMask(self->vk_command_buffer,
                                  VK_STENCIL_FACE_FRONT_BIT,
                                  s.front_face_stencil.write_mask);
        t->CmdSetStencilOpEXT(self->vk_command_buffer,
                              VK_STENCIL_FACE_FRONT_BIT,
                              (VkStencilOp) s.front_face_stencil.fail_op,
                              (VkStencilOp) s.front_face_stencil.pass_op,
                              (VkStencilOp) s.front_face_stencil.depth_fail_op,
                              (VkCompareOp) s.front_face_stencil.compare_op);

        t->CmdSetStencilReference(self->vk_command_buffer,
                                  VK_STENCIL_FACE_BACK_BIT,
                                  s.back_face_stencil.reference);
        t->CmdSetStencilCompareMask(self->vk_command_buffer,
                                    VK_STENCIL_FACE_BACK_BIT,
                                    s.back_face_stencil.compare_mask);
        t->CmdSetStencilWriteMask(self->vk_command_buffer,
                                  VK_STENCIL_FACE_BACK_BIT,
                                  s.back_face_stencil.write_mask);
        t->CmdSetStencilOpEXT(self->vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                              (VkStencilOp) s.back_face_stencil.fail_op,
                              (VkStencilOp) s.back_face_stencil.pass_op,
                              (VkStencilOp) s.back_face_stencil.depth_fail_op,
                              (VkCompareOp) s.back_face_stencil.compare_op);
        t->CmdSetCullModeEXT(self->vk_command_buffer,
                             (VkCullModeFlags) s.cull_mode);
        t->CmdSetFrontFaceEXT(self->vk_command_buffer,
                              (VkFrontFace) s.front_face);
        t->CmdSetDepthTestEnableEXT(self->vk_command_buffer,
                                    s.depth_test_enable);
        t->CmdSetDepthCompareOpEXT(self->vk_command_buffer,
                                   (VkCompareOp) s.depth_compare_op);
        t->CmdSetDepthWriteEnableEXT(self->vk_command_buffer,
                                     s.depth_write_enable);
        t->CmdSetDepthBoundsTestEnableEXT(self->vk_command_buffer,
                                          s.depth_bounds_test_enable);
      }
      break;
      case CommandType::BindVertexBuffer:
      {
        t->CmdBindVertexBuffers(self->vk_command_buffer, cmd.vertex_buffer.v0,
                                1, &cmd.vertex_buffer.v1->vk_buffer,
                                &cmd.vertex_buffer.v2);
      }
      break;
      case CommandType::BindIndexBuffer:
      {
        t->CmdBindIndexBuffer(
            self->vk_command_buffer, cmd.index_buffer.v0->vk_buffer,
            cmd.index_buffer.v1, (VkIndexType) cmd.index_buffer.v2);
      }
      break;
      case CommandType::Draw:
      {
        t->CmdDraw(self->vk_command_buffer, cmd.draw_indexed.v0,
                   cmd.draw_indexed.v1, cmd.draw_indexed.v2,
                   cmd.draw_indexed.v3);
      }
      break;
      case CommandType::DrawIndexed:
      {
        t->CmdDrawIndexed(self->vk_command_buffer, cmd.draw_indexed.v0,
                          cmd.draw_indexed.v1, cmd.draw_indexed.v2,
                          cmd.draw_indexed.v3, cmd.draw_indexed.v4);
      }
      break;
      case CommandType::DrawIndirect:
      {
        t->CmdDrawIndirect(
            self->vk_command_buffer, cmd.draw_indirect.v0->vk_buffer,
            cmd.draw_indirect.v1, cmd.draw_indirect.v2, cmd.draw_indirect.v3);
      }
      break;
      case CommandType::DrawIndexedIndirect:
      {
        t->CmdDrawIndexedIndirect(
            self->vk_command_buffer, cmd.draw_indirect.v0->vk_buffer,
            cmd.draw_indirect.v1, cmd.draw_indirect.v2, cmd.draw_indirect.v3);
      }
      break;
      default:
      {
      }
      break;
    }
  }

  t->CmdEndRenderingKHR(self->vk_command_buffer);
  self->reset_context();
}

void CommandEncoderInterface::bind_compute_pipeline(
    gpu::CommandEncoder self_, gpu::ComputePipeline pipeline)
{
  ENCODE_PRELUDE();
  ComputePassContext &ctx = self->compute_ctx;

  CHECK(self->is_in_compute_pass());

  self->state  = CommandEncoderState::ComputePass;
  ctx.pipeline = (ComputePipeline *) pipeline;

  self->dev->vk_table.CmdBindPipeline(self->vk_command_buffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      ctx.pipeline->vk_pipeline);
}

void CommandEncoderInterface::bind_graphics_pipeline(
    gpu::CommandEncoder self_, gpu::GraphicsPipeline pipeline_)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx      = self->render_ctx;
  GraphicsPipeline  *pipeline = (GraphicsPipeline *) pipeline_;

  CHECK(self->is_in_render_pass());
  CHECK(pipeline != nullptr);
  validate_render_pass_compatible(self_, pipeline_);
  ctx.pipeline = pipeline;
  if (!ctx.commands.push(
          Command{.type = CommandType::BindPipeline, .pipeline = pipeline}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::bind_descriptor_sets(
    gpu::CommandEncoder self_, Span<gpu::DescriptorSet const> descriptor_sets,
    Span<u32 const> dynamic_offsets)
{
  ENCODE_PRELUDE();
  u32 const num_sets            = descriptor_sets.size32();
  u32 const num_dynamic_offsets = dynamic_offsets.size32();
  u64 const ubo_offset_alignment =
      self->dev->phy_dev.vk_properties.limits.minUniformBufferOffsetAlignment;
  u64 const ssbo_offset_alignment =
      self->dev->phy_dev.vk_properties.limits.minStorageBufferOffsetAlignment;

  CHECK(self->is_in_pass());
  CHECK(num_sets <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS);
  CHECK(num_dynamic_offsets <= (gpu::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS +
                                gpu::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS));

  for (u32 offset : dynamic_offsets)
  {
    CHECK(mem::is_aligned<u64>(ubo_offset_alignment, offset) ||
          mem::is_aligned<u64>(ssbo_offset_alignment, offset));
  }

  if (self->is_in_compute_pass())
  {
    CHECK(self->compute_ctx.pipeline != nullptr);
    CHECK(self->compute_ctx.pipeline->num_sets == num_sets);
    VkDescriptorSet vk_sets[gpu::MAX_PIPELINE_DESCRIPTOR_SETS];
    for (u32 i = 0; i < num_sets; i++)
    {
      self->compute_ctx.sets[i] = (DescriptorSet *) descriptor_sets[i];
    }
    self->compute_ctx.num_sets = num_sets;

    self->dev->vk_table.CmdBindDescriptorSets(
        self->vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        self->compute_ctx.pipeline->vk_layout, 0, num_sets, vk_sets,
        num_dynamic_offsets, dynamic_offsets.data());
  }
  else if (self->is_in_render_pass())
  {
    CHECK(self->render_ctx.pipeline != nullptr);
    CHECK(self->render_ctx.pipeline->num_sets == num_sets);
    DescriptorSet **sets;
    if (!self->render_ctx.arg_pool.nalloc(num_sets, &sets))
    {
      self->status = Status::OutOfHostMemory;
      return;
    }
    u32 *offsets;
    if (!self->render_ctx.arg_pool.nalloc(num_dynamic_offsets, &offsets))
    {
      self->render_ctx.arg_pool.ndealloc(sets, num_sets);
      self->status = Status::OutOfHostMemory;
      return;
    }
    mem::copy(descriptor_sets, (gpu::DescriptorSet *) sets);
    mem::copy(dynamic_offsets, offsets);
    if (!self->render_ctx.commands.push(
            Command{.type = CommandType::BindDescriptorSets,
                    .set  = {sets, num_sets, offsets, num_dynamic_offsets}}))
    {
      self->render_ctx.arg_pool.ndealloc(offsets, num_dynamic_offsets);
      self->render_ctx.arg_pool.ndealloc(sets, num_sets);
      self->status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoderInterface::push_constants(gpu::CommandEncoder self_,
                                             Span<u8 const> push_constants_data)
{
  ENCODE_PRELUDE();
  CHECK(push_constants_data.size_bytes() <= gpu::MAX_PUSH_CONSTANTS_SIZE);
  u32 const push_constants_size = (u32) push_constants_data.size_bytes();
  CHECK(mem::is_aligned(4U, push_constants_size));
  CHECK(self->is_in_pass());

  if (self->is_in_compute_pass())
  {
    CHECK(self->compute_ctx.pipeline != nullptr);
    CHECK(push_constants_size ==
          self->compute_ctx.pipeline->push_constants_size);
    self->dev->vk_table.CmdPushConstants(
        self->vk_command_buffer, self->compute_ctx.pipeline->vk_layout,
        VK_SHADER_STAGE_ALL, 0, self->compute_ctx.pipeline->push_constants_size,
        push_constants_data.data());
  }
  else if (self->is_in_render_pass())
  {
    CHECK(self->render_ctx.pipeline != nullptr);
    CHECK(push_constants_size ==
          self->render_ctx.pipeline->push_constants_size);
    u8 *data;
    CHECK(self->render_ctx.arg_pool.nalloc(push_constants_size, &data));
    mem::copy(push_constants_data, data);
    if (!self->render_ctx.commands.push(
            Command{.type          = CommandType::PushConstants,
                    .push_constant = {data, push_constants_size}}))
    {
      self->status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoderInterface::dispatch(gpu::CommandEncoder self_,
                                       u32 group_count_x, u32 group_count_y,
                                       u32 group_count_z)
{
  ENCODE_PRELUDE();
  ComputePassContext &ctx = self->compute_ctx;

  CHECK(self->is_in_compute_pass());

  CHECK(ctx.pipeline != nullptr);
  CHECK(group_count_x <=
        self->dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[0]);
  CHECK(group_count_y <=
        self->dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[1]);
  CHECK(group_count_z <=
        self->dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[2]);

  for (u32 i = 0; i < ctx.num_sets; i++)
  {
    access_compute_bindings(*self, *ctx.sets[i]);
  }

  self->dev->vk_table.CmdDispatch(self->vk_command_buffer, group_count_x,
                                  group_count_y, group_count_z);
}

void CommandEncoderInterface::dispatch_indirect(gpu::CommandEncoder self_,
                                                gpu::Buffer buffer_, u64 offset)
{
  ENCODE_PRELUDE();
  ComputePassContext &ctx    = self->compute_ctx;
  Buffer *const       buffer = (Buffer *) buffer_;

  CHECK(self->is_in_compute_pass());
  CHECK(ctx.pipeline != nullptr);
  CHECK(has_bits(buffer->desc.usage, gpu::BufferUsage::IndirectBuffer));
  CHECK(is_valid_buffer_access(buffer->desc.size, offset,
                               sizeof(gpu::DispatchCommand), 4));

  for (u32 i = 0; i < ctx.num_sets; i++)
  {
    access_compute_bindings(*self, *ctx.sets[i]);
  }

  self->dev->vk_table.CmdDispatchIndirect(self->vk_command_buffer,
                                          buffer->vk_buffer, offset);
}

void CommandEncoderInterface::set_graphics_state(
    gpu::CommandEncoder self_, gpu::GraphicsState const &state)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  CHECK(self->is_in_render_pass());
  CHECK(state.viewport.min_depth >= 0.0F);
  CHECK(state.viewport.min_depth <= 1.0F);
  CHECK(state.viewport.max_depth >= 0.0F);
  CHECK(state.viewport.max_depth <= 1.0F);
  ctx.has_state = true;

  if (!ctx.commands.push(
          Command{.type = CommandType::SetGraphicsState, .state = state}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::bind_vertex_buffers(
    gpu::CommandEncoder self_, Span<gpu::Buffer const> vertex_buffers,
    Span<u64 const> offsets)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  CHECK(self->is_in_render_pass());
  u32 const num_vertex_buffers = vertex_buffers.size32();
  CHECK(num_vertex_buffers > 0);
  CHECK(num_vertex_buffers <= gpu::MAX_VERTEX_ATTRIBUTES);
  CHECK(offsets.size() == vertex_buffers.size());
  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    u64 const     offset = offsets[i];
    Buffer *const buffer = (Buffer *) vertex_buffers[i];
    CHECK(offset < buffer->desc.size);
    CHECK(has_bits(buffer->desc.usage, gpu::BufferUsage::VertexBuffer));
  }

  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    if (!ctx.commands.push(Command{
            .type          = CommandType::BindVertexBuffer,
            .vertex_buffer = {i, (Buffer *) vertex_buffers[i], offsets[i]}}))
    {
      self->status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoderInterface::bind_index_buffer(gpu::CommandEncoder self_,
                                                gpu::Buffer    index_buffer_,
                                                u64            offset,
                                                gpu::IndexType index_type)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx          = self->render_ctx;
  Buffer *const      index_buffer = (Buffer *) index_buffer_;
  u64 const          index_size   = index_type_size(index_type);

  CHECK(self->is_in_render_pass());
  CHECK(offset < index_buffer->desc.size);
  CHECK(mem::is_aligned(index_size, offset));
  CHECK(has_bits(index_buffer->desc.usage, gpu::BufferUsage::IndexBuffer));

  ctx.index_buffer        = index_buffer;
  ctx.index_type          = index_type;
  ctx.index_buffer_offset = offset;
  if (!ctx.commands.push(
          Command{.type         = CommandType::BindIndexBuffer,
                  .index_buffer = {index_buffer, offset, index_type}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::draw(gpu::CommandEncoder self_, u32 vertex_count,
                                   u32 instance_count, u32 first_vertex_id,
                                   u32 first_instance_id)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  CHECK(self->is_in_render_pass());
  CHECK(ctx.pipeline != nullptr);
  CHECK(ctx.has_state);

  if (!ctx.commands.push(Command{.type = CommandType::Draw,
                                 .draw = {vertex_count, instance_count,
                                          first_vertex_id, first_instance_id}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::draw_indexed(gpu::CommandEncoder self_,
                                           u32 first_index, u32 num_indices,
                                           i32 vertex_offset,
                                           u32 first_instance_id,
                                           u32 num_instances)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  CHECK(self->is_in_render_pass());
  CHECK(ctx.pipeline != nullptr);
  CHECK(ctx.index_buffer != nullptr);
  u64 const index_size = index_type_size(ctx.index_type);
  CHECK((ctx.index_buffer_offset + first_index * index_size) <
        ctx.index_buffer->desc.size);
  CHECK((ctx.index_buffer_offset + (first_index + num_indices) * index_size) <=
        ctx.index_buffer->desc.size);
  CHECK(ctx.has_state);

  if (!ctx.commands.push(
          Command{.type         = CommandType::DrawIndexed,
                  .draw_indexed = {first_index, num_indices, vertex_offset,
                                   first_instance_id, num_instances}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::draw_indirect(gpu::CommandEncoder self_,
                                            gpu::Buffer buffer_, u64 offset,
                                            u32 draw_count, u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx    = self->render_ctx;
  Buffer *const      buffer = (Buffer *) buffer_;

  CHECK(self->is_in_render_pass());
  CHECK(ctx.pipeline != nullptr);
  CHECK(has_bits(buffer->desc.usage, gpu::BufferUsage::IndirectBuffer));
  CHECK(offset < buffer->desc.size);
  CHECK((offset + (u64) draw_count * stride) <= buffer->desc.size);
  CHECK(mem::is_aligned(4U, stride));
  CHECK(stride >= sizeof(gpu::DrawCommand));
  CHECK(ctx.has_state);

  if (!ctx.commands.push(
          Command{.type          = CommandType::DrawIndirect,
                  .draw_indirect = {buffer, offset, draw_count, stride}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::draw_indexed_indirect(gpu::CommandEncoder self_,
                                                    gpu::Buffer         buffer_,
                                                    u64 offset, u32 draw_count,
                                                    u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx    = self->render_ctx;
  Buffer *const      buffer = (Buffer *) buffer_;

  CHECK(self->is_in_render_pass());
  CHECK(ctx.pipeline != nullptr);
  CHECK(ctx.index_buffer != nullptr);
  CHECK(has_bits(buffer->desc.usage, gpu::BufferUsage::IndirectBuffer));
  CHECK(offset < buffer->desc.size);
  CHECK((offset + (u64) draw_count * stride) <= buffer->desc.size);
  CHECK(mem::is_aligned(4U, stride));
  CHECK(stride >= sizeof(gpu::DrawIndexedCommand));
  CHECK(ctx.has_state);

  if (!ctx.commands.push(
          Command{.type          = CommandType::DrawIndexedIndirect,
                  .draw_indirect = {buffer, offset, draw_count, stride}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

}        // namespace vk
}        // namespace ash
