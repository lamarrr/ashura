/// SPDX-License-Identifier: MIT
#include "ashura/gpu/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "vulkan/vulkan.h"
#include <cstring>

namespace ash
{
namespace vk
{

#define BUFFER_FROM_VIEW(buffer_view) \
  ((Buffer *) (((BufferView *) (buffer_view))->info.buffer))
#define IMAGE_FROM_VIEW(image_view) \
  ((Image *) (((ImageView *) (image_view))->info.image))

VkResult DebugMarkerSetObjectTagEXT_Stub(VkDevice,
                                         const VkDebugMarkerObjectTagInfoEXT *)
{
  return VK_SUCCESS;
}

VkResult
  DebugMarkerSetObjectNameEXT_Stub(VkDevice,
                                   VkDebugMarkerObjectNameInfoEXT const *)
{
  return VK_SUCCESS;
}

void CmdDebugMarkerBeginEXT_Stub(VkCommandBuffer,
                                 VkDebugMarkerMarkerInfoEXT const *)
{
}

void CmdDebugMarkerEndEXT_Stub(VkCommandBuffer)
{
}

void CmdDebugMarkerInsertEXT_Stub(VkCommandBuffer,
                                  VkDebugMarkerMarkerInfoEXT const *)
{
}

VkResult SetDebugUtilsObjectNameEXT_Stub(VkDevice,
                                         VkDebugUtilsObjectNameInfoEXT const *)
{
  return VK_SUCCESS;
}

bool load_instance_table(VkInstance                instance,
                         PFN_vkGetInstanceProcAddr GetInstanceProcAddr,
                         InstanceTable & vk_table, bool validation_enabled)
{
  bool all_loaded = true;

#define LOAD_VK(function)                                             \
  vk_table.function =                                                 \
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

bool load_device_table(VkDevice dev, InstanceTable const & instance_table,
                       DeviceTable & vk_table, bool debug_marker_enabled)
{
  mem::zero(&vk_table, 1);
  bool all_loaded = true;

#define LOAD_VK(function)                                                     \
  vk_table.function =                                                         \
    (PFN_vk##function) instance_table.GetDeviceProcAddr(dev, "vk" #function); \
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

void load_vma_table(InstanceTable const & instance_table,
                    DeviceTable const &   vk_table,
                    VmaVulkanFunctions &  vma_table)
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
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT       message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT              message_type,
                 VkDebugUtilsMessengerCallbackDataEXT const * data, void *)
{
  LogLevel level = LogLevel::Trace;
  if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
  {
    level = LogLevel::Debug;
  }
  else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
  {
    level = LogLevel::Warning;
  }
  else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
  {
    level = LogLevel::Info;
  }
  else
  {
    level = LogLevel::Trace;
  }

  auto message_type_s = string_VkDebugUtilsMessageTypeFlagsEXT(message_type);
  logger->log(level, "[Type: {}, Id: {}, Name: {} ] {}"_str,
              span(message_type_s), data->messageIdNumber,
              cstr_span(data->pMessageIdName),
              data->pMessage == nullptr ? "(empty message)"_str :
                                          cstr_span(data->pMessage));
  if (data->objectCount != 0)
  {
    logger->log(level, "Objects Involved:"_str);
    for (u32 i = 0; i < data->objectCount; i++)
    {
      logger->log(level, "[Type: {}] {}"_str,
                  cstr_span(string_VkObjectType(data->pObjects[i].objectType)),
                  data->pObjects[i].pObjectName == nullptr ?
                    "(unnamed)"_str :
                    cstr_span(data->pObjects[i].pObjectName));
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

inline bool sync_buffer_state(BufferState & state, BufferAccess request,
                              VkBufferMemoryBarrier & barrier,
                              VkPipelineStageFlags &  src_stages,
                              VkPipelineStageFlags &  dst_stages)
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
inline bool sync_image_state(ImageAspectState & state, ImageAccess request,
                             VkImageMemoryBarrier & barrier,
                             VkPipelineStageFlags & src_stages,
                             VkPipelineStageFlags & dst_stages)
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
      CHECK_UNREACHABLE();
  }
}

inline bool is_valid_buffer_access(u64 size, u64 access_offset, u64 access_size,
                                   u64 offset_alignment = 1)
{
  access_size =
    (access_size == gpu::WHOLE_SIZE) ? (size - access_offset) : access_size;
  return (access_size > 0) && (access_offset < size) &&
         ((access_offset + access_size) <= size) &&
         is_aligned(offset_alignment, access_offset);
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

Result<Dyn<gpu::Instance *>, Status> create_instance(AllocatorRef allocator,
                                                     bool enable_validation)
{
  u32      num_exts;
  VkResult result =
    vkEnumerateInstanceExtensionProperties(nullptr, &num_exts, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkExtensionProperties * exts;
  if (!allocator->nalloc(num_exts, exts))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer exts_{[&] { allocator->ndealloc(num_exts, exts); }};

  {
    u32 num_read_exts = num_exts;
    result =
      vkEnumerateInstanceExtensionProperties(nullptr, &num_read_exts, exts);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    CHECK(num_read_exts == num_exts, "");
  }

  u32 num_layers;
  result = vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkLayerProperties * layers;

  if (!allocator->nalloc(num_layers, layers))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer layers_{[&] { allocator->ndealloc(num_layers, layers); }};

  {
    u32 num_read_layers = num_layers;
    result = vkEnumerateInstanceLayerProperties(&num_read_layers, layers);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    CHECK(num_read_layers == num_layers, "");
  }

  trace("Available Extensions:"_str);

  for (VkExtensionProperties const & ext : Span{exts, num_exts})
  {
    trace("{}\t\t(spec version {}.{}.{} variant {})"_str,
          cstr_span(ext.extensionName), VK_API_VERSION_MAJOR(ext.specVersion),
          VK_API_VERSION_MINOR(ext.specVersion),
          VK_API_VERSION_PATCH(ext.specVersion),
          VK_API_VERSION_VARIANT(ext.specVersion));
  }

  trace("Available Layers:"_str);

  for (VkLayerProperties const & layer : Span{layers, num_layers})
  {
    trace("{}\t\t(spec version {}.{}.{} variant {}, implementation version: "
          "{}.{}.{} variant {})"_str,
          cstr_span(layer.layerName), VK_API_VERSION_MAJOR(layer.specVersion),
          VK_API_VERSION_MINOR(layer.specVersion),
          VK_API_VERSION_PATCH(layer.specVersion),
          VK_API_VERSION_VARIANT(layer.specVersion),
          VK_API_VERSION_MAJOR(layer.implementationVersion),
          VK_API_VERSION_MINOR(layer.implementationVersion),
          VK_API_VERSION_PATCH(layer.implementationVersion),
          VK_API_VERSION_VARIANT(layer.implementationVersion));
  }

  char const * load_exts[16];
  u32          num_load_exts = 0;

  constexpr char const *           OPTIONAL_EXTS[]  = {"VK_KHR_surface",
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
        has_optional_ext.set_bit(iopt);
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
      warn("Required Vulkan "
           "Extension: {}  is not supported on device",
           VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
  }

  char const * load_layers[16];
  u32          num_load_layers      = 0;
  bool         has_validation_layer = false;

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
      warn("Required Layer: VK_LAYER_KHRONOS_validation is "
           "not supported");
    }
  }

  bool const validation_enabled =
    enable_validation && has_debug_utils_ext && has_validation_layer;

  // setup before vkInstance to allow debug reporter report
  // messages through the pointer to it
  Result instance_result = dyn<Instance>(inplace, allocator);

  if (!instance_result)
  {
    return Err{gpu::Status::OutOfHostMemory};
  }

  Dyn<Instance *> instance = instance_result.unwrap();

  instance->allocator          = allocator;
  instance->vk_table           = {};
  instance->vk_instance        = nullptr;
  instance->vk_debug_messenger = nullptr;
  instance->validation_enabled = validation_enabled;

  VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pNext = nullptr,
                             .pApplicationName   = CLIENT_NAME,
                             .applicationVersion = CLIENT_VERSION,
                             .pEngineName        = ENGINE_NAME,
                             .engineVersion      = ENGINE_VERSION,
                             .apiVersion         = VK_API_VERSION_1_1};

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
    .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext           = nullptr,
    .flags           = 0,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debug_callback,
    .pUserData       = instance.get()};

  // .pNext helps to debug issues with vkDestroyInstance and vkCreateInstance
  // i.e. (before and after the debug messenger is installed)
  VkInstanceCreateInfo create_info{
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext                   = enable_validation ? &debug_create_info : nullptr,
    .flags                   = has_optional_ext.get(10) ?
                                 VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR :
                                 ((VkInstanceCreateFlags) 0),
    .pApplicationInfo        = &app_info,
    .enabledLayerCount       = num_load_layers,
    .ppEnabledLayerNames     = load_layers,
    .enabledExtensionCount   = num_load_exts,
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
                            validation_enabled),
        "");

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

  vk_instance = nullptr;

  return Ok{cast<gpu::Instance *>(std::move(instance))};
}
}    // namespace vk

namespace gpu
{

Result<Dyn<gpu::Instance *>, Status>
  create_vulkan_instance(AllocatorRef allocator, bool enable_validation)
{
  return vk::create_instance(allocator, enable_validation);
}

}    // namespace gpu

namespace vk
{

Instance::~Instance()
{
  if (vk_instance == nullptr)
  {
    return;
  }

  if (validation_enabled)
  {
    vk_table.DestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger,
                                           nullptr);
  }
  vk_table.DestroyInstance(vk_instance, nullptr);
}

void check_device_limits(VkPhysicalDeviceLimits limits)
{
  CHECK(limits.maxImageDimension1D >= gpu::MAX_IMAGE_EXTENT_1D, "");
  CHECK(limits.maxImageDimension2D >= gpu::MAX_IMAGE_EXTENT_2D, "");
  CHECK(limits.maxImageDimension3D >= gpu::MAX_IMAGE_EXTENT_3D, "");
  CHECK(limits.maxImageDimensionCube >= gpu::MAX_IMAGE_EXTENT_CUBE, "");
  CHECK(limits.maxImageArrayLayers >= gpu::MAX_IMAGE_ARRAY_LAYERS, "");
  CHECK(limits.maxViewportDimensions[0] >= gpu::MAX_VIEWPORT_EXTENT, "");
  CHECK(limits.maxViewportDimensions[1] >= gpu::MAX_VIEWPORT_EXTENT, "");
  CHECK(limits.maxFramebufferWidth >= gpu::MAX_FRAMEBUFFER_EXTENT, "");
  CHECK(limits.maxFramebufferHeight >= gpu::MAX_FRAMEBUFFER_EXTENT, "");
  CHECK(limits.maxFramebufferLayers >= gpu::MAX_FRAMEBUFFER_LAYERS, "");
  CHECK(limits.maxVertexInputAttributes >= gpu::MAX_VERTEX_ATTRIBUTES, "");
  CHECK(limits.maxVertexInputBindings >= gpu::MAX_VERTEX_ATTRIBUTES, "");
  CHECK(limits.maxPushConstantsSize >= gpu::MAX_PUSH_CONSTANTS_SIZE, "");
  CHECK(limits.maxBoundDescriptorSets >= gpu::MAX_PIPELINE_DESCRIPTOR_SETS, "");
  CHECK(limits.maxPerStageDescriptorInputAttachments >=
          gpu::MAX_PIPELINE_INPUT_ATTACHMENTS,
        "");
  CHECK(limits.maxUniformBufferRange >= gpu::MAX_UNIFORM_BUFFER_RANGE, "");
  CHECK(limits.maxColorAttachments >= gpu::MAX_PIPELINE_COLOR_ATTACHMENTS, "");
  CHECK(limits.maxSamplerAnisotropy >= gpu::MAX_SAMPLER_ANISOTROPY, "");
  CHECK(has_bits(limits.framebufferColorSampleCounts,
                 (VkSampleCountFlags) gpu::REQUIRED_COLOR_SAMPLE_COUNTS),
        "");
  CHECK(has_bits(limits.framebufferDepthSampleCounts,
                 (VkSampleCountFlags) gpu::REQUIRED_DEPTH_SAMPLE_COUNTS),
        "");
  CHECK(limits.minTexelBufferOffsetAlignment <= gpu::BUFFER_OFFSET_ALIGNMENT,
        "");
  CHECK(limits.minUniformBufferOffsetAlignment <= gpu::BUFFER_OFFSET_ALIGNMENT,
        "");
  CHECK(limits.minStorageBufferOffsetAlignment <= gpu::BUFFER_OFFSET_ALIGNMENT,
        "");
}

void check_device_features(VkPhysicalDeviceFeatures feat)
{
  CHECK(feat.samplerAnisotropy == VK_TRUE, "");
  CHECK(feat.shaderUniformBufferArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.shaderSampledImageArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.shaderStorageBufferArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.shaderStorageImageArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.multiDrawIndirect == VK_TRUE, "");
  CHECK(feat.drawIndirectFirstInstance == VK_TRUE, "");
  CHECK(feat.imageCubeArray == VK_TRUE, "");
  CHECK(feat.pipelineStatisticsQuery == VK_TRUE, "");
}

Result<gpu::Device *, Status>
  Instance::create_device(AllocatorRef                allocator,
                          Span<gpu::DeviceType const> preferred_types,
                          u32                         buffering)
{
  constexpr u32 MAX_QUEUE_FAMILIES = 16;

  CHECK(buffering > 0, "");
  CHECK(buffering <= gpu::MAX_FRAME_BUFFERING, "");

  u32      num_devs;
  VkResult result =
    vk_table.EnumeratePhysicalDevices(vk_instance, &num_devs, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  if (num_devs == 0)
  {
    return Err{Status::DeviceLost};
  }

  VkPhysicalDevice * vk_phy_devs;

  if (!allocator->nalloc(num_devs, vk_phy_devs))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_phy_devs_{[&] { allocator->ndealloc(num_devs, vk_phy_devs); }};

  {
    u32 num_read_devs = num_devs;
    result = vk_table.EnumeratePhysicalDevices(vk_instance, &num_read_devs,
                                               vk_phy_devs);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    CHECK(num_read_devs == num_devs, "");
  }

  PhysicalDevice * physical_devs;
  if (!allocator->nalloc(num_devs, physical_devs))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer physical_devs_{[&] { allocator->ndealloc(num_devs, physical_devs); }};

  for (u32 i = 0; i < num_devs; i++)
  {
    PhysicalDevice & dev    = physical_devs[i];
    VkPhysicalDevice vk_dev = vk_phy_devs[i];
    dev.vk_phy_dev          = vk_dev;
    vk_table.GetPhysicalDeviceFeatures(vk_dev, &dev.vk_features);
    vk_table.GetPhysicalDeviceMemoryProperties(vk_dev,
                                               &dev.vk_memory_properties);
    vk_table.GetPhysicalDeviceProperties(vk_dev, &dev.vk_properties);
  }

  trace("Available Devices:"_str);
  for (u32 i = 0; i < num_devs; i++)
  {
    PhysicalDevice const &             dev        = physical_devs[i];
    VkPhysicalDeviceProperties const & properties = dev.vk_properties;
    trace("[Device: {}] {} {} Vulkan API version {}.{}.{} variant {}, Driver "
          "Version: {}, Vendor ID: {}, Device ID: {}"_str,
          i, cstr_span(string_VkPhysicalDeviceType(properties.deviceType)),
          properties.deviceName, VK_API_VERSION_MAJOR(properties.apiVersion),
          VK_API_VERSION_MINOR(properties.apiVersion),
          VK_API_VERSION_PATCH(properties.apiVersion),
          VK_API_VERSION_VARIANT(properties.apiVersion),
          properties.driverVersion, properties.vendorID, properties.deviceID);

    u32 num_queue_families;
    vk_table.GetPhysicalDeviceQueueFamilyProperties(
      dev.vk_phy_dev, &num_queue_families, nullptr);

    CHECK(num_queue_families <= MAX_QUEUE_FAMILIES, "");

    VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

    {
      u32 num_read_queue_families = num_queue_families;
      vk_table.GetPhysicalDeviceQueueFamilyProperties(
        dev.vk_phy_dev, &num_queue_families, queue_family_properties);
      CHECK(num_read_queue_families == num_queue_families, "");
    }

    for (u32 i = 0; i < num_queue_families; i++)
    {
      trace("\t\tQueue Family: {}, Count: {}, Flags: {}", i,
            queue_family_properties[i].queueCount,
            string_VkQueueFlags(queue_family_properties[i].queueFlags));
    }
  }

  u32 selected_dev_idx      = num_devs;
  u32 selected_queue_family = VK_QUEUE_FAMILY_IGNORED;

  for (usize i = 0; i < size32(preferred_types); i++)
  {
    for (u32 idev = 0; idev < num_devs && selected_dev_idx == num_devs; idev++)
    {
      PhysicalDevice const & dev = physical_devs[idev];

      u32 num_queue_families;
      vk_table.GetPhysicalDeviceQueueFamilyProperties(
        dev.vk_phy_dev, &num_queue_families, nullptr);

      CHECK(num_queue_families <= MAX_QUEUE_FAMILIES, "");

      VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

      {
        u32 num_read_queue_families = num_queue_families;
        vk_table.GetPhysicalDeviceQueueFamilyProperties(
          dev.vk_phy_dev, &num_queue_families, queue_family_properties);
        CHECK(num_read_queue_families == num_queue_families, "");
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
            selected_dev_idx      = idev;
            selected_queue_family = iqueue_family;
            break;
          }
        }
      }
    }
  }

  if (selected_dev_idx == num_devs)
  {
    trace("No Suitable Device Found");
    return Err{Status::DeviceLost};
  }

  PhysicalDevice selected_dev = physical_devs[selected_dev_idx];

  check_device_limits(selected_dev.vk_properties.limits);
  check_device_features(selected_dev.vk_features);

  trace("Selected Device {}", selected_dev_idx);

  u32 num_exts;
  result = vk_table.EnumerateDeviceExtensionProperties(
    selected_dev.vk_phy_dev, nullptr, &num_exts, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkExtensionProperties * exts;

  if (!allocator->nalloc(num_exts, exts))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer exts_{[&] { allocator->ndealloc(num_exts, exts); }};

  {
    u32 num_read_exts = num_exts;
    result            = vk_table.EnumerateDeviceExtensionProperties(
      selected_dev.vk_phy_dev, nullptr, &num_read_exts, exts);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
    CHECK(num_exts == num_read_exts, "");
  }

  u32 num_layers;
  result = vk_table.EnumerateDeviceLayerProperties(selected_dev.vk_phy_dev,
                                                   &num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkLayerProperties * layers;

  if (!allocator->nalloc(num_layers, layers))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer layers_{[&] { allocator->ndealloc(num_layers, layers); }};

  {
    u32 num_read_layers = num_layers;
    result = vk_table.EnumerateDeviceLayerProperties(selected_dev.vk_phy_dev,
                                                     &num_read_layers, layers);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
    CHECK(num_read_layers == num_layers, "");
  }

  trace("Available Extensions:"_str);

  for (u32 i = 0; i < num_exts; i++)
  {
    VkExtensionProperties const & ext = exts[i];
    trace("\t\t{} (spec version: {}.{}.{} variant {})"_str,
          cstr_span(ext.extensionName), VK_API_VERSION_MAJOR(ext.specVersion),
          VK_API_VERSION_MINOR(ext.specVersion),
          VK_API_VERSION_PATCH(ext.specVersion),
          VK_API_VERSION_VARIANT(ext.specVersion));
  }

  trace("Available Layers:");

  for (u32 i = 0; i < num_layers; i++)
  {
    VkLayerProperties const & layer = layers[i];

    trace("\t\t{} (spec version: {}.{}.{} variant {}, implementation version: "
          "{}.{}.{} variant {})"_str,
          cstr_span(layer.layerName), VK_API_VERSION_MAJOR(layer.specVersion),
          VK_API_VERSION_MINOR(layer.specVersion),
          VK_API_VERSION_PATCH(layer.specVersion),
          VK_API_VERSION_VARIANT(layer.specVersion),
          VK_API_VERSION_MAJOR(layer.implementationVersion),
          VK_API_VERSION_MINOR(layer.implementationVersion),
          VK_API_VERSION_PATCH(layer.implementationVersion),
          VK_API_VERSION_VARIANT(layer.implementationVersion));
  }

  constexpr char const * required_exts[] = {
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

  char const * load_exts[16];
  u32          num_load_exts = 0;

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
      trace("Required Extension: {} is not present", required_exts[i]);
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

  char const * load_layers[8];
  u32          num_load_layers = 0;

  // optional
  if (vk_debug_messenger != nullptr && has_validation_layer)
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
    .robustBufferAccess             = VK_FALSE,
    .fullDrawIndexUint32            = VK_FALSE,
    .imageCubeArray                 = VK_TRUE,
    .independentBlend               = VK_FALSE,
    .geometryShader                 = VK_FALSE,
    .tessellationShader             = VK_FALSE,
    .sampleRateShading              = VK_FALSE,
    .dualSrcBlend                   = VK_FALSE,
    .logicOp                        = VK_FALSE,
    .multiDrawIndirect              = VK_TRUE,
    .drawIndirectFirstInstance      = VK_TRUE,
    .depthClamp                     = VK_FALSE,
    .depthBiasClamp                 = VK_FALSE,
    .fillModeNonSolid               = selected_dev.vk_features.fillModeNonSolid,
    .depthBounds                    = VK_FALSE,
    .wideLines                      = VK_FALSE,
    .largePoints                    = VK_FALSE,
    .alphaToOne                     = VK_FALSE,
    .multiViewport                  = VK_FALSE,
    .samplerAnisotropy              = VK_TRUE,
    .textureCompressionETC2         = VK_FALSE,
    .textureCompressionASTC_LDR     = VK_FALSE,
    .textureCompressionBC           = VK_FALSE,
    .occlusionQueryPrecise          = VK_FALSE,
    .pipelineStatisticsQuery        = VK_TRUE,
    .vertexPipelineStoresAndAtomics = VK_FALSE,
    .fragmentStoresAndAtomics       = VK_FALSE,
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

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT
    extended_dynamic_state_features{
      .sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .pNext                = &separate_depth_stencil_layout_feature,
      .extendedDynamicState = VK_TRUE};

  VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
    .pNext = &extended_dynamic_state_features,
    .dynamicRendering = VK_TRUE};

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features{
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
    .pNext = &dynamic_rendering_features,
    .shaderInputAttachmentArrayDynamicIndexing          = VK_TRUE,
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
  result = vk_table.CreateDevice(selected_dev.vk_phy_dev, &create_info, nullptr,
                                 &vk_dev);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  DeviceTable        vk_dev_table;
  VmaVulkanFunctions vma_table;
  CHECK(load_device_table(vk_dev, vk_table, vk_dev_table, has_debug_marker_ext),
        "");

  load_vma_table(vk_table, vk_dev_table, vma_table);

  defer vk_dev_{[&] {
    if (vk_dev != nullptr)
    {
      vk_dev_table.DestroyDevice(vk_dev, nullptr);
    }
  }};

  VkQueue vk_queue;
  vk_dev_table.GetDeviceQueue(vk_dev, selected_queue_family, 0, &vk_queue);

  VmaAllocatorCreateInfo vma_create_info{
    .flags          = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
    .physicalDevice = selected_dev.vk_phy_dev,
    .device         = vk_dev,
    .preferredLargeHeapBlockSize    = 0,
    .pAllocationCallbacks           = nullptr,
    .pDeviceMemoryCallbacks         = nullptr,
    .pHeapSizeLimit                 = nullptr,
    .pVulkanFunctions               = &vma_table,
    .instance                       = vk_instance,
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

  Device * dev;

  if (!allocator->nalloc(1, dev))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (dev) Device{};

  dev->allocator     = allocator;
  dev->instance      = this;
  dev->phy_dev       = selected_dev;
  dev->vk_table      = vk_dev_table;
  dev->vma_table     = vma_table;
  dev->vk_dev        = vk_dev;
  dev->queue_family  = selected_queue_family;
  dev->vk_queue      = vk_queue;
  dev->vma_allocator = vma_allocator;
  dev->frame_ctx     = FrameContext{};
  dev->scratch       = Vec<u8>{allocator};

  defer dev_{[&] {
    if (dev != nullptr)
    {
      uninit((gpu::Device *) dev);
    }
  }};

  Status status = dev->init_frame_context(buffering);

  if (status != Status::Success)
  {
    return Err{status};
  }

  Device * out  = dev;
  vma_allocator = nullptr;
  vk_dev        = nullptr;
  dev           = nullptr;

  return Ok<gpu::Device *>{out};
}

gpu::Backend Instance::get_backend()
{
  return gpu::Backend::Vulkan;
}

void Instance::uninit(gpu::Device * device_)
{
  Device * const dev = (Device *) device_;

  if (dev == nullptr)
  {
    return;
  }

  dev->uninit();
  vmaDestroyAllocator(dev->vma_allocator);
  dev->vk_table.DestroyDevice(dev->vk_dev, nullptr);
  allocator->ndealloc(1, dev);
}

void Instance::uninit(gpu::Surface surface)
{
  vk_table.DestroySurfaceKHR(vk_instance, (Surface) surface, nullptr);
}

void Device::set_resource_name(Str label, void const * resource,
                               VkObjectType               type,
                               VkDebugReportObjectTypeEXT debug_type)
{
  char label_c_str[256];
  CHECK(to_c_str(label, label_c_str), "");
  VkDebugUtilsObjectNameInfoEXT name_info{
    .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
    .pNext        = nullptr,
    .objectType   = type,
    .objectHandle = (u64) resource,
    .pObjectName  = label_c_str};
  instance->vk_table.SetDebugUtilsObjectNameEXT(vk_dev, &name_info);
  VkDebugMarkerObjectNameInfoEXT debug_info{
    .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
    .pNext       = nullptr,
    .objectType  = debug_type,
    .object      = (u64) resource,
    .pObjectName = label_c_str};
  vk_table.DebugMarkerSetObjectNameEXT(vk_dev, &debug_info);
}

gpu::DeviceProperties Device::get_properties()
{
  VkPhysicalDeviceFeatures const &   vk_features   = phy_dev.vk_features;
  VkPhysicalDeviceProperties const & vk_properties = phy_dev.vk_properties;

  bool has_uma = false;
  for (u32 i = 0; i < phy_dev.vk_memory_properties.memoryTypeCount; i++)
  {
    if (has_bits(phy_dev.vk_memory_properties.memoryTypes[i].propertyFlags,
                 (VkMemoryPropertyFlags) (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)))
    {
      has_uma = true;
      break;
    }
  }

  gpu::DeviceProperties properties{
    .api_version             = vk_properties.apiVersion,
    .driver_version          = vk_properties.driverVersion,
    .vendor_id               = vk_properties.vendorID,
    .device_id               = vk_properties.deviceID,
    .device_name             = cstr_span(vk_properties.deviceName),
    .type                    = (gpu::DeviceType) vk_properties.deviceType,
    .has_unified_memory      = has_uma,
    .has_non_solid_fill_mode = (vk_features.fillModeNonSolid == VK_TRUE),
    .timestamp_period        = vk_properties.limits.timestampPeriod,
    .max_compute_work_group_invocations =
      vk_properties.limits.maxComputeWorkGroupInvocations,
    .max_compute_shared_memory_size =
      vk_properties.limits.maxComputeSharedMemorySize};

  mem::copy(Span{vk_properties.limits.maxComputeWorkGroupCount, 3},
            properties.max_compute_work_group_count);
  mem::copy(Span{vk_properties.limits.maxComputeWorkGroupSize, 3},
            properties.max_compute_work_group_size);

  return properties;
}

Result<gpu::FormatProperties, Status>
  Device::get_format_properties(gpu::Format format)
{
  VkFormatProperties props;
  instance->vk_table.GetPhysicalDeviceFormatProperties(
    phy_dev.vk_phy_dev, (VkFormat) format, &props);
  return Ok(gpu::FormatProperties{
    .linear_tiling_features = (gpu::FormatFeatures) props.linearTilingFeatures,
    .optimal_tiling_features =
      (gpu::FormatFeatures) props.optimalTilingFeatures,
    .buffer_features = (gpu::FormatFeatures) props.bufferFeatures});
}

Result<gpu::Buffer, Status> Device::create_buffer(gpu::BufferInfo const & info)
{
  CHECK(info.size != 0, "");
  CHECK(info.usage != gpu::BufferUsage::None, "");

  VkBufferCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size  = info.size,
                                 .usage = (VkBufferUsageFlags) info.usage,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 1,
                                 .pQueueFamilyIndices   = nullptr};
  VmaAllocationCreateInfo alloc_create_info{
    .flags =
      info.host_mapped ?
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
    vmaCreateBuffer(vma_allocator, &create_info, &alloc_create_info, &vk_buffer,
                    &vma_allocation, nullptr);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_buffer, VK_OBJECT_TYPE_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);

  Buffer * buffer;
  if (!allocator->nalloc(1, buffer))
  {
    vmaDestroyBuffer(vma_allocator, vk_buffer, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  new (buffer) Buffer{
    .info = info, .vk_buffer = vk_buffer, .vma_allocation = vma_allocation};

  return Ok{(gpu::Buffer) buffer};
}

Result<gpu::BufferView, Status>
  Device::create_buffer_view(gpu::BufferViewInfo const & info)
{
  Buffer * const buffer = (Buffer *) info.buffer;

  CHECK(buffer != nullptr, "");
  CHECK(has_any_bit(buffer->info.usage, gpu::BufferUsage::UniformTexelBuffer |
                                          gpu::BufferUsage::StorageTexelBuffer),
        "");
  CHECK(info.format != gpu::Format::Undefined, "");
  CHECK(is_valid_buffer_access(buffer->info.size, info.offset, info.size), "");

  u64 const view_size = info.size == gpu::WHOLE_SIZE ?
                          (buffer->info.size - info.offset) :
                          info.size;

  VkBufferViewCreateInfo create_info{
    .sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
    .pNext  = nullptr,
    .flags  = 0,
    .buffer = buffer->vk_buffer,
    .format = (VkFormat) info.format,
    .offset = info.offset,
    .range  = info.size};

  VkBufferView vk_view;

  VkResult result =
    vk_table.CreateBufferView(vk_dev, &create_info, nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_view, VK_OBJECT_TYPE_BUFFER_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);

  BufferView * view;

  if (!allocator->nalloc(1, view))
  {
    vk_table.DestroyBufferView(vk_dev, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) BufferView{.info = info, .vk_view = vk_view};

  view->info.size = view_size;

  return Ok{(gpu::BufferView) view};
}

Result<gpu::Image, Status> Device::create_image(gpu::ImageInfo const & info)
{
  CHECK(info.format != gpu::Format::Undefined, "");
  CHECK(info.usage != gpu::ImageUsage::None, "");
  CHECK(info.aspects != gpu::ImageAspects::None, "");
  CHECK(info.sample_count != gpu::SampleCount::None, "");
  CHECK(info.extent.x != 0, "");
  CHECK(info.extent.y != 0, "");
  CHECK(info.extent.z != 0, "");
  CHECK(info.mip_levels > 0, "");
  CHECK(info.mip_levels <= num_mip_levels(info.extent), "");
  CHECK(info.array_layers > 0, "");
  CHECK(info.array_layers <= gpu::MAX_IMAGE_ARRAY_LAYERS, "");

  switch (info.type)
  {
    case gpu::ImageType::Type1D:
      CHECK(info.extent.x <= gpu::MAX_IMAGE_EXTENT_1D, "");
      CHECK(info.extent.y == 1, "");
      CHECK(info.extent.z == 1, "");
      break;

    case gpu::ImageType::Type2D:
      CHECK(info.extent.x <= gpu::MAX_IMAGE_EXTENT_2D, "");
      CHECK(info.extent.y <= gpu::MAX_IMAGE_EXTENT_2D, "");
      CHECK(info.extent.z == 1, "");
      break;

    case gpu::ImageType::Type3D:
      CHECK(info.extent.x <= gpu::MAX_IMAGE_EXTENT_3D, "");
      CHECK(info.extent.y <= gpu::MAX_IMAGE_EXTENT_3D, "");
      CHECK(info.extent.z <= gpu::MAX_IMAGE_EXTENT_3D, "");
      break;

    default:
      break;
  }

  VkImageCreateInfo create_info{
    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext                 = nullptr,
    .flags                 = 0,
    .imageType             = (VkImageType) info.type,
    .format                = (VkFormat) info.format,
    .extent                = VkExtent3D{.width  = info.extent.x,
                                        .height = info.extent.y,
                                        .depth  = info.extent.z},
    .mipLevels             = info.mip_levels,
    .arrayLayers           = info.array_layers,
    .samples               = (VkSampleCountFlagBits) info.sample_count,
    .tiling                = VK_IMAGE_TILING_OPTIMAL,
    .usage                 = (VkImageUsageFlags) info.usage,
    .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices   = nullptr,
    .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
  };
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

  VkResult result =
    vmaCreateImage(vma_allocator, &create_info, &vma_allocation_create_info,
                   &vk_image, &vma_allocation, &vma_allocation_info);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_image, VK_OBJECT_TYPE_IMAGE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);

  Image * image;

  if (!allocator->nalloc(1, image))
  {
    vmaDestroyImage(vma_allocator, vk_image, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  // separate states for depth and stencil image aspects
  u32 const num_aspects = has_bits(info.aspects, gpu::ImageAspects::Depth |
                                                   gpu::ImageAspects::Stencil) ?
                            2 :
                            1;

  InplaceVec<ImageAspectState, MAX_IMAGE_ASPECTS> states;
  states.resize(num_aspects).unwrap();

  new (image) Image{.info                = info,
                    .is_swapchain_image  = false,
                    .vk_image            = vk_image,
                    .vma_allocation      = vma_allocation,
                    .vma_allocation_info = vma_allocation_info,
                    .aspect_states       = states};

  return Ok{(gpu::Image) image};
}

Result<gpu::ImageView, Status>
  Device::create_image_view(gpu::ImageViewInfo const & info)
{
  Image * const src_image = (Image *) info.image;

  CHECK(info.image != nullptr, "");
  CHECK(info.view_format != gpu::Format::Undefined, "");
  CHECK(is_image_view_type_compatible(src_image->info.type, info.view_type),
        "");
  CHECK(is_valid_image_access(
          src_image->info.aspects, src_image->info.mip_levels,
          src_image->info.array_layers, info.aspects, info.first_mip_level,
          info.num_mip_levels, info.first_array_layer, info.num_array_layers),
        "");

  u32 const mip_levels = info.num_mip_levels == gpu::REMAINING_MIP_LEVELS ?
                           (src_image->info.mip_levels - info.first_mip_level) :
                           info.num_mip_levels;
  u32 const array_layers =
    info.num_array_layers == gpu::REMAINING_ARRAY_LAYERS ?
      (src_image->info.array_layers - info.first_array_layer) :
      info.num_array_layers;

  VkImageViewCreateInfo create_info{
    .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext      = nullptr,
    .flags      = 0,
    .image      = src_image->vk_image,
    .viewType   = (VkImageViewType) info.view_type,
    .format     = (VkFormat) info.view_format,
    .components = VkComponentMapping{.r = (VkComponentSwizzle) info.mapping.r,
                                     .g = (VkComponentSwizzle) info.mapping.g,
                                     .b = (VkComponentSwizzle) info.mapping.b,
                                     .a = (VkComponentSwizzle) info.mapping.a},
    .subresourceRange =
      VkImageSubresourceRange{.aspectMask   = (VkImageAspectFlags) info.aspects,
                                     .baseMipLevel = info.first_mip_level,
                                     .levelCount   = info.num_mip_levels,
                                     .baseArrayLayer = info.first_array_layer,
                                     .layerCount     = info.num_array_layers}
  };

  VkImageView vk_view;
  VkResult    result =
    vk_table.CreateImageView(vk_dev, &create_info, nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_view, VK_OBJECT_TYPE_IMAGE_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);

  ImageView * view;
  if (!allocator->nalloc(1, view))
  {
    vk_table.DestroyImageView(vk_dev, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) ImageView{.info = info, .vk_view = vk_view};

  view->info.num_mip_levels   = mip_levels;
  view->info.num_array_layers = array_layers;

  return Ok{(gpu::ImageView) view};
}

Result<gpu::Sampler, Status>
  Device::create_sampler(gpu::SamplerInfo const & info)
{
  CHECK(!(info.anisotropy_enable &&
          (info.max_anisotropy > gpu::MAX_SAMPLER_ANISOTROPY)),
        "");
  CHECK(!(info.anisotropy_enable && (info.max_anisotropy < 1.0)), "");

  VkSamplerCreateInfo create_info{
    .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .pNext                   = nullptr,
    .flags                   = 0,
    .magFilter               = (VkFilter) info.mag_filter,
    .minFilter               = (VkFilter) info.min_filter,
    .mipmapMode              = (VkSamplerMipmapMode) info.mip_map_mode,
    .addressModeU            = (VkSamplerAddressMode) info.address_mode_u,
    .addressModeV            = (VkSamplerAddressMode) info.address_mode_v,
    .addressModeW            = (VkSamplerAddressMode) info.address_mode_w,
    .mipLodBias              = info.mip_lod_bias,
    .anisotropyEnable        = (VkBool32) info.anisotropy_enable,
    .maxAnisotropy           = info.max_anisotropy,
    .compareEnable           = (VkBool32) info.compare_enable,
    .compareOp               = (VkCompareOp) info.compare_op,
    .minLod                  = info.min_lod,
    .maxLod                  = info.max_lod,
    .borderColor             = (VkBorderColor) info.border_color,
    .unnormalizedCoordinates = (VkBool32) info.unnormalized_coordinates};

  VkSampler vk_sampler;
  VkResult  result =
    vk_table.CreateSampler(vk_dev, &create_info, nullptr, &vk_sampler);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_sampler, VK_OBJECT_TYPE_SAMPLER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT);

  return Ok{(gpu::Sampler) vk_sampler};
}

Result<gpu::Shader, Status> Device::create_shader(gpu::ShaderInfo const & info)
{
  CHECK(info.spirv_code.size_bytes() > 0, "");

  VkShaderModuleCreateInfo create_info{
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .codeSize = info.spirv_code.size_bytes(),
    .pCode    = info.spirv_code.data()};

  VkShaderModule vk_shader;
  VkResult       result =
    vk_table.CreateShaderModule(vk_dev, &create_info, nullptr, &vk_shader);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_shader, VK_OBJECT_TYPE_SHADER_MODULE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT);

  return Ok{(gpu::Shader) vk_shader};
}

Result<gpu::DescriptorSetLayout, Status> Device::create_descriptor_set_layout(
  gpu::DescriptorSetLayoutInfo const & info)
{
  u32                              num_descriptors     = 0;
  u32                              num_variable_length = 0;
  Array<u32, NUM_DESCRIPTOR_TYPES> sizing              = {};

  for (gpu::DescriptorBindingInfo const & info : info.bindings)
  {
    num_descriptors += info.count;
    sizing[(u32) info.type] += info.count;
    num_variable_length += info.is_variable_length ? 1 : 0;
  }

  u32 num_dynamic_storage_buffers =
    sizing[(u32) gpu::DescriptorType::DynamicStorageBuffer];
  u32 num_dynamic_uniform_buffers =
    sizing[(u32) gpu::DescriptorType::DynamicUniformBuffer];

  CHECK(info.bindings.size() > 0, "");
  CHECK(info.bindings.size() <= gpu::MAX_DESCRIPTOR_SET_BINDINGS, "");
  CHECK(num_dynamic_storage_buffers <=
          gpu::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS,
        "");
  CHECK(num_dynamic_uniform_buffers <=
          gpu::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS,
        "");
  CHECK(num_descriptors <= gpu::MAX_DESCRIPTOR_SET_DESCRIPTORS, "");
  CHECK(num_variable_length <= 1, "");
  CHECK(!(num_variable_length > 0 &&
          (num_dynamic_storage_buffers > 0 || num_dynamic_uniform_buffers > 0)),
        "");

  for (auto [i, binding] : enumerate<u32>(info.bindings))
  {
    CHECK(binding.count > 0, "");
    CHECK(binding.count <= gpu::MAX_BINDING_DESCRIPTORS, "");
    CHECK(!(binding.is_variable_length && (i != (info.bindings.size() - 1))),
          "");
  }

  InplaceVec<VkDescriptorSetLayoutBinding, gpu::MAX_DESCRIPTOR_SET_BINDINGS>
    vk_bindings;
  InplaceVec<VkDescriptorBindingFlagsEXT, gpu::MAX_DESCRIPTOR_SET_BINDINGS>
    vk_binding_flags;

  for (auto [i, binding] : enumerate<u32>(info.bindings))
  {
    VkShaderStageFlags stage_flags =
      (VkShaderStageFlags) (binding.type ==
                                gpu::DescriptorType::InputAttachment ?
                              VK_SHADER_STAGE_FRAGMENT_BIT :
                              VK_SHADER_STAGE_ALL);
    vk_bindings
      .push(VkDescriptorSetLayoutBinding{.binding = i,
                                         .descriptorType =
                                           (VkDescriptorType) binding.type,
                                         .descriptorCount    = binding.count,
                                         .stageFlags         = stage_flags,
                                         .pImmutableSamplers = nullptr})
      .unwrap();

    VkDescriptorBindingFlagsEXT const vk_flags =
      VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT |
      VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
      (binding.is_variable_length ?
         VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT :
         0);
    vk_binding_flags.push(vk_flags).unwrap();
  }

  VkDescriptorSetLayoutBindingFlagsCreateInfoEXT vk_binding_flags_create_info{
    .sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
    .pNext         = nullptr,
    .bindingCount  = size32(vk_binding_flags),
    .pBindingFlags = vk_binding_flags.data()};

  VkDescriptorSetLayoutCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .pNext = &vk_binding_flags_create_info,
    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
    .bindingCount = size32(vk_bindings),
    .pBindings    = vk_bindings.data()};

  VkDescriptorSetLayout vk_layout;
  VkResult result = vk_table.CreateDescriptorSetLayout(vk_dev, &create_info,
                                                       nullptr, &vk_layout);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_layout_{[&] {
    if (vk_layout != nullptr)
    {
      vk_table.DestroyDescriptorSetLayout(vk_dev, vk_layout, nullptr);
    }
  }};

  set_resource_name(info.label, vk_layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);

  DescriptorSetLayout * layout;
  if (!allocator->nalloc(1, layout))
  {
    return Err{Status::OutOfHostMemory};
  }

  InplaceVec<gpu::DescriptorBindingInfo, gpu::MAX_DESCRIPTOR_SET_BINDINGS>
    bindings;
  bindings.extend(info.bindings).unwrap();

  new (layout) DescriptorSetLayout{.vk_layout           = vk_layout,
                                   .bindings            = bindings,
                                   .sizing              = sizing,
                                   .num_variable_length = num_variable_length};

  vk_layout = nullptr;

  return Ok{(gpu::DescriptorSetLayout) layout};
}

Result<gpu::DescriptorSet, Status>
  Device::create_descriptor_set(gpu::DescriptorSetInfo const & info)
{
  DescriptorSetLayout * const layout = (DescriptorSetLayout *) info.layout;
  CHECK(info.variable_lengths.size() == layout->num_variable_length, "");

  {
    // variablie length binding index
    u32 vlb_idx = 0;
    for (gpu::DescriptorBindingInfo const & binding : layout->bindings)
    {
      if (binding.is_variable_length)
      {
        CHECK(info.variable_lengths[vlb_idx] <= binding.count, "");
        CHECK(info.variable_lengths[vlb_idx] > 0, "");
        vlb_idx++;
      }
    }
  }

  InplaceVec<u32, gpu::MAX_DESCRIPTOR_SET_BINDINGS> bindings_sizes;

  Array<u32, NUM_DESCRIPTOR_TYPES> type_count;

  {
    u32 vlb_idx = 0;
    for (gpu::DescriptorBindingInfo const & binding : layout->bindings)
    {
      u32 count = 0;
      if (!binding.is_variable_length)
      {
        count = binding.count;
      }
      else
      {
        count = info.variable_lengths[vlb_idx];
        vlb_idx++;
      }

      type_count[(u32) binding.type] += count;
      bindings_sizes.push(count).unwrap();
    }
  }

  InplaceVec<VkDescriptorPoolSize, NUM_DESCRIPTOR_TYPES> pool_sizes;

  for (auto [i, count] : enumerate<u32>(type_count))
  {
    if (type_count[i] == 0)
    {
      continue;
    }

    pool_sizes
      .push(VkDescriptorPoolSize{.type            = (VkDescriptorType) i,
                                 .descriptorCount = type_count[i]})
      .unwrap();
  }

  VkDescriptorPoolCreateInfo create_info{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext = nullptr,
    .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT |
             VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
    .maxSets       = 1,
    .poolSizeCount = size32(pool_sizes),
    .pPoolSizes    = pool_sizes.data()};

  VkDescriptorPool vk_pool;
  VkResult         result =
    vk_table.CreateDescriptorPool(vk_dev, &create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_pool_{
    [&] { vk_table.DestroyDescriptorPool(vk_dev, vk_pool, nullptr); }};

  VkDescriptorSetVariableDescriptorCountAllocateInfoEXT var_alloc_info{
    .sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
    .pNext              = nullptr,
    .descriptorSetCount = size32(info.variable_lengths),
    .pDescriptorCounts  = info.variable_lengths.data()};

  VkDescriptorSetAllocateInfo alloc_info{
    .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .pNext              = &var_alloc_info,
    .descriptorPool     = vk_pool,
    .descriptorSetCount = 1,
    .pSetLayouts        = &layout->vk_layout};

  VkDescriptorSet vk_set;
  result = vk_table.AllocateDescriptorSets(vk_dev, &alloc_info, &vk_set);

  // must not have these errors
  CHECK(result != VK_ERROR_OUT_OF_POOL_MEMORY &&
          result != VK_ERROR_FRAGMENTED_POOL,
        "");

  if (result != VK_SUCCESS)
  {
    return Err{(gpu::Status) result};
  }

  set_resource_name(info.label, vk_set, VK_OBJECT_TYPE_DESCRIPTOR_SET,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT);

  set_resource_name(info.label, vk_pool, VK_OBJECT_TYPE_DESCRIPTOR_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT);

  InplaceVec<DescriptorBinding, gpu::MAX_DESCRIPTOR_SET_BINDINGS> bindings;

  for (auto [size, info] : zip(bindings_sizes, layout->bindings))
  {
    bindings
      .push(DescriptorBinding{
        .sync_resources = none, .type = info.type, .size = size})
      .unwrap();
  }

  for (auto & binding : bindings)
  {
    switch (binding.type)
    {
      case gpu::DescriptorType::DynamicStorageBuffer:
      case gpu::DescriptorType::DynamicUniformBuffer:
      case gpu::DescriptorType::StorageBuffer:
      case gpu::DescriptorType::UniformBuffer:
      case gpu::DescriptorType::StorageTexelBuffer:
      case gpu::DescriptorType::UniformTexelBuffer:
      {
        auto resources = Vec<Buffer *>::make(binding.size, allocator).unwrap();
        resources.resize(binding.size).unwrap();
        binding.sync_resources = std::move(resources);
      }
      break;

      case gpu::DescriptorType::SampledImage:
      case gpu::DescriptorType::CombinedImageSampler:
      case gpu::DescriptorType::StorageImage:
      case gpu::DescriptorType::InputAttachment:
      {
        auto resources = Vec<Image *>::make(binding.size, allocator).unwrap();
        resources.resize(binding.size).unwrap();
        binding.sync_resources = std::move(resources);
      }
      break;

      case gpu::DescriptorType::Sampler:
        break;

      default:
        break;
    }
  }

  DescriptorSet * set;

  if (!allocator->nalloc(1, set))
  {
    return Err{(gpu::Status) result};
  }

  new (set) DescriptorSet{
    .vk_set = vk_set, .vk_pool = vk_pool, .bindings = std::move(bindings)};

  vk_pool = nullptr;
  vk_set  = nullptr;

  return Ok{(gpu::DescriptorSet) set};
}

Result<gpu::PipelineCache, Status>
  Device::create_pipeline_cache(gpu::PipelineCacheInfo const & info)
{
  VkPipelineCacheCreateInfo create_info{
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .initialDataSize = info.initial_data.size_bytes(),
    .pInitialData    = info.initial_data.data()};

  VkPipelineCache vk_cache;
  VkResult        result =
    vk_table.CreatePipelineCache(vk_dev, &create_info, nullptr, &vk_cache);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_cache, VK_OBJECT_TYPE_PIPELINE_CACHE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT);

  return Ok{(gpu::PipelineCache) vk_cache};
}

Result<gpu::ComputePipeline, Status>
  Device::create_compute_pipeline(gpu::ComputePipelineInfo const & info)
{
  CHECK(info.descriptor_set_layouts.size() <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS,
        "");
  CHECK(info.push_constants_size <= gpu::MAX_PUSH_CONSTANTS_SIZE, "");
  CHECK(is_aligned(4U, info.push_constants_size), "");
  CHECK(info.compute_shader.entry_point.size() > 0 &&
          info.compute_shader.entry_point.size() < 256,
        "");
  CHECK(info.compute_shader.shader != nullptr, "");

  InplaceVec<VkDescriptorSetLayout, gpu::MAX_PIPELINE_DESCRIPTOR_SETS>
    vk_descriptor_set_layouts;

  for (auto layout : info.descriptor_set_layouts)
  {
    vk_descriptor_set_layouts.push(((DescriptorSetLayout *) layout)->vk_layout)
      .unwrap();
  }

  VkSpecializationInfo vk_specialization{
    .mapEntryCount = size32(info.compute_shader.specialization_constants),
    .pMapEntries   = (VkSpecializationMapEntry const *)
                     info.compute_shader.specialization_constants.data(),
    .dataSize = info.compute_shader.specialization_constants_data.size_bytes(),
    .pData    = info.compute_shader.specialization_constants_data.data()};

  char entry_point[256];
  CHECK(to_c_str(info.compute_shader.entry_point, entry_point), "");

  VkPipelineShaderStageCreateInfo vk_stage{
    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext               = nullptr,
    .flags               = 0,
    .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
    .module              = (Shader) info.compute_shader.shader,
    .pName               = entry_point,
    .pSpecializationInfo = &vk_specialization};

  VkPushConstantRange push_constants_range{.stageFlags =
                                             VK_SHADER_STAGE_COMPUTE_BIT,
                                           .offset = 0,
                                           .size   = info.push_constants_size};

  VkPipelineLayoutCreateInfo layout_create_info{
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext                  = nullptr,
    .flags                  = 0,
    .setLayoutCount         = size32(vk_descriptor_set_layouts),
    .pSetLayouts            = vk_descriptor_set_layouts.data(),
    .pushConstantRangeCount = info.push_constants_size == 0 ? 0U : 1U,
    .pPushConstantRanges =
      info.push_constants_size == 0 ? nullptr : &push_constants_range};

  VkPipelineLayout vk_layout;
  VkResult result = vk_table.CreatePipelineLayout(vk_dev, &layout_create_info,
                                                  nullptr, &vk_layout);
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
  result = vk_table.CreateComputePipelines(
    vk_dev, info.cache == nullptr ? nullptr : (PipelineCache) info.cache, 1,
    &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyPipelineLayout(vk_dev, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_pipeline, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
  set_resource_name(info.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);

  ComputePipeline * pipeline;
  if (!allocator->nalloc(1, pipeline))
  {
    vk_table.DestroyPipelineLayout(vk_dev, vk_layout, nullptr);
    vk_table.DestroyPipeline(vk_dev, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
    ComputePipeline{.vk_pipeline         = vk_pipeline,
                    .vk_layout           = vk_layout,
                    .push_constants_size = info.push_constants_size,
                    .num_sets            = size32(info.descriptor_set_layouts)};

  return Ok{(gpu::ComputePipeline) pipeline};
}

Result<gpu::GraphicsPipeline, Status>
  Device::create_graphics_pipeline(gpu::GraphicsPipelineInfo const & info)
{
  CHECK(!(info.rasterization_state.polygon_mode != gpu::PolygonMode::Fill &&
          !phy_dev.vk_features.fillModeNonSolid),
        "");
  CHECK(info.descriptor_set_layouts.size() <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS,
        "");
  CHECK(info.push_constants_size <= gpu::MAX_PUSH_CONSTANTS_SIZE, "");
  CHECK(is_aligned(4U, info.push_constants_size), "");
  CHECK(!info.vertex_shader.entry_point.is_empty(), "");
  CHECK(!info.fragment_shader.entry_point.is_empty(), "");
  CHECK(info.vertex_attributes.size() <= gpu::MAX_VERTEX_ATTRIBUTES, "");
  CHECK(info.color_blend_state.attachments.size() <=
          gpu::MAX_PIPELINE_COLOR_ATTACHMENTS,
        "");

  InplaceVec<VkDescriptorSetLayout, gpu::MAX_PIPELINE_DESCRIPTOR_SETS>
    vk_descriptor_set_layouts;

  for (auto layout : info.descriptor_set_layouts)
  {
    vk_descriptor_set_layouts.push(((DescriptorSetLayout *) layout)->vk_layout)
      .unwrap();
  }

  VkSpecializationInfo vk_vs_specialization{
    .mapEntryCount = size32(info.vertex_shader.specialization_constants),
    .pMapEntries   = (VkSpecializationMapEntry const *)
                     info.vertex_shader.specialization_constants.data(),
    .dataSize = info.vertex_shader.specialization_constants_data.size_bytes(),
    .pData    = info.vertex_shader.specialization_constants_data.data()};

  VkSpecializationInfo vk_fs_specialization{
    .mapEntryCount = size32(info.fragment_shader.specialization_constants),
    .pMapEntries   = (VkSpecializationMapEntry const *)
                     info.fragment_shader.specialization_constants.data(),
    .dataSize = info.fragment_shader.specialization_constants_data.size_bytes(),
    .pData    = info.fragment_shader.specialization_constants_data.data()};

  char vs_entry_point[256];
  char fs_entry_point[256];
  CHECK(to_c_str(info.vertex_shader.entry_point, vs_entry_point), "");
  CHECK(to_c_str(info.fragment_shader.entry_point, fs_entry_point), "");

  VkPipelineShaderStageCreateInfo vk_stages[2] = {
    {.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
     .pNext               = nullptr,
     .flags               = 0,
     .stage               = VK_SHADER_STAGE_VERTEX_BIT,
     .module              = (Shader) info.vertex_shader.shader,
     .pName               = vs_entry_point,
     .pSpecializationInfo = &vk_vs_specialization},
    {.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
     .pNext               = nullptr,
     .flags               = 0,
     .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
     .module              = (Shader) info.fragment_shader.shader,
     .pName               = fs_entry_point,
     .pSpecializationInfo = &vk_fs_specialization}
  };

  VkPushConstantRange push_constants_range{.stageFlags = VK_SHADER_STAGE_ALL,
                                           .offset     = 0,
                                           .size = info.push_constants_size};

  VkPipelineLayoutCreateInfo layout_create_info{
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext                  = nullptr,
    .flags                  = 0,
    .setLayoutCount         = size32(vk_descriptor_set_layouts),
    .pSetLayouts            = vk_descriptor_set_layouts.data(),
    .pushConstantRangeCount = info.push_constants_size == 0 ? 0U : 1U,
    .pPushConstantRanges =
      info.push_constants_size == 0 ? nullptr : &push_constants_range};

  VkPipelineLayout vk_layout;

  VkResult result = vk_table.CreatePipelineLayout(vk_dev, &layout_create_info,
                                                  nullptr, &vk_layout);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  InplaceVec<VkVertexInputBindingDescription, gpu::MAX_VERTEX_ATTRIBUTES>
    input_bindings;

  for (auto binding : info.vertex_input_bindings)
  {
    input_bindings
      .push(VkVertexInputBindingDescription{
        .binding   = binding.binding,
        .stride    = binding.stride,
        .inputRate = (VkVertexInputRate) binding.input_rate})
      .unwrap();
  }

  InplaceVec<VkVertexInputAttributeDescription, gpu::MAX_VERTEX_ATTRIBUTES>
    attributes;

  for (auto attribute : info.vertex_attributes)
  {
    attributes
      .push(
        VkVertexInputAttributeDescription{.location = attribute.location,
                                          .binding  = attribute.binding,
                                          .format = (VkFormat) attribute.format,
                                          .offset = attribute.offset})
      .unwrap();
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_state{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .vertexBindingDescriptionCount   = size32(input_bindings),
    .pVertexBindingDescriptions      = input_bindings.data(),
    .vertexAttributeDescriptionCount = size32(attributes),
    .pVertexAttributeDescriptions    = attributes.data()};

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
    .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .topology = (VkPrimitiveTopology) info.primitive_topology,
    .primitiveRestartEnable = VK_FALSE};

  VkViewport viewport{
    .x = 0, .y = 0, .width = 0, .height = 0, .minDepth = 0, .maxDepth = 1};
  VkRect2D scissor{
    .offset = {0, 0},
      .extent = {0, 0}
  };

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
    .depthClampEnable = (VkBool32) info.rasterization_state.depth_clamp_enable,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode     = (VkPolygonMode) info.rasterization_state.polygon_mode,
    .cullMode        = (VkCullModeFlags) info.rasterization_state.cull_mode,
    .frontFace       = (VkFrontFace) info.rasterization_state.front_face,
    .depthBiasEnable = (VkBool32) info.rasterization_state.depth_bias_enable,
    .depthBiasConstantFactor =
      info.rasterization_state.depth_bias_constant_factor,
    .depthBiasClamp       = info.rasterization_state.depth_bias_clamp,
    .depthBiasSlopeFactor = info.rasterization_state.depth_bias_slope_factor,
    .lineWidth            = 1.0F};

  VkPipelineMultisampleStateCreateInfo multisample_state{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .rasterizationSamples =
      (VkSampleCountFlagBits) info.rasterization_state.sample_count,
    .sampleShadingEnable   = (VkBool32) false,
    .minSampleShading      = 1,
    .pSampleMask           = nullptr,
    .alphaToCoverageEnable = (VkBool32) false,
    .alphaToOneEnable      = (VkBool32) false};

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthTestEnable  = (VkBool32) info.depth_stencil_state.depth_test_enable,
    .depthWriteEnable = (VkBool32) info.depth_stencil_state.depth_write_enable,
    .depthCompareOp   = (VkCompareOp) info.depth_stencil_state.depth_compare_op,
    .depthBoundsTestEnable =
      (VkBool32) info.depth_stencil_state.depth_bounds_test_enable,
    .stencilTestEnable =
      (VkBool32) info.depth_stencil_state.stencil_test_enable,
    .front =
      VkStencilOpState{
                       .failOp = (VkStencilOp) info.depth_stencil_state.front_stencil.fail_op,
                       .passOp = (VkStencilOp) info.depth_stencil_state.front_stencil.pass_op,
                       .depthFailOp =
          (VkStencilOp) info.depth_stencil_state.front_stencil.depth_fail_op,
                       .compareOp =
          (VkCompareOp) info.depth_stencil_state.front_stencil.compare_op,
                       .compareMask = info.depth_stencil_state.front_stencil.compare_mask,
                       .writeMask   = info.depth_stencil_state.front_stencil.write_mask,
                       .reference   = info.depth_stencil_state.front_stencil.reference},
    .back =
      VkStencilOpState{
                       .failOp = (VkStencilOp) info.depth_stencil_state.back_stencil.fail_op,
                       .passOp = (VkStencilOp) info.depth_stencil_state.back_stencil.pass_op,
                       .depthFailOp =
          (VkStencilOp) info.depth_stencil_state.back_stencil.depth_fail_op,
                       .compareOp =
          (VkCompareOp) info.depth_stencil_state.back_stencil.compare_op,
                       .compareMask = info.depth_stencil_state.back_stencil.compare_mask,
                       .writeMask   = info.depth_stencil_state.back_stencil.write_mask,
                       .reference   = info.depth_stencil_state.back_stencil.reference },
    .minDepthBounds = info.depth_stencil_state.min_depth_bounds,
    .maxDepthBounds = info.depth_stencil_state.max_depth_bounds
  };

  InplaceVec<VkPipelineColorBlendAttachmentState,
             gpu::MAX_PIPELINE_COLOR_ATTACHMENTS>
    attachment_states;

  for (auto state : info.color_blend_state.attachments)
  {
    attachment_states
      .push(VkPipelineColorBlendAttachmentState{
        .blendEnable         = (VkBool32) state.blend_enable,
        .srcColorBlendFactor = (VkBlendFactor) state.src_color_blend_factor,
        .dstColorBlendFactor = (VkBlendFactor) state.dst_color_blend_factor,
        .colorBlendOp        = (VkBlendOp) state.color_blend_op,
        .srcAlphaBlendFactor = (VkBlendFactor) state.src_alpha_blend_factor,
        .dstAlphaBlendFactor = (VkBlendFactor) state.dst_alpha_blend_factor,
        .alphaBlendOp        = (VkBlendOp) state.alpha_blend_op,
        .colorWriteMask      = (VkColorComponentFlags) state.color_write_mask})
      .unwrap();
  }

  VkPipelineColorBlendStateCreateInfo color_blend_state{
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .logicOpEnable   = (VkBool32) info.color_blend_state.logic_op_enable,
    .logicOp         = (VkLogicOp) info.color_blend_state.logic_op,
    .attachmentCount = size32(attachment_states),
    .pAttachments    = attachment_states.data(),
    .blendConstants  = {info.color_blend_state.blend_constant.x,
                        info.color_blend_state.blend_constant.y,
                        info.color_blend_state.blend_constant.z,
                        info.color_blend_state.blend_constant.w}
  };

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
    .dynamicStateCount = size32(dynamic_states),
    .pDynamicStates    = dynamic_states};

  InplaceVec<VkFormat, gpu::MAX_PIPELINE_COLOR_ATTACHMENTS> color_formats;

  for (auto fmt : info.color_formats)
  {
    color_formats.push((VkFormat) fmt).unwrap();
  }

  VkFormat depth_format =
    (VkFormat) info.depth_format.unwrap_or(gpu::Format::Undefined);
  VkFormat stencil_format =
    (VkFormat) info.stencil_format.unwrap_or(gpu::Format::Undefined);

  VkPipelineRenderingCreateInfoKHR rendering_info{
    .sType    = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
    .pNext    = nullptr,
    .viewMask = 0,
    .colorAttachmentCount    = size32(color_formats),
    .pColorAttachmentFormats = color_formats.data(),
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
  result = vk_table.CreateGraphicsPipelines(
    vk_dev, info.cache == nullptr ? nullptr : (PipelineCache) info.cache, 1,
    &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyPipelineLayout(vk_dev, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_pipeline, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
  set_resource_name(info.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);

  GraphicsPipeline * pipeline;
  if (!allocator->nalloc(1, pipeline))
  {
    vk_table.DestroyPipelineLayout(vk_dev, vk_layout, nullptr);
    vk_table.DestroyPipeline(vk_dev, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
    GraphicsPipeline{.vk_pipeline         = vk_pipeline,
                     .vk_layout           = vk_layout,
                     .push_constants_size = info.push_constants_size,
                     .num_sets            = size32(info.descriptor_set_layouts),
                     .depth_fmt           = info.depth_format,
                     .stencil_fmt         = info.stencil_format,
                     .sample_count = info.rasterization_state.sample_count};

  pipeline->color_fmts.extend(info.color_formats).unwrap();

  return Ok{(gpu::GraphicsPipeline) pipeline};
}

/// old swapchain will be retired and destroyed irregardless of whether new
/// swapchain recreation fails.
VkResult Device::recreate_swapchain(Swapchain * swapchain)
{
  CHECK(swapchain->info.preferred_extent.x > 0, "");
  CHECK(swapchain->info.preferred_extent.y > 0, "");
  CHECK(swapchain->info.preferred_buffering <= gpu::MAX_SWAPCHAIN_IMAGES, "");

  VkSurfaceCapabilitiesKHR surface_capabilities;
  VkResult result = instance->vk_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(
    phy_dev.vk_phy_dev, swapchain->vk_surface, &surface_capabilities);

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
                 (VkImageUsageFlags) swapchain->info.usage),
        "");
  CHECK(has_bits(surface_capabilities.supportedCompositeAlpha,
                 (VkImageUsageFlags) swapchain->info.composite_alpha),
        "");

  // take ownership of internal data for re-use/release
  VkSwapchainKHR old_vk_swapchain = swapchain->vk_swapchain;
  defer          old_vk_swapchain_{[&] {
    if (old_vk_swapchain != nullptr)
    {
      vk_table.DestroySwapchainKHR(vk_dev, old_vk_swapchain, nullptr);
    }
  }};

  swapchain->is_out_of_date  = true;
  swapchain->is_optimal      = false;
  swapchain->is_zero_sized   = false;
  swapchain->format          = gpu::SurfaceFormat{};
  swapchain->usage           = gpu::ImageUsage::None;
  swapchain->present_mode    = gpu::PresentMode::Immediate;
  swapchain->extent          = Vec2U{};
  swapchain->composite_alpha = gpu::CompositeAlpha::None;
  swapchain->image_impls     = {};
  swapchain->images          = {};
  swapchain->vk_images       = {};
  swapchain->current_image   = 0;
  swapchain->vk_swapchain    = nullptr;

  VkExtent2D vk_extent;

  if (surface_capabilities.currentExtent.width == 0xFFFF'FFFFU &&
      surface_capabilities.currentExtent.height == 0xFFFF'FFFFU)
  {
    vk_extent.width  = clamp(swapchain->info.preferred_extent.x,
                             surface_capabilities.minImageExtent.width,
                             surface_capabilities.maxImageExtent.width);
    vk_extent.height = clamp(swapchain->info.preferred_extent.y,
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
    min_image_count = clamp(swapchain->info.preferred_buffering,
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
    .imageFormat      = (VkFormat) swapchain->info.format.format,
    .imageColorSpace  = (VkColorSpaceKHR) swapchain->info.format.color_space,
    .imageExtent      = vk_extent,
    .imageArrayLayers = 1,
    .imageUsage       = (VkImageUsageFlags) swapchain->info.usage,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 1,
    .pQueueFamilyIndices   = nullptr,
    .preTransform          = surface_capabilities.currentTransform,
    .compositeAlpha =
      (VkCompositeAlphaFlagBitsKHR) swapchain->info.composite_alpha,
    .presentMode  = (VkPresentModeKHR) swapchain->info.present_mode,
    .clipped      = VK_TRUE,
    .oldSwapchain = old_vk_swapchain};

  VkSwapchainKHR new_vk_swapchain;

  result = vk_table.CreateSwapchainKHR(vk_dev, &create_info, nullptr,
                                       &new_vk_swapchain);

  CHECK(result == VK_SUCCESS, "");

  defer new_vk_swapchain_{[&] {
    if (new_vk_swapchain != nullptr)
    {
      vk_table.DestroySwapchainKHR(vk_dev, new_vk_swapchain, nullptr);
    }
  }};

  u32 num_images;
  result = vk_table.GetSwapchainImagesKHR(vk_dev, new_vk_swapchain, &num_images,
                                          nullptr);

  CHECK(result == VK_SUCCESS, "");

  swapchain->vk_images.resize(num_images).unwrap();
  swapchain->image_impls.resize(num_images).unwrap();
  swapchain->images.resize(num_images).unwrap();

  result = vk_table.GetSwapchainImagesKHR(vk_dev, new_vk_swapchain, &num_images,
                                          swapchain->vk_images.data());

  CHECK(result == VK_SUCCESS, "");

  for (auto [vk, impl, img] :
       zip(swapchain->vk_images, swapchain->image_impls, swapchain->images))
  {
    InplaceVec<ImageAspectState, MAX_IMAGE_ASPECTS> states;
    states.resize(1).unwrap();    // 1 color aspect
    impl = Image{
      .info =
        gpu::ImageInfo{.type    = gpu::ImageType::Type2D,
                       .format  = swapchain->info.format.format,
                       .usage   = swapchain->info.usage,
                       .aspects = gpu::ImageAspects::Color,
                       .extent  = Vec3U{vk_extent.width, vk_extent.height, 1},
                       .mip_levels   = 1,
                       .array_layers = 1},
      .is_swapchain_image  = true,
      .vk_image            = vk,
      .vma_allocation      = nullptr,
      .vma_allocation_info = {},
      .aspect_states       = states
    };
    img = (gpu::Image) &impl;
  }

  set_resource_name(swapchain->info.label, new_vk_swapchain,
                    VK_OBJECT_TYPE_SWAPCHAIN_KHR,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT);
  for (u32 i = 0; i < num_images; i++)
  {
    set_resource_name(swapchain->info.label, swapchain->vk_images[i],
                      VK_OBJECT_TYPE_IMAGE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);
  }

  swapchain->is_out_of_date  = false;
  swapchain->is_optimal      = true;
  swapchain->is_zero_sized   = false;
  swapchain->format          = swapchain->info.format;
  swapchain->usage           = swapchain->info.usage;
  swapchain->present_mode    = swapchain->info.present_mode;
  swapchain->extent.x        = vk_extent.width;
  swapchain->extent.y        = vk_extent.height;
  swapchain->composite_alpha = swapchain->info.composite_alpha;
  swapchain->current_image   = 0;
  swapchain->vk_swapchain    = new_vk_swapchain;
  new_vk_swapchain           = nullptr;

  return VK_SUCCESS;
}

Status Device::init_command_encoder(CommandEncoder * enc)
{
  VkCommandPoolCreateInfo command_pool_create_info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = nullptr,
    .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = queue_family};

  VkCommandPool vk_command_pool;
  VkResult      result = vk_table.CreateCommandPool(
    vk_dev, &command_pool_create_info, nullptr, &vk_command_pool);

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
  result =
    vk_table.AllocateCommandBuffers(vk_dev, &allocate_info, &vk_command_buffer);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyCommandPool(vk_dev, vk_command_pool, nullptr);
    return (Status) result;
  }

  set_resource_name("Frame Command Buffer"_str, vk_command_buffer,
                    VK_OBJECT_TYPE_COMMAND_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT);

  new (enc) CommandEncoder{};

  enc->allocator         = allocator;
  enc->dev               = this;
  enc->arg_pool          = ArenaPool{allocator};
  enc->vk_command_pool   = vk_command_pool;
  enc->vk_command_buffer = vk_command_buffer;
  enc->status            = Status::Success;
  enc->state             = CommandEncoderState::Reset;
  enc->render_ctx        = RenderPassContext{
           .arg_pool     = ArenaPool{allocator},
           .command_pool = ArenaPool{allocator},
           .commands     = Vec<Command>{enc->render_ctx.command_pool.ref()}};
  enc->compute_ctx = {};

  return Status::Success;
}

void Device::uninit(CommandEncoder * enc)
{
  if (enc == nullptr)
  {
    return;
  }
  enc->render_ctx.commands.reset();
  vk_table.DestroyCommandPool(vk_dev, enc->vk_command_pool, nullptr);
}

Status Device::init_frame_context(u32 buffering)
{
  FrameContext & ctx = frame_ctx;
  ctx.tail_frame     = 0;
  ctx.current_frame  = 0;
  ctx.ring_index     = 0;
  ctx.tail_frame     = 0;

  bool success = false;

  defer _{[&] {
    if (!success)
    {
      for (auto & enc : ctx.encoders.view().rev())
      {
        uninit(&enc);
      }
      ctx.encoders.clear();
      ctx.encoders_impl.clear();

      for (auto & sem : ctx.acquire_semaphores.view().rev())
      {
        vk_table.DestroySemaphore(vk_dev, sem, nullptr);
      }
      ctx.acquire_semaphores.clear();

      for (auto & fence : ctx.submit_fences.view().rev())
      {
        vk_table.DestroyFence(vk_dev, fence, nullptr);
      }
      ctx.submit_fences.clear();

      for (auto & sem : ctx.submit_semaphores.view().rev())
      {
        vk_table.DestroySemaphore(vk_dev, sem, nullptr);
      }
      ctx.submit_semaphores.clear();
    }
  }};

  for (auto i : range(buffering))
  {
    Status status = init_command_encoder(ctx.encoders.data() + i);
    if (status != Status::Success)
    {
      return status;
    }
    ctx.encoders.extend_uninit(1).unwrap();
  }

  VkSemaphoreCreateInfo sem_info{.sType =
                                   VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0};

  for (auto i : range(buffering))
  {
    VkResult result = vk_table.CreateSemaphore(
      vk_dev, &sem_info, nullptr, ctx.acquire_semaphores.data() + i);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }
    ctx.acquire_semaphores.extend_uninit(1).unwrap();
    set_resource_name("Frame Acquire Semaphore"_str, ctx.acquire_semaphores[i],
                      VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
  }

  VkFenceCreateInfo fence_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                               .pNext = nullptr,
                               .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  for (auto i : range(buffering))
  {
    VkResult result = vk_table.CreateFence(vk_dev, &fence_info, nullptr,
                                           ctx.submit_fences.data() + i);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }
    ctx.submit_fences.extend_uninit(1).unwrap();
    set_resource_name("Frame Submit Fence"_str, ctx.submit_fences[i],
                      VK_OBJECT_TYPE_FENCE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);
  }

  for (auto i : range(buffering))
  {
    VkResult result = vk_table.CreateSemaphore(
      vk_dev, &sem_info, nullptr, ctx.submit_semaphores.data() + i);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }
    ctx.submit_semaphores.extend_uninit(1).unwrap();
    set_resource_name("Frame Submit Semaphore"_str, ctx.submit_semaphores[i],
                      VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
  }

  success = true;

  // self-referential
  for (auto & enc : ctx.encoders)
  {
    ctx.encoders_impl.push(&enc).unwrap();
  }

  return Status::Success;
}

void Device::uninit()
{
  for (auto & enc : frame_ctx.encoders.view().rev())
  {
    uninit(&enc);
  }

  for (VkSemaphore sem : frame_ctx.acquire_semaphores.view().rev())
  {
    vk_table.DestroySemaphore(vk_dev, sem, nullptr);
  }

  for (VkFence fence : frame_ctx.submit_fences.view().rev())
  {
    vk_table.DestroyFence(vk_dev, fence, nullptr);
  }

  for (VkSemaphore sem : frame_ctx.submit_semaphores.view().rev())
  {
    vk_table.DestroySemaphore(vk_dev, sem, nullptr);
  }
}

Result<gpu::Swapchain, Status>
  Device::create_swapchain(gpu::Surface               surface,
                           gpu::SwapchainInfo const & info)
{
  CHECK(info.preferred_extent.x > 0, "");
  CHECK(info.preferred_extent.y > 0, "");

  Swapchain * swapchain;
  if (!allocator->nalloc(1, swapchain))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (swapchain) Swapchain{.info            = info,
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
                            .current_image   = 0,
                            .vk_swapchain    = nullptr,
                            .vk_surface      = (VkSurfaceKHR) surface};

  return Ok{(gpu::Swapchain) swapchain};
}

Result<gpu::TimeStampQuery, Status> Device::create_timestamp_query(u32 count)
{
  CHECK(count > 0, "");

  VkQueryPoolCreateInfo create_info{.sType =
                                      VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                                    .pNext      = nullptr,
                                    .flags      = 0,
                                    .queryType  = VK_QUERY_TYPE_TIMESTAMP,
                                    .queryCount = count,
                                    .pipelineStatistics = 0};
  VkQueryPool           vk_pool;
  VkResult              result =
    vk_table.CreateQueryPool(vk_dev, &create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{(gpu::TimeStampQuery) vk_pool};
}

Result<gpu::StatisticsQuery, Status> Device::create_statistics_query(u32 count)
{
  CHECK(count > 0, "");

  if (phy_dev.vk_features.pipelineStatisticsQuery != VK_TRUE)
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
    .queryCount         = count,
    .pipelineStatistics = QUERY_STATS};

  VkQueryPool vk_pool;
  VkResult    result =
    vk_table.CreateQueryPool(vk_dev, &create_info, nullptr, &vk_pool);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{(gpu::StatisticsQuery) vk_pool};
}

void Device::uninit(gpu::Buffer buffer_)
{
  Buffer * const buffer = (Buffer *) buffer_;

  if (buffer == nullptr)
  {
    return;
  }

  for (auto binder : buffer->state.binders)
  {
    binder.set->bindings[binder.binding].sync_resources[v2][binder.element] =
      nullptr;
  }

  vmaDestroyBuffer(vma_allocator, buffer->vk_buffer, buffer->vma_allocation);
  buffer->~Buffer();
  allocator->ndealloc(1, buffer);
}

void Device::uninit(gpu::BufferView buffer_view_)
{
  BufferView * const buffer_view = (BufferView *) buffer_view_;

  if (buffer_view == nullptr)
  {
    return;
  }

  vk_table.DestroyBufferView(vk_dev, buffer_view->vk_view, nullptr);
  buffer_view->~BufferView();
  allocator->ndealloc(1, buffer_view);
}

void Device::uninit(gpu::Image image_)
{
  Image * const image = (Image *) image_;

  if (image == nullptr)
  {
    return;
  }

  CHECK(!image->is_swapchain_image, "");

  for (auto & state : image->aspect_states)
  {
    for (auto binder : state.binders)
    {
      binder.set->bindings[binder.binding].sync_resources[v1][binder.element] =
        nullptr;
    }
  }

  vmaDestroyImage(vma_allocator, image->vk_image, image->vma_allocation);
  image->~Image();
  allocator->ndealloc(1, image);
}

void Device::uninit(gpu::ImageView image_view_)
{
  ImageView * const image_view = (ImageView *) image_view_;

  if (image_view == nullptr)
  {
    return;
  }

  vk_table.DestroyImageView(vk_dev, image_view->vk_view, nullptr);
  image_view->~ImageView();
  allocator->ndealloc(1, image_view);
}

void Device::uninit(gpu::Sampler sampler_)
{
  vk_table.DestroySampler(vk_dev, (Sampler) sampler_, nullptr);
}

void Device::uninit(gpu::Shader shader_)
{
  vk_table.DestroyShaderModule(vk_dev, (Shader) shader_, nullptr);
}

void Device::uninit(gpu::DescriptorSetLayout layout_)
{
  DescriptorSetLayout * const layout = (DescriptorSetLayout *) layout_;

  if (layout == nullptr)
  {
    return;
  }

  vk_table.DestroyDescriptorSetLayout(vk_dev, layout->vk_layout, nullptr);
  layout->~DescriptorSetLayout();
  allocator->ndealloc(1, layout);
}

void Device::uninit(gpu::DescriptorSet set_)
{
  DescriptorSet * const set = (DescriptorSet *) set_;

  if (set == nullptr)
  {
    return;
  }

  for (auto & binding : set->bindings)
  {
    binding.sync_resources.match(
      [&](None) {},
      [&](Span<Image *> images) {
        for (Image * img : images)
        {
          for (auto & state : img->aspect_states)
          {
            remove_binder(state.binders, set);
          }
        }
        allocator->ndealloc(images.size(), images.data());
      },
      [&](Span<Buffer *> buffers) {
        for (Buffer * buffer : buffers)
        {
          remove_binder(buffer->state.binders, set);
        }
        allocator->ndealloc(buffers.size(), buffers.data());
      });
  }

  vk_table.DestroyDescriptorPool(vk_dev, set->vk_pool, nullptr);
  set->~DescriptorSet();
  allocator->ndealloc(1, set);
}

void Device::uninit(gpu::PipelineCache cache_)
{
  vk_table.DestroyPipelineCache(vk_dev, (PipelineCache) cache_, nullptr);
}

void Device::uninit(gpu::ComputePipeline pipeline_)
{
  ComputePipeline * const pipeline = (ComputePipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  vk_table.DestroyPipeline(vk_dev, pipeline->vk_pipeline, nullptr);
  vk_table.DestroyPipelineLayout(vk_dev, pipeline->vk_layout, nullptr);
  pipeline->~ComputePipeline();
  allocator->ndealloc(1, pipeline);
}

void Device::uninit(gpu::GraphicsPipeline pipeline_)
{
  GraphicsPipeline * const pipeline = (GraphicsPipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  vk_table.DestroyPipeline(vk_dev, pipeline->vk_pipeline, nullptr);
  vk_table.DestroyPipelineLayout(vk_dev, pipeline->vk_layout, nullptr);
  pipeline->~GraphicsPipeline();
  allocator->ndealloc(1, pipeline);
}

void Device::uninit(gpu::Swapchain swapchain_)
{
  Swapchain * const swapchain = (Swapchain *) swapchain_;

  if (swapchain == nullptr)
  {
    return;
  }

  vk_table.DestroySwapchainKHR(vk_dev, swapchain->vk_swapchain, nullptr);
  swapchain->~Swapchain();
  allocator->ndealloc(1, swapchain);
}

void Device::uninit(gpu::TimeStampQuery query_)
{
  VkQueryPool const vk_pool = (VkQueryPool) query_;

  vk_table.DestroyQueryPool(vk_dev, vk_pool, nullptr);
}

void Device::uninit(gpu::StatisticsQuery query_)
{
  VkQueryPool const vk_pool = (VkQueryPool) query_;

  vk_table.DestroyQueryPool(vk_dev, vk_pool, nullptr);
}

gpu::FrameContext Device::get_frame_context()
{
  return gpu::FrameContext{.tail       = frame_ctx.tail_frame,
                           .current    = frame_ctx.current_frame,
                           .encoders   = frame_ctx.encoders_impl,
                           .ring_index = frame_ctx.ring_index};
}

Result<void *, Status> Device::map_buffer_memory(gpu::Buffer buffer_)
{
  Buffer * const buffer = (Buffer *) buffer_;
  CHECK(buffer->info.host_mapped, "");

  void *   map;
  VkResult result = vmaMapMemory(vma_allocator, buffer->vma_allocation, &map);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{(void *) map};
}

void Device::unmap_buffer_memory(gpu::Buffer buffer_)
{
  Buffer * const buffer = (Buffer *) buffer_;

  CHECK(buffer->info.host_mapped, "");

  vmaUnmapMemory(vma_allocator, buffer->vma_allocation);
}

Result<Void, Status>
  Device::invalidate_mapped_buffer_memory(gpu::Buffer      buffer_,
                                          gpu::MemoryRange range)
{
  Buffer * const buffer = (Buffer *) buffer_;

  CHECK(buffer->info.host_mapped, "");
  CHECK(range.offset < buffer->info.size, "");
  CHECK(range.size == gpu::WHOLE_SIZE ||
          (range.offset + range.size) <= buffer->info.size,
        "");

  VkResult result = vmaInvalidateAllocation(
    vma_allocator, buffer->vma_allocation, range.offset, range.size);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<Void, Status> Device::flush_mapped_buffer_memory(gpu::Buffer buffer_,
                                                        gpu::MemoryRange range)
{
  Buffer * const buffer = (Buffer *) buffer_;

  CHECK(buffer->info.host_mapped, "");
  CHECK(range.offset < buffer->info.size, "");
  CHECK(range.size == gpu::WHOLE_SIZE ||
          (range.offset + range.size) <= buffer->info.size,
        "");

  VkResult result = vmaFlushAllocation(vma_allocator, buffer->vma_allocation,
                                       range.offset, range.size);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<usize, Status> Device::get_pipeline_cache_size(gpu::PipelineCache cache)
{
  usize size;

  VkResult result = vk_table.GetPipelineCacheData(vk_dev, (PipelineCache) cache,
                                                  &size, nullptr);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{(usize) size};
}

Result<Void, Status> Device::get_pipeline_cache_data(gpu::PipelineCache cache,
                                                     Vec<u8> &          out)
{
  usize size = 0;

  VkResult result = vk_table.GetPipelineCacheData(vk_dev, (PipelineCache) cache,
                                                  &size, nullptr);

  if (result == VK_SUCCESS)
  {
    return Ok{};
  }

  if (result != VK_INCOMPLETE)
  {
    return Err{(Status) result};
  }

  usize const offset = out.size();

  if (!out.extend_uninit(size))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = vk_table.GetPipelineCacheData(vk_dev, (PipelineCache) cache, &size,
                                         out.data() + offset);

  if (result != VK_SUCCESS)
  {
    out.resize_uninit(offset).unwrap();
    return Err{(Status) result};
  }

  return Ok{};
}

Result<Void, Status>
  Device::merge_pipeline_cache(gpu::PipelineCache             dst,
                               Span<gpu::PipelineCache const> srcs)
{
  u32 const num_srcs = size32(srcs);

  CHECK(num_srcs > 0, "");

  VkResult result = vk_table.MergePipelineCaches(
    vk_dev, (PipelineCache) dst, num_srcs, (PipelineCache *) srcs.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

void Device::update_descriptor_set(gpu::DescriptorSetUpdate const & update)
{
  if (update.buffers.is_empty() && update.texel_buffers.is_empty() &&
      update.images.is_empty())
  {
    return;
  }

  DescriptorSet * const set = (DescriptorSet *) update.set;

  CHECK(update.binding < set->bindings.size(), "");
  DescriptorBinding & binding = set->bindings[update.binding];
  CHECK(update.element < binding.size, "");

  switch (binding.type)
  {
    case gpu::DescriptorType::DynamicStorageBuffer:
    case gpu::DescriptorType::StorageBuffer:
      for (gpu::BufferBinding const & b : update.buffers)
      {
        Buffer * buffer = (Buffer *) b.buffer;
        CHECK(buffer != nullptr, "");
        CHECK(has_bits(buffer->info.usage, gpu::BufferUsage::StorageBuffer),
              "");
        CHECK(is_valid_buffer_access(buffer->info.size, b.offset, b.size,
                                     gpu::BUFFER_OFFSET_ALIGNMENT),
              "");
      }
      break;

    case gpu::DescriptorType::DynamicUniformBuffer:
    case gpu::DescriptorType::UniformBuffer:
      for (gpu::BufferBinding const & b : update.buffers)
      {
        Buffer * buffer = (Buffer *) b.buffer;
        CHECK(buffer != nullptr, "");
        CHECK(has_bits(buffer->info.usage, gpu::BufferUsage::UniformBuffer),
              "");
        CHECK(is_valid_buffer_access(buffer->info.size, b.offset, b.size,
                                     gpu::BUFFER_OFFSET_ALIGNMENT),
              "");
      }
      break;

    case gpu::DescriptorType::Sampler:
    {
      for (gpu::ImageBinding const & b : update.images)
      {
        Sampler * sampler = (Sampler *) b.sampler;
        CHECK(sampler != nullptr, "");
      }
    }
    break;

    case gpu::DescriptorType::SampledImage:
    case gpu::DescriptorType::CombinedImageSampler:
    case gpu::DescriptorType::InputAttachment:
    {
      for (gpu::ImageBinding const & b : update.images)
      {
        ImageView * view = (ImageView *) b.image_view;
        CHECK(view != nullptr, "");
        Image * image = (Image *) view->info.image;
        CHECK(has_bits(image->info.usage, gpu::ImageUsage::Sampled), "");
        CHECK(image->info.sample_count == gpu::SampleCount::C1, "");
      }
    }
    break;

    case gpu::DescriptorType::StorageImage:
    {
      for (gpu::ImageBinding const & b : update.images)
      {
        ImageView * view = (ImageView *) b.image_view;
        CHECK(view != nullptr, "");
        Image * image = (Image *) view->info.image;
        CHECK(has_bits(image->info.usage, gpu::ImageUsage::Storage), "");
        CHECK(image->info.sample_count == gpu::SampleCount::C1, "");
      }
    }
    break;

    case gpu::DescriptorType::StorageTexelBuffer:
      for (gpu::BufferView const & v : update.texel_buffers)
      {
        BufferView * view = (BufferView *) v;
        CHECK(view != nullptr, "");
        Buffer * buffer = (Buffer *) view->info.buffer;
        CHECK(
          has_bits(buffer->info.usage, gpu::BufferUsage::StorageTexelBuffer),
          "");
      }
      break;

    case gpu::DescriptorType::UniformTexelBuffer:
      for (gpu::BufferView const & v : update.texel_buffers)
      {
        BufferView * view = (BufferView *) v;
        CHECK(view != nullptr, "");
        Buffer * buffer = (Buffer *) view->info.buffer;
        CHECK(
          has_bits(buffer->info.usage, gpu::BufferUsage::UniformTexelBuffer),
          "");
      }
      break;

    default:
      CHECK_UNREACHABLE();
  }

  VkDescriptorImageInfo *  pImageInfo       = nullptr;
  VkDescriptorBufferInfo * pBufferInfo      = nullptr;
  VkBufferView *           pTexelBufferView = nullptr;
  u32                      count            = 0;

  scratch
    .resize(max(sizeof(VkDescriptorBufferInfo), sizeof(VkDescriptorImageInfo),
                sizeof(VkBufferView)) *
            binding.size)
    .unwrap();

  switch (binding.type)
  {
    case gpu::DescriptorType::DynamicStorageBuffer:
    case gpu::DescriptorType::DynamicUniformBuffer:
    case gpu::DescriptorType::StorageBuffer:
    case gpu::DescriptorType::UniformBuffer:
    {
      pBufferInfo = (VkDescriptorBufferInfo *) scratch.data();
      count       = size32(update.buffers);
      for (u32 i = 0; i < size32(update.buffers); i++)
      {
        gpu::BufferBinding const & b      = update.buffers[i];
        Buffer *                   buffer = (Buffer *) b.buffer;
        pBufferInfo[i]                    = VkDescriptorBufferInfo{
                             .buffer = (buffer == nullptr) ? nullptr : buffer->vk_buffer,
                             .offset = b.offset,
                             .range  = b.size};
      }
    }
    break;

    case gpu::DescriptorType::Sampler:
    {
      pImageInfo = (VkDescriptorImageInfo *) scratch.data();
      count      = size32(update.images);
      for (u32 i = 0; i < update.images.size(); i++)
      {
        pImageInfo[i] =
          VkDescriptorImageInfo{.sampler   = (Sampler) update.images[i].sampler,
                                .imageView = nullptr,
                                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED};
      }
    }
    break;

    case gpu::DescriptorType::SampledImage:
    {
      pImageInfo = (VkDescriptorImageInfo *) scratch.data();
      count      = size32(update.images);
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView * view = (ImageView *) update.images[i].image_view;
        pImageInfo[i]    = VkDescriptorImageInfo{
             .sampler     = nullptr,
             .imageView   = (view == nullptr) ? nullptr : view->vk_view,
             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      }
    }
    break;

    case gpu::DescriptorType::CombinedImageSampler:
    {
      pImageInfo = (VkDescriptorImageInfo *) scratch.data();
      count      = size32(update.images);
      for (u32 i = 0; i < update.images.size(); i++)
      {
        gpu::ImageBinding const & b    = update.images[i];
        ImageView *               view = (ImageView *) b.image_view;
        pImageInfo[i]                  = VkDescriptorImageInfo{
                           .sampler     = (Sampler) b.sampler,
                           .imageView   = (view == nullptr) ? nullptr : view->vk_view,
                           .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      }
    }
    break;

    case gpu::DescriptorType::StorageImage:
    {
      pImageInfo = (VkDescriptorImageInfo *) scratch.data();
      count      = size32(update.images);
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView * view = (ImageView *) update.images[i].image_view;
        pImageInfo[i]    = VkDescriptorImageInfo{
             .sampler     = nullptr,
             .imageView   = (view == nullptr) ? nullptr : view->vk_view,
             .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
      }
    }
    break;

    case gpu::DescriptorType::InputAttachment:
    {
      pImageInfo = (VkDescriptorImageInfo *) scratch.data();
      count      = size32(update.images);
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView * view = (ImageView *) update.images[i].image_view;
        pImageInfo[i]    = VkDescriptorImageInfo{
             .sampler     = nullptr,
             .imageView   = (view == nullptr) ? nullptr : view->vk_view,
             .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      }
    }
    break;

    case gpu::DescriptorType::StorageTexelBuffer:
    case gpu::DescriptorType::UniformTexelBuffer:
    {
      pTexelBufferView = (VkBufferView *) scratch.data();
      count            = size32(update.texel_buffers);
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView * view   = (BufferView *) update.texel_buffers[i];
        pTexelBufferView[i] = (view == nullptr) ? nullptr : view->vk_view;
      }
    }
    break;

    default:
      CHECK_UNREACHABLE();
  }

  scratch.clear();

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

  vk_table.UpdateDescriptorSets(vk_dev, 1, &vk_write, 0, nullptr);

  switch (binding.type)
  {
    case gpu::DescriptorType::DynamicStorageBuffer:
    case gpu::DescriptorType::DynamicUniformBuffer:
    case gpu::DescriptorType::StorageBuffer:
    case gpu::DescriptorType::UniformBuffer:
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        Buffer * const buffer = (Buffer *) update.buffers[i].buffer;
        binding.update(buffer, Binder{.set     = set,
                                      .binding = update.binding,
                                      .element = update.element + i});
      }
      break;

    case gpu::DescriptorType::StorageTexelBuffer:
    case gpu::DescriptorType::UniformTexelBuffer:
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView const * view = (BufferView *) update.texel_buffers[i];
        binding.update(view == nullptr ? nullptr : BUFFER_FROM_VIEW(view),
                       Binder{.set     = set,
                              .binding = update.binding,
                              .element = update.element + i});
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
        ImageView const * view = (ImageView *) update.images[i].image_view;
        auto const aspect = (view->info.aspects == gpu::ImageAspects::Stencil) ?
                              STENCIL_ASPECT_IDX :
                              MAIN_ASPECT_IDX;
        binding.update(view == nullptr ? nullptr : IMAGE_FROM_VIEW(view),
                       aspect,
                       Binder{.set     = set,
                              .binding = update.binding,
                              .element = update.element + i});
      }
      break;

    default:
      CHECK_UNREACHABLE();
  }
}

Result<Void, Status> Device::wait_idle()
{
  VkResult result = vk_table.DeviceWaitIdle(vk_dev);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<Void, Status> Device::wait_queue_idle()
{
  VkResult result = vk_table.QueueWaitIdle(vk_queue);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<Void, Status>
  Device::get_surface_formats(gpu::Surface              surface_,
                              Vec<gpu::SurfaceFormat> & formats)
{
  VkSurfaceKHR const surface = (VkSurfaceKHR) surface_;

  u32      num_supported;
  VkResult result = instance->vk_table.GetPhysicalDeviceSurfaceFormatsKHR(
    phy_dev.vk_phy_dev, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkSurfaceFormatKHR * vk_formats;
  if (!allocator->nalloc(num_supported, vk_formats))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_formats_{[&] { allocator->ndealloc(num_supported, vk_formats); }};

  {
    u32 num_read = num_supported;
    result       = instance->vk_table.GetPhysicalDeviceSurfaceFormatsKHR(
      phy_dev.vk_phy_dev, surface, &num_supported, vk_formats);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      return Err{(Status) result};
    }

    CHECK(num_read == num_supported && result != VK_INCOMPLETE, "");
  }

  usize const offset = formats.size();

  if (!formats.extend_uninit(num_supported))
  {
    return Err{Status::OutOfHostMemory};
  }

  for (u32 i = 0; i < num_supported; i++)
  {
    formats[offset + i].format = (gpu::Format) vk_formats[i].format;
    formats[offset + i].color_space =
      (gpu::ColorSpace) vk_formats[i].colorSpace;
  }

  return Ok{};
}

Result<Void, Status>
  Device::get_surface_present_modes(gpu::Surface            surface_,
                                    Vec<gpu::PresentMode> & modes)
{
  VkSurfaceKHR const surface = (VkSurfaceKHR) surface_;

  u32      num_supported;
  VkResult result = instance->vk_table.GetPhysicalDeviceSurfacePresentModesKHR(
    phy_dev.vk_phy_dev, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkPresentModeKHR * vk_present_modes;
  if (!allocator->nalloc(num_supported, vk_present_modes))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_present_modes_{
    [&] { allocator->ndealloc(num_supported, vk_present_modes); }};

  {
    u32 num_read = num_supported;
    result       = instance->vk_table.GetPhysicalDeviceSurfacePresentModesKHR(
      phy_dev.vk_phy_dev, surface, &num_supported, vk_present_modes);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      return Err{(Status) result};
    }

    CHECK(num_read == num_supported && result != VK_INCOMPLETE, "");
  }

  usize const offset = modes.size();

  if (!modes.extend_uninit(num_supported))
  {
    return Err{Status::OutOfHostMemory};
  }

  for (u32 i = 0; i < num_supported; i++)
  {
    modes[offset + i] = (gpu::PresentMode) vk_present_modes[i];
  }

  return Ok{};
}

Result<gpu::SurfaceCapabilities, Status>
  Device::get_surface_capabilities(gpu::Surface surface_)
{
  VkSurfaceKHR const       surface = (VkSurfaceKHR) surface_;
  VkSurfaceCapabilitiesKHR capabilities;
  VkResult result = instance->vk_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(
    phy_dev.vk_phy_dev, surface, &capabilities);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{
    gpu::SurfaceCapabilities{
                             .image_usage = (gpu::ImageUsage) capabilities.supportedUsageFlags,
                             .composite_alpha =
        (gpu::CompositeAlpha) capabilities.supportedCompositeAlpha}
  };
}

Result<gpu::SwapchainState, Status>
  Device::get_swapchain_state(gpu::Swapchain swapchain_)
{
  Swapchain * const swapchain = (Swapchain *) swapchain_;

  gpu::SwapchainState state{.extent = swapchain->extent,
                            .format = swapchain->info.format,
                            .images = swapchain->images};

  if (swapchain->is_zero_sized)
  {
    state.current_image = none;
  }
  else
  {
    state.current_image = swapchain->current_image;
  }
  return Ok{state};
}

Result<Void, Status>
  Device::invalidate_swapchain(gpu::Swapchain             swapchain_,
                               gpu::SwapchainInfo const & info)
{
  CHECK(info.preferred_extent.x > 0, "");
  CHECK(info.preferred_extent.y > 0, "");
  Swapchain * const swapchain = (Swapchain *) swapchain_;
  swapchain->is_optimal       = false;
  swapchain->info             = info;
  return Ok{Void{}};
}

Result<Void, Status> Device::begin_frame(gpu::Swapchain swapchain_)
{
  FrameContext &    ctx          = frame_ctx;
  Swapchain * const swapchain    = (Swapchain *) swapchain_;
  VkFence           submit_fence = ctx.submit_fences[ctx.ring_index];
  CommandEncoder &  enc          = ctx.encoders[ctx.ring_index];

  CHECK(!enc.is_recording(), "");

  VkResult result =
    vk_table.WaitForFences(vk_dev, 1, &submit_fence, VK_TRUE, U64_MAX);

  CHECK(result == VK_SUCCESS, "");

  result = vk_table.ResetFences(vk_dev, 1, &submit_fence);

  CHECK(result == VK_SUCCESS, "");

  if (swapchain != nullptr)
  {
    if (swapchain->is_out_of_date || !swapchain->is_optimal ||
        swapchain->vk_swapchain == nullptr)
    {
      // await all pending submitted operations on the device possibly using
      // the swapchain, to avoid destroying whilst in use
      result = vk_table.DeviceWaitIdle(vk_dev);
      CHECK(result == VK_SUCCESS, "");

      result = recreate_swapchain(swapchain);
      CHECK(result == VK_SUCCESS, "");
    }

    if (!swapchain->is_zero_sized)
    {
      u32 next_image;
      result = vk_table.AcquireNextImageKHR(
        vk_dev, swapchain->vk_swapchain, U64_MAX,
        ctx.acquire_semaphores[ctx.ring_index], nullptr, &next_image);

      if (result == VK_SUBOPTIMAL_KHR)
      {
        swapchain->is_optimal = false;
      }
      else
      {
        CHECK(result == VK_SUCCESS, "");
      }

      swapchain->current_image = next_image;
    }
  }

  vk_table.ResetCommandBuffer(enc.vk_command_buffer, 0);

  enc.clear_context();

  VkCommandBufferBeginInfo info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext            = nullptr,
    .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = nullptr};
  result = vk_table.BeginCommandBuffer(enc.vk_command_buffer, &info);
  CHECK(result == VK_SUCCESS, "");

  ctx.swapchain = swapchain;

  return Ok{Void{}};
}

Result<Void, Status> Device::submit_frame(gpu::Swapchain swapchain_)
{
  FrameContext &        ctx            = frame_ctx;
  Swapchain * const     swapchain      = (Swapchain *) swapchain_;
  VkFence const         submit_fence   = ctx.submit_fences[ctx.ring_index];
  CommandEncoder &      enc            = ctx.encoders[ctx.ring_index];
  VkCommandBuffer const command_buffer = enc.vk_command_buffer;
  VkSemaphore const submit_semaphore   = ctx.submit_semaphores[ctx.ring_index];
  VkSemaphore const acquire_semaphore  = ctx.acquire_semaphores[ctx.ring_index];
  bool const was_acquired = swapchain != nullptr && !swapchain->is_zero_sized;
  bool const can_present = swapchain != nullptr && !swapchain->is_out_of_date &&
                           !swapchain->is_zero_sized;

  CHECK(swapchain == ctx.swapchain, "");

  CHECK(enc.is_recording(), "");

  if (was_acquired)
  {
    enc.access_image_all_aspects(
      swapchain->image_impls[swapchain->current_image],
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_NONE,
      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }

  VkResult result = vk_table.EndCommandBuffer(command_buffer);
  CHECK(result == VK_SUCCESS, "");
  CHECK(enc.status == gpu::Status::Success, "");

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

  result = vk_table.QueueSubmit(vk_queue, 1, &submit_info, submit_fence);

  enc.state = CommandEncoderState::End;

  CHECK(result == VK_SUCCESS, "");

  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands could have been executed.
  ctx.current_frame++;
  ctx.tail_frame =
    max(ctx.current_frame, (gpu::FrameId) ctx.buffering()) - ctx.buffering();
  ctx.ring_index = (ctx.ring_index + 1) % ctx.buffering();

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
    result = vk_table.QueuePresentKHR(vk_queue, &present_info);

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
      CHECK(result == VK_SUCCESS, "");
    }
  }

  return Ok{Void{}};
}

Result<Void, Status>
  Device::get_timestamp_query_result(gpu::TimeStampQuery query_, Slice32 range,
                                     Vec<u64> & timestamps)
{
  if (range.span == 0)
  {
    return Ok{};
  }

  VkQueryPool const vk_pool = (VkQueryPool) query_;

  usize const offset = timestamps.size();
  timestamps.extend_uninit(range.span).unwrap();

  VkResult result = vk_table.GetQueryPoolResults(
    vk_dev, vk_pool, range.offset, range.span, sizeof(u64) * range.span,
    timestamps.data() + offset, sizeof(u64), VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    timestamps.resize(offset).unwrap();
    return Err{(Status) result};
  }

  return Ok{};
}

Result<Void, Status>
  Device::get_statistics_query_result(gpu::StatisticsQuery           query_,
                                      Slice32                        range,
                                      Vec<gpu::PipelineStatistics> & statistics)
{
  if (phy_dev.vk_features.pipelineStatisticsQuery != VK_TRUE)
  {
    return Err{Status::FeatureNotPresent};
  }

  if (range.span == 0)
  {
    return Ok{};
  }

  VkQueryPool const vk_pool = (VkQueryPool) query_;

  usize const offset = statistics.size();
  statistics.extend_uninit(range.span).unwrap();

  VkResult result = vk_table.GetQueryPoolResults(
    vk_dev, vk_pool, range.offset, range.span,
    sizeof(gpu::PipelineStatistics) * range.span, statistics.data() + offset,
    sizeof(gpu::PipelineStatistics), VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    statistics.resize(offset).unwrap();
    return Err{(Status) result};
  }

  return Ok{};
}

#define ENCODE_PRELUDE()         \
  CHECK(is_recording(), "");     \
  if (status != Status::Success) \
  {                              \
    return;                      \
  }                              \
  defer pool_reclaim_            \
  {                              \
    [&] { arg_pool.reclaim(); }  \
  }

void CommandEncoder::reset_timestamp_query(gpu::TimeStampQuery query_,
                                           Slice32             range)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  CHECK(!is_in_pass(), "");

  dev->vk_table.CmdResetQueryPool(vk_command_buffer, vk_pool, range.offset,
                                  range.span);
}

void CommandEncoder::reset_statistics_query(gpu::StatisticsQuery query_,
                                            Slice32              range)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  CHECK(!is_in_pass(), "");

  dev->vk_table.CmdResetQueryPool(vk_command_buffer, vk_pool, range.offset,
                                  range.span);
}

void CommandEncoder::write_timestamp(gpu::TimeStampQuery query_,
                                     gpu::PipelineStages stage, u32 index)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  dev->vk_table.CmdWriteTimestamp(
    vk_command_buffer, (VkPipelineStageFlagBits) stage, vk_pool, index);
}

void CommandEncoder::begin_statistics(gpu::StatisticsQuery query_, u32 index)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  dev->vk_table.CmdBeginQuery(vk_command_buffer, vk_pool, index, 0);
}

void CommandEncoder::end_statistics(gpu::StatisticsQuery query_, u32 index)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  dev->vk_table.CmdEndQuery(vk_command_buffer, vk_pool, index);
}

void CommandEncoder::begin_debug_marker(Str region_name, Vec4 color)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  char region_name_cstr[256];
  CHECK(to_c_str(region_name, region_name_cstr), "");

  VkDebugMarkerMarkerInfoEXT info{
    .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
    .pNext       = nullptr,
    .pMarkerName = region_name_cstr,
    .color       = {color.x, color.y, color.z, color.w}
  };
  dev->vk_table.CmdDebugMarkerBeginEXT(vk_command_buffer, &info);
}

void CommandEncoder::end_debug_marker()
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  dev->vk_table.CmdDebugMarkerEndEXT(vk_command_buffer);
}

void CommandEncoder::fill_buffer(gpu::Buffer dst_, u64 offset, u64 size,
                                 u32 data)
{
  ENCODE_PRELUDE();
  Buffer * const dst = (Buffer *) dst_;

  CHECK(!is_in_pass(), "");
  CHECK(has_bits(dst->info.usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->info.size, offset, size, 4), "");
  CHECK(is_aligned<u64>(4, size), "");

  access_buffer(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);
  dev->vk_table.CmdFillBuffer(vk_command_buffer, dst->vk_buffer, offset, size,
                              data);
}

void CommandEncoder::copy_buffer(gpu::Buffer src_, gpu::Buffer dst_,
                                 Span<gpu::BufferCopy const> copies)
{
  ENCODE_PRELUDE();
  Buffer * const src        = (Buffer *) src_;
  Buffer * const dst        = (Buffer *) dst_;
  u32 const      num_copies = size32(copies);

  CHECK(!is_in_pass(), "");
  CHECK(has_bits(src->info.usage, gpu::BufferUsage::TransferSrc), "");
  CHECK(has_bits(dst->info.usage, gpu::BufferUsage::TransferDst), "");
  CHECK(num_copies > 0, "");
  for (gpu::BufferCopy const & copy : copies)
  {
    CHECK(is_valid_buffer_access(src->info.size, copy.src_offset, copy.size),
          "");
    CHECK(is_valid_buffer_access(dst->info.size, copy.dst_offset, copy.size),
          "");
  }

  VkBufferCopy * vk_copies;

  if (!arg_pool.nalloc(num_copies, vk_copies))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferCopy const & copy = copies[i];
    vk_copies[i]                 = VkBufferCopy{.srcOffset = copy.src_offset,
                                                .dstOffset = copy.dst_offset,
                                                .size      = copy.size};
  }

  access_buffer(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT);
  access_buffer(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  dev->vk_table.CmdCopyBuffer(vk_command_buffer, src->vk_buffer, dst->vk_buffer,
                              num_copies, vk_copies);
}

void CommandEncoder::update_buffer(Span<u8 const> src, u64 dst_offset,
                                   gpu::Buffer dst_)
{
  ENCODE_PRELUDE();
  Buffer * const dst       = (Buffer *) dst_;
  u64 const      copy_size = src.size_bytes();

  CHECK(!is_in_pass(), "");
  CHECK(has_bits(dst->info.usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->info.size, dst_offset, copy_size, 4), "");
  CHECK(is_aligned<u64>(4, copy_size), "");
  CHECK(copy_size <= gpu::MAX_UPDATE_BUFFER_SIZE, "");

  access_buffer(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  dev->vk_table.CmdUpdateBuffer(vk_command_buffer, dst->vk_buffer, dst_offset,
                                (u64) src.size(), src.data());
}

void CommandEncoder::clear_color_image(
  gpu::Image dst_, gpu::Color value,
  Span<gpu::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  Image * const dst        = (Image *) dst_;
  u32 const     num_ranges = size32(ranges);

  static_assert(sizeof(gpu::Color) == sizeof(VkClearColorValue));
  CHECK(!is_in_pass(), "");
  CHECK(has_bits(dst->info.usage, gpu::ImageUsage::TransferDst), "");
  CHECK(num_ranges > 0, "");
  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    CHECK(is_valid_image_access(
            dst->info.aspects, dst->info.mip_levels, dst->info.array_layers,
            range.aspects, range.first_mip_level, range.num_mip_levels,
            range.first_array_layer, range.num_array_layers),
          "");
  }

  VkImageSubresourceRange * vk_ranges;
  if (!arg_pool.nalloc(num_ranges, vk_ranges))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    vk_ranges[i] =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.first_mip_level,
                              .levelCount     = range.num_mip_levels,
                              .baseArrayLayer = range.first_array_layer,
                              .layerCount     = range.num_array_layers};
  }

  access_image_all_aspects(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkClearColorValue vk_color;
  std::memcpy(&vk_color, &value, sizeof(VkClearColorValue));

  dev->vk_table.CmdClearColorImage(vk_command_buffer, dst->vk_image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   &vk_color, num_ranges, vk_ranges);
}

void CommandEncoder::clear_depth_stencil_image(
  gpu::Image dst_, gpu::DepthStencil value,
  Span<gpu::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  Image * const dst        = (Image *) dst_;
  u32 const     num_ranges = size32(ranges);

  static_assert(sizeof(gpu::DepthStencil) == sizeof(VkClearDepthStencilValue));
  CHECK(!is_in_pass(), "");
  CHECK(num_ranges > 0, "");
  CHECK(has_bits(dst->info.usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    CHECK(is_valid_image_access(
            dst->info.aspects, dst->info.mip_levels, dst->info.array_layers,
            range.aspects, range.first_mip_level, range.num_mip_levels,
            range.first_array_layer, range.num_array_layers),
          "");
  }

  VkImageSubresourceRange * vk_ranges;
  if (!arg_pool.nalloc(num_ranges, vk_ranges))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    vk_ranges[i] =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.first_mip_level,
                              .levelCount     = range.num_mip_levels,
                              .baseArrayLayer = range.first_array_layer,
                              .layerCount     = range.num_array_layers};
  }

  access_image_all_aspects(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkClearDepthStencilValue vk_depth_stencil;
  std::memcpy(&vk_depth_stencil, &value, sizeof(gpu::DepthStencil));

  dev->vk_table.CmdClearDepthStencilImage(
    vk_command_buffer, dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    &vk_depth_stencil, num_ranges, vk_ranges);
}

void CommandEncoder::copy_image(gpu::Image src_, gpu::Image dst_,
                                Span<gpu::ImageCopy const> copies)
{
  ENCODE_PRELUDE();
  Image * const src        = (Image *) src_;
  Image * const dst        = (Image *) dst_;
  u32 const     num_copies = size32(copies);

  CHECK(!is_in_pass(), "");
  CHECK(num_copies > 0, "");
  CHECK(has_bits(src->info.usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->info.usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::ImageCopy const & copy = copies[i];

    CHECK(is_valid_image_access(src->info.aspects, src->info.mip_levels,
                                src->info.array_layers, copy.src_layers.aspects,
                                copy.src_layers.mip_level, 1,
                                copy.src_layers.first_array_layer,
                                copy.src_layers.num_array_layers),
          "");
    CHECK(is_valid_image_access(dst->info.aspects, dst->info.mip_levels,
                                dst->info.array_layers, copy.dst_layers.aspects,
                                copy.dst_layers.mip_level, 1,
                                copy.dst_layers.first_array_layer,
                                copy.dst_layers.num_array_layers),
          "");

    Vec3U src_extent = mip_size(src->info.extent, copy.src_layers.mip_level);
    Vec3U dst_extent = mip_size(dst->info.extent, copy.dst_layers.mip_level);
    CHECK(copy.src_area.extent.x > 0, "");
    CHECK(copy.src_area.extent.y > 0, "");
    CHECK(copy.src_area.extent.z > 0, "");
    CHECK(copy.src_area.offset.x <= src_extent.x, "");
    CHECK(copy.src_area.offset.y <= src_extent.y, "");
    CHECK(copy.src_area.offset.z <= src_extent.z, "");
    CHECK(copy.src_area.end().x <= src_extent.x, "");
    CHECK(copy.src_area.end().y <= src_extent.y, "");
    CHECK(copy.src_area.end().z <= src_extent.z, "");
    CHECK(copy.dst_offset.x <= dst_extent.x, "");
    CHECK(copy.dst_offset.y <= dst_extent.y, "");
    CHECK(copy.dst_offset.z <= dst_extent.z, "");
    CHECK((copy.dst_offset.x + copy.src_area.extent.x) <= dst_extent.x, "");
    CHECK((copy.dst_offset.y + copy.src_area.extent.y) <= dst_extent.y, "");
    CHECK((copy.dst_offset.z + copy.src_area.extent.z) <= dst_extent.z, "");
  }

  VkImageCopy * vk_copies;
  if (!arg_pool.nalloc(num_copies, vk_copies))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::ImageCopy const &   copy = copies[i];
    VkImageSubresourceLayers src_subresource{
      .aspectMask     = (VkImageAspectFlags) copy.src_layers.aspects,
      .mipLevel       = copy.src_layers.mip_level,
      .baseArrayLayer = copy.src_layers.first_array_layer,
      .layerCount     = copy.src_layers.num_array_layers};
    VkOffset3D               src_offset{(i32) copy.src_area.offset.x,
                          (i32) copy.src_area.offset.y,
                          (i32) copy.src_area.offset.z};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) copy.dst_layers.aspects,
      .mipLevel       = copy.dst_layers.mip_level,
      .baseArrayLayer = copy.dst_layers.first_array_layer,
      .layerCount     = copy.dst_layers.num_array_layers};
    VkOffset3D dst_offset{(i32) copy.dst_offset.x, (i32) copy.dst_offset.y,
                          (i32) copy.dst_offset.z};
    VkExtent3D extent{copy.src_area.extent.x, copy.src_area.extent.y,
                      copy.src_area.extent.z};

    vk_copies[i] = VkImageCopy{.srcSubresource = src_subresource,
                               .srcOffset      = src_offset,
                               .dstSubresource = dst_subresource,
                               .dstOffset      = dst_offset,
                               .extent         = extent};
  }

  access_image_all_aspects(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image_all_aspects(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  dev->vk_table.CmdCopyImage(
    vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoder::copy_buffer_to_image(
  gpu::Buffer src_, gpu::Image dst_, Span<gpu::BufferImageCopy const> copies)
{
  ENCODE_PRELUDE();
  Buffer * const src        = (Buffer *) src_;
  Image * const  dst        = (Image *) dst_;
  u32 const      num_copies = size32(copies);

  CHECK(!is_in_pass(), "");
  CHECK(num_copies > 0, "");
  CHECK(has_bits(src->info.usage, gpu::BufferUsage::TransferSrc), "");
  CHECK(has_bits(dst->info.usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferImageCopy const & copy = copies[i];
    CHECK(is_valid_buffer_access(src->info.size, copy.buffer_offset,
                                 gpu::WHOLE_SIZE),
          "");

    CHECK(is_valid_image_access(
            dst->info.aspects, dst->info.mip_levels, dst->info.array_layers,
            copy.image_layers.aspects, copy.image_layers.mip_level, 1,
            copy.image_layers.first_array_layer,
            copy.image_layers.num_array_layers),
          "");

    CHECK(copy.image_area.extent.x > 0, "");
    CHECK(copy.image_area.extent.y > 0, "");
    CHECK(copy.image_area.extent.z > 0, "");
    Vec3U dst_extent = mip_size(dst->info.extent, copy.image_layers.mip_level);
    CHECK(copy.image_area.extent.x <= dst_extent.x, "");
    CHECK(copy.image_area.extent.y <= dst_extent.y, "");
    CHECK(copy.image_area.extent.z <= dst_extent.z, "");
    CHECK(copy.image_area.end().x <= dst_extent.x, "");
    CHECK(copy.image_area.end().y <= dst_extent.y, "");
    CHECK(copy.image_area.end().z <= dst_extent.z, "");
  }

  VkBufferImageCopy * vk_copies;
  if (!arg_pool.nalloc(num_copies, vk_copies))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferImageCopy const & copy = copies[i];
    VkImageSubresourceLayers     image_subresource{
          .aspectMask     = (VkImageAspectFlags) copy.image_layers.aspects,
          .mipLevel       = copy.image_layers.mip_level,
          .baseArrayLayer = copy.image_layers.first_array_layer,
          .layerCount     = copy.image_layers.num_array_layers};
    vk_copies[i] = VkBufferImageCopy{
      .bufferOffset      = copy.buffer_offset,
      .bufferRowLength   = copy.buffer_row_length,
      .bufferImageHeight = copy.buffer_image_height,
      .imageSubresource  = image_subresource,
      .imageOffset       = VkOffset3D{(i32) copy.image_area.offset.x,
                                      (i32) copy.image_area.offset.y,
                                      (i32) copy.image_area.offset.z},
      .imageExtent =
        VkExtent3D{copy.image_area.extent.x,       copy.image_area.extent.y,
                                      copy.image_area.extent.z      }
    };
  }

  access_buffer(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT);
  access_image_all_aspects(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  dev->vk_table.CmdCopyBufferToImage(
    vk_command_buffer, src->vk_buffer, dst->vk_image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoder::blit_image(gpu::Image src_, gpu::Image dst_,
                                Span<gpu::ImageBlit const> blits,
                                gpu::Filter                filter)
{
  ENCODE_PRELUDE();
  Image * const src       = (Image *) src_;
  Image * const dst       = (Image *) dst_;
  u32 const     num_blits = size32(blits);

  CHECK(!is_in_pass(), "");
  CHECK(num_blits > 0, "");
  CHECK(has_bits(src->info.usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->info.usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_blits; i++)
  {
    gpu::ImageBlit const & blit = blits[i];

    CHECK(is_valid_image_access(src->info.aspects, src->info.mip_levels,
                                src->info.array_layers, blit.src_layers.aspects,
                                blit.src_layers.mip_level, 1,
                                blit.src_layers.first_array_layer,
                                blit.src_layers.num_array_layers),
          "");

    CHECK(is_valid_image_access(dst->info.aspects, dst->info.mip_levels,
                                dst->info.array_layers, blit.dst_layers.aspects,
                                blit.dst_layers.mip_level, 1,
                                blit.dst_layers.first_array_layer,
                                blit.dst_layers.num_array_layers),
          "");

    Vec3U src_extent = mip_size(src->info.extent, blit.src_layers.mip_level);
    Vec3U dst_extent = mip_size(dst->info.extent, blit.dst_layers.mip_level);
    CHECK(blit.src_area.offset.x <= src_extent.x, "");
    CHECK(blit.src_area.offset.y <= src_extent.y, "");
    CHECK(blit.src_area.offset.z <= src_extent.z, "");
    CHECK(blit.src_area.end().x <= src_extent.x, "");
    CHECK(blit.src_area.end().y <= src_extent.y, "");
    CHECK(blit.src_area.end().z <= src_extent.z, "");
    CHECK(blit.dst_area.offset.x <= dst_extent.x, "");
    CHECK(blit.dst_area.offset.y <= dst_extent.y, "");
    CHECK(blit.dst_area.offset.z <= dst_extent.z, "");
    CHECK(blit.dst_area.end().x <= dst_extent.x, "");
    CHECK(blit.dst_area.end().y <= dst_extent.y, "");
    CHECK(blit.dst_area.end().z <= dst_extent.z, "");
  }

  VkImageBlit * vk_blits;
  if (!arg_pool.nalloc(num_blits, vk_blits))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_blits; i++)
  {
    gpu::ImageBlit const &   blit = blits[i];
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
      .srcOffsets     = {VkOffset3D{(i32) blit.src_area.offset.x,
                                (i32) blit.src_area.offset.y,
                                (i32) blit.src_area.offset.z},
                         VkOffset3D{(i32) blit.src_area.end().x,
                                (i32) blit.src_area.end().y,
                                (i32) blit.src_area.end().z}},
      .dstSubresource = dst_subresource,
      .dstOffsets     = {
                         VkOffset3D{(i32) blit.dst_area.offset.x, (i32) blit.dst_area.offset.y,
                   (i32) blit.dst_area.offset.z},
                         VkOffset3D{(i32) blit.dst_area.end().x, (i32) blit.dst_area.end().y,
                   (i32) blit.dst_area.end().z}             }
    };
  }

  access_image_all_aspects(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image_all_aspects(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  dev->vk_table.CmdBlitImage(
    vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_blits, vk_blits,
    (VkFilter) filter);
}

void CommandEncoder::resolve_image(gpu::Image src_, gpu::Image dst_,
                                   Span<gpu::ImageResolve const> resolves)
{
  ENCODE_PRELUDE();
  Image * const src          = (Image *) src_;
  Image * const dst          = (Image *) dst_;
  u32 const     num_resolves = size32(resolves);

  CHECK(!is_in_pass(), "");
  CHECK(num_resolves > 0, "");
  CHECK(has_bits(src->info.usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->info.usage, gpu::ImageUsage::TransferDst), "");
  CHECK(has_bits(dst->info.sample_count, gpu::SampleCount::C1), "");

  for (u32 i = 0; i < num_resolves; i++)
  {
    gpu::ImageResolve const & resolve = resolves[i];

    CHECK(is_valid_image_access(
            src->info.aspects, src->info.mip_levels, src->info.array_layers,
            resolve.src_layers.aspects, resolve.src_layers.mip_level, 1,
            resolve.src_layers.first_array_layer,
            resolve.src_layers.num_array_layers),
          "");
    CHECK(is_valid_image_access(
            dst->info.aspects, dst->info.mip_levels, dst->info.array_layers,
            resolve.dst_layers.aspects, resolve.dst_layers.mip_level, 1,
            resolve.dst_layers.first_array_layer,
            resolve.dst_layers.num_array_layers),
          "");

    Vec3U src_extent = mip_size(src->info.extent, resolve.src_layers.mip_level);
    Vec3U dst_extent = mip_size(dst->info.extent, resolve.dst_layers.mip_level);
    CHECK(resolve.src_area.extent.x > 0, "");
    CHECK(resolve.src_area.extent.y > 0, "");
    CHECK(resolve.src_area.extent.z > 0, "");
    CHECK(resolve.src_area.offset.x <= src_extent.x, "");
    CHECK(resolve.src_area.offset.y <= src_extent.y, "");
    CHECK(resolve.src_area.offset.z <= src_extent.z, "");
    CHECK(resolve.src_area.end().x <= src_extent.x, "");
    CHECK(resolve.src_area.end().y <= src_extent.y, "");
    CHECK(resolve.src_area.end().z <= src_extent.z, "");
    CHECK(resolve.dst_offset.x <= dst_extent.x, "");
    CHECK(resolve.dst_offset.y <= dst_extent.y, "");
    CHECK(resolve.dst_offset.z <= dst_extent.z, "");
    CHECK((resolve.dst_offset.x + resolve.src_area.extent.x) <= dst_extent.x,
          "");
    CHECK((resolve.dst_offset.y + resolve.src_area.extent.y) <= dst_extent.y,
          "");
    CHECK((resolve.dst_offset.z + resolve.src_area.extent.z) <= dst_extent.z,
          "");
  }

  VkImageResolve * vk_resolves;
  if (!arg_pool.nalloc<VkImageResolve>(num_resolves, vk_resolves))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_resolves; i++)
  {
    gpu::ImageResolve const & resolve = resolves[i];
    VkImageSubresourceLayers  src_subresource{
       .aspectMask     = (VkImageAspectFlags) resolve.src_layers.aspects,
       .mipLevel       = resolve.src_layers.mip_level,
       .baseArrayLayer = resolve.src_layers.first_array_layer,
       .layerCount     = resolve.src_layers.num_array_layers};
    VkOffset3D               src_offset{(i32) resolve.src_area.offset.x,
                          (i32) resolve.src_area.offset.y,
                          (i32) resolve.src_area.offset.z};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) resolve.dst_layers.aspects,
      .mipLevel       = resolve.dst_layers.mip_level,
      .baseArrayLayer = resolve.dst_layers.first_array_layer,
      .layerCount     = resolve.dst_layers.num_array_layers};
    VkOffset3D dst_offset{(i32) resolve.dst_offset.x,
                          (i32) resolve.dst_offset.y,
                          (i32) resolve.dst_offset.z};
    VkExtent3D extent{resolve.src_area.extent.x, resolve.src_area.extent.y,
                      resolve.src_area.extent.z};

    vk_resolves[i] = VkImageResolve{.srcSubresource = src_subresource,
                                    .srcOffset      = src_offset,
                                    .dstSubresource = dst_subresource,
                                    .dstOffset      = dst_offset,
                                    .extent         = extent};
  }

  access_image_all_aspects(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_READ_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image_all_aspects(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_ACCESS_TRANSFER_WRITE_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  dev->vk_table.CmdResolveImage(
    vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_resolves,
    vk_resolves);
}

void CommandEncoder::begin_compute_pass()
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");

  state = CommandEncoderState::ComputePass;
}

void CommandEncoder::end_compute_pass()
{
  ENCODE_PRELUDE();
  CHECK(is_in_compute_pass(), "");

  clear_context();
}

void validate_attachment(gpu::RenderingAttachment const & info,
                         gpu::ImageAspects aspects, gpu::ImageUsage usage)
{
  CHECK(!(info.resolve_mode != gpu::ResolveModes::None && info.view == nullptr),
        "");
  CHECK(
    !(info.resolve_mode != gpu::ResolveModes::None && info.resolve == nullptr),
    "");
  if (info.view != nullptr)
  {
    CHECK(has_bits(IMAGE_FROM_VIEW(info.view)->info.aspects, aspects), "");
    CHECK(has_bits(IMAGE_FROM_VIEW(info.view)->info.usage, usage), "");
    CHECK(
      !(info.resolve_mode != gpu::ResolveModes::None &&
        IMAGE_FROM_VIEW(info.view)->info.sample_count == gpu::SampleCount::C1),
      "");
  }
  if (info.resolve != nullptr)
  {
    CHECK(has_bits(IMAGE_FROM_VIEW(info.resolve)->info.aspects, aspects), "");
    CHECK(has_bits(IMAGE_FROM_VIEW(info.resolve)->info.usage, usage), "");
    CHECK(IMAGE_FROM_VIEW(info.resolve)->info.sample_count ==
            gpu::SampleCount::C1,
          "");
  }
}

void CommandEncoder::begin_rendering(gpu::RenderingInfo const & info)
{
  ENCODE_PRELUDE();

  CHECK(!is_in_pass(), "");
  CHECK(info.color_attachments.size() <= gpu::MAX_PIPELINE_COLOR_ATTACHMENTS,
        "");
  CHECK(info.render_area.extent.x > 0, "");
  CHECK(info.render_area.extent.y > 0, "");
  CHECK(info.num_layers > 0, "");

  for (gpu::RenderingAttachment const & attachment : info.color_attachments)
  {
    validate_attachment(attachment, gpu::ImageAspects::Color,
                        gpu::ImageUsage::ColorAttachment);
  }

  info.depth_attachment.match([](auto & depth) {
    validate_attachment(depth, gpu::ImageAspects::Depth,
                        gpu::ImageUsage::DepthStencilAttachment);
  });

  info.stencil_attachment.match([](auto & stencil) {
    validate_attachment(stencil, gpu::ImageAspects::Stencil,
                        gpu::ImageUsage::DepthStencilAttachment);
  });

  clear_context();
  render_ctx.color_attachments.extend(info.color_attachments).unwrap();
  render_ctx.depth_attachment   = info.depth_attachment;
  render_ctx.stencil_attachment = info.stencil_attachment;
  state                         = CommandEncoderState::RenderPass;
  render_ctx.render_area        = info.render_area;
  render_ctx.num_layers         = info.num_layers;
}

constexpr VkAccessFlags
  color_attachment_access(gpu::RenderingAttachment const & attachment)
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
  depth_attachment_access(gpu::RenderingAttachment const & attachment)
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
  stencil_attachment_access(gpu::RenderingAttachment const & attachment)
{
  return depth_attachment_access(attachment);
}

void CommandEncoder::end_rendering()
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;
  DeviceTable const * t   = &dev->vk_table;

  CHECK(is_in_render_pass(), "");

  // synchronization pass: bindings
  for (Command const & cmd : render_ctx.commands)
  {
    cmd.match(
      [&](CmdBindDescriptorSets const & c) {
        for (auto set : c.sets)
        {
          access_graphics_bindings(*set);
        }
      },
      [&](CmdBindGraphicsPipeline const &) {}, [&](CmdPushConstants const &) {},
      [&](CmdSetGraphicsState const &) {},
      [&](CmdBindVertexBuffer const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
      },
      [&](CmdBindIndexBuffer const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_INDEX_READ_BIT);
      },
      [&](CmdDraw const &) {}, [&](CmdDrawIndexed const &) {},
      [&](CmdDrawIndirect const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                      VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
      },
      [&](CmdDrawIndexedIndirect const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                      VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
      });
  }

  // synchronization pass: attachments
  {
    InplaceVec<VkRenderingAttachmentInfoKHR,
               gpu::MAX_PIPELINE_COLOR_ATTACHMENTS>
      vk_color_attachments{};

    constexpr VkPipelineStageFlags RESOLVE_STAGE =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    constexpr VkAccessFlags RESOLVE_COLOR_SRC_ACCESS =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    constexpr VkAccessFlags RESOLVE_COLOR_DST_ACCESS =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    constexpr VkAccessFlags RESOLVE_DEPTH_STENCIL_SRC_ACCESS =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    constexpr VkAccessFlags RESOLVE_DEPTH_STENCIL_DST_ACCESS =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    constexpr VkImageLayout RESOLVE_COLOR_LAYOUT =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    constexpr VkImageLayout RESOLVE_DEPTH_LAYOUT =
      VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;

    constexpr VkImageLayout RESOLVE_STENCIL_LAYOUT =
      VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;

    for (auto const & attachment : ctx.color_attachments)
    {
      VkAccessFlags        access     = color_attachment_access(attachment);
      VkImageView          vk_view    = nullptr;
      VkImageView          vk_resolve = nullptr;
      VkPipelineStageFlags stages =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkClearValue  clear_value{.color{
         .uint32{attachment.clear.color.u32.x, attachment.clear.color.u32.y,
                attachment.clear.color.u32.z, attachment.clear.color.u32.w}}};

      if (attachment.resolve_mode != gpu::ResolveModes::None)
      {
        access |= RESOLVE_COLOR_SRC_ACCESS;
        stages |= RESOLVE_STAGE;
      }

      if (attachment.view != nullptr)
      {
        ImageView * view = (ImageView *) attachment.view;
        vk_view          = view->vk_view;

        access_image_aspect(*IMAGE_FROM_VIEW(view), stages, access, layout,
                            gpu::ImageAspects::Color, COLOR_ASPECT_IDX);

        if (attachment.resolve_mode != gpu::ResolveModes::None)
        {
          ImageView * resolve = (ImageView *) attachment.resolve;
          vk_resolve          = resolve->vk_view;
          access_image_aspect(*IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                              RESOLVE_COLOR_DST_ACCESS, RESOLVE_COLOR_LAYOUT,
                              gpu::ImageAspects::Color, COLOR_ASPECT_IDX);
        }
      }

      vk_color_attachments
        .push(VkRenderingAttachmentInfoKHR{
          .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
          .pNext              = nullptr,
          .imageView          = vk_view,
          .imageLayout        = layout,
          .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
          .resolveImageView   = vk_resolve,
          .resolveImageLayout = RESOLVE_COLOR_LAYOUT,
          .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
          .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
          .clearValue         = clear_value})
        .unwrap();
    }

    auto vk_depth_attachment = ctx.depth_attachment.map([&](auto & attachment) {
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
        access |= RESOLVE_DEPTH_STENCIL_SRC_ACCESS;
        stages |= RESOLVE_STAGE;
      }

      VkClearValue clear_value{
        .depthStencil{.depth = attachment.clear.depth_stencil.depth}};

      if (attachment.view != nullptr)
      {
        ImageView * view = (ImageView *) attachment.view;
        vk_view          = view->vk_view;

        access_image_aspect(*IMAGE_FROM_VIEW(view), stages, access, layout,
                            gpu::ImageAspects::Depth, DEPTH_ASPECT_IDX);

        if (attachment.resolve_mode != gpu::ResolveModes::None)
        {
          ImageView * resolve = (ImageView *) attachment.resolve;
          vk_resolve          = resolve->vk_view;
          access_image_aspect(*IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                              RESOLVE_DEPTH_STENCIL_DST_ACCESS,
                              RESOLVE_DEPTH_LAYOUT, gpu::ImageAspects::Depth,
                              DEPTH_ASPECT_IDX);
        }
      }

      return VkRenderingAttachmentInfoKHR{
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
    });

    auto vk_stencil_attachment =
      ctx.stencil_attachment.map([&](auto & attachment) {
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
          access |= RESOLVE_DEPTH_STENCIL_SRC_ACCESS;
          stages |= RESOLVE_STAGE;
        }

        VkClearValue clear_value{
          .depthStencil{.stencil = attachment.clear.depth_stencil.stencil}};

        if (attachment.view != nullptr)
        {
          ImageView * view = (ImageView *) attachment.view;
          vk_view          = view->vk_view;

          access_image_aspect(*IMAGE_FROM_VIEW(view), stages, access, layout,
                              gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);

          if (attachment.resolve_mode != gpu::ResolveModes::None)
          {
            ImageView * resolve = (ImageView *) attachment.resolve;
            vk_resolve          = resolve->vk_view;
            access_image_aspect(*IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                                RESOLVE_DEPTH_STENCIL_DST_ACCESS,
                                RESOLVE_STENCIL_LAYOUT,
                                gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);
          }
        }

        return VkRenderingAttachmentInfoKHR{
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
      });

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
      .colorAttachmentCount = size32(vk_color_attachments),
      .pColorAttachments    = vk_color_attachments.data(),
      .pDepthAttachment     = vk_depth_attachment.as_ptr().unwrap_or(nullptr),
      .pStencilAttachment = vk_stencil_attachment.as_ptr().unwrap_or(nullptr)
    };

    t->CmdBeginRenderingKHR(vk_command_buffer, &begin_info);
  }

  GraphicsPipeline * pipeline = nullptr;

  for (Command const & cmd : ctx.commands)
  {
    cmd.match(
      [&](CmdBindDescriptorSets const & c) {
        InplaceVec<VkDescriptorSet, gpu::MAX_PIPELINE_DESCRIPTOR_SETS> vk_sets;

        for (auto & set : c.sets)
        {
          vk_sets.push(set->vk_set).unwrap();
        }

        t->CmdBindDescriptorSets(
          vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          pipeline->vk_layout, 0, size32(vk_sets), vk_sets.data(),
          size32(c.dynamic_offsets), c.dynamic_offsets.data());
      },
      [&](CmdBindGraphicsPipeline const & c) {
        pipeline = c.pipeline;
        t->CmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipeline->vk_pipeline);
      },
      [&](CmdPushConstants const & c) {
        t->CmdPushConstants(
          vk_command_buffer, pipeline->vk_layout, VK_SHADER_STAGE_ALL, 0,
          size32(c.constant.view().as_u8()), c.constant.view().as_u8().data());
      },
      [&](CmdSetGraphicsState const & c) {
        gpu::GraphicsState const & s = c.state;

        VkRect2D vk_scissor{
          .offset =
            VkOffset2D{(i32) s.scissor.offset.x, (i32) s.scissor.offset.y},
          .extent = VkExtent2D{s.scissor.extent.x,       s.scissor.extent.y      }
        };
        t->CmdSetScissor(vk_command_buffer, 0, 1, &vk_scissor);

        VkViewport vk_viewport{.x        = s.viewport.offset.x,
                               .y        = s.viewport.offset.y,
                               .width    = s.viewport.extent.x,
                               .height   = s.viewport.extent.y,
                               .minDepth = s.viewport.min_depth,
                               .maxDepth = s.viewport.max_depth};
        t->CmdSetViewport(vk_command_buffer, 0, 1, &vk_viewport);

        f32 vk_constant[4] = {s.blend_constant.x, s.blend_constant.y,
                              s.blend_constant.z, s.blend_constant.w};
        t->CmdSetBlendConstants(vk_command_buffer, vk_constant);

        t->CmdSetStencilTestEnableEXT(vk_command_buffer, s.stencil_test_enable);

        t->CmdSetStencilReference(vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT,
                                  s.front_face_stencil.reference);
        t->CmdSetStencilCompareMask(vk_command_buffer,
                                    VK_STENCIL_FACE_FRONT_BIT,
                                    s.front_face_stencil.compare_mask);
        t->CmdSetStencilWriteMask(vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT,
                                  s.front_face_stencil.write_mask);
        t->CmdSetStencilOpEXT(vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT,
                              (VkStencilOp) s.front_face_stencil.fail_op,
                              (VkStencilOp) s.front_face_stencil.pass_op,
                              (VkStencilOp) s.front_face_stencil.depth_fail_op,
                              (VkCompareOp) s.front_face_stencil.compare_op);

        t->CmdSetStencilReference(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                                  s.back_face_stencil.reference);
        t->CmdSetStencilCompareMask(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                                    s.back_face_stencil.compare_mask);
        t->CmdSetStencilWriteMask(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                                  s.back_face_stencil.write_mask);
        t->CmdSetStencilOpEXT(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                              (VkStencilOp) s.back_face_stencil.fail_op,
                              (VkStencilOp) s.back_face_stencil.pass_op,
                              (VkStencilOp) s.back_face_stencil.depth_fail_op,
                              (VkCompareOp) s.back_face_stencil.compare_op);
        t->CmdSetCullModeEXT(vk_command_buffer, (VkCullModeFlags) s.cull_mode);
        t->CmdSetFrontFaceEXT(vk_command_buffer, (VkFrontFace) s.front_face);
        t->CmdSetDepthTestEnableEXT(vk_command_buffer, s.depth_test_enable);
        t->CmdSetDepthCompareOpEXT(vk_command_buffer,
                                   (VkCompareOp) s.depth_compare_op);
        t->CmdSetDepthWriteEnableEXT(vk_command_buffer, s.depth_write_enable);
        t->CmdSetDepthBoundsTestEnableEXT(vk_command_buffer,
                                          s.depth_bounds_test_enable);
      },
      [&](CmdBindVertexBuffer const & c) {
        t->CmdBindVertexBuffers(vk_command_buffer, c.binding, 1,
                                &c.buffer->vk_buffer, &c.offset);
      },
      [&](CmdBindIndexBuffer const & c) {
        t->CmdBindIndexBuffer(vk_command_buffer, c.buffer->vk_buffer, c.offset,
                              (VkIndexType) c.index_type);
      },
      [&](CmdDraw const & c) {
        t->CmdDraw(vk_command_buffer, c.vertex_count, c.instance_count,
                   c.first_vertex, c.first_instance);
      },
      [&](CmdDrawIndexed const & c) {
        t->CmdDrawIndexed(vk_command_buffer, c.num_indices, c.num_instances,
                          c.first_index, c.vertex_offset, c.first_instance);
      },
      [&](CmdDrawIndirect const & c) {
        t->CmdDrawIndirect(vk_command_buffer, c.buffer->vk_buffer, c.offset,
                           c.draw_count, c.stride);
      },
      [&](CmdDrawIndexedIndirect const & c) {
        t->CmdDrawIndexedIndirect(vk_command_buffer, c.buffer->vk_buffer,
                                  c.offset, c.draw_count, c.stride);
      });
  }

  t->CmdEndRenderingKHR(vk_command_buffer);
  clear_context();
}

void CommandEncoder::bind_compute_pipeline(gpu::ComputePipeline pipeline)
{
  ENCODE_PRELUDE();
  ComputePassContext & ctx = compute_ctx;

  CHECK(is_in_compute_pass(), "");

  state        = CommandEncoderState::ComputePass;
  ctx.pipeline = (ComputePipeline *) pipeline;

  dev->vk_table.CmdBindPipeline(vk_command_buffer,
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                ctx.pipeline->vk_pipeline);
}

void CommandEncoder::validate_render_pass_compatible(
  gpu::GraphicsPipeline pipeline_)
{
  RenderPassContext const & ctx      = render_ctx;
  GraphicsPipeline *        pipeline = (GraphicsPipeline *) pipeline_;

  CHECK(pipeline->color_fmts.size() == ctx.color_attachments.size(), "");
  CHECK(!(pipeline->depth_fmt.is_none() && ctx.depth_attachment.is_some()), "");
  CHECK(!(pipeline->stencil_fmt.is_none() && ctx.depth_attachment.is_some()),
        "");

  for (auto [pipeline_fmt, attachment] :
       zip(pipeline->color_fmts, ctx.color_attachments))
  {
    if (pipeline_fmt != gpu::Format::Undefined)
    {
      CHECK(attachment.view != nullptr, "");
      CHECK(pipeline_fmt == IMAGE_FROM_VIEW(attachment.view)->info.format, "");
      CHECK(pipeline->sample_count ==
              IMAGE_FROM_VIEW(attachment.view)->info.sample_count,
            "");
    }
  }

  ctx.depth_attachment.match([&](auto & attachment) {
    CHECK(attachment.view != nullptr, "");
    CHECK(pipeline->depth_fmt == IMAGE_FROM_VIEW(attachment.view)->info.format,
          "");
  });

  ctx.stencil_attachment.match([&](auto & attachment) {
    CHECK(attachment.view != nullptr, "");
    CHECK(pipeline->stencil_fmt ==
            IMAGE_FROM_VIEW(attachment.view)->info.format,
          "");
  });
}

void CommandEncoder::access_buffer(Buffer & buffer, VkPipelineStageFlags stages,
                                   VkAccessFlags access)
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
    dev->vk_table.CmdPipelineBarrier(vk_command_buffer, src_stages, dst_stages,
                                     0, 0, nullptr, 1, &barrier, 0, nullptr);
  }
}

void CommandEncoder::access_image_aspect(
  Image & image, VkPipelineStageFlags stages, VkAccessFlags access,
  VkImageLayout layout, gpu::ImageAspects aspects, u32 aspect_index)
{
  VkImageMemoryBarrier barrier;
  VkPipelineStageFlags src_stages;
  VkPipelineStageFlags dst_stages;
  if (sync_image_state(
        image.aspect_states[aspect_index],
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
    dev->vk_table.CmdPipelineBarrier(vk_command_buffer, src_stages, dst_stages,
                                     0, 0, nullptr, 0, nullptr, 1, &barrier);
  }
}

void CommandEncoder::access_image_all_aspects(Image &              image,
                                              VkPipelineStageFlags stages,
                                              VkAccessFlags        access,
                                              VkImageLayout        layout)
{
  if (has_bits(image.info.aspects,
               gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil))
  {
    access_image_aspect(image, stages, access, layout, gpu::ImageAspects::Depth,
                        DEPTH_ASPECT_IDX);
    access_image_aspect(image, stages, access, layout,
                        gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);
  }
  else
  {
    access_image_aspect(image, stages, access, layout, image.info.aspects, 0);
  }
}

void CommandEncoder::access_compute_bindings(DescriptorSet const & set)
{
  for (auto binding : set.bindings)
  {
    switch (binding.type)
    {
      case gpu::DescriptorType::CombinedImageSampler:
      case gpu::DescriptorType::SampledImage:
        for (Image * img : binding.sync_resources[v1])
        {
          if (img != nullptr)
          {
            access_image_all_aspects(*img, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     VK_ACCESS_SHADER_READ_BIT,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

      case gpu::DescriptorType::StorageImage:
        for (Image * img : binding.sync_resources[v1])
        {
          if (img != nullptr)
          {
            access_image_all_aspects(*img, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     VK_ACCESS_SHADER_READ_BIT |
                                       VK_ACCESS_SHADER_WRITE_BIT,
                                     VK_IMAGE_LAYOUT_GENERAL);
          }
        }
        break;

      case gpu::DescriptorType::UniformBuffer:
      case gpu::DescriptorType::DynamicUniformBuffer:
      case gpu::DescriptorType::UniformTexelBuffer:
        for (Buffer * buffer : binding.sync_resources[v2])
        {
          if (buffer != nullptr)
          {
            access_buffer(*buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

      case gpu::DescriptorType::StorageBuffer:
      case gpu::DescriptorType::DynamicStorageBuffer:
      case gpu::DescriptorType::StorageTexelBuffer:
        for (Buffer * buffer : binding.sync_resources[v2])
        {
          if (buffer != nullptr)
          {
            access_buffer(*buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT |
                            VK_ACCESS_SHADER_WRITE_BIT);
          }
        }
        break;

      case gpu::DescriptorType::InputAttachment:
        break;

      default:
        CHECK_UNREACHABLE();
    }
  }
}

void CommandEncoder::access_graphics_bindings(DescriptorSet const & set)
{
  for (auto const & binding : set.bindings)
  {
    switch (binding.type)
    {
      case gpu::DescriptorType::CombinedImageSampler:
      case gpu::DescriptorType::SampledImage:
      case gpu::DescriptorType::InputAttachment:
        for (Image * img : binding.sync_resources[v1])
        {
          if (img != nullptr)
          {
            access_image_all_aspects(*img,
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
        for (Buffer * buffer : binding.sync_resources[v2])
        {
          if (buffer != nullptr)
          {
            access_buffer(*buffer,
                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        // only readonly storage images are supported
      case gpu::DescriptorType::StorageImage:
        for (Image * img : binding.sync_resources[v1])
        {
          if (img != nullptr)
          {
            access_image_all_aspects(*img,
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
        for (Buffer * buffer : binding.sync_resources[v2])
        {
          if (buffer != nullptr)
          {
            access_buffer(*buffer,
                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

      case gpu::DescriptorType::Sampler:
        break;
      default:
        CHECK_UNREACHABLE();
    }
  }
}

void CommandEncoder::bind_graphics_pipeline(gpu::GraphicsPipeline pipeline_)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx      = render_ctx;
  GraphicsPipeline *  pipeline = (GraphicsPipeline *) pipeline_;

  CHECK(is_in_render_pass(), "");
  CHECK(pipeline != nullptr, "");
  validate_render_pass_compatible(pipeline_);
  ctx.pipeline = pipeline;
  if (!ctx.commands.push(CmdBindGraphicsPipeline{.pipeline = pipeline}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::bind_descriptor_sets(
  Span<gpu::DescriptorSet const> descriptor_sets,
  Span<u32 const>                dynamic_offsets)
{
  ENCODE_PRELUDE();

  CHECK(is_in_pass(), "");
  CHECK(size32(descriptor_sets) <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS, "");
  CHECK(size32(dynamic_offsets) <= (gpu::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS +
                                    gpu::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS),
        "");

  for (u32 offset : dynamic_offsets)
  {
    CHECK(is_aligned<u64>(gpu::BUFFER_OFFSET_ALIGNMENT, offset), "");
  }

  if (is_in_compute_pass())
  {
    CHECK(compute_ctx.pipeline != nullptr, "");
    CHECK(compute_ctx.pipeline->num_sets == descriptor_sets.size(), "");

    compute_ctx.sets.clear();
    for (auto set : descriptor_sets)
    {
      compute_ctx.sets.push((DescriptorSet *) set).unwrap();
    }

    InplaceVec<VkDescriptorSet, gpu::MAX_PIPELINE_DESCRIPTOR_SETS> vk_sets;
    for (auto & set : descriptor_sets)
    {
      vk_sets.push(((DescriptorSet *) set)->vk_set).unwrap();
    }

    dev->vk_table.CmdBindDescriptorSets(
      vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      compute_ctx.pipeline->vk_layout, 0, size32(vk_sets), vk_sets.data(),
      size32(dynamic_offsets), dynamic_offsets.data());
  }
  else if (is_in_render_pass())
  {
    CHECK(render_ctx.pipeline != nullptr, "");
    CHECK(render_ctx.pipeline->num_sets == descriptor_sets.size(), "");

    auto sets =
      PinVec<DescriptorSet *>::make(size(descriptor_sets), render_ctx.arg_pool)
        .unwrap();
    auto offsets =
      PinVec<u32>::make(size(dynamic_offsets), render_ctx.arg_pool).unwrap();

    sets.extend(descriptor_sets.reinterpret<DescriptorSet * const>()).unwrap();
    offsets.extend(dynamic_offsets).unwrap();

    if (!render_ctx.commands.push(CmdBindDescriptorSets{
          .sets = std::move(sets), .dynamic_offsets = std::move(offsets)}))
    {
      status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoder::push_constants(Span<u8 const> push_constants_data)
{
  ENCODE_PRELUDE();
  CHECK(push_constants_data.size_bytes() <= gpu::MAX_PUSH_CONSTANTS_SIZE, "");
  u32 const push_constants_size = (u32) push_constants_data.size_bytes();
  CHECK(is_aligned(4U, push_constants_size), "");
  CHECK(is_in_pass(), "");

  if (is_in_compute_pass())
  {
    CHECK(compute_ctx.pipeline != nullptr, "");
    CHECK(push_constants_size == compute_ctx.pipeline->push_constants_size, "");
    dev->vk_table.CmdPushConstants(
      vk_command_buffer, compute_ctx.pipeline->vk_layout, VK_SHADER_STAGE_ALL,
      0, compute_ctx.pipeline->push_constants_size, push_constants_data.data());
  }
  else if (is_in_render_pass())
  {
    // [ ] are the commands destroyed correctly?
    CHECK(render_ctx.pipeline != nullptr, "");
    CHECK(push_constants_size == render_ctx.pipeline->push_constants_size, "");

    auto constant =
      PinVec<u8>::make(push_constants_size, render_ctx.arg_pool).unwrap();
    constant.extend(push_constants_data).unwrap();

    if (!render_ctx.commands.push(
          CmdPushConstants{.constant = std::move(constant)}))
    {
      status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoder::dispatch(u32 group_count_x, u32 group_count_y,
                              u32 group_count_z)
{
  ENCODE_PRELUDE();
  ComputePassContext & ctx = compute_ctx;

  CHECK(is_in_compute_pass(), "");

  CHECK(ctx.pipeline != nullptr, "");
  CHECK(group_count_x <=
          dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[0],
        "");
  CHECK(group_count_y <=
          dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[1],
        "");
  CHECK(group_count_z <=
          dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[2],
        "");

  for (auto set : ctx.sets)
  {
    access_compute_bindings(*set);
  }

  dev->vk_table.CmdDispatch(vk_command_buffer, group_count_x, group_count_y,
                            group_count_z);
}

void CommandEncoder::dispatch_indirect(gpu::Buffer buffer_, u64 offset)
{
  ENCODE_PRELUDE();
  ComputePassContext & ctx    = compute_ctx;
  Buffer * const       buffer = (Buffer *) buffer_;

  CHECK(is_in_compute_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(has_bits(buffer->info.usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(is_valid_buffer_access(buffer->info.size, offset,
                               sizeof(gpu::DispatchCommand), 4),
        "");

  for (auto set : ctx.sets)
  {
    access_compute_bindings(*set);
  }

  dev->vk_table.CmdDispatchIndirect(vk_command_buffer, buffer->vk_buffer,
                                    offset);
}

void CommandEncoder::set_graphics_state(gpu::GraphicsState const & state)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  CHECK(state.viewport.min_depth >= 0.0F, "");
  CHECK(state.viewport.min_depth <= 1.0F, "");
  CHECK(state.viewport.max_depth >= 0.0F, "");
  CHECK(state.viewport.max_depth <= 1.0F, "");
  ctx.has_state = true;

  if (!ctx.commands.push(CmdSetGraphicsState{.state = state}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::bind_vertex_buffers(Span<gpu::Buffer const> vertex_buffers,
                                         Span<u64 const>         offsets)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  u32 const num_vertex_buffers = size32(vertex_buffers);
  CHECK(num_vertex_buffers > 0, "");
  CHECK(num_vertex_buffers <= gpu::MAX_VERTEX_ATTRIBUTES, "");
  CHECK(offsets.size() == vertex_buffers.size(), "");
  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    u64 const      offset = offsets[i];
    Buffer * const buffer = (Buffer *) vertex_buffers[i];
    CHECK(offset < buffer->info.size, "");
    CHECK(has_bits(buffer->info.usage, gpu::BufferUsage::VertexBuffer), "");
  }

  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    if (!ctx.commands.push(
          CmdBindVertexBuffer{.binding = i,
                              .buffer  = (Buffer *) vertex_buffers[i],
                              .offset  = offsets[i]}))
    {
      status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoder::bind_index_buffer(gpu::Buffer index_buffer_, u64 offset,
                                       gpu::IndexType index_type)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx          = render_ctx;
  Buffer * const      index_buffer = (Buffer *) index_buffer_;
  u64 const           index_size   = index_type_size(index_type);

  CHECK(is_in_render_pass(), "");
  CHECK(offset < index_buffer->info.size, "");
  CHECK(is_aligned(index_size, offset), "");
  CHECK(has_bits(index_buffer->info.usage, gpu::BufferUsage::IndexBuffer), "");

  ctx.index_buffer        = index_buffer;
  ctx.index_type          = index_type;
  ctx.index_buffer_offset = offset;
  if (!ctx.commands.push(CmdBindIndexBuffer{
        .buffer = index_buffer, .offset = offset, .index_type = index_type}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw(u32 vertex_count, u32 instance_count,
                          u32 first_vertex, u32 first_instance)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDraw{.vertex_count   = vertex_count,
                                 .instance_count = instance_count,
                                 .first_vertex   = first_vertex,
                                 .first_instance = first_instance}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw_indexed(u32 first_index, u32 num_indices,
                                  i32 vertex_offset, u32 first_instance,
                                  u32 num_instances)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(ctx.index_buffer != nullptr, "");
  u64 const index_size = index_type_size(ctx.index_type);
  CHECK((ctx.index_buffer_offset + first_index * index_size) <
          ctx.index_buffer->info.size,
        "");
  CHECK((ctx.index_buffer_offset + (first_index + num_indices) * index_size) <=
          ctx.index_buffer->info.size,
        "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDrawIndexed{.first_index    = first_index,
                                        .num_indices    = num_indices,
                                        .vertex_offset  = vertex_offset,
                                        .first_instance = first_instance,
                                        .num_instances  = num_instances}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw_indirect(gpu::Buffer buffer_, u64 offset,
                                   u32 draw_count, u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx    = render_ctx;
  Buffer * const      buffer = (Buffer *) buffer_;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(has_bits(buffer->info.usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(offset < buffer->info.size, "");
  CHECK((offset + (u64) draw_count * stride) <= buffer->info.size, "");
  CHECK(is_aligned(4U, stride), "");
  CHECK(stride >= sizeof(gpu::DrawCommand), "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDrawIndirect{.buffer     = buffer,
                                         .offset     = offset,
                                         .draw_count = draw_count,
                                         .stride     = stride}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw_indexed_indirect(gpu::Buffer buffer_, u64 offset,
                                           u32 draw_count, u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx    = render_ctx;
  Buffer * const      buffer = (Buffer *) buffer_;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(ctx.index_buffer != nullptr, "");
  CHECK(has_bits(buffer->info.usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(offset < buffer->info.size, "");
  CHECK((offset + (u64) draw_count * stride) <= buffer->info.size, "");
  CHECK(is_aligned(4U, stride), "");
  CHECK(stride >= sizeof(gpu::DrawIndexedCommand), "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDrawIndexedIndirect{.buffer     = buffer,
                                                .offset     = offset,
                                                .draw_count = draw_count,
                                                .stride     = stride}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

}    // namespace vk
}    // namespace ash
