#include "ashura/gfx/vulkan.h"
#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "vulkan/vulkan.h"
#include <stdlib.h>

#ifndef VK_LAYER_KHRONOS_VALIDATION_NAME
#  define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"
#endif

namespace ash
{
namespace vk
{

#define sPANIC_IF(logger, description, ...)                                \
  do                                                                       \
  {                                                                        \
    if (!(__VA_ARGS__))                                                    \
    {                                                                      \
      (logger)->panic(description, " (expression: " #__VA_ARGS__,          \
                      ") [function: ", SourceLocation::current().function, \
                      ", file: ", SourceLocation::current().file, ":",     \
                      SourceLocation::current().line, ":",                 \
                      SourceLocation::current().column, "]");              \
    }                                                                      \
  } while (false)

#define sVALIDATE(...) \
  sPANIC_IF(self->logger, "Validation Failed: " #__VA_ARGS__, __VA_ARGS__)

#define sCHECK(...) \
  sPANIC_IF(self->logger, "Check Failed: " #__VA_ARGS__, __VA_ARGS__)

#define sCHECK_EX(logger, ...) \
  sPANIC_IF(logger, "Check Failed: " #__VA_ARGS__, __VA_ARGS__)

#define sUNREACHABLE() abort()

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
                         InstanceTable &vk_table, bool validation_layer_enabled)
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

  if (validation_layer_enabled)
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
                       DeviceTable &vk_table)
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
  LOAD_VK(CreateFramebuffer);
  LOAD_VK(CreateGraphicsPipelines);
  LOAD_VK(CreateImage);
  LOAD_VK(CreateImageView);
  LOAD_VK(CreatePipelineCache);
  LOAD_VK(CreatePipelineLayout);
  LOAD_VK(CreateQueryPool);
  LOAD_VK(CreateRenderPass);
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
  LOAD_VK(DestroyFramebuffer);
  LOAD_VK(DestroyImage);
  LOAD_VK(DestroyImageView);
  LOAD_VK(DestroyPipeline);
  LOAD_VK(DestroyPipelineCache);
  LOAD_VK(DestroyPipelineLayout);
  LOAD_VK(DestroyQueryPool);
  LOAD_VK(DestroyRenderPass);
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
  LOAD_VK(CmdBeginRenderPass);
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
  LOAD_VK(CmdEndRenderPass);
  LOAD_VK(CmdFillBuffer);
  LOAD_VK(CmdNextSubpass);
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

  LOAD_VK(CreateSwapchainKHR);
  LOAD_VK(DestroySwapchainKHR);
  LOAD_VK(GetSwapchainImagesKHR);
  LOAD_VK(AcquireNextImageKHR);
  LOAD_VK(QueuePresentKHR);

#undef LOAD_VK

#define LOAD_VK_STUBBED(function)                                          \
  vk_table.function = (PFN_vk##function) instance_table.GetDeviceProcAddr( \
      dev, "vk" #function);                                                \
  vk_table.function =                                                      \
      (vk_table.function != nullptr) ? vk_table.function : function##_Stub;

  LOAD_VK_STUBBED(DebugMarkerSetObjectTagEXT);
  LOAD_VK_STUBBED(DebugMarkerSetObjectNameEXT);

  LOAD_VK_STUBBED(CmdDebugMarkerBeginEXT);
  LOAD_VK_STUBBED(CmdDebugMarkerEndEXT);
  LOAD_VK_STUBBED(CmdDebugMarkerInsertEXT);

#undef LOAD_VK_STUBBED

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

static VkBool32 VKAPI_ATTR VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    VkDebugUtilsMessengerCallbackDataEXT const *data, void *user_data)
{
  Instance *const instance = (Instance *) user_data;

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

  instance->logger->log(
      level, "[Type: ", string_VkDebugUtilsMessageTypeFlagsEXT(message_type),
      ", Id: ", data->messageIdNumber, ", Name: ", data->pMessageIdName, "] ",
      data->pMessage == nullptr ? "(empty message)" : data->pMessage);
  if (data->objectCount != 0)
  {
    instance->logger->log(level, "Objects Involved:");
    for (u32 i = 0; i < data->objectCount; i++)
    {
      instance->logger->log(
          level, "[Type: ", string_VkObjectType(data->pObjects[i].objectType),
          "] ",
          data->pObjects[i].pObjectName == nullptr ?
              "(unnamed)" :
              data->pObjects[i].pObjectName);
    }
  }

  return VK_FALSE;
}

constexpr VkAccessFlags
    color_attachment_image_access(gfx::RenderPassAttachment const &attachment)
{
  VkAccessFlags access = VK_ACCESS_NONE;

  if (attachment.load_op == gfx::LoadOp::Clear ||
      attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store)
  {
    access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }

  if (attachment.load_op == gfx::LoadOp::Load)
  {
    access |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  }

  return access;
}

constexpr VkAccessFlags depth_stencil_attachment_image_access(
    gfx::RenderPassAttachment const &attachment)
{
  VkAccessFlags access = VK_ACCESS_NONE;

  if (attachment.load_op == gfx::LoadOp::Clear ||
      attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store ||
      attachment.store_op == gfx::StoreOp::DontCare ||
      attachment.stencil_load_op == gfx::LoadOp::Clear ||
      attachment.stencil_load_op == gfx::LoadOp::DontCare ||
      attachment.stencil_store_op == gfx::StoreOp::Store ||
      attachment.stencil_store_op == gfx::StoreOp::DontCare)
  {
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  }

  if (attachment.load_op == gfx::LoadOp::Load ||
      attachment.stencil_load_op == gfx::LoadOp::Load)
  {
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  }

  return access;
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

inline bool sync_buffer(BufferState &state, BufferAccess request,
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
inline bool sync_image(ImageState &state, ImageAccess request,
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
  if (sync_buffer(buffer.state,
                  BufferAccess{.stages = stages, .access = access}, barrier,
                  src_stages, dst_stages))
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

inline void access_image(CommandEncoder const &enc, Image &image,
                         VkPipelineStageFlags stages, VkAccessFlags access,
                         VkImageLayout layout)
{
  VkImageMemoryBarrier barrier;
  VkPipelineStageFlags src_stages;
  VkPipelineStageFlags dst_stages;
  if (sync_image(
          image.state,
          ImageAccess{.stages = stages, .access = access, .layout = layout},
          barrier, src_stages, dst_stages))
  {
    barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = image.vk_image;
    barrier.subresourceRange.aspectMask =
        (VkImageAspectFlags) image.desc.aspects;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
    enc.dev->vk_table.CmdPipelineBarrier(enc.vk_command_buffer, src_stages,
                                         dst_stages, 0, 0, nullptr, 0, nullptr,
                                         1, &barrier);
  }
}

inline void access_compute_bindings(CommandEncoder const &enc,
                                    DescriptorSet const  &set)
{
  for (u32 ibinding = 0; ibinding < set.num_bindings; ibinding++)
  {
    DescriptorBinding const &binding = set.bindings[ibinding];
    switch (binding.type)
    {
      case gfx::DescriptorType::CombinedImageSampler:
      case gfx::DescriptorType::SampledImage:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image(enc, *binding.images[i],
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

      case gfx::DescriptorType::StorageImage:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image(enc, *binding.images[i],
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                         VK_IMAGE_LAYOUT_GENERAL);
          }
        }
        break;

      case gfx::DescriptorType::UniformBuffer:
      case gfx::DescriptorType::DynamicUniformBuffer:
      case gfx::DescriptorType::UniformTexelBuffer:
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

      case gfx::DescriptorType::StorageBuffer:
      case gfx::DescriptorType::DynamicStorageBuffer:
      case gfx::DescriptorType::StorageTexelBuffer:
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

      case gfx::DescriptorType::InputAttachment:
        break;

      default:
        sUNREACHABLE();
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
      case gfx::DescriptorType::CombinedImageSampler:
      case gfx::DescriptorType::SampledImage:
      case gfx::DescriptorType::InputAttachment:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image(enc, *binding.images[i],
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

      case gfx::DescriptorType::UniformTexelBuffer:
      case gfx::DescriptorType::UniformBuffer:
      case gfx::DescriptorType::DynamicUniformBuffer:
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
      case gfx::DescriptorType::StorageImage:
        for (u32 i = 0; i < binding.count; i++)
        {
          if (binding.images[i] != nullptr)
          {
            access_image(enc, *binding.images[i],
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL);
          }
        }
        break;

        // only readonly storage buffers are supported
      case gfx::DescriptorType::StorageTexelBuffer:
      case gfx::DescriptorType::StorageBuffer:
      case gfx::DescriptorType::DynamicStorageBuffer:
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
      default:
        sUNREACHABLE();
    }
  }
}

inline bool is_render_pass_compatible(RenderPass const      &render_pass,
                                      Span<ImageView *const> color_attachments,
                                      ImageView const *depth_stencil_attachment)
{
  // also depends on the formats of the input attachments which can't be
  // determined here
  // our render_passes uses same initial and final layouts
  if (render_pass.num_color_attachments != color_attachments.size())
  {
    return false;
  }

  if ((render_pass.depth_stencil_attachment.format == gfx::Format::Undefined) &&
      (depth_stencil_attachment != nullptr))
  {
    return false;
  }

  if (depth_stencil_attachment != nullptr &&
      (render_pass.depth_stencil_attachment.format !=
       IMAGE_FROM_VIEW(depth_stencil_attachment)->desc.format))
  {
    return false;
  }

  for (usize i = 0; i < render_pass.num_color_attachments; i++)
  {
    if (render_pass.color_attachments[i].format !=
        IMAGE_FROM_VIEW(color_attachments[i])->desc.format)
    {
      return false;
    }
  }

  return true;
}

inline bool is_image_view_type_compatible(gfx::ImageType     image_type,
                                          gfx::ImageViewType view_type)
{
  switch (view_type)
  {
    case gfx::ImageViewType::Type1D:
    case gfx::ImageViewType::Type1DArray:
      return image_type == gfx::ImageType::Type1D;
    case gfx::ImageViewType::Type2D:
    case gfx::ImageViewType::Type2DArray:
      return image_type == gfx::ImageType::Type2D ||
             image_type == gfx::ImageType::Type3D;
    case gfx::ImageViewType::TypeCube:
    case gfx::ImageViewType::TypeCubeArray:
      return image_type == gfx::ImageType::Type2D;
    case gfx::ImageViewType::Type3D:
      return image_type == gfx::ImageType::Type3D;
    default:
      return false;
  }
}

inline u64 index_type_size(gfx::IndexType type)
{
  switch (type)
  {
    case gfx::IndexType::Uint16:
      return 2;
    case gfx::IndexType::Uint32:
      return 4;
    default:
      sUNREACHABLE();
  }
}

inline bool is_valid_buffer_access(u64 size, u64 access_offset, u64 access_size,
                                   u64 alignment = 1)
{
  access_size =
      (access_size == gfx::WHOLE_SIZE) ? (size - access_offset) : access_size;
  return (access_offset < size) && ((access_offset + access_size) <= size) &&
         mem::is_aligned(alignment, access_offset) && (access_size > 0);
}

inline bool is_valid_image_access(gfx::ImageAspects aspects, u32 num_levels,
                                  u32               num_layers,
                                  gfx::ImageAspects access_aspects,
                                  u32 access_level, u32 num_access_levels,
                                  u32 access_layer, u32 num_access_layers)
{
  num_access_levels = num_access_levels == gfx::REMAINING_MIP_LEVELS ?
                          (num_levels - access_level) :
                          num_access_levels;
  num_access_layers = num_access_layers == gfx::REMAINING_ARRAY_LAYERS ?
                          (num_access_layers - access_layer) :
                          num_access_layers;
  return access_level < num_levels && access_layer < num_layers &&
         (access_level + num_access_levels) <= num_levels &&
         (access_layer + num_access_layers) <= num_layers &&
         num_access_levels > 0 && num_access_layers > 0 &&
         has_bits(aspects, access_aspects) &&
         access_aspects != gfx::ImageAspects::None;
}

Result<gfx::InstanceImpl, Status> create_instance(AllocatorImpl allocator,
                                                  Logger       *logger,
                                                  bool enable_validation_layer)
{
  bool     enable_debug_messenger = enable_validation_layer;
  u32      num_extensions;
  VkResult result =
      vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkExtensionProperties *extensions =
      allocator.allocate_typed<VkExtensionProperties>(num_extensions);

  if (num_extensions != 0 && extensions == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  {
    u32 num_read_extensions = num_extensions;
    result                  = vkEnumerateInstanceExtensionProperties(
        nullptr, &num_read_extensions, extensions);

    if (result != VK_SUCCESS)
    {
      allocator.deallocate_typed(extensions, num_extensions);
      return Err{(Status) result};
    }

    sCHECK_EX(logger, num_read_extensions == num_extensions);
  }

  u32 num_layers;
  result = vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    allocator.deallocate_typed(extensions, num_extensions);
    return Err{(Status) result};
  }

  VkLayerProperties *layers =
      allocator.allocate_typed<VkLayerProperties>(num_layers);

  if (num_layers != 0 && layers == nullptr)
  {
    allocator.deallocate_typed(extensions, num_extensions);
    return Err{Status::OutOfHostMemory};
  }

  {
    u32 num_read_layers = num_layers;
    result = vkEnumerateInstanceLayerProperties(&num_read_layers, layers);

    if (result != VK_SUCCESS)
    {
      allocator.deallocate_typed(extensions, num_extensions);
      allocator.deallocate_typed(layers, num_layers);
      return Err{(Status) result};
    }

    sCHECK_EX(logger, num_read_layers == num_layers);
  }

  logger->trace("Available Extensions:");

  for (VkExtensionProperties const &extension :
       Span{extensions, num_extensions})
  {
    logger->trace(extension.extensionName, "\t\t(spec version ",
                  VK_API_VERSION_MAJOR(extension.specVersion), ".",
                  VK_API_VERSION_MINOR(extension.specVersion), ".",
                  VK_API_VERSION_PATCH(extension.specVersion), " variant ",
                  VK_API_VERSION_VARIANT(extension.specVersion), ")");
  }

  logger->trace("Available Validation Layers:");

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

  char const *load_extensions[16];
  char const *load_layers[16];
  u32         num_load_extensions = 0;
  u32         num_load_layers     = 0;

  constexpr char const *OPTIONAL_EXTENSIONS[] = {
      "VK_KHR_surface",         "VK_KHR_android_surface", "VK_MVK_ios_surface",
      "VK_MVK_macos_surface",   "VK_EXT_metal_surface",   "VK_NN_vi_surface",
      "VK_KHR_wayland_surface", "VK_KHR_win32_surface",   "VK_KHR_xcb_surface",
      "VK_KHR_xlib_surface"};

  for (char const *extension : OPTIONAL_EXTENSIONS)
  {
    if (!find(Span<VkExtensionProperties const>{extensions, num_extensions},
              extension,
              [](VkExtensionProperties const &property, char const *find_name) {
                return strcmp(property.extensionName, find_name) == 0;
              })
             .is_empty())
    {
      load_extensions[num_load_extensions] = extension;
      num_load_extensions++;
    }
  }

  if (enable_debug_messenger)
  {
    if (find(Span<VkExtensionProperties const>{extensions, num_extensions},
             VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
             [](VkExtensionProperties const &property, char const *find_name) {
               return strcmp(property.extensionName, find_name) == 0;
             })
            .is_empty())
    {
      logger->warn(
          "Required Vulkan Validation Layer: " VK_EXT_DEBUG_UTILS_EXTENSION_NAME
          " is not supported");
      enable_debug_messenger = false;
    }
    else
    {
      load_extensions[num_load_extensions] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
      num_load_extensions++;
    }
  }

  if (enable_validation_layer)
  {
    if (find(Span<VkLayerProperties const>{layers, num_layers},
             "VK_LAYER_KHRONOS_validation",
             [](VkLayerProperties const &property, char const *find_name) {
               return strcmp(property.layerName, find_name) == 0;
             })
            .is_empty())
    {
      logger->warn(
          "Required Vulkan Validation Layer: VK_LAYER_KHRONOS_validation is "
          "not supported");
      enable_validation_layer = false;
    }
    else
    {
      load_layers[num_load_layers] = "VK_LAYER_KHRONOS_validation";
      num_load_layers++;
    }
  }

  allocator.deallocate_typed(extensions, num_extensions);
  allocator.deallocate_typed(layers, num_layers);

  Instance *instance = allocator.allocate_typed<Instance>(1);

  if (instance == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (instance) Instance{.allocator                = allocator,
                          .logger                   = logger,
                          .vk_table                 = {},
                          .vk_instance              = nullptr,
                          .vk_debug_messenger       = nullptr,
                          .validation_layer_enabled = enable_validation_layer};

  VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pNext = nullptr,
                             .pApplicationName   = CLIENT_NAME,
                             .applicationVersion = CLIENT_VERSION,
                             .pEngineName        = ENGINE_NAME,
                             .engineVersion      = ENGINE_VERSION,
                             .apiVersion         = 0};

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
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = enable_validation_layer ? &debug_create_info : nullptr,
      .flags = 0,
      .pApplicationInfo        = &app_info,
      .enabledLayerCount       = num_load_layers,
      .ppEnabledLayerNames     = load_layers,
      .enabledExtensionCount   = num_load_extensions,
      .ppEnabledExtensionNames = load_extensions};

  VkInstance vk_instance;

  result = vkCreateInstance(&create_info, nullptr, &vk_instance);
  if (result != VK_SUCCESS)
  {
    allocator.deallocate_typed(instance, 1);
    return Err{(Status) result};
  }

  InstanceTable vk_table;

  sCHECK_EX(logger, load_instance_table(vk_instance, vkGetInstanceProcAddr,
                                        vk_table, enable_validation_layer));

  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;

  if (enable_debug_messenger)
  {
    result = vk_table.CreateDebugUtilsMessengerEXT(
        vk_instance, &debug_create_info, nullptr, &vk_debug_messenger);
    if (result != VK_SUCCESS)
    {
      vk_table.DestroyInstance(vk_instance, nullptr);
      // destroy our instance object after to allow debug reporter report
      // messages through the pointer to it
      allocator.deallocate_typed(instance, 1);
      return Err{(Status) result};
    }
  }

  instance->vk_table           = vk_table;
  instance->vk_instance        = vk_instance;
  instance->vk_debug_messenger = vk_debug_messenger;

  return Ok{gfx::InstanceImpl{.self      = (gfx::Instance) instance,
                              .interface = &instance_interface}};
}
}        // namespace vk

namespace gfx
{

Result<InstanceImpl, Status>
    create_vulkan_instance(AllocatorImpl allocator, Logger *logger,
                           bool enable_validation_layer)
{
  return vk::create_instance(allocator, logger, enable_validation_layer);
}

}        // namespace gfx

namespace vk
{

void InstanceInterface::destroy(gfx::Instance instance_)
{
  Instance *const instance = (Instance *) instance_;

  if (instance == nullptr)
  {
    return;
  }
  if (instance->validation_layer_enabled)
  {
    instance->vk_table.DestroyDebugUtilsMessengerEXT(
        instance->vk_instance, instance->vk_debug_messenger, nullptr);
  }
  instance->vk_table.DestroyInstance(instance->vk_instance, nullptr);
  instance->allocator.deallocate_typed(instance, 1);
}

void check_device_limits(Instance *self, VkPhysicalDeviceLimits limits)
{
  sVALIDATE(limits.maxImageDimension1D >= gfx::MAX_IMAGE_EXTENT);
  sVALIDATE(limits.maxImageDimension2D >= gfx::MAX_IMAGE_EXTENT);
  sVALIDATE(limits.maxImageDimension3D >= gfx::MAX_IMAGE_EXTENT);
  sVALIDATE(limits.maxImageDimensionCube >= gfx::MAX_IMAGE_EXTENT);
  sVALIDATE(limits.maxImageArrayLayers >= gfx::MAX_IMAGE_ARRAY_LAYERS);
  sVALIDATE(limits.maxViewportDimensions[0] >= gfx::MAX_VIEWPORT_EXTENT);
  sVALIDATE(limits.maxViewportDimensions[1] >= gfx::MAX_VIEWPORT_EXTENT);
  sVALIDATE(limits.maxFramebufferWidth >= gfx::MAX_FRAMEBUFFER_EXTENT);
  sVALIDATE(limits.maxFramebufferHeight >= gfx::MAX_FRAMEBUFFER_EXTENT);
  sVALIDATE(limits.maxFramebufferLayers >= gfx::MAX_FRAMEBUFFER_LAYERS);
  sVALIDATE(limits.maxVertexInputAttributes >= gfx::MAX_VERTEX_ATTRIBUTES);
  sVALIDATE(limits.maxVertexInputBindings >= gfx::MAX_VERTEX_ATTRIBUTES);
  sVALIDATE(limits.maxPushConstantsSize >= gfx::MAX_PUSH_CONSTANTS_SIZE);
  sVALIDATE(limits.maxBoundDescriptorSets >= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  sVALIDATE(limits.maxPerStageDescriptorInputAttachments >=
            gfx::MAX_PIPELINE_INPUT_ATTACHMENTS);
  sVALIDATE(limits.maxComputeWorkGroupCount[0] >=
            gfx::MAX_COMPUTE_WORK_GROUP_COUNT);
  sVALIDATE(limits.maxComputeWorkGroupCount[1] >=
            gfx::MAX_COMPUTE_WORK_GROUP_COUNT);
  sVALIDATE(limits.maxComputeWorkGroupCount[2] >=
            gfx::MAX_COMPUTE_WORK_GROUP_COUNT);
  sVALIDATE(limits.maxComputeWorkGroupSize[0] >=
            gfx::MAX_COMPUTE_WORK_GROUP_SIZE);
  sVALIDATE(limits.maxComputeWorkGroupSize[1] >=
            gfx::MAX_COMPUTE_WORK_GROUP_SIZE);
  sVALIDATE(limits.maxComputeWorkGroupSize[2] >=
            gfx::MAX_COMPUTE_WORK_GROUP_SIZE);
  sVALIDATE(limits.maxComputeWorkGroupInvocations >=
            gfx::MAX_COMPUTE_WORK_GROUP_INVOCATIONS);
  sVALIDATE(limits.maxComputeSharedMemorySize >=
            gfx::MAX_COMPUTE_SHARED_MEMORY_SIZE);
  sVALIDATE(limits.maxUniformBufferRange >= gfx::MAX_UNIFORM_BUFFER_RANGE);
  sVALIDATE(limits.maxColorAttachments >= gfx::MAX_PIPELINE_COLOR_ATTACHMENTS);
  sVALIDATE(limits.maxSamplerAnisotropy >= gfx::MAX_SAMPLER_ANISOTROPY);
  sVALIDATE(limits.maxClipDistances >= gfx::MAX_CLIP_DISTANCES);
  sVALIDATE(limits.maxCullDistances >= gfx::MAX_CULL_DISTANCES);
  sVALIDATE(limits.maxCombinedClipAndCullDistances >=
            gfx::MAX_COMBINED_CLIP_AND_CULL_DISTANCES);
}

void check_device_features(Instance *self, VkPhysicalDeviceFeatures feat)
{
  sVALIDATE(feat.samplerAnisotropy == VK_TRUE);
  sVALIDATE(feat.shaderUniformBufferArrayDynamicIndexing == VK_TRUE);
  sVALIDATE(feat.shaderSampledImageArrayDynamicIndexing == VK_TRUE);
  sVALIDATE(feat.shaderStorageBufferArrayDynamicIndexing == VK_TRUE);
  sVALIDATE(feat.shaderStorageImageArrayDynamicIndexing == VK_TRUE);
  sVALIDATE(feat.shaderClipDistance == VK_TRUE);
  sVALIDATE(feat.shaderCullDistance == VK_TRUE);
  sVALIDATE(feat.multiDrawIndirect == VK_TRUE);
  sVALIDATE(feat.drawIndirectFirstInstance == VK_TRUE);
  sVALIDATE(feat.imageCubeArray == VK_TRUE);
}

void set_resource_name(Device *dev, Span<char const> label,
                       void const *resource, VkObjectType type,
                       VkDebugReportObjectTypeEXT debug_type)
{
  char buff[256];
  to_c_str(label, to_span(buff));
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
      .logger            = dev->logger,
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

void destroy_command_encoder(Device *dev, CommandEncoder *enc)
{
  enc->render_ctx.commands.reset();
  dev->vk_table.DestroyCommandPool(dev->vk_dev, enc->vk_command_pool, nullptr);
}

Status create_frame_context(Device *dev, FrameContext *ctx, u32 buffering)
{
  CommandEncoder encs[gfx::MAX_FRAME_BUFFERING];
  u32            num_encs = 0;

  defer encs_del{[&] {
    for (u32 i = 0; i < num_encs; i++)
    {
      destroy_command_encoder(dev, encs + i);
    }
  }};

  for (; num_encs < buffering; num_encs++)
  {
    Status status = create_command_encoder(dev, encs + num_encs);
    if (status != Status::Success)
    {
      return status;
    }
  }

  VkSemaphore acquire[gfx::MAX_FRAME_BUFFERING];
  u32         num_acquire = 0;

  defer acquire_del{[&] {
    for (u32 i = num_acquire; i-- > 0;)
    {
      dev->vk_table.DestroySemaphore(dev->vk_dev, acquire[i], nullptr);
    }
  }};

  VkSemaphoreCreateInfo sem_info{.sType =
                                     VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0};

  for (; num_acquire < buffering; num_acquire++)
  {
    VkResult result = dev->vk_table.CreateSemaphore(
        dev->vk_dev, &sem_info, nullptr, acquire + num_acquire);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }
    set_resource_name(dev, "Frame Acquire Semaphore"_span, acquire[num_acquire],
                      VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
  }

  VkFence submit_f[gfx::MAX_FRAME_BUFFERING];
  u32     num_submit_f = 0;

  defer submit_f_del{[&] {
    for (u32 i = num_submit_f; i-- > 0;)
    {
      dev->vk_table.DestroyFence(dev->vk_dev, submit_f[i], nullptr);
    }
  }};

  VkFenceCreateInfo fence_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                               .pNext = nullptr,
                               .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  for (; num_submit_f < buffering; num_submit_f++)
  {
    VkResult result = dev->vk_table.CreateFence(
        dev->vk_dev, &fence_info, nullptr, submit_f + num_submit_f);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }

    set_resource_name(dev, "Frame Submit Fence"_span, submit_f[num_submit_f],
                      VK_OBJECT_TYPE_FENCE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);
  }

  VkSemaphore submit_s[gfx::MAX_FRAME_BUFFERING];
  u32         num_submit_s = 0;

  defer submit_s_del{[&] {
    for (u32 i = num_submit_s; i-- > 0;)
    {
      dev->vk_table.DestroySemaphore(dev->vk_dev, submit_s[i], nullptr);
    }
  }};

  for (; num_submit_s < buffering; num_submit_s++)
  {
    VkResult result = dev->vk_table.CreateSemaphore(
        dev->vk_dev, &sem_info, nullptr, submit_s + num_submit_s);
    if (result != VK_SUCCESS)
    {
      return (Status) result;
    }
    set_resource_name(dev, "Frame Submit Semaphore"_span,
                      submit_s[num_submit_s], VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
  }

  new (ctx) FrameContext{.tail_frame    = 0,
                         .current_frame = 0,
                         .ring_index    = 0,
                         .buffering     = buffering};

  mem::copy(encs, ctx->encs, buffering);
  mem::copy(acquire, ctx->acquire, buffering);
  mem::copy(submit_f, ctx->submit_f, buffering);
  mem::copy(submit_s, ctx->submit_s, buffering);

  num_encs     = 0;
  num_acquire  = 0;
  num_submit_f = 0;
  num_submit_s = 0;

  // self-referential
  for (u32 i = 0; i < buffering; i++)
  {
    ctx->encs_impl[i] =
        gfx::CommandEncoderImpl{.self = (gfx::CommandEncoder)(ctx->encs + i),
                                .interface = &command_encoder_interface};
  }

  return Status::Success;
}

void destroy_frame_context(Device *dev, FrameContext *ctx)
{
  for (u32 i = ctx->buffering; i-- > 0;)
  {
    destroy_command_encoder(dev, ctx->encs + i);
  }
  for (u32 i = ctx->buffering; i-- > 0;)
  {
    dev->vk_table.DestroySemaphore(dev->vk_dev, ctx->acquire[i], nullptr);
  }
  for (u32 i = ctx->buffering; i-- > 0;)
  {
    dev->vk_table.DestroyFence(dev->vk_dev, ctx->submit_f[i], nullptr);
  }
  for (u32 i = ctx->buffering; i-- > 0;)
  {
    dev->vk_table.DestroySemaphore(dev->vk_dev, ctx->submit_s[i], nullptr);
  }
}

void destroy_descriptor_heap(Device *self, DescriptorHeap *heap)
{
  for (u32 i = heap->num_pools; i-- > 0;)
  {
    self->vk_table.DestroyDescriptorPool(self->vk_dev, heap->pools[i].vk_pool,
                                         nullptr);
  }
  heap->allocator.deallocate_typed(heap->pools, heap->num_pools);
  heap->allocator.deallocate(MAX_STANDARD_ALIGNMENT, heap->scratch,
                             heap->scratch_size);
}

Result<gfx::DeviceImpl, Status> InstanceInterface::create_device(
    gfx::Instance self_, AllocatorImpl allocator,
    Span<gfx::DeviceType const> preferred_types,
    Span<gfx::Surface const> compatible_surfaces, u32 buffering)
{
  Instance *const self               = (Instance *) self_;
  u32 const       num_surfaces       = (u32) compatible_surfaces.size();
  constexpr u32   MAX_QUEUE_FAMILIES = 16;

  sVALIDATE(buffering > 0);
  sVALIDATE(buffering <= gfx::MAX_FRAME_BUFFERING);

  u32      num_devs;
  VkResult result = self->vk_table.EnumeratePhysicalDevices(self->vk_instance,
                                                            &num_devs, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  if (num_devs == 0)
  {
    self->logger->trace("No Physical Device Found");
    return Err{Status::DeviceLost};
  }

  VkPhysicalDevice *vk_phy_devs =
      self->allocator.allocate_typed<VkPhysicalDevice>(num_devs);

  if (vk_phy_devs == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_phy_devs_del{
      [&] { self->allocator.deallocate_typed(vk_phy_devs, num_devs); }};

  {
    u32 num_read_devs = num_devs;
    result            = self->vk_table.EnumeratePhysicalDevices(
        self->vk_instance, &num_read_devs, vk_phy_devs);

    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    sCHECK(num_read_devs == num_devs);
  }

  PhysicalDevice *physical_devs =
      self->allocator.allocate_typed<PhysicalDevice>(num_devs);

  if (physical_devs == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  defer physical_devs_del{
      [&] { self->allocator.deallocate_typed(physical_devs, num_devs); }};

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

  self->logger->trace("Available Devices:");
  for (u32 i = 0; i < num_devs; i++)
  {
    PhysicalDevice const             &dev        = physical_devs[i];
    VkPhysicalDeviceProperties const &properties = dev.vk_properties;
    self->logger->trace(
        "[Device: ", i, "] ",
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

    sCHECK(num_queue_families <= MAX_QUEUE_FAMILIES);

    VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

    {
      u32 num_read_queue_families = num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          dev.vk_phy_dev, &num_queue_families, queue_family_properties);
      sCHECK(num_read_queue_families == num_queue_families);
    }

    for (u32 iqueue_family = 0; iqueue_family < num_queue_families;
         iqueue_family++)
    {
      self->logger->trace(
          "\t\tQueue Family: ", iqueue_family,
          ", Count: ", queue_family_properties[iqueue_family].queueCount,
          ", Flags: ",
          string_VkQueueFlags(
              queue_family_properties[iqueue_family].queueFlags));
    }
  }

  u32 selected_dev_index    = num_devs;
  u32 selected_queue_family = VK_QUEUE_FAMILY_IGNORED;

  for (usize i = 0; i < (u32) preferred_types.size(); i++)
  {
    for (u32 idevice = 0; idevice < num_devs && selected_dev_index == num_devs;
         idevice++)
    {
      PhysicalDevice const &dev = physical_devs[idevice];

      u32 num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          dev.vk_phy_dev, &num_queue_families, nullptr);

      sCHECK(num_queue_families <= MAX_QUEUE_FAMILIES);

      VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

      {
        u32 num_read_queue_families = num_queue_families;
        self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
            dev.vk_phy_dev, &num_queue_families, queue_family_properties);
        sCHECK(num_read_queue_families == num_queue_families);
      }

      if (((VkPhysicalDeviceType) preferred_types[i]) ==
          dev.vk_properties.deviceType)
      {
        for (u32 iqueue_family = 0; iqueue_family < num_queue_families &&
                                    selected_dev_index == num_devs;
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
              selected_dev_index    = idevice;
              selected_queue_family = iqueue_family;
              break;
            }
          }
        }
      }
    }
  }

  if (selected_dev_index == num_devs)
  {
    self->logger->trace("No Suitable Device Found");
    return Err{Status::DeviceLost};
  }

  PhysicalDevice selected_dev = physical_devs[selected_dev_index];

  check_device_limits(self, selected_dev.vk_properties.limits);
  check_device_features(self, selected_dev.vk_features);

  self->logger->trace("Selected Device ", selected_dev_index);

  u32 num_extensions;
  result = self->vk_table.EnumerateDeviceExtensionProperties(
      selected_dev.vk_phy_dev, nullptr, &num_extensions, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkExtensionProperties *extensions =
      self->allocator.allocate_typed<VkExtensionProperties>(num_extensions);

  if (num_extensions > 0 && extensions == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  defer extensions_del{
      [&] { self->allocator.deallocate_typed(extensions, num_extensions); }};

  {
    u32 num_read_extensions = num_extensions;
    result                  = self->vk_table.EnumerateDeviceExtensionProperties(
        selected_dev.vk_phy_dev, nullptr, &num_read_extensions, extensions);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
    sCHECK(num_extensions == num_read_extensions);
  }

  u32 num_layers;
  result = self->vk_table.EnumerateDeviceLayerProperties(
      selected_dev.vk_phy_dev, &num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkLayerProperties *layers =
      self->allocator.allocate_typed<VkLayerProperties>(num_layers);

  if (num_layers > 0 && layers == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  defer layers_del{
      [&] { self->allocator.deallocate_typed(layers, num_layers); }};

  {
    u32 num_read_layers = num_layers;
    result              = self->vk_table.EnumerateDeviceLayerProperties(
        selected_dev.vk_phy_dev, &num_read_layers, layers);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
    sCHECK(num_read_layers == num_layers);
  }

  self->logger->trace("Available Extensions:");

  for (u32 i = 0; i < num_extensions; i++)
  {
    VkExtensionProperties const &ext = extensions[i];
    self->logger->trace("\t\t", ext.extensionName, " (spec version: ",
                        VK_API_VERSION_MAJOR(ext.specVersion), ".",
                        VK_API_VERSION_MINOR(ext.specVersion), ".",
                        VK_API_VERSION_PATCH(ext.specVersion), " variant ",
                        VK_API_VERSION_VARIANT(ext.specVersion), ")");
  }

  self->logger->trace("Available Layers:");

  for (u32 i = 0; i < num_layers; i++)
  {
    VkLayerProperties const &layer = layers[i];

    self->logger->trace("\t\t", layer.layerName, " (spec version: ",
                        VK_API_VERSION_MAJOR(layer.specVersion), ".",
                        VK_API_VERSION_MINOR(layer.specVersion), ".",
                        VK_API_VERSION_PATCH(layer.specVersion), " variant ",
                        VK_API_VERSION_VARIANT(layer.specVersion),
                        ", "
                        "implementation version: ",
                        layer.implementationVersion, ")");
  }

  bool has_swapchain_ext           = false;
  bool has_debug_marker_ext        = false;
  bool has_descriptor_indexing_ext = false;
  bool has_validation_layer        = false;

  for (u32 i = 0; i < num_extensions; i++)
  {
    if (strcmp(extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) ==
        0)
    {
      has_swapchain_ext = true;
    }
    if (strcmp(extensions[i].extensionName,
               VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
    {
      has_debug_marker_ext = true;
    }

    if (strcmp(extensions[i].extensionName,
               VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0)
    {
      has_descriptor_indexing_ext = true;
    }
  }

  for (u32 i = 0; i < num_layers; i++)
  {
    if (strcmp(layers[i].layerName, VK_LAYER_KHRONOS_VALIDATION_NAME) == 0)
    {
      has_validation_layer = true;
      break;
    }
  }

  char const *load_extensions[4];
  u32         num_load_extensions = 0;
  char const *load_layers[4];
  u32         num_load_layers = 0;

  // required
  load_extensions[num_load_extensions] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  num_load_extensions++;
  load_extensions[num_load_extensions] =
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
  num_load_extensions++;

  // optional, stubbed
  if (has_debug_marker_ext)
  {
    load_extensions[num_load_extensions] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
    num_load_extensions++;
  }

  // optional
  if (self->validation_layer_enabled && has_validation_layer)
  {
    load_layers[num_load_layers] = VK_LAYER_KHRONOS_VALIDATION_NAME;
    num_load_layers++;
  }

  // required
  if (!has_swapchain_ext)
  {
    self->logger->trace("Required Extension: " VK_KHR_SWAPCHAIN_EXTENSION_NAME
                        " Not Present");
    return Err{Status::ExtensionNotPresent};
  }

  // required
  if (!has_descriptor_indexing_ext)
  {
    self->logger->trace(
        "Required Extension: " VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
        " Not Present");
    return Err{Status::ExtensionNotPresent};
  }

  f32 const queue_priority = 1.0F;

  VkDeviceQueueCreateInfo queue_create_info{
      .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = 0,
      .queueFamilyIndex = selected_queue_family,
      .queueCount       = 1,
      .pQueuePriorities = &queue_priority};

  VkPhysicalDeviceFeatures features;
  mem::zero(&features, 1);
  features.samplerAnisotropy                       = VK_TRUE;
  features.shaderUniformBufferArrayDynamicIndexing = VK_TRUE;
  features.shaderSampledImageArrayDynamicIndexing  = VK_TRUE;
  features.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
  features.shaderStorageImageArrayDynamicIndexing  = VK_TRUE;
  features.shaderClipDistance                      = VK_TRUE;
  features.shaderCullDistance                      = VK_TRUE;
  features.multiDrawIndirect                       = VK_TRUE;
  features.drawIndirectFirstInstance               = VK_TRUE;
  features.imageCubeArray                          = VK_TRUE;

  VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_features{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
      .pNext                                              = nullptr,
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
                                 .enabledExtensionCount   = num_load_extensions,
                                 .ppEnabledExtensionNames = load_extensions,
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
  sCHECK(load_device_table(vk_dev, self->vk_table, vk_table));

  load_vma_table(self->vk_table, vk_table, vma_table);

  defer vk_dev_del{[&] {
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

  defer vma_allocator_del{[&] {
    if (vma_allocator != nullptr)
    {
      vmaDestroyAllocator(vma_allocator);
    }
  }};

  Device *dev = self->allocator.allocate_typed<Device>(1);

  if (dev == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (dev) Device{.allocator     = allocator,
                   .logger        = self->logger,
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
                                      .pool_size = gfx::MAX_BINDING_DESCRIPTORS,
                                      .scratch   = nullptr,
                                      .num_pools = 0,
                                      .scratch_size = 0}};

  defer device_del{[&] {
    if (dev != nullptr)
    {
      destroy_device((gfx::Instance) self, (gfx::Device) dev);
    }
  }};

  Status status = create_frame_context(dev, &dev->frame_ctx, buffering);

  if (status != Status::Success)
  {
    return Err{status};
  }

  vma_allocator = nullptr;
  vk_dev        = nullptr;
  dev           = nullptr;

  return Ok{gfx::DeviceImpl{.self      = (gfx::Device) dev,
                            .interface = &device_interface}};
}

gfx::Backend InstanceInterface::get_backend(gfx::Instance)
{
  return gfx::Backend::Vulkan;
}

void InstanceInterface::destroy_device(gfx::Instance instance_,
                                       gfx::Device   device_)
{
  Instance *const instance = (Instance *) instance_;
  Device *const   dev      = (Device *) device_;

  if (dev == nullptr)
  {
    return;
  }

  destroy_frame_context(dev, &dev->frame_ctx);
  destroy_descriptor_heap(dev, &dev->descriptor_heap);
  vmaDestroyAllocator(dev->vma_allocator);
  dev->vk_table.DestroyDevice(dev->vk_dev, nullptr);
  instance->allocator.deallocate_typed(dev, 1);
}

void InstanceInterface::destroy_surface(gfx::Instance self_,
                                        gfx::Surface  surface)
{
  Instance *const self = (Instance *) self_;
  self->vk_table.DestroySurfaceKHR(self->vk_instance, (Surface) surface,
                                   nullptr);
}

gfx::DeviceProperties DeviceInterface::get_device_properties(gfx::Device self_)
{
  Device *const                     self          = (Device *) self_;
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

  return gfx::DeviceProperties{
      .api_version    = vk_properties.apiVersion,
      .driver_version = vk_properties.driverVersion,
      .vendor_id      = vk_properties.vendorID,
      .device_id      = vk_properties.deviceID,
      .device_name =
          Span{vk_properties.deviceName, strlen(vk_properties.deviceName)},
      .type               = (gfx::DeviceType) vk_properties.deviceType,
      .has_unified_memory = has_uma,
      .texel_buffer_offset_alignment =
          vk_properties.limits.minTexelBufferOffsetAlignment,
      .uniform_buffer_offset_alignment =
          vk_properties.limits.minUniformBufferOffsetAlignment,
      .storage_buffer_offset_alignment =
          vk_properties.limits.minStorageBufferOffsetAlignment,
      .timestamp_period = vk_properties.limits.timestampPeriod};
}

Result<gfx::FormatProperties, Status>
    DeviceInterface::get_format_properties(gfx::Device self_,
                                           gfx::Format format)
{
  Device *const      self = (Device *) self_;
  VkFormatProperties props;
  self->instance->vk_table.GetPhysicalDeviceFormatProperties(
      self->phy_dev.vk_phy_dev, (VkFormat) format, &props);
  return Ok(gfx::FormatProperties{
      .linear_tiling_features =
          (gfx::FormatFeatures) props.linearTilingFeatures,
      .optimal_tiling_features =
          (gfx::FormatFeatures) props.optimalTilingFeatures,
      .buffer_features = (gfx::FormatFeatures) props.bufferFeatures});
}

Result<gfx::Buffer, Status>
    DeviceInterface::create_buffer(gfx::Device            self_,
                                   gfx::BufferDesc const &desc)
{
  Device *const self = (Device *) self_;

  sVALIDATE(desc.size != 0);
  sVALIDATE(desc.usage != gfx::BufferUsage::None);

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
  VmaAllocationInfo vma_allocation_info;
  VmaAllocation     vma_allocation;
  VkBuffer          vk_buffer;
  VkResult          result =
      vmaCreateBuffer(self->vma_allocator, &create_info, &alloc_create_info,
                      &vk_buffer, &vma_allocation, &vma_allocation_info);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_buffer, VK_OBJECT_TYPE_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);

  Buffer *buffer = self->allocator.allocate_typed<Buffer>(1);
  if (buffer == nullptr)
  {
    vmaDestroyBuffer(self->vma_allocator, vk_buffer, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  new (buffer) Buffer{.desc                = desc,
                      .vk_buffer           = vk_buffer,
                      .vma_allocation      = vma_allocation,
                      .vma_allocation_info = vma_allocation_info,
                      .host_map            = vma_allocation_info.pMappedData};

  return Ok{(gfx::Buffer) buffer};
}

Result<gfx::BufferView, Status>
    DeviceInterface::create_buffer_view(gfx::Device                self_,
                                        gfx::BufferViewDesc const &desc)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) desc.buffer;

  sVALIDATE(buffer != nullptr);
  sVALIDATE(has_any_bit(buffer->desc.usage,
                        gfx::BufferUsage::UniformTexelBuffer |
                            gfx::BufferUsage::StorageTexelBuffer));
  sVALIDATE(desc.format != gfx::Format::Undefined);
  sVALIDATE(is_valid_buffer_access(buffer->desc.size, desc.offset, desc.size));

  u64 const view_size = desc.size == gfx::WHOLE_SIZE ?
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

  BufferView *view = self->allocator.allocate_typed<BufferView>(1);

  if (view == nullptr)
  {
    self->vk_table.DestroyBufferView(self->vk_dev, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) BufferView{.desc = desc, .vk_view = vk_view};

  view->desc.size = view_size;

  return Ok{(gfx::BufferView) view};
}

Result<gfx::Image, Status>
    DeviceInterface::create_image(gfx::Device self_, gfx::ImageDesc const &desc)
{
  Device *const self = (Device *) self_;

  sVALIDATE(desc.format != gfx::Format::Undefined);
  sVALIDATE(desc.usage != gfx::ImageUsage::None);
  sVALIDATE(desc.aspects != gfx::ImageAspects::None);
  sVALIDATE(desc.sample_count != gfx::SampleCount::None);
  sVALIDATE(desc.extent.x != 0);
  sVALIDATE(desc.extent.x <= gfx::MAX_IMAGE_EXTENT);
  sVALIDATE(desc.extent.y != 0);
  sVALIDATE(desc.extent.y <= gfx::MAX_IMAGE_EXTENT);
  sVALIDATE(desc.extent.z != 0);
  sVALIDATE(desc.extent.z <= gfx::MAX_IMAGE_EXTENT);
  sVALIDATE(desc.mip_levels > 0);
  sVALIDATE(desc.mip_levels <= num_mip_levels(desc.extent));
  sVALIDATE(desc.array_layers > 0);
  sVALIDATE(desc.array_layers <= gfx::MAX_IMAGE_ARRAY_LAYERS);
  sVALIDATE(!(desc.type == gfx::ImageType::Type2D && desc.extent.z != 1));
  sVALIDATE(!(desc.type == gfx::ImageType::Type3D && desc.array_layers != 1));

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
                                .samples     = VK_SAMPLE_COUNT_1_BIT,
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

  Image *image = self->allocator.allocate_typed<Image>(1);

  if (image == nullptr)
  {
    vmaDestroyImage(self->vma_allocator, vk_image, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  new (image) Image{.desc                = desc,
                    .is_swapchain_image  = false,
                    .vk_image            = vk_image,
                    .vma_allocation      = vma_allocation,
                    .vma_allocation_info = vma_allocation_info,
                    .state               = {}};

  return Ok{(gfx::Image) image};
}

Result<gfx::ImageView, Status>
    DeviceInterface::create_image_view(gfx::Device               self_,
                                       gfx::ImageViewDesc const &desc)
{
  Device *const self      = (Device *) self_;
  Image *const  src_image = (Image *) desc.image;

  sVALIDATE(desc.image != nullptr);
  sVALIDATE(desc.view_format != gfx::Format::Undefined);
  sVALIDATE(
      is_image_view_type_compatible(src_image->desc.type, desc.view_type));
  sVALIDATE(is_valid_image_access(
      src_image->desc.aspects, src_image->desc.mip_levels,
      src_image->desc.array_layers, desc.aspects, desc.first_mip_level,
      desc.num_mip_levels, desc.first_array_layer, desc.num_array_layers));

  u32 const mip_levels =
      desc.num_mip_levels == gfx::REMAINING_MIP_LEVELS ?
          (src_image->desc.mip_levels - desc.first_mip_level) :
          desc.num_mip_levels;
  u32 const array_layers =
      desc.num_array_layers == gfx::REMAINING_ARRAY_LAYERS ?
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

  ImageView *view = self->allocator.allocate_typed<ImageView>(1);
  if (view == nullptr)
  {
    self->vk_table.DestroyImageView(self->vk_dev, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) ImageView{.desc = desc, .vk_view = vk_view};

  view->desc.num_mip_levels   = mip_levels;
  view->desc.num_array_layers = array_layers;

  return Ok{(gfx::ImageView) view};
}

Result<gfx::Sampler, Status>
    DeviceInterface::create_sampler(gfx::Device             self_,
                                    gfx::SamplerDesc const &desc)
{
  Device *const self = (Device *) self_;
  sVALIDATE(!(desc.anisotropy_enable &&
              (desc.max_anisotropy > gfx::MAX_SAMPLER_ANISOTROPY)));
  sVALIDATE(!(desc.anisotropy_enable && (desc.max_anisotropy < 1.0)));

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

  return Ok{(gfx::Sampler) vk_sampler};
}

Result<gfx::Shader, Status>
    DeviceInterface::create_shader(gfx::Device            self_,
                                   gfx::ShaderDesc const &desc)
{
  Device *const self = (Device *) self_;

  sVALIDATE(desc.spirv_code.size_bytes() > 0);

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

  return Ok{(gfx::Shader) vk_shader};
}

Result<gfx::RenderPass, Status>
    DeviceInterface::create_render_pass(gfx::Device                self_,
                                        gfx::RenderPassDesc const &desc)
{
  Device *const self = (Device *) self_;

  sVALIDATE(desc.color_attachments.size() <=
            gfx::MAX_PIPELINE_COLOR_ATTACHMENTS);
  sVALIDATE(desc.input_attachments.size() <=
            gfx::MAX_PIPELINE_INPUT_ATTACHMENTS);

  // render_pass attachment descriptions are packed in the following order:
  // [color_attachments..., depth_stencil_attachment, input_attachments...]
  VkAttachmentDescription vk_attachments[gfx::MAX_PIPELINE_COLOR_ATTACHMENTS +
                                         1 +
                                         gfx::MAX_PIPELINE_INPUT_ATTACHMENTS];
  VkAttachmentReference
      vk_color_attachments[gfx::MAX_PIPELINE_COLOR_ATTACHMENTS];
  VkAttachmentReference vk_depth_stencil_attachment;
  VkAttachmentReference
             vk_input_attachments[gfx::MAX_PIPELINE_INPUT_ATTACHMENTS];
  u32 const  num_color_attachments = (u32) desc.color_attachments.size();
  bool const has_depth_stencil_attachment =
      desc.depth_stencil_attachment.format != gfx::Format::Undefined;
  u32 const num_input_attachments = (u32) desc.input_attachments.size();
  u32 const num_attachments       = num_color_attachments +
                              (has_depth_stencil_attachment ? 1U : 0U) +
                              num_input_attachments;

  u32 iattachment = 0;
  for (u32 icolor_attachment = 0; icolor_attachment < num_color_attachments;
       icolor_attachment++, iattachment++)
  {
    gfx::RenderPassAttachment const &attachment =
        desc.color_attachments[icolor_attachment];
    vk_attachments[iattachment] = VkAttachmentDescription{
        .flags          = 0,
        .format         = (VkFormat) attachment.format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = (VkAttachmentLoadOp) attachment.load_op,
        .storeOp        = (VkAttachmentStoreOp) attachment.store_op,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    vk_color_attachments[icolor_attachment] = VkAttachmentReference{
        .attachment = iattachment,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  }

  if (has_depth_stencil_attachment)
  {
    VkImageLayout layout =
        has_write_access(depth_stencil_attachment_image_access(
            desc.depth_stencil_attachment)) ?
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    vk_attachments[iattachment] = VkAttachmentDescription{
        .flags   = 0,
        .format  = (VkFormat) desc.depth_stencil_attachment.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp  = (VkAttachmentLoadOp) desc.depth_stencil_attachment.load_op,
        .storeOp = (VkAttachmentStoreOp) desc.depth_stencil_attachment.store_op,
        .stencilLoadOp =
            (VkAttachmentLoadOp) desc.depth_stencil_attachment.stencil_load_op,
        .stencilStoreOp = (VkAttachmentStoreOp)
                              desc.depth_stencil_attachment.stencil_store_op,
        .initialLayout = layout,
        .finalLayout   = layout};

    vk_depth_stencil_attachment = VkAttachmentReference{
        .attachment = iattachment,
        .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    iattachment++;
  }

  for (u32 iinput_attachment = 0; iinput_attachment < num_input_attachments;
       iinput_attachment++, iattachment++)
  {
    gfx::RenderPassAttachment const &attachment =
        desc.input_attachments[iinput_attachment];
    vk_attachments[iattachment] = VkAttachmentDescription{
        .flags          = 0,
        .format         = (VkFormat) attachment.format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = (VkAttachmentLoadOp) attachment.load_op,
        .storeOp        = (VkAttachmentStoreOp) attachment.store_op,
        .stencilLoadOp  = (VkAttachmentLoadOp) attachment.stencil_load_op,
        .stencilStoreOp = (VkAttachmentStoreOp) attachment.stencil_store_op,
        .initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    vk_input_attachments[iinput_attachment] = VkAttachmentReference{
        .attachment = iattachment,
        .layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }

  VkSubpassDescription vk_subpass{
      .flags                = 0,
      .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = num_input_attachments,
      .pInputAttachments    = vk_input_attachments,
      .colorAttachmentCount = num_color_attachments,
      .pColorAttachments    = vk_color_attachments,
      .pResolveAttachments  = nullptr,
      .pDepthStencilAttachment =
          has_depth_stencil_attachment ? &vk_depth_stencil_attachment : nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments    = nullptr};

  VkRenderPassCreateInfo create_info{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0,
      .attachmentCount = num_attachments,
      .pAttachments    = vk_attachments,
      .subpassCount    = 1,
      .pSubpasses      = &vk_subpass,
      .dependencyCount = 0,
      .pDependencies   = nullptr};
  VkRenderPass vk_render_pass;

  VkResult result = self->vk_table.CreateRenderPass(self->vk_dev, &create_info,
                                                    nullptr, &vk_render_pass);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_render_pass,
                    VK_OBJECT_TYPE_RENDER_PASS,
                    VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT);

  RenderPass *render_pass = self->allocator.allocate_typed<RenderPass>(1);
  if (render_pass == nullptr)
  {
    self->vk_table.DestroyRenderPass(self->vk_dev, vk_render_pass, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (render_pass)
      RenderPass{.color_attachments        = {},
                 .input_attachments        = {},
                 .depth_stencil_attachment = desc.depth_stencil_attachment,
                 .num_color_attachments    = num_color_attachments,
                 .num_input_attachments    = num_input_attachments,
                 .vk_render_pass           = vk_render_pass};

  mem::copy(desc.color_attachments, render_pass->color_attachments);
  mem::copy(desc.input_attachments, render_pass->input_attachments);

  return Ok{(gfx::RenderPass) render_pass};
}

Result<gfx::Framebuffer, Status>
    DeviceInterface::create_framebuffer(gfx::Device                 self_,
                                        gfx::FramebufferDesc const &desc)
{
  Device *const     self                  = (Device *) self_;
  RenderPass *const render_pass           = (RenderPass *) desc.render_pass;
  u32 const         num_color_attachments = (u32) desc.color_attachments.size();
  bool const        has_depth_stencil_attachment =
      desc.depth_stencil_attachment != nullptr;
  u32 const num_attachments =
      num_color_attachments + (has_depth_stencil_attachment ? 1U : 0U);
  VkImageView vk_attachments[gfx::MAX_PIPELINE_COLOR_ATTACHMENTS + 1];

  sVALIDATE(desc.extent.x > 0);
  sVALIDATE(desc.extent.x <= gfx::MAX_FRAMEBUFFER_EXTENT);
  sVALIDATE(desc.extent.y > 0);
  sVALIDATE(desc.extent.y <= gfx::MAX_FRAMEBUFFER_EXTENT);
  sVALIDATE(desc.layers > 0);
  sVALIDATE(desc.layers <= gfx::MAX_FRAMEBUFFER_LAYERS);
  sVALIDATE((desc.color_attachments.size() > 0) ||
            (desc.depth_stencil_attachment != nullptr));

  for (gfx::ImageView attachment : desc.color_attachments)
  {
    ImageView *const view  = (ImageView *) attachment;
    Image *const     image = IMAGE_FROM_VIEW(attachment);
    gfx::Extent3D    extent =
        mip_down(image->desc.extent, view->desc.first_mip_level);
    sVALIDATE(has_bits(image->desc.usage, gfx::ImageUsage::ColorAttachment));
    sVALIDATE(has_bits(view->desc.aspects, gfx::ImageAspects::Color));
    sVALIDATE(view->desc.num_array_layers >= desc.layers);
    sVALIDATE(extent.x >= desc.extent.x);
    sVALIDATE(extent.y >= desc.extent.y);
  }

  if (desc.depth_stencil_attachment != nullptr)
  {
    ImageView *const view  = (ImageView *) desc.depth_stencil_attachment;
    Image *const     image = IMAGE_FROM_VIEW(view);
    gfx::Extent3D    extent =
        mip_down(image->desc.extent, view->desc.first_mip_level);
    sVALIDATE(
        has_bits(image->desc.usage, gfx::ImageUsage::DepthStencilAttachment));
    sVALIDATE(has_any_bit(view->desc.aspects, gfx::ImageAspects::Depth |
                                                  gfx::ImageAspects::Stencil));
    sVALIDATE(view->desc.num_array_layers >= desc.layers);
    sVALIDATE(extent.x >= desc.extent.x);
    sVALIDATE(extent.y >= desc.extent.y);
  }

  sVALIDATE(is_render_pass_compatible(
      *render_pass,
      Span{(ImageView *const *) desc.color_attachments.data(),
           desc.color_attachments.size()},
      (ImageView *) desc.depth_stencil_attachment));

  u32 ivk_attachment = 0;
  for (u32 icolor_attachment = 0; icolor_attachment < num_color_attachments;
       icolor_attachment++, ivk_attachment++)
  {
    vk_attachments[ivk_attachment] =
        ((ImageView *) desc.color_attachments[icolor_attachment])->vk_view;
  }

  if (has_depth_stencil_attachment)
  {
    vk_attachments[ivk_attachment] =
        ((ImageView *) desc.depth_stencil_attachment)->vk_view;
  }

  VkFramebufferCreateInfo create_info{
      .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0,
      .renderPass      = render_pass->vk_render_pass,
      .attachmentCount = num_attachments,
      .pAttachments    = vk_attachments,
      .width           = desc.extent.x,
      .height          = desc.extent.y,
      .layers          = desc.layers};

  VkFramebuffer vk_framebuffer;

  VkResult result = self->vk_table.CreateFramebuffer(self->vk_dev, &create_info,
                                                     nullptr, &vk_framebuffer);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_framebuffer,
                    VK_OBJECT_TYPE_FRAMEBUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT);

  Framebuffer *framebuffer = self->allocator.allocate_typed<Framebuffer>(1);
  if (framebuffer == nullptr)
  {
    self->vk_table.DestroyFramebuffer(self->vk_dev, vk_framebuffer, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (framebuffer) Framebuffer{.color_attachments = {},
                                .depth_stencil_attachment =
                                    (ImageView *) desc.depth_stencil_attachment,
                                .num_color_attachments = num_color_attachments,
                                .extent                = desc.extent,
                                .layers                = desc.layers,
                                .vk_framebuffer        = vk_framebuffer};

  mem::copy(desc.color_attachments,
            (gfx::ImageView *) framebuffer->color_attachments);

  return Ok{(gfx::Framebuffer) framebuffer};
}

Result<gfx::DescriptorSetLayout, Status>
    DeviceInterface::create_descriptor_set_layout(
        gfx::Device self_, gfx::DescriptorSetLayoutDesc const &desc)
{
  Device *const self                         = (Device *) self_;
  u32 const     num_bindings                 = (u32) desc.bindings.size();
  u32           num_descriptors              = 0;
  u32           num_variable_length          = 0;
  u32           sizing[NUM_DESCRIPTOR_TYPES] = {};

  for (gfx::DescriptorBindingDesc const &desc : desc.bindings)
  {
    num_descriptors += desc.count;
    sizing[(u32) desc.type] += desc.count;
    num_variable_length += desc.is_variable_length ? 0 : 1;
  }

  u32 num_dynamic_storage_buffers =
      sizing[(u32) gfx::DescriptorType::DynamicStorageBuffer];
  u32 num_dynamic_uniform_buffers =
      sizing[(u32) gfx::DescriptorType::DynamicUniformBuffer];

  sVALIDATE(num_bindings > 0);
  sVALIDATE(num_bindings <= gfx::MAX_DESCRIPTOR_SET_BINDINGS);
  sVALIDATE(num_dynamic_storage_buffers <=
            gfx::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS);
  sVALIDATE(num_dynamic_uniform_buffers <=
            gfx::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS);
  sVALIDATE(num_descriptors <= gfx::MAX_DESCRIPTOR_SET_DESCRIPTORS);
  sVALIDATE(num_variable_length <= 1);

  for (u32 i = 0; i < num_bindings; i++)
  {
    sVALIDATE(desc.bindings[i].count > 0);
    sVALIDATE(desc.bindings[i].count <= gfx::MAX_BINDING_DESCRIPTORS);
    sVALIDATE(!(desc.bindings[i].is_variable_length &&
                (i != (desc.bindings.size() - 1))));
  }

  VkDescriptorSetLayoutBinding vk_bindings[gfx::MAX_DESCRIPTOR_SET_BINDINGS];
  VkDescriptorBindingFlagsEXT
      vk_binding_flags[gfx::MAX_DESCRIPTOR_SET_BINDINGS];

  for (u32 i = 0; i < num_bindings; i++)
  {
    gfx::DescriptorBindingDesc const &binding = desc.bindings[i];
    VkShaderStageFlags                stage_flags =
        (VkShaderStageFlags) (binding.type ==
                                      gfx::DescriptorType::InputAttachment ?
                                  VK_SHADER_STAGE_FRAGMENT_BIT :
                                  VK_SHADER_STAGE_ALL);
    vk_bindings[i] = VkDescriptorSetLayoutBinding{
        .binding            = i,
        .descriptorType     = (VkDescriptorType) binding.type,
        .descriptorCount    = binding.count,
        .stageFlags         = stage_flags,
        .pImmutableSamplers = nullptr};
    VkDescriptorBindingFlagsEXT vk_flags =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    if (binding.is_variable_length)
    {
      vk_flags |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
    }
    vk_binding_flags[i] = vk_flags;
  }

  VkDescriptorSetLayoutBindingFlagsCreateInfoEXT vk_binding_flags_create_info{
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
      .pNext         = nullptr,
      .bindingCount  = (u32) desc.bindings.size(),
      .pBindingFlags = vk_binding_flags};

  VkDescriptorSetLayoutCreateInfo create_info{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext        = &vk_binding_flags_create_info,
      .flags        = 0,
      .bindingCount = num_bindings,
      .pBindings    = vk_bindings};

  VkDescriptorSetLayout vk_layout;
  VkResult              result = self->vk_table.CreateDescriptorSetLayout(
      self->vk_dev, &create_info, nullptr, &vk_layout);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_layout_del{[&] {
    if (vk_layout != nullptr)
    {
      self->vk_table.DestroyDescriptorSetLayout(self->vk_dev, vk_layout,
                                                nullptr);
    }
  }};

  set_resource_name(self, desc.label, vk_layout,
                    VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);

  DescriptorSetLayout *layout =
      self->allocator.allocate_typed<DescriptorSetLayout>(1);
  if (layout == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (layout) DescriptorSetLayout{.vk_layout           = vk_layout,
                                   .num_bindings        = num_bindings,
                                   .num_variable_length = num_variable_length};

  mem::copy(desc.bindings, layout->bindings);
  mem::copy(sizing, layout->sizing, NUM_DESCRIPTOR_TYPES);
  vk_layout = nullptr;

  return Ok{(gfx::DescriptorSetLayout) layout};
}

Result<gfx::DescriptorSet, Status>
    DeviceInterface::create_descriptor_set(gfx::Device              self_,
                                           gfx::DescriptorSetLayout layout_,
                                           Span<u32 const> variable_lengths)
{
  Device *const              self   = (Device *) self_;
  DescriptorSetLayout *const layout = (DescriptorSetLayout *) layout_;
  DescriptorHeap            &heap   = self->descriptor_heap;
  sVALIDATE(variable_lengths.size() == layout->num_variable_length);

  {
    u32 vla_idx = 0;
    for (u32 i = 0; i < layout->num_bindings; i++)
    {
      if (layout->bindings[i].is_variable_length)
      {
        sVALIDATE(variable_lengths[vla_idx] <= layout->bindings[i].count);
        vla_idx++;
      }
    }
  }

  u32 descriptor_usage[NUM_DESCRIPTOR_TYPES]                = {};
  u32 resource_sync_count[gfx::MAX_DESCRIPTOR_SET_BINDINGS] = {};

  {
    u32 vla_idx = 0;
    for (u32 i = 0; i < layout->num_bindings; i++)
    {
      gfx::DescriptorBindingDesc const &desc  = layout->bindings[i];
      u32                               count = 0;
      if (!desc.is_variable_length)
      {
        count += desc.count;
      }
      else
      {
        count += variable_lengths[vla_idx];
        vla_idx++;
      }

      switch (desc.type)
      {
        case gfx::DescriptorType::CombinedImageSampler:
        case gfx::DescriptorType::SampledImage:
        case gfx::DescriptorType::StorageImage:
        case gfx::DescriptorType::UniformTexelBuffer:
        case gfx::DescriptorType::StorageTexelBuffer:
        case gfx::DescriptorType::UniformBuffer:
        case gfx::DescriptorType::StorageBuffer:
        case gfx::DescriptorType::DynamicUniformBuffer:
        case gfx::DescriptorType::DynamicStorageBuffer:
        case gfx::DescriptorType::InputAttachment:
          resource_sync_count[i] = count;
          break;
        default:
          break;
      }

      descriptor_usage[(u32) desc.type] += count;
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

    defer vk_pool_del{[&] {
      self->vk_table.DestroyDescriptorPool(self->vk_dev, vk_pool, nullptr);
    }};

    DescriptorPool *pools = heap.allocator.reallocate_typed(
        heap.pools, heap.num_pools, heap.num_pools + 1);
    if (pools == nullptr)
    {
      return Err{Status::OutOfHostMemory};
    }

    DescriptorPool *pool = pools + heap.num_pools;

    fill(pool->avail, heap.pool_size);
    pool->vk_pool = vk_pool;

    heap.num_pools++;
    heap.pools = pools;
    vk_pool    = nullptr;
  }

  DescriptorBinding bindings[gfx::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  u32               num_bindings                               = 0;

  defer bindings_del{[&] {
    for (u32 i = num_bindings; i-- > 0;)
    {
      heap.allocator.deallocate_typed(bindings[i].resources, bindings[i].count);
    }
  }};

  for (; num_bindings < layout->num_bindings; num_bindings++)
  {
    gfx::DescriptorBindingDesc const &desc  = layout->bindings[num_bindings];
    void                            **rsrcs = nullptr;
    u32                               count = resource_sync_count[num_bindings];
    rsrcs = heap.allocator.allocate_zeroed_typed<void *>(count);
    if (rsrcs == nullptr)
    {
      return Err{Status::OutOfHostMemory};
    }
    bindings[num_bindings] = DescriptorBinding{
        .resources = rsrcs, .count = count, .type = desc.type};
  }

  VkDescriptorSetVariableDescriptorCountAllocateInfoEXT var_alloc_info{
      .sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
      .pNext              = nullptr,
      .descriptorSetCount = (u32) variable_lengths.size(),
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
  sCHECK(result != VK_ERROR_OUT_OF_POOL_MEMORY &&
         result != VK_ERROR_FRAGMENTED_POOL);

  for (u32 i = 0; i < NUM_DESCRIPTOR_TYPES; i++)
  {
    heap.pools[ipool].avail[i] -= descriptor_usage[i];
  }

  DescriptorSet *set = heap.allocator.allocate_typed<DescriptorSet>(1);

  if (set == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (set) DescriptorSet{
      .vk_set = vk_set, .num_bindings = num_bindings, .pool = ipool};

  mem::copy(bindings, set->bindings, num_bindings);
  num_bindings = 0;

  return Ok{(gfx::DescriptorSet) set};
}

Result<gfx::PipelineCache, Status>
    DeviceInterface::create_pipeline_cache(gfx::Device                   self_,
                                           gfx::PipelineCacheDesc const &desc)
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

  return Ok{(gfx::PipelineCache) vk_cache};
}

Result<gfx::ComputePipeline, Status> DeviceInterface::create_compute_pipeline(
    gfx::Device self_, gfx::ComputePipelineDesc const &desc)
{
  Device *const self                = (Device *) self_;
  u32 const     num_descriptor_sets = (u32) desc.descriptor_set_layouts.size();

  sVALIDATE(num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  sVALIDATE(desc.push_constants_size <= gfx::MAX_PUSH_CONSTANTS_SIZE);
  sVALIDATE(mem::is_aligned(4, desc.push_constants_size));
  sVALIDATE(desc.compute_shader.entry_point.size() > 0 &&
            desc.compute_shader.entry_point.size() < 256);
  sVALIDATE(desc.compute_shader.shader != nullptr);

  VkDescriptorSetLayout
      vk_descriptor_set_layouts[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
  for (u32 i = 0; i < num_descriptor_sets; i++)
  {
    vk_descriptor_set_layouts[i] =
        ((DescriptorSetLayout *) desc.descriptor_set_layouts[i])->vk_layout;
  }

  VkSpecializationInfo vk_specialization{
      .mapEntryCount =
          (u32) desc.compute_shader.specialization_constants.size(),
      .pMapEntries = (VkSpecializationMapEntry const *)
                         desc.compute_shader.specialization_constants.data(),
      .dataSize =
          desc.compute_shader.specialization_constants_data.size_bytes(),
      .pData = desc.compute_shader.specialization_constants_data.data()};

  char entry_point[256];
  to_c_str(desc.compute_shader.entry_point, to_span(entry_point));

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

  ComputePipeline *pipeline =
      self->allocator.allocate_typed<ComputePipeline>(1);
  if (pipeline == nullptr)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_dev, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_dev, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
      ComputePipeline{.vk_pipeline         = vk_pipeline,
                      .vk_layout           = vk_layout,
                      .push_constants_size = desc.push_constants_size};

  return Ok{(gfx::ComputePipeline) pipeline};
}

Result<gfx::GraphicsPipeline, Status> DeviceInterface::create_graphics_pipeline(
    gfx::Device self_, gfx::GraphicsPipelineDesc const &desc)
{
  Device *const self                        = (Device *) self_;
  constexpr u32 NUM_PIPELINE_DYNAMIC_STATES = 6U;
  u32 const     num_descriptor_sets = (u32) desc.descriptor_set_layouts.size();
  u32 const     num_input_bindings  = (u32) desc.vertex_input_bindings.size();
  u32 const     num_attributes      = (u32) desc.vertex_attributes.size();
  u32 const     num_color_attachments =
      (u32) desc.color_blend_state.attachments.size();

  sVALIDATE(num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  sVALIDATE(desc.push_constants_size <= gfx::MAX_PUSH_CONSTANTS_SIZE);
  sVALIDATE(mem::is_aligned(4, desc.push_constants_size));
  sVALIDATE(desc.vertex_shader.entry_point.size() > 0 &&
            desc.vertex_shader.entry_point.size() <= 255);
  sVALIDATE(desc.fragment_shader.entry_point.size() > 0 &&
            desc.fragment_shader.entry_point.size() <= 255);
  sVALIDATE(num_attributes <= gfx::MAX_VERTEX_ATTRIBUTES);

  VkDescriptorSetLayout
      vk_descriptor_set_layouts[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
  for (u32 i = 0; i < num_descriptor_sets; i++)
  {
    vk_descriptor_set_layouts[i] =
        ((DescriptorSetLayout *) desc.descriptor_set_layouts[i])->vk_layout;
  }

  VkSpecializationInfo vk_vs_specialization{
      .mapEntryCount = (u32) desc.vertex_shader.specialization_constants.size(),
      .pMapEntries   = (VkSpecializationMapEntry const *)
                         desc.vertex_shader.specialization_constants.data(),
      .dataSize = desc.vertex_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.vertex_shader.specialization_constants_data.data()};

  VkSpecializationInfo vk_fs_specialization{
      .mapEntryCount =
          (u32) desc.fragment_shader.specialization_constants.size(),
      .pMapEntries = (VkSpecializationMapEntry const *)
                         desc.fragment_shader.specialization_constants.data(),
      .dataSize =
          desc.fragment_shader.specialization_constants_data.size_bytes(),
      .pData = desc.fragment_shader.specialization_constants_data.data()};

  char vs_entry_point[256];
  char fs_entry_point[256];
  to_c_str(desc.vertex_shader.entry_point, to_span(vs_entry_point));
  to_c_str(desc.fragment_shader.entry_point, to_span(fs_entry_point));

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

  VkVertexInputBindingDescription input_bindings[gfx::MAX_VERTEX_ATTRIBUTES];
  for (u32 iinput_bindings = 0; iinput_bindings < num_input_bindings;
       iinput_bindings++)
  {
    gfx::VertexInputBinding const &binding =
        desc.vertex_input_bindings[iinput_bindings];
    input_bindings[iinput_bindings] = VkVertexInputBindingDescription{
        .binding   = binding.binding,
        .stride    = binding.stride,
        .inputRate = (VkVertexInputRate) binding.input_rate,
    };
  }

  VkVertexInputAttributeDescription attributes[gfx::MAX_VERTEX_ATTRIBUTES];
  for (u32 iattribute = 0; iattribute < num_attributes; iattribute++)
  {
    gfx::VertexAttribute const &attribute = desc.vertex_attributes[iattribute];
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
      attachment_states[gfx::MAX_PIPELINE_COLOR_ATTACHMENTS];

  for (u32 icolor_attachment = 0; icolor_attachment < num_color_attachments;
       icolor_attachment++)
  {
    gfx::PipelineColorBlendAttachmentState const &state =
        desc.color_blend_state.attachments[icolor_attachment];
    attachment_states[icolor_attachment] = VkPipelineColorBlendAttachmentState{
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
      .attachmentCount = num_color_attachments,
      .pAttachments    = attachment_states,
      .blendConstants  = {desc.color_blend_state.blend_constant.x,
                          desc.color_blend_state.blend_constant.y,
                          desc.color_blend_state.blend_constant.z,
                          desc.color_blend_state.blend_constant.w}};

  VkDynamicState dynamic_states[NUM_PIPELINE_DYNAMIC_STATES] = {
      VK_DYNAMIC_STATE_VIEWPORT,          VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_BLEND_CONSTANTS,   VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
      VK_DYNAMIC_STATE_STENCIL_REFERENCE, VK_DYNAMIC_STATE_STENCIL_WRITE_MASK};

  VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext             = nullptr,
      .flags             = 0,
      .dynamicStateCount = NUM_PIPELINE_DYNAMIC_STATES,
      .pDynamicStates    = dynamic_states};

  VkGraphicsPipelineCreateInfo create_info{
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext               = nullptr,
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
      .renderPass          = ((RenderPass *) desc.render_pass)->vk_render_pass,
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

  GraphicsPipeline *pipeline =
      self->allocator.allocate_typed<GraphicsPipeline>(1);
  if (pipeline == nullptr)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_dev, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_dev, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
      GraphicsPipeline{.vk_pipeline         = vk_pipeline,
                       .vk_layout           = vk_layout,
                       .push_constants_size = desc.push_constants_size};

  return Ok{(gfx::GraphicsPipeline) pipeline};
}

/// old swapchain will be retired and destroyed irregardless of whether new
/// swapchain recreation fails.
inline VkResult recreate_swapchain(Device *self, Swapchain *swapchain)
{
  sVALIDATE(swapchain->desc.preferred_extent.x > 0);
  sVALIDATE(swapchain->desc.preferred_extent.y > 0);
  sVALIDATE(swapchain->desc.preferred_buffering <= gfx::MAX_SWAPCHAIN_IMAGES);

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

  sVALIDATE(has_bits(surface_capabilities.supportedUsageFlags,
                     (VkImageUsageFlags) swapchain->desc.usage));
  sVALIDATE(has_bits(surface_capabilities.supportedCompositeAlpha,
                     (VkImageUsageFlags) swapchain->desc.composite_alpha));

  // take ownership of internal data for re-use/release
  VkSwapchainKHR old_vk_swapchain = swapchain->vk_swapchain;
  defer          old_vk_swapchain_del{[&] {
    if (old_vk_swapchain != nullptr)
    {
      self->vk_table.DestroySwapchainKHR(self->vk_dev, old_vk_swapchain,
                                                  nullptr);
    }
  }};

  swapchain->is_out_of_date  = true;
  swapchain->is_optimal      = false;
  swapchain->is_zero_sized   = false;
  swapchain->format          = gfx::SurfaceFormat{};
  swapchain->usage           = gfx::ImageUsage::None;
  swapchain->present_mode    = gfx::PresentMode::Immediate;
  swapchain->extent          = gfx::Extent{};
  swapchain->composite_alpha = gfx::CompositeAlpha::None;
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

  sCHECK(result == VK_SUCCESS);

  defer new_vk_swapchain_del{[&] {
    if (new_vk_swapchain != nullptr)
    {
      self->vk_table.DestroySwapchainKHR(self->vk_dev, new_vk_swapchain,
                                         nullptr);
    }
  }};

  u32 num_images;
  result = self->vk_table.GetSwapchainImagesKHR(self->vk_dev, new_vk_swapchain,
                                                &num_images, nullptr);

  sCHECK(result == VK_SUCCESS);

  sCHECK(num_images <= gfx::MAX_SWAPCHAIN_IMAGES);

  result = self->vk_table.GetSwapchainImagesKHR(
      self->vk_dev, new_vk_swapchain, &num_images, swapchain->vk_images);

  sCHECK(result == VK_SUCCESS);

  for (u32 i = 0; i < num_images; i++)
  {
    swapchain->image_impls[i] = Image{
        .desc                = gfx::ImageDesc{.type         = gfx::ImageType::Type2D,
                                              .format       = swapchain->desc.format.format,
                                              .usage        = swapchain->desc.usage,
                                              .aspects      = gfx::ImageAspects::Color,
                                              .extent       = gfx::Extent3D{vk_extent.width,
                                                       vk_extent.height, 1},
                                              .mip_levels   = 1,
                                              .array_layers = 1},
        .is_swapchain_image  = true,
        .vk_image            = swapchain->vk_images[i],
        .vma_allocation      = nullptr,
        .vma_allocation_info = {},
        .state               = {}};
    swapchain->images[i] = (gfx::Image)(swapchain->image_impls + i);
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

Result<gfx::Swapchain, Status>
    DeviceInterface::create_swapchain(gfx::Device self_, gfx::Surface surface,
                                      gfx::SwapchainDesc const &desc)
{
  Device *const self = (Device *) self_;

  sVALIDATE(desc.preferred_extent.x > 0);
  sVALIDATE(desc.preferred_extent.y > 0);

  Swapchain *swapchain = self->allocator.allocate_typed<Swapchain>(1);
  if (swapchain == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (swapchain) Swapchain{.desc            = desc,
                            .is_out_of_date  = true,
                            .is_optimal      = false,
                            .is_zero_sized   = false,
                            .format          = {},
                            .usage           = {},
                            .present_mode    = gfx::PresentMode::Immediate,
                            .extent          = {},
                            .composite_alpha = gfx::CompositeAlpha::None,
                            .image_impls     = {},
                            .images          = {},
                            .vk_images       = {},
                            .num_images      = 0,
                            .current_image   = 0,
                            .vk_swapchain    = nullptr,
                            .vk_surface      = (VkSurfaceKHR) surface};

  return Ok{(gfx::Swapchain) swapchain};
}

Result<gfx::TimeStampQuery, Status>
    DeviceInterface::create_timestamp_query(gfx::Device self_)
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

  return Ok{(gfx::TimeStampQuery) vk_pool};
}

Result<gfx::StatisticsQuery, Status>
    DeviceInterface::create_statistics_query(gfx::Device self_)
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

  return Ok{(gfx::StatisticsQuery) vk_pool};
}

void DeviceInterface::destroy_buffer(gfx::Device self_, gfx::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  if (buffer == nullptr)
  {
    return;
  }

  vmaDestroyBuffer(self->vma_allocator, buffer->vk_buffer,
                   buffer->vma_allocation);
  self->allocator.deallocate_typed(buffer, 1);
}

void DeviceInterface::destroy_buffer_view(gfx::Device     self_,
                                          gfx::BufferView buffer_view_)
{
  Device *const     self        = (Device *) self_;
  BufferView *const buffer_view = (BufferView *) buffer_view_;

  if (buffer_view == nullptr)
  {
    return;
  }

  self->vk_table.DestroyBufferView(self->vk_dev, buffer_view->vk_view, nullptr);
  self->allocator.deallocate_typed(buffer_view, 1);
}

void DeviceInterface::destroy_image(gfx::Device self_, gfx::Image image_)
{
  Device *const self  = (Device *) self_;
  Image *const  image = (Image *) image_;

  if (image == nullptr)
  {
    return;
  }

  sVALIDATE(!image->is_swapchain_image);

  vmaDestroyImage(self->vma_allocator, image->vk_image, image->vma_allocation);
  self->allocator.deallocate_typed(image, 1);
}

void DeviceInterface::destroy_image_view(gfx::Device    self_,
                                         gfx::ImageView image_view_)
{
  Device *const    self       = (Device *) self_;
  ImageView *const image_view = (ImageView *) image_view_;

  if (image_view == nullptr)
  {
    return;
  }

  self->vk_table.DestroyImageView(self->vk_dev, image_view->vk_view, nullptr);
  self->allocator.deallocate_typed(image_view, 1);
}

void DeviceInterface::destroy_sampler(gfx::Device self_, gfx::Sampler sampler_)
{
  Device *const self = (Device *) self_;

  self->vk_table.DestroySampler(self->vk_dev, (Sampler) sampler_, nullptr);
}

void DeviceInterface::destroy_shader(gfx::Device self_, gfx::Shader shader_)
{
  Device *const self = (Device *) self_;

  self->vk_table.DestroyShaderModule(self->vk_dev, (Shader) shader_, nullptr);
}

void DeviceInterface::destroy_render_pass(gfx::Device     self_,
                                          gfx::RenderPass render_pass_)
{
  Device *const     self        = (Device *) self_;
  RenderPass *const render_pass = (RenderPass *) render_pass_;

  if (render_pass == nullptr)
  {
    return;
  }

  self->vk_table.DestroyRenderPass(self->vk_dev, render_pass->vk_render_pass,
                                   nullptr);
  self->allocator.deallocate_typed(render_pass, 1);
}

void DeviceInterface::destroy_framebuffer(gfx::Device      self_,
                                          gfx::Framebuffer framebuffer_)
{
  Device *const      self        = (Device *) self_;
  Framebuffer *const framebuffer = (Framebuffer *) framebuffer_;

  if (framebuffer == nullptr)
  {
    return;
  }

  self->vk_table.DestroyFramebuffer(self->vk_dev, framebuffer->vk_framebuffer,
                                    nullptr);
  self->allocator.deallocate_typed(framebuffer, 1);
}

void DeviceInterface::destroy_descriptor_set_layout(
    gfx::Device self_, gfx::DescriptorSetLayout layout_)
{
  Device *const              self   = (Device *) self_;
  DescriptorSetLayout *const layout = (DescriptorSetLayout *) layout_;

  if (layout == nullptr)
  {
    return;
  }

  self->vk_table.DestroyDescriptorSetLayout(self->vk_dev, layout->vk_layout,
                                            nullptr);
  self->allocator.deallocate_typed(layout->bindings, layout->num_bindings);
  self->allocator.deallocate_typed(layout, 1);
}

void DeviceInterface::destroy_descriptor_set(gfx::Device        self_,
                                             gfx::DescriptorSet set_)
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

  sCHECK(result == VK_SUCCESS);

  for (u32 i = 0; i < set->num_bindings; i++)
  {
    pool->avail[(u32) set->bindings[i].type] += set->bindings[i].count;
  }

  for (u32 i = set->num_bindings; i-- > 0;)
  {
    heap.allocator.deallocate_typed(set->bindings[i].resources,
                                    set->num_bindings);
  }
  heap.allocator.deallocate_typed(set, 1);
}

void DeviceInterface::destroy_pipeline_cache(gfx::Device        self_,
                                             gfx::PipelineCache cache_)
{
  Device *const self = (Device *) self_;
  self->vk_table.DestroyPipelineCache(self->vk_dev, (PipelineCache) cache_,
                                      nullptr);
}

void DeviceInterface::destroy_compute_pipeline(gfx::Device          self_,
                                               gfx::ComputePipeline pipeline_)
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
  self->allocator.deallocate_typed(pipeline, 1);
}

void DeviceInterface::destroy_graphics_pipeline(gfx::Device           self_,
                                                gfx::GraphicsPipeline pipeline_)
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
  self->allocator.deallocate_typed(pipeline, 1);
}

void DeviceInterface::destroy_swapchain(gfx::Device    self_,
                                        gfx::Swapchain swapchain_)
{
  Device *const    self      = (Device *) self_;
  Swapchain *const swapchain = (Swapchain *) swapchain_;

  if (swapchain == nullptr)
  {
    return;
  }

  self->vk_table.DestroySwapchainKHR(self->vk_dev, swapchain->vk_swapchain,
                                     nullptr);
  self->allocator.deallocate_typed(swapchain, 1);
}

void DeviceInterface::destroy_timestamp_query(gfx::Device         self_,
                                              gfx::TimeStampQuery query_)
{
  Device *const     self    = (Device *) self_;
  VkQueryPool const vk_pool = (VkQueryPool) query_;

  self->vk_table.DestroyQueryPool(self->vk_dev, vk_pool, nullptr);
}

void DeviceInterface::destroy_statistics_query(gfx::Device          self_,
                                               gfx::StatisticsQuery query_)
{
  Device *const     self    = (Device *) self_;
  VkQueryPool const vk_pool = (VkQueryPool) query_;

  self->vk_table.DestroyQueryPool(self->vk_dev, vk_pool, nullptr);
}

gfx::FrameContext DeviceInterface::get_frame_context(gfx::Device self_)
{
  FrameContext const &ctx = ((Device *) self_)->frame_ctx;

  return gfx::FrameContext{.buffering  = ctx.buffering,
                           .tail       = ctx.tail_frame,
                           .current    = ctx.current_frame,
                           .encoders   = Span{ctx.encs_impl, ctx.buffering},
                           .ring_index = ctx.ring_index};
}

Result<void *, Status>
    DeviceInterface::get_buffer_memory_map(gfx::Device self_,
                                           gfx::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  sVALIDATE(buffer->desc.host_mapped);

  return Ok{(void *) buffer->host_map};
}

Result<Void, Status> DeviceInterface::invalidate_buffer_memory_map(
    gfx::Device self_, gfx::Buffer buffer_, gfx::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  sVALIDATE(buffer->desc.host_mapped);
  sVALIDATE(range.offset < buffer->desc.size);
  sVALIDATE(range.size == gfx::WHOLE_SIZE ||
            (range.offset + range.size) <= buffer->desc.size);

  VkResult result = vmaInvalidateAllocation(
      self->vma_allocator, buffer->vma_allocation, range.offset, range.size);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::flush_buffer_memory_map(
    gfx::Device self_, gfx::Buffer buffer_, gfx::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  sVALIDATE(buffer->desc.host_mapped);
  sVALIDATE(range.offset < buffer->desc.size);
  sVALIDATE(range.size == gfx::WHOLE_SIZE ||
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
    DeviceInterface::get_pipeline_cache_size(gfx::Device        self_,
                                             gfx::PipelineCache cache)
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
    gfx::Device self_, gfx::PipelineCache cache, Span<u8> out)
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
    DeviceInterface::merge_pipeline_cache(gfx::Device                    self_,
                                          gfx::PipelineCache             dst,
                                          Span<gfx::PipelineCache const> srcs)
{
  Device *const self     = (Device *) self_;
  u32 const     num_srcs = (u32) srcs.size();

  sVALIDATE(num_srcs > 0);

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
    gfx::Device self_, gfx::DescriptorSetUpdate const &update)
{
  Device *const         self = (Device *) self_;
  DescriptorHeap *const heap = &self->descriptor_heap;
  DescriptorSet *const  set  = (DescriptorSet *) update.set;
  u64 const             ubo_offset_alignment =
      self->phy_dev.vk_properties.limits.minUniformBufferOffsetAlignment;
  u64 const ssbo_offset_alignment =
      self->phy_dev.vk_properties.limits.minStorageBufferOffsetAlignment;

  sVALIDATE(update.binding <= set->num_bindings);
  DescriptorBinding &binding = set->bindings[update.binding];
  sVALIDATE(update.element < binding.count);
  usize info_size = 0;
  u32   count     = 0;

  switch (binding.type)
  {
    case gfx::DescriptorType::DynamicStorageBuffer:
    case gfx::DescriptorType::StorageBuffer:
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        gfx::BufferBinding const &b      = update.buffers[i];
        Buffer                   *buffer = (Buffer *) b.buffer;
        if (buffer != nullptr)
        {
          sVALIDATE(
              has_bits(buffer->desc.usage, gfx::BufferUsage::StorageBuffer));
          sVALIDATE(is_valid_buffer_access(buffer->desc.size, b.offset, b.size,
                                           ubo_offset_alignment));
        }
      }
      break;

    case gfx::DescriptorType::DynamicUniformBuffer:
    case gfx::DescriptorType::UniformBuffer:
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        gfx::BufferBinding const &b      = update.buffers[i];
        Buffer                   *buffer = (Buffer *) b.buffer;
        if (buffer != nullptr)
        {
          sVALIDATE(
              has_bits(buffer->desc.usage, gfx::BufferUsage::UniformBuffer));
          sVALIDATE(is_valid_buffer_access(buffer->desc.size, b.offset, b.size,
                                           ssbo_offset_alignment));
        }
      }
      break;

    case gfx::DescriptorType::Sampler:
      break;

    case gfx::DescriptorType::SampledImage:
    case gfx::DescriptorType::CombinedImageSampler:
    case gfx::DescriptorType::InputAttachment:
    {
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        if (view != nullptr)
        {
          Image *image = (Image *) view->desc.image;
          sVALIDATE(has_bits(image->desc.usage, gfx::ImageUsage::Sampled));
        }
      }
    }
    break;

    case gfx::DescriptorType::StorageImage:
    {
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        if (view != nullptr)
        {
          Image *image = (Image *) view->desc.image;
          sVALIDATE(has_bits(image->desc.usage, gfx::ImageUsage::Storage));
        }
      }
    }
    break;

    case gfx::DescriptorType::StorageTexelBuffer:
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView *view = (BufferView *) update.texel_buffers[i];
        if (view != nullptr)
        {
          Buffer *buffer = (Buffer *) view->desc.buffer;
          sVALIDATE(has_bits(buffer->desc.usage,
                             gfx::BufferUsage::StorageTexelBuffer));
        }
      }
      break;

    case gfx::DescriptorType::UniformTexelBuffer:
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView *view = (BufferView *) update.texel_buffers[i];
        if (view != nullptr)
        {
          Buffer *buffer = (Buffer *) view->desc.buffer;
          sVALIDATE(has_bits(buffer->desc.usage,
                             gfx::BufferUsage::UniformTexelBuffer));
        }
      }
      break;

    default:
      sUNREACHABLE();
  }

  switch (binding.type)
  {
    case gfx::DescriptorType::DynamicStorageBuffer:
    case gfx::DescriptorType::DynamicUniformBuffer:
    case gfx::DescriptorType::StorageBuffer:
    case gfx::DescriptorType::UniformBuffer:
      sVALIDATE(update.element < binding.count);
      sVALIDATE((update.element + update.buffers.size()) <= binding.count);
      info_size = sizeof(VkDescriptorBufferInfo) * update.buffers.size();
      count     = (u32) update.buffers.size();
      break;

    case gfx::DescriptorType::StorageTexelBuffer:
    case gfx::DescriptorType::UniformTexelBuffer:
      sVALIDATE(update.element < binding.count);
      sVALIDATE((update.element + update.texel_buffers.size()) <=
                binding.count);
      info_size = sizeof(VkBufferView) * update.texel_buffers.size();
      count     = (u32) update.texel_buffers.size();
      break;

    case gfx::DescriptorType::SampledImage:
    case gfx::DescriptorType::CombinedImageSampler:
    case gfx::DescriptorType::StorageImage:
    case gfx::DescriptorType::InputAttachment:
    case gfx::DescriptorType::Sampler:
      sVALIDATE(update.element < binding.count);
      sVALIDATE((update.element + update.images.size()) <= binding.count);
      info_size = sizeof(VkDescriptorImageInfo) * update.images.size();
      count     = (u32) update.images.size();
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
    void *mem = heap->allocator.reallocate(
        MAX_STANDARD_ALIGNMENT, heap->scratch, heap->scratch_size, info_size);
    sCHECK(mem != nullptr);
    heap->scratch_size = info_size;
    heap->scratch      = mem;
  }

  VkDescriptorImageInfo  *pImageInfo       = nullptr;
  VkDescriptorBufferInfo *pBufferInfo      = nullptr;
  VkBufferView           *pTexelBufferView = nullptr;

  switch (binding.type)
  {
    case gfx::DescriptorType::DynamicStorageBuffer:
    case gfx::DescriptorType::DynamicUniformBuffer:
    case gfx::DescriptorType::StorageBuffer:
    case gfx::DescriptorType::UniformBuffer:
    {
      pBufferInfo = (VkDescriptorBufferInfo *) heap->scratch;
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        gfx::BufferBinding const &b      = update.buffers[i];
        Buffer                   *buffer = (Buffer *) b.buffer;
        pBufferInfo[i]                   = VkDescriptorBufferInfo{
                              .buffer = (buffer == nullptr) ? nullptr : buffer->vk_buffer,
                              .offset = b.offset,
                              .range  = b.size};
      }
    }
    break;

    case gfx::DescriptorType::Sampler:
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

    case gfx::DescriptorType::SampledImage:
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

    case gfx::DescriptorType::CombinedImageSampler:
    {
      pImageInfo = (VkDescriptorImageInfo *) heap->scratch;
      for (u32 i = 0; i < update.images.size(); i++)
      {
        gfx::ImageBinding const &b    = update.images[i];
        ImageView               *view = (ImageView *) b.image_view;
        pImageInfo[i]                 = VkDescriptorImageInfo{
                            .sampler     = (Sampler) b.sampler,
                            .imageView   = (view == nullptr) ? nullptr : view->vk_view,
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
      }
    }
    break;

    case gfx::DescriptorType::StorageImage:
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

    case gfx::DescriptorType::InputAttachment:
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

    case gfx::DescriptorType::StorageTexelBuffer:
    case gfx::DescriptorType::UniformTexelBuffer:
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
      sUNREACHABLE();
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
    case gfx::DescriptorType::DynamicStorageBuffer:
    case gfx::DescriptorType::DynamicUniformBuffer:
    case gfx::DescriptorType::StorageBuffer:
    case gfx::DescriptorType::UniformBuffer:
      for (u32 i = 0; i < update.buffers.size(); i++)
      {
        binding.buffers[update.element + i] =
            (Buffer *) update.buffers[i].buffer;
      }
      break;

    case gfx::DescriptorType::StorageTexelBuffer:
    case gfx::DescriptorType::UniformTexelBuffer:
      for (u32 i = 0; i < update.texel_buffers.size(); i++)
      {
        BufferView *view = (BufferView *) update.texel_buffers[i];
        binding.buffers[update.element + i] =
            (view == nullptr) ? nullptr : (Buffer *) view->desc.buffer;
      }
      break;

    case gfx::DescriptorType::Sampler:
      break;
    case gfx::DescriptorType::SampledImage:
    case gfx::DescriptorType::CombinedImageSampler:
    case gfx::DescriptorType::StorageImage:
    case gfx::DescriptorType::InputAttachment:
      for (u32 i = 0; i < update.images.size(); i++)
      {
        ImageView *view = (ImageView *) update.images[i].image_view;
        binding.images[update.element + i] =
            (view == nullptr) ? nullptr : (Image *) view->desc.image;
      }
      break;

    default:
      sUNREACHABLE();
  }
}

Result<Void, Status> DeviceInterface::wait_idle(gfx::Device self_)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.DeviceWaitIdle(self->vk_dev);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::wait_queue_idle(gfx::Device self_)
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
    gfx::Device self_, gfx::Surface surface_, Span<gfx::SurfaceFormat> formats)
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

  VkSurfaceFormatKHR *vk_formats =
      self->allocator.allocate_typed<VkSurfaceFormatKHR>(num_supported);
  if (vk_formats == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  {
    u32 num_read = num_supported;
    result       = self->instance->vk_table.GetPhysicalDeviceSurfaceFormatsKHR(
        self->phy_dev.vk_phy_dev, surface, &num_supported, vk_formats);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      self->allocator.deallocate_typed(vk_formats, num_supported);
      return Err{(Status) result};
    }

    sCHECK(num_read == num_supported && result != VK_INCOMPLETE);
  }

  u32 num_copies = min(num_supported, (u32) formats.size());

  for (u32 i = 0; i < num_copies; i++)
  {
    formats[i].format      = (gfx::Format) vk_formats[i].format;
    formats[i].color_space = (gfx::ColorSpace) vk_formats[i].colorSpace;
  }

  self->allocator.deallocate_typed(vk_formats, num_supported);

  return Ok{(u32) num_supported};
}

Result<u32, Status> DeviceInterface::get_surface_present_modes(
    gfx::Device self_, gfx::Surface surface_, Span<gfx::PresentMode> modes)
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

  VkPresentModeKHR *vk_present_modes =
      self->allocator.allocate_typed<VkPresentModeKHR>(num_supported);
  if (vk_present_modes == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  {
    u32 num_read = num_supported;
    result = self->instance->vk_table.GetPhysicalDeviceSurfacePresentModesKHR(
        self->phy_dev.vk_phy_dev, surface, &num_supported, vk_present_modes);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      self->allocator.deallocate_typed(vk_present_modes, num_supported);
      return Err{(Status) result};
    }

    sCHECK(num_read == num_supported && result != VK_INCOMPLETE);
  }

  u32 num_copies = min(num_supported, (u32) modes.size());

  for (u32 i = 0; i < num_copies; i++)
  {
    modes[i] = (gfx::PresentMode) vk_present_modes[i];
  }

  self->allocator.deallocate_typed(vk_present_modes, num_supported);

  return Ok{(u32) num_supported};
}

Result<gfx::SurfaceCapabilities, Status>
    DeviceInterface::get_surface_capabilities(gfx::Device  self_,
                                              gfx::Surface surface_)
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

  return Ok{gfx::SurfaceCapabilities{
      .image_usage = (gfx::ImageUsage) capabilities.supportedUsageFlags,
      .composite_alpha =
          (gfx::CompositeAlpha) capabilities.supportedCompositeAlpha}};
}

Result<gfx::SwapchainState, Status>
    DeviceInterface::get_swapchain_state(gfx::Device, gfx::Swapchain swapchain_)
{
  Swapchain *const swapchain = (Swapchain *) swapchain_;

  gfx::SwapchainState state{.extent = swapchain->extent,
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
    DeviceInterface::invalidate_swapchain(gfx::Device               self_,
                                          gfx::Swapchain            swapchain_,
                                          gfx::SwapchainDesc const &desc)
{
  Device *const self = (Device *) self_;
  sVALIDATE(desc.preferred_extent.x > 0);
  sVALIDATE(desc.preferred_extent.y > 0);
  Swapchain *const swapchain = (Swapchain *) swapchain_;
  swapchain->is_optimal      = false;
  swapchain->desc            = desc;
  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::begin_frame(gfx::Device    self_,
                                                  gfx::Swapchain swapchain_)
{
  Device *const    self         = (Device *) self_;
  FrameContext    &ctx          = self->frame_ctx;
  Swapchain *const swapchain    = (Swapchain *) swapchain_;
  VkFence          submit_fence = ctx.submit_f[ctx.ring_index];
  CommandEncoder  &enc          = ctx.encs[ctx.ring_index];

  sVALIDATE(!enc.is_recording());

  VkResult result = self->vk_table.WaitForFences(self->vk_dev, 1, &submit_fence,
                                                 VK_TRUE, U64_MAX);

  sCHECK(result == VK_SUCCESS);

  result = self->vk_table.ResetFences(self->vk_dev, 1, &submit_fence);

  sCHECK(result == VK_SUCCESS);

  if (swapchain->is_out_of_date || !swapchain->is_optimal ||
      swapchain->vk_swapchain == nullptr)
  {
    // await all pending submitted operations on the device possibly using
    // the swapchain, to avoid destroying whilst in use
    result = self->vk_table.DeviceWaitIdle(self->vk_dev);
    sCHECK(result == VK_SUCCESS);

    result = recreate_swapchain(self, swapchain);
    sCHECK(result == VK_SUCCESS);
  }

  if (!swapchain->is_zero_sized)
  {
    u32 next_image;
    result = self->vk_table.AcquireNextImageKHR(
        self->vk_dev, swapchain->vk_swapchain, U64_MAX,
        ctx.acquire[ctx.ring_index], nullptr, &next_image);

    if (result == VK_SUBOPTIMAL_KHR)
    {
      swapchain->is_optimal = false;
    }
    else
    {
      sCHECK(result == VK_SUCCESS);
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
  sCHECK(result == VK_SUCCESS);

  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::submit_frame(gfx::Device    self_,
                                                   gfx::Swapchain swapchain_)
{
  Device *const         self              = (Device *) self_;
  FrameContext         &ctx               = self->frame_ctx;
  Swapchain *const      swapchain         = (Swapchain *) swapchain_;
  VkFence const         submit_fence      = ctx.submit_f[ctx.ring_index];
  CommandEncoder       &enc               = ctx.encs[ctx.ring_index];
  VkCommandBuffer const command_buffer    = enc.vk_command_buffer;
  VkSemaphore const     submit_semaphore  = ctx.submit_s[ctx.ring_index];
  VkSemaphore const     acquire_semaphore = ctx.acquire[ctx.ring_index];
  bool const            was_acquired      = !swapchain->is_zero_sized;
  bool const            can_present =
      !(swapchain->is_out_of_date || swapchain->is_zero_sized);

  sVALIDATE(enc.is_recording());

  if (was_acquired)
  {
    access_image(enc, swapchain->image_impls[swapchain->current_image],
                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_NONE,
                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }

  VkResult result = self->vk_table.EndCommandBuffer(command_buffer);
  sCHECK(result == VK_SUCCESS);
  sCHECK(enc.status == gfx::Status::Success);

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

  sCHECK(result == VK_SUCCESS);

  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands could have been executed.
  ctx.current_frame++;
  ctx.tail_frame =
      max(ctx.current_frame, (gfx::FrameId) ctx.buffering) - ctx.buffering;
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
      sCHECK(result == VK_SUCCESS);
    }
  }

  return Ok{Void{}};
}

Result<u64, Status>
    DeviceInterface::get_timestamp_query_result(gfx::Device         self_,
                                                gfx::TimeStampQuery query_)
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

Result<gfx::PipelineStatistics, Status>
    DeviceInterface::get_statistics_query_result(gfx::Device          self_,
                                                 gfx::StatisticsQuery query_)
{
  Device *const self = (Device *) self_;

  if (self->phy_dev.vk_features.pipelineStatisticsQuery != VK_TRUE)
  {
    return Err{Status::FeatureNotPresent};
  }

  VkQueryPool const vk_pool = (VkQueryPool) query_;

  gfx::PipelineStatistics stats;
  VkResult                result = self->vk_table.GetQueryPoolResults(
      self->vk_dev, vk_pool, 0, 1, sizeof(gfx::PipelineStatistics), &stats,
      sizeof(u64), VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{stats};
}

#define ENCODE_PRELUDE()                                 \
  CommandEncoder *const self = (CommandEncoder *) self_; \
  if (self->status != Status::Success)                   \
  {                                                      \
    return;                                              \
  }                                                      \
  defer prelude_reset_arg_pool                           \
  {                                                      \
    [&] { self->arg_pool.reset(); }                      \
  }

void CommandEncoderInterface::reset_timestamp_query(gfx::CommandEncoder self_,
                                                    gfx::TimeStampQuery query_)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  sVALIDATE(!self->is_in_pass());

  self->dev->vk_table.CmdResetQueryPool(self->vk_command_buffer, vk_pool, 0, 1);
}

void CommandEncoderInterface::reset_statistics_query(
    gfx::CommandEncoder self_, gfx::StatisticsQuery query_)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  sVALIDATE(!self->is_in_pass());

  self->dev->vk_table.CmdResetQueryPool(self->vk_command_buffer, vk_pool, 0, 1);
}

void CommandEncoderInterface::write_timestamp(gfx::CommandEncoder self_,
                                              gfx::TimeStampQuery query_)
{
  ENCODE_PRELUDE();
  sVALIDATE(!self->is_in_pass());
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  self->dev->vk_table.CmdWriteTimestamp(self->vk_command_buffer,
                                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                        vk_pool, 0);
}

void CommandEncoderInterface::begin_statistics(gfx::CommandEncoder  self_,
                                               gfx::StatisticsQuery query_)
{
  ENCODE_PRELUDE();
  sVALIDATE(!self->is_in_pass());
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  self->dev->vk_table.CmdBeginQuery(self->vk_command_buffer, vk_pool, 0, 0);
}

void CommandEncoderInterface::end_statistics(gfx::CommandEncoder  self_,
                                             gfx::StatisticsQuery query_)
{
  ENCODE_PRELUDE();
  sVALIDATE(!self->is_in_pass());
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  self->dev->vk_table.CmdEndQuery(self->vk_command_buffer, vk_pool, 0);
}

void CommandEncoderInterface::begin_debug_marker(gfx::CommandEncoder self_,
                                                 Span<char const> region_name,
                                                 Vec4             color)
{
  ENCODE_PRELUDE();
  sVALIDATE(!self->is_in_pass());
  sVALIDATE(region_name.size() < 256);
  char region_name_cstr[256];
  to_c_str(region_name, to_span(region_name_cstr));

  VkDebugMarkerMarkerInfoEXT info{
      .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
      .pNext       = nullptr,
      .pMarkerName = region_name_cstr,
      .color       = {color.x, color.y, color.z, color.w}};
  self->dev->vk_table.CmdDebugMarkerBeginEXT(self->vk_command_buffer, &info);
}

void CommandEncoderInterface::end_debug_marker(gfx::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  sVALIDATE(!self->is_in_pass());
  self->dev->vk_table.CmdDebugMarkerEndEXT(self->vk_command_buffer);
}

void CommandEncoderInterface::fill_buffer(gfx::CommandEncoder self_,
                                          gfx::Buffer dst_, u64 offset,
                                          u64 size, u32 data)
{
  ENCODE_PRELUDE();
  Buffer *const dst = (Buffer *) dst_;

  sVALIDATE(!self->is_in_pass());
  sVALIDATE(has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  sVALIDATE(is_valid_buffer_access(dst->desc.size, offset, size, 4));

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);
  self->dev->vk_table.CmdFillBuffer(self->vk_command_buffer, dst->vk_buffer,
                                    offset, size, data);
}

void CommandEncoderInterface::copy_buffer(gfx::CommandEncoder self_,
                                          gfx::Buffer src_, gfx::Buffer dst_,
                                          Span<gfx::BufferCopy const> copies)
{
  ENCODE_PRELUDE();
  Buffer *const src        = (Buffer *) src_;
  Buffer *const dst        = (Buffer *) dst_;
  u32 const     num_copies = (u32) copies.size();

  sVALIDATE(!self->is_in_pass());
  sVALIDATE(has_bits(src->desc.usage, gfx::BufferUsage::TransferSrc));
  sVALIDATE(has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  sVALIDATE(num_copies > 0);
  for (gfx::BufferCopy const &copy : copies)
  {
    sVALIDATE(
        is_valid_buffer_access(src->desc.size, copy.src_offset, copy.size));
    sVALIDATE(
        is_valid_buffer_access(dst->desc.size, copy.dst_offset, copy.size));
  }

  VkBufferCopy *vk_copies =
      self->arg_pool.allocate_typed<VkBufferCopy>(num_copies);

  if (vk_copies == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferCopy const &copy = copies[i];
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

void CommandEncoderInterface::update_buffer(gfx::CommandEncoder self_,
                                            Span<u8 const> src, u64 dst_offset,
                                            gfx::Buffer dst_)
{
  ENCODE_PRELUDE();
  Buffer *const dst       = (Buffer *) dst_;
  u64 const     copy_size = src.size_bytes();

  sVALIDATE(!self->is_in_pass());
  sVALIDATE(has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  sVALIDATE(is_valid_buffer_access(dst->desc.size, dst_offset, copy_size, 4));

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  self->dev->vk_table.CmdUpdateBuffer(self->vk_command_buffer, dst->vk_buffer,
                                      dst_offset, (u64) src.size(), src.data());
}

void CommandEncoderInterface::clear_color_image(
    gfx::CommandEncoder self_, gfx::Image dst_, gfx::Color clear_color,
    Span<gfx::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  Image *const dst        = (Image *) dst_;
  u32 const    num_ranges = (u32) ranges.size();

  static_assert(sizeof(gfx::Color) == sizeof(VkClearColorValue));
  sVALIDATE(!self->is_in_pass());
  sVALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  sVALIDATE(num_ranges > 0);
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    sVALIDATE(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        range.aspects, range.first_mip_level, range.num_mip_levels,
        range.first_array_layer, range.num_array_layers));
  }

  VkImageSubresourceRange *vk_ranges =
      self->arg_pool.allocate_typed<VkImageSubresourceRange>(num_ranges);

  if (vk_ranges == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    vk_ranges[i]                            = VkImageSubresourceRange{
                                   .aspectMask     = (VkImageAspectFlags) range.aspects,
                                   .baseMipLevel   = range.first_mip_level,
                                   .levelCount     = range.num_mip_levels,
                                   .baseArrayLayer = range.first_array_layer,
                                   .layerCount     = range.num_array_layers};
  }

  access_image(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
    gfx::CommandEncoder self_, gfx::Image dst_,
    gfx::DepthStencil                      clear_depth_stencil,
    Span<gfx::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  Image *const dst        = (Image *) dst_;
  u32 const    num_ranges = (u32) ranges.size();

  static_assert(sizeof(gfx::DepthStencil) == sizeof(VkClearDepthStencilValue));
  sVALIDATE(!self->is_in_pass());
  sVALIDATE(num_ranges > 0);
  sVALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    sVALIDATE(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        range.aspects, range.first_mip_level, range.num_mip_levels,
        range.first_array_layer, range.num_array_layers));
  }

  VkImageSubresourceRange *vk_ranges =
      self->arg_pool.allocate_typed<VkImageSubresourceRange>(num_ranges);

  if (vk_ranges == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    vk_ranges[i]                            = VkImageSubresourceRange{
                                   .aspectMask     = (VkImageAspectFlags) range.aspects,
                                   .baseMipLevel   = range.first_mip_level,
                                   .levelCount     = range.num_mip_levels,
                                   .baseArrayLayer = range.first_array_layer,
                                   .layerCount     = range.num_array_layers};
  }

  access_image(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_WRITE_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkClearDepthStencilValue vk_clear_depth_stencil;
  memcpy(&vk_clear_depth_stencil, &clear_depth_stencil,
         sizeof(gfx::DepthStencil));

  self->dev->vk_table.CmdClearDepthStencilImage(
      self->vk_command_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_clear_depth_stencil, num_ranges,
      vk_ranges);
}

void CommandEncoderInterface::copy_image(gfx::CommandEncoder self_,
                                         gfx::Image src_, gfx::Image dst_,
                                         Span<gfx::ImageCopy const> copies)
{
  ENCODE_PRELUDE();
  Image *const src        = (Image *) src_;
  Image *const dst        = (Image *) dst_;
  u32 const    num_copies = (u32) copies.size();

  sVALIDATE(!self->is_in_pass());
  sVALIDATE(num_copies > 0);
  sVALIDATE(has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  sVALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::ImageCopy const &copy = copies[i];

    sVALIDATE(is_valid_image_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        copy.src_layers.aspects, copy.src_layers.mip_level, 1,
        copy.src_layers.first_array_layer, copy.src_layers.num_array_layers));
    sVALIDATE(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        copy.dst_layers.aspects, copy.dst_layers.mip_level, 1,
        copy.dst_layers.first_array_layer, copy.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, copy.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.dst_layers.mip_level);
    sVALIDATE(copy.extent.x > 0);
    sVALIDATE(copy.extent.y > 0);
    sVALIDATE(copy.extent.z > 0);
    sVALIDATE(copy.src_offset.x <= src_extent.x);
    sVALIDATE(copy.src_offset.y <= src_extent.y);
    sVALIDATE(copy.src_offset.z <= src_extent.z);
    sVALIDATE((copy.src_offset.x + copy.extent.x) <= src_extent.x);
    sVALIDATE((copy.src_offset.y + copy.extent.x) <= src_extent.y);
    sVALIDATE((copy.src_offset.z + copy.extent.x) <= src_extent.z);
    sVALIDATE(copy.dst_offset.x <= dst_extent.x);
    sVALIDATE(copy.dst_offset.y <= dst_extent.y);
    sVALIDATE(copy.dst_offset.z <= dst_extent.z);
    sVALIDATE((copy.dst_offset.x + copy.extent.x) <= dst_extent.x);
    sVALIDATE((copy.dst_offset.y + copy.extent.x) <= dst_extent.y);
    sVALIDATE((copy.dst_offset.z + copy.extent.x) <= dst_extent.z);
  }

  VkImageCopy *vk_copies =
      self->arg_pool.allocate_typed<VkImageCopy>(num_copies);

  if (vk_copies == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::ImageCopy const    &copy = copies[i];
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

  access_image(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_READ_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_WRITE_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  self->dev->vk_table.CmdCopyImage(
      self->vk_command_buffer, src->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoderInterface::copy_buffer_to_image(
    gfx::CommandEncoder self_, gfx::Buffer src_, gfx::Image dst_,
    Span<gfx::BufferImageCopy const> copies)
{
  ENCODE_PRELUDE();
  Buffer *const src        = (Buffer *) src_;
  Image *const  dst        = (Image *) dst_;
  u32 const     num_copies = (u32) copies.size();

  sVALIDATE(!self->is_in_pass());
  sVALIDATE(num_copies > 0);
  sVALIDATE(has_bits(src->desc.usage, gfx::BufferUsage::TransferSrc));
  sVALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferImageCopy const &copy = copies[i];
    sVALIDATE(is_valid_buffer_access(src->desc.size, copy.buffer_offset,
                                     gfx::WHOLE_SIZE));

    sVALIDATE(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        copy.image_layers.aspects, copy.image_layers.mip_level, 1,
        copy.image_layers.first_array_layer,
        copy.image_layers.num_array_layers));

    sVALIDATE(copy.image_extent.x > 0);
    sVALIDATE(copy.image_extent.y > 0);
    sVALIDATE(copy.image_extent.z > 0);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.image_layers.mip_level);
    sVALIDATE(copy.image_extent.x <= dst_extent.x);
    sVALIDATE(copy.image_extent.y <= dst_extent.y);
    sVALIDATE(copy.image_extent.z <= dst_extent.z);
    sVALIDATE((copy.image_offset.x + copy.image_extent.x) <= dst_extent.x);
    sVALIDATE((copy.image_offset.y + copy.image_extent.y) <= dst_extent.y);
    sVALIDATE((copy.image_offset.z + copy.image_extent.z) <= dst_extent.z);
  }

  VkBufferImageCopy *vk_copies =
      self->arg_pool.allocate_typed<VkBufferImageCopy>(num_copies);

  if (vk_copies == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferImageCopy const &copy = copies[i];
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
  access_image(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_WRITE_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  self->dev->vk_table.CmdCopyBufferToImage(
      self->vk_command_buffer, src->vk_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoderInterface::blit_image(gfx::CommandEncoder self_,
                                         gfx::Image src_, gfx::Image dst_,
                                         Span<gfx::ImageBlit const> blits,
                                         gfx::Filter                filter)
{
  ENCODE_PRELUDE();
  Image *const src       = (Image *) src_;
  Image *const dst       = (Image *) dst_;
  u32 const    num_blits = (u32) blits.size();

  sVALIDATE(!self->is_in_pass());
  sVALIDATE(num_blits > 0);
  sVALIDATE(has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  sVALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_blits; i++)
  {
    gfx::ImageBlit const &blit = blits[i];

    sVALIDATE(is_valid_image_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        blit.src_layers.aspects, blit.src_layers.mip_level, 1,
        blit.src_layers.first_array_layer, blit.src_layers.num_array_layers));

    sVALIDATE(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        blit.dst_layers.aspects, blit.dst_layers.mip_level, 1,
        blit.dst_layers.first_array_layer, blit.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, blit.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, blit.dst_layers.mip_level);
    sVALIDATE(blit.src_offsets[0].x <= src_extent.x);
    sVALIDATE(blit.src_offsets[0].y <= src_extent.y);
    sVALIDATE(blit.src_offsets[0].z <= src_extent.z);
    sVALIDATE(blit.src_offsets[1].x <= src_extent.x);
    sVALIDATE(blit.src_offsets[1].y <= src_extent.y);
    sVALIDATE(blit.src_offsets[1].z <= src_extent.z);
    sVALIDATE(blit.dst_offsets[0].x <= dst_extent.x);
    sVALIDATE(blit.dst_offsets[0].y <= dst_extent.y);
    sVALIDATE(blit.dst_offsets[0].z <= dst_extent.z);
    sVALIDATE(blit.dst_offsets[1].x <= dst_extent.x);
    sVALIDATE(blit.dst_offsets[1].y <= dst_extent.y);
    sVALIDATE(blit.dst_offsets[1].z <= dst_extent.z);
    sVALIDATE(!((src->desc.type == gfx::ImageType::Type1D) &&
                (blit.src_offsets[0].y != 0 | blit.src_offsets[1].y != 1)));
    sVALIDATE(!((src->desc.type == gfx::ImageType::Type1D ||
                 src->desc.type == gfx::ImageType::Type2D) &&
                (blit.src_offsets[0].z != 0 | blit.src_offsets[1].z != 1)));
    sVALIDATE(!((dst->desc.type == gfx::ImageType::Type1D) &&
                (blit.dst_offsets[0].y != 0 | blit.dst_offsets[1].y != 1)));
    sVALIDATE(!((dst->desc.type == gfx::ImageType::Type1D ||
                 dst->desc.type == gfx::ImageType::Type2D) &&
                (blit.src_offsets[0].z != 0 | blit.dst_offsets[1].z != 1)));
  }

  VkImageBlit *vk_blits = self->arg_pool.allocate_typed<VkImageBlit>(num_blits);

  if (vk_blits == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_blits; i++)
  {
    gfx::ImageBlit const    &blit = blits[i];
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

  access_image(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_READ_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_WRITE_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  self->dev->vk_table.CmdBlitImage(self->vk_command_buffer, src->vk_image,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   dst->vk_image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   num_blits, vk_blits, (VkFilter) filter);
}

void CommandEncoderInterface::resolve_image(
    gfx::CommandEncoder self_, gfx::Image src_, gfx::Image dst_,
    Span<gfx::ImageResolve const> resolves)
{
  ENCODE_PRELUDE();
  Image *const src          = (Image *) src_;
  Image *const dst          = (Image *) dst_;
  u32 const    num_resolves = (u32) resolves.size();

  sVALIDATE(!self->is_in_pass());
  sVALIDATE(num_resolves > 0);
  sVALIDATE(has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  sVALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  sVALIDATE(has_bits(dst->desc.sample_count, gfx::SampleCount::Count1));

  for (u32 i = 0; i < num_resolves; i++)
  {
    gfx::ImageResolve const &resolve = resolves[i];

    sVALIDATE(is_valid_image_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        resolve.src_layers.aspects, resolve.src_layers.mip_level, 1,
        resolve.src_layers.first_array_layer,
        resolve.src_layers.num_array_layers));
    sVALIDATE(is_valid_image_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        resolve.dst_layers.aspects, resolve.dst_layers.mip_level, 1,
        resolve.dst_layers.first_array_layer,
        resolve.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, resolve.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, resolve.dst_layers.mip_level);
    sVALIDATE(resolve.extent.x > 0);
    sVALIDATE(resolve.extent.y > 0);
    sVALIDATE(resolve.extent.z > 0);
    sVALIDATE(resolve.src_offset.x <= src_extent.x);
    sVALIDATE(resolve.src_offset.y <= src_extent.y);
    sVALIDATE(resolve.src_offset.z <= src_extent.z);
    sVALIDATE((resolve.src_offset.x + resolve.extent.x) <= src_extent.x);
    sVALIDATE((resolve.src_offset.y + resolve.extent.x) <= src_extent.y);
    sVALIDATE((resolve.src_offset.z + resolve.extent.x) <= src_extent.z);
    sVALIDATE(resolve.dst_offset.x <= dst_extent.x);
    sVALIDATE(resolve.dst_offset.y <= dst_extent.y);
    sVALIDATE(resolve.dst_offset.z <= dst_extent.z);
    sVALIDATE((resolve.dst_offset.x + resolve.extent.x) <= dst_extent.x);
    sVALIDATE((resolve.dst_offset.y + resolve.extent.x) <= dst_extent.y);
    sVALIDATE((resolve.dst_offset.z + resolve.extent.x) <= dst_extent.z);
  }

  VkImageResolve *vk_resolves =
      self->arg_pool.allocate_typed<VkImageResolve>(num_resolves);

  if (vk_resolves == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_resolves; i++)
  {
    gfx::ImageResolve const &resolve = resolves[i];
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

  access_image(*self, *src, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_READ_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_WRITE_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  self->dev->vk_table.CmdResolveImage(
      self->vk_command_buffer, src->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_resolves, vk_resolves);
}

void CommandEncoderInterface::begin_compute_pass(gfx::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  sVALIDATE(!self->is_in_pass());

  self->state = CommandEncoderState::ComputePass;
}

void CommandEncoderInterface::end_compute_pass(gfx::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  sVALIDATE(self->is_in_compute_pass());

  self->reset_context();
}

void CommandEncoderInterface::begin_render_pass(
    gfx::CommandEncoder self_, gfx::Framebuffer framebuffer_,
    gfx::RenderPass render_pass_, gfx::Offset render_offset,
    gfx::Extent                   render_extent,
    Span<gfx::Color const>        color_attachments_clear_values,
    Span<gfx::DepthStencil const> depth_stencil_attachment_clear_value)
{
  ENCODE_PRELUDE();
  Framebuffer *const framebuffer = (Framebuffer *) framebuffer_;
  RenderPass *const  render_pass = (RenderPass *) render_pass_;
  u32 const          num_color_clear_values =
      (u32) color_attachments_clear_values.size();
  u32 const num_depth_clear_values =
      (u32) depth_stencil_attachment_clear_value.size();

  sVALIDATE(render_pass != nullptr);
  sVALIDATE(!self->is_in_pass());
  sVALIDATE(num_depth_clear_values == 0 || num_depth_clear_values == 1);
  sVALIDATE(is_render_pass_compatible(
      *render_pass,
      Span{framebuffer->color_attachments, framebuffer->num_color_attachments},
      framebuffer->depth_stencil_attachment));
  sVALIDATE(color_attachments_clear_values.size() <=
            framebuffer->num_color_attachments);
  sVALIDATE(render_extent.x > 0);
  sVALIDATE(render_extent.y > 0);
  sVALIDATE(render_offset.x <= framebuffer->extent.x);
  sVALIDATE(render_offset.y <= framebuffer->extent.y);
  sVALIDATE((render_offset.x + render_extent.x) <= framebuffer->extent.x);
  sVALIDATE((render_offset.y + render_extent.y) <= framebuffer->extent.y);

  self->reset_context();
  self->state                  = CommandEncoderState::RenderPass;
  self->render_ctx.offset      = render_offset;
  self->render_ctx.extent      = render_extent;
  self->render_ctx.render_pass = render_pass;
  self->render_ctx.framebuffer = framebuffer;
  mem::copy(color_attachments_clear_values,
            self->render_ctx.color_clear_values);
  self->render_ctx.depth_stencil_clear_value =
      (num_depth_clear_values == 0) ? gfx::DepthStencil{} :
                                      depth_stencil_attachment_clear_value[0];
  self->render_ctx.num_color_clear_values         = num_color_clear_values;
  self->render_ctx.num_depth_stencil_clear_values = num_depth_clear_values;
}

void CommandEncoderInterface::end_render_pass(gfx::CommandEncoder self_)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());

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
      {
      }
      break;
    }
  }

  for (u32 i = 0; i < ctx.framebuffer->num_color_attachments; i++)
  {
    access_image(
        *self, *IMAGE_FROM_VIEW(ctx.framebuffer->color_attachments[i]),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        color_attachment_image_access(ctx.render_pass->color_attachments[i]),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }

  if (ctx.framebuffer->depth_stencil_attachment != nullptr)
  {
    VkAccessFlags access = depth_stencil_attachment_image_access(
        ctx.render_pass->depth_stencil_attachment);
    access_image(*self,
                 *IMAGE_FROM_VIEW(ctx.framebuffer->depth_stencil_attachment),
                 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                 access,
                 has_write_access(access) ?
                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
  }

  {
    VkClearValue vk_clear_values[gfx::MAX_PIPELINE_COLOR_ATTACHMENTS + 1];

    u32 ivk_clear_value = 0;
    for (u32 icolor_clear_value = 0;
         icolor_clear_value < ctx.num_color_clear_values;
         icolor_clear_value++, ivk_clear_value++)
    {
      gfx::Color const &color = ctx.color_clear_values[icolor_clear_value];
      memcpy(&vk_clear_values[ivk_clear_value].color, &color,
             sizeof(gfx::Color));
    }

    if (ctx.num_depth_stencil_clear_values > 0)
    {
      vk_clear_values[ivk_clear_value].depthStencil = {
          .depth   = ctx.depth_stencil_clear_value.depth,
          .stencil = ctx.depth_stencil_clear_value.stencil};
    }

    u32 const num_clear_values =
        ctx.num_color_clear_values + ctx.num_depth_stencil_clear_values;

    VkRect2D vk_render_area{
        .offset = VkOffset2D{.x = (i32) ctx.offset.x, .y = (i32) ctx.offset.y},
        .extent = VkExtent2D{.width = ctx.extent.x, .height = ctx.extent.y}};
    VkRenderPassBeginInfo begin_info{
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext           = nullptr,
        .renderPass      = ctx.render_pass->vk_render_pass,
        .framebuffer     = ctx.framebuffer->vk_framebuffer,
        .renderArea      = vk_render_area,
        .clearValueCount = num_clear_values,
        .pClearValues    = vk_clear_values};

    self->dev->vk_table.CmdBeginRenderPass(self->vk_command_buffer, &begin_info,
                                           VK_SUBPASS_CONTENTS_INLINE);
  }

  GraphicsPipeline *pipeline = nullptr;

  for (Command const &cmd : ctx.commands)
  {
    switch (cmd.type)
    {
      case CommandType::BindDescriptorSets:
      {
        VkDescriptorSet vk_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
        for (u32 i = 0; i < cmd.set.v1; i++)
        {
          vk_sets[i] = cmd.set.v0[i]->vk_set;
        }

        self->dev->vk_table.CmdBindDescriptorSets(
            self->vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->vk_layout, 0, cmd.set.v1, vk_sets, cmd.set.v3,
            cmd.set.v2);
      }
      break;
      case CommandType::BindPipeline:
      {
        pipeline = cmd.pipeline;
        self->dev->vk_table.CmdBindPipeline(self->vk_command_buffer,
                                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                                            pipeline->vk_pipeline);
      }
      break;
      case CommandType::PushConstants:
      {
        self->dev->vk_table.CmdPushConstants(
            self->vk_command_buffer, pipeline->vk_layout, VK_SHADER_STAGE_ALL,
            0, pipeline->push_constants_size, cmd.push_constant.v0);
      }
      break;
      case CommandType::SetViewport:
      {
        VkViewport vk_viewport{.x        = cmd.viewport.offset.x,
                               .y        = cmd.viewport.offset.y,
                               .width    = cmd.viewport.extent.x,
                               .height   = cmd.viewport.extent.y,
                               .minDepth = cmd.viewport.min_depth,
                               .maxDepth = cmd.viewport.max_depth};
        self->dev->vk_table.CmdSetViewport(self->vk_command_buffer, 0, 1,
                                           &vk_viewport);
      }
      break;
      case CommandType::SetScissor:
      {
        VkRect2D vk_scissor{
            .offset =
                VkOffset2D{(i32) cmd.scissor.v0.x, (i32) cmd.scissor.v0.y},
            .extent = VkExtent2D{cmd.scissor.v1.x, cmd.scissor.v1.y}};
        self->dev->vk_table.CmdSetScissor(self->vk_command_buffer, 0, 1,
                                          &vk_scissor);
      }
      break;
      case CommandType::SetBlendConstant:
      {
        f32 vk_constant[4] = {cmd.blend_constant.x, cmd.blend_constant.y,
                              cmd.blend_constant.z, cmd.blend_constant.w};
        self->dev->vk_table.CmdSetBlendConstants(self->vk_command_buffer,
                                                 vk_constant);
      }
      break;
      case CommandType::SetStencilCompareMask:
      {
        self->dev->vk_table.CmdSetStencilCompareMask(
            self->vk_command_buffer, (VkStencilFaceFlags) cmd.stencil.v0,
            cmd.stencil.v1);
      }
      break;
      case CommandType::SetStencilReference:
      {
        self->dev->vk_table.CmdSetStencilReference(
            self->vk_command_buffer, (VkStencilFaceFlags) cmd.stencil.v0,
            cmd.stencil.v1);
      }
      break;
      case CommandType::SetStencilWriteMask:
      {
        self->dev->vk_table.CmdSetStencilWriteMask(
            self->vk_command_buffer, (VkStencilFaceFlags) cmd.stencil.v0,
            cmd.stencil.v1);
      }
      break;
      case CommandType::BindVertexBuffer:
      {
        self->dev->vk_table.CmdBindVertexBuffers(
            self->vk_command_buffer, cmd.vertex_buffer.v0, 1,
            &cmd.vertex_buffer.v1->vk_buffer, &cmd.vertex_buffer.v2);
      }
      break;
      case CommandType::BindIndexBuffer:
      {
        self->dev->vk_table.CmdBindIndexBuffer(
            self->vk_command_buffer, cmd.index_buffer.v0->vk_buffer,
            cmd.index_buffer.v1, (VkIndexType) cmd.index_buffer.v2);
      }
      break;
      case CommandType::Draw:
      {
        self->dev->vk_table.CmdDraw(self->vk_command_buffer,
                                    cmd.draw_indexed.v0, cmd.draw_indexed.v1,
                                    cmd.draw_indexed.v2, cmd.draw_indexed.v3);
      }
      break;
      case CommandType::DrawIndexed:
      {
        self->dev->vk_table.CmdDrawIndexed(
            self->vk_command_buffer, cmd.draw_indexed.v0, cmd.draw_indexed.v1,
            cmd.draw_indexed.v2, cmd.draw_indexed.v3, cmd.draw_indexed.v4);
      }
      break;
      case CommandType::DrawIndirect:
      {
        self->dev->vk_table.CmdDrawIndirect(
            self->vk_command_buffer, cmd.draw_indirect.v0->vk_buffer,
            cmd.draw_indirect.v1, cmd.draw_indirect.v2, cmd.draw_indirect.v3);
      }
      break;
      case CommandType::DrawIndexedIndirect:
      {
        self->dev->vk_table.CmdDrawIndexedIndirect(
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

  self->dev->vk_table.CmdEndRenderPass(self->vk_command_buffer);
  self->reset_context();
}

void CommandEncoderInterface::bind_compute_pipeline(
    gfx::CommandEncoder self_, gfx::ComputePipeline pipeline)
{
  ENCODE_PRELUDE();
  ComputePassContext &ctx = self->compute_ctx;

  sVALIDATE(self->is_in_compute_pass());

  self->state  = CommandEncoderState::ComputePass;
  ctx.pipeline = (ComputePipeline *) pipeline;

  self->dev->vk_table.CmdBindPipeline(self->vk_command_buffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE,
                                      ctx.pipeline->vk_pipeline);
}

void CommandEncoderInterface::bind_graphics_pipeline(
    gfx::CommandEncoder self_, gfx::GraphicsPipeline pipeline_)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx      = self->render_ctx;
  GraphicsPipeline  *pipeline = (GraphicsPipeline *) pipeline_;

  sVALIDATE(self->is_in_render_pass());
  sVALIDATE(pipeline != nullptr);
  ctx.pipeline = pipeline;
  if (!ctx.commands.push(
          Command{.type = CommandType::BindPipeline, .pipeline = pipeline}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::bind_descriptor_sets(
    gfx::CommandEncoder self_, Span<gfx::DescriptorSet const> descriptor_sets,
    Span<u32 const> dynamic_offsets)
{
  ENCODE_PRELUDE();
  u32 const num_sets            = (u32) descriptor_sets.size();
  u32 const num_dynamic_offsets = (u32) dynamic_offsets.size();
  u64 const ubo_offset_alignment =
      self->dev->phy_dev.vk_properties.limits.minUniformBufferOffsetAlignment;
  u64 const ssbo_offset_alignment =
      self->dev->phy_dev.vk_properties.limits.minStorageBufferOffsetAlignment;

  sVALIDATE(self->is_in_pass());
  sVALIDATE(num_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  sVALIDATE(num_dynamic_offsets <= (gfx::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS +
                                    gfx::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS));

  for (u32 offset : dynamic_offsets)
  {
    sVALIDATE(mem::is_aligned(ubo_offset_alignment, offset) ||
              mem::is_aligned(ssbo_offset_alignment, offset));
  }

  if (self->is_in_compute_pass())
  {
    sVALIDATE(self->compute_ctx.pipeline != nullptr);
    VkDescriptorSet vk_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
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
    sVALIDATE(self->render_ctx.pipeline != nullptr);
    DescriptorSet **sets =
        self->render_ctx.arg_pool.allocate_typed<DescriptorSet *>(num_sets);
    if (sets == nullptr)
    {
      self->status = Status::OutOfHostMemory;
      return;
    }
    u32 *offsets =
        self->render_ctx.arg_pool.allocate_typed<u32>(num_dynamic_offsets);
    if (sets == nullptr)
    {
      self->render_ctx.arg_pool.deallocate_typed(sets, num_sets);
      self->status = Status::OutOfHostMemory;
      return;
    }
    mem::copy(descriptor_sets, (gfx::DescriptorSet *) sets);
    mem::copy(dynamic_offsets, offsets);
    if (!self->render_ctx.commands.push(
            Command{.type = CommandType::BindDescriptorSets,
                    .set  = {sets, num_sets, offsets, num_dynamic_offsets}}))
    {
      self->render_ctx.arg_pool.deallocate_typed(offsets, num_dynamic_offsets);
      self->render_ctx.arg_pool.deallocate_typed(sets, num_sets);
      self->status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoderInterface::push_constants(gfx::CommandEncoder self_,
                                             Span<u8 const> push_constants_data)
{
  ENCODE_PRELUDE();
  sVALIDATE(push_constants_data.size_bytes() <= gfx::MAX_PUSH_CONSTANTS_SIZE);
  u32 const push_constants_size = (u32) push_constants_data.size_bytes();
  sVALIDATE(mem::is_aligned(4, push_constants_size));
  sVALIDATE(self->is_in_pass());

  if (self->is_in_compute_pass())
  {
    sVALIDATE(self->compute_ctx.pipeline != nullptr);
    sVALIDATE(push_constants_size ==
              self->compute_ctx.pipeline->push_constants_size);
    self->dev->vk_table.CmdPushConstants(
        self->vk_command_buffer, self->compute_ctx.pipeline->vk_layout,
        VK_SHADER_STAGE_ALL, 0, self->compute_ctx.pipeline->push_constants_size,
        push_constants_data.data());
  }
  else if (self->is_in_render_pass())
  {
    sVALIDATE(self->render_ctx.pipeline != nullptr);
    sVALIDATE(push_constants_size ==
              self->render_ctx.pipeline->push_constants_size);
    u8 *data =
        self->render_ctx.arg_pool.allocate_typed<u8>(push_constants_size);
    sCHECK(data != nullptr);
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

void CommandEncoderInterface::dispatch(gfx::CommandEncoder self_,
                                       u32 group_count_x, u32 group_count_y,
                                       u32 group_count_z)
{
  ENCODE_PRELUDE();
  ComputePassContext &ctx = self->compute_ctx;

  sVALIDATE(self->is_in_compute_pass());

  sVALIDATE(ctx.pipeline != nullptr);
  sVALIDATE(group_count_x <= gfx::MAX_COMPUTE_WORK_GROUP_COUNT);
  sVALIDATE(group_count_y <= gfx::MAX_COMPUTE_WORK_GROUP_COUNT);
  sVALIDATE(group_count_z <= gfx::MAX_COMPUTE_WORK_GROUP_COUNT);

  for (u32 i = 0; i < ctx.num_sets; i++)
  {
    access_compute_bindings(*self, *ctx.sets[i]);
  }

  self->dev->vk_table.CmdDispatch(self->vk_command_buffer, group_count_x,
                                  group_count_y, group_count_z);
}

void CommandEncoderInterface::dispatch_indirect(gfx::CommandEncoder self_,
                                                gfx::Buffer buffer_, u64 offset)
{
  ENCODE_PRELUDE();
  ComputePassContext &ctx    = self->compute_ctx;
  Buffer *const       buffer = (Buffer *) buffer_;

  sVALIDATE(self->is_in_compute_pass());
  sVALIDATE(ctx.pipeline != nullptr);
  sVALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  sVALIDATE(is_valid_buffer_access(buffer->desc.size, offset,
                                   sizeof(gfx::DispatchCommand), 4));

  for (u32 i = 0; i < ctx.num_sets; i++)
  {
    access_compute_bindings(*self, *ctx.sets[i]);
  }

  self->dev->vk_table.CmdDispatchIndirect(self->vk_command_buffer,
                                          buffer->vk_buffer, offset);
}

void CommandEncoderInterface::set_viewport(gfx::CommandEncoder  self_,
                                           gfx::Viewport const &viewport)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());
  sVALIDATE(viewport.min_depth >= 0.0F);
  sVALIDATE(viewport.max_depth <= 1.0F);

  if (!ctx.commands.push(
          Command{.type = CommandType::SetViewport, .viewport = viewport}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_scissor(gfx::CommandEncoder self_,
                                          gfx::Offset         scissor_offset,
                                          gfx::Extent         scissor_extent)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());

  if (!ctx.commands.push(Command{.type    = CommandType::SetScissor,
                                 .scissor = {scissor_offset, scissor_extent}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_blend_constants(gfx::CommandEncoder self_,
                                                  Vec4 blend_constant)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());

  if (!ctx.commands.push(Command{.type = CommandType::SetBlendConstant,
                                 .blend_constant = blend_constant}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_stencil_compare_mask(
    gfx::CommandEncoder self_, gfx::StencilFaces faces, u32 mask)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());

  if (!ctx.commands.push(Command{.type    = CommandType::SetStencilCompareMask,
                                 .stencil = {faces, mask}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_stencil_reference(gfx::CommandEncoder self_,
                                                    gfx::StencilFaces   faces,
                                                    u32 reference)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());

  if (!ctx.commands.push(Command{.type    = CommandType::SetStencilReference,
                                 .stencil = {faces, reference}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_stencil_write_mask(gfx::CommandEncoder self_,
                                                     gfx::StencilFaces   faces,
                                                     u32                 mask)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());

  if (!ctx.commands.push(Command{.type    = CommandType::SetStencilWriteMask,
                                 .stencil = {faces, mask}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::bind_vertex_buffers(
    gfx::CommandEncoder self_, Span<gfx::Buffer const> vertex_buffers,
    Span<u64 const> offsets)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());
  u32 const num_vertex_buffers = (u32) vertex_buffers.size();
  sVALIDATE(num_vertex_buffers > 0);
  sVALIDATE(num_vertex_buffers <= gfx::MAX_VERTEX_ATTRIBUTES);
  sVALIDATE(offsets.size() == vertex_buffers.size());
  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    u64 const     offset = offsets[i];
    Buffer *const buffer = (Buffer *) vertex_buffers[i];
    sVALIDATE(offset < buffer->desc.size);
    sVALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::VertexBuffer));
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

void CommandEncoderInterface::bind_index_buffer(gfx::CommandEncoder self_,
                                                gfx::Buffer    index_buffer_,
                                                u64            offset,
                                                gfx::IndexType index_type)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx          = self->render_ctx;
  Buffer *const      index_buffer = (Buffer *) index_buffer_;
  u64 const          index_size   = index_type_size(index_type);

  sVALIDATE(self->is_in_render_pass());
  sVALIDATE(offset < index_buffer->desc.size);
  sVALIDATE(mem::is_aligned(index_size, offset));
  sVALIDATE(has_bits(index_buffer->desc.usage, gfx::BufferUsage::IndexBuffer));

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

void CommandEncoderInterface::draw(gfx::CommandEncoder self_, u32 vertex_count,
                                   u32 instance_count, u32 first_vertex_id,
                                   u32 first_instance_id)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());
  sVALIDATE(ctx.pipeline != nullptr);

  if (!ctx.commands.push(Command{.type = CommandType::Draw,
                                 .draw = {vertex_count, instance_count,
                                          first_vertex_id, first_instance_id}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::draw_indexed(gfx::CommandEncoder self_,
                                           u32 first_index, u32 num_indices,
                                           i32 vertex_offset,
                                           u32 first_instance_id,
                                           u32 num_instances)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx = self->render_ctx;

  sVALIDATE(self->is_in_render_pass());
  sVALIDATE(ctx.pipeline != nullptr);
  sVALIDATE(ctx.index_buffer != nullptr);
  u64 const index_size = index_type_size(ctx.index_type);
  sVALIDATE((ctx.index_buffer_offset + first_index * index_size) <
            ctx.index_buffer->desc.size);
  sVALIDATE(
      (ctx.index_buffer_offset + (first_index + num_indices) * index_size) <=
      ctx.index_buffer->desc.size);

  if (!ctx.commands.push(
          Command{.type         = CommandType::DrawIndexed,
                  .draw_indexed = {first_index, num_indices, vertex_offset,
                                   first_instance_id, num_instances}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::draw_indirect(gfx::CommandEncoder self_,
                                            gfx::Buffer buffer_, u64 offset,
                                            u32 draw_count, u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx    = self->render_ctx;
  Buffer *const      buffer = (Buffer *) buffer_;

  sVALIDATE(self->is_in_render_pass());
  sVALIDATE(ctx.pipeline != nullptr);
  sVALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  sVALIDATE(offset < buffer->desc.size);
  sVALIDATE((offset + (u64) draw_count * stride) <= buffer->desc.size);
  sVALIDATE(mem::is_aligned(4, stride));
  sVALIDATE(stride >= sizeof(gfx::DrawCommand));

  if (!ctx.commands.push(
          Command{.type          = CommandType::DrawIndirect,
                  .draw_indirect = {buffer, offset, draw_count, stride}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::draw_indexed_indirect(gfx::CommandEncoder self_,
                                                    gfx::Buffer         buffer_,
                                                    u64 offset, u32 draw_count,
                                                    u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext &ctx    = self->render_ctx;
  Buffer *const      buffer = (Buffer *) buffer_;

  sVALIDATE(self->is_in_render_pass());
  sVALIDATE(ctx.pipeline != nullptr);
  sVALIDATE(ctx.index_buffer != nullptr);
  sVALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  sVALIDATE(offset < buffer->desc.size);
  sVALIDATE((offset + (u64) draw_count * stride) <= buffer->desc.size);
  sVALIDATE(mem::is_aligned(4, stride));
  sVALIDATE(stride >= sizeof(gfx::DrawIndexedCommand));

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
