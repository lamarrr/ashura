#include "ashura/gfx/vulkan.h"
#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "ashura/std/source_location.h"
#include "vulkan/vulkan.h"
#include <new>
#include <stdlib.h>

#ifndef VK_LAYER_KHRONOS_VALIDATION_NAME
#  define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"
#endif

namespace ash
{
namespace vk
{

#define PANIC_IF(logger, description, ...)                                 \
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

// TODO(lamarrr): tracer for vma memory allocations
// trace u64, i64, f64 etc
#define VALIDATE(...) \
  PANIC_IF(self->logger, "Validation Failed: " #__VA_ARGS__, __VA_ARGS__)

#define CHECK(...) \
  PANIC_IF(self->logger, "Check Failed: " #__VA_ARGS__, __VA_ARGS__)

#define CHECK_EX(logger, ...) \
  PANIC_IF(logger, "Check Failed: " #__VA_ARGS__, __VA_ARGS__)

#define UNREACHABLE() abort()

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

bool load_device_table(VkDevice device, InstanceTable const &instance_table,
                       DeviceTable &vk_table)
{
  mem::zero(&vk_table, 1);
  bool all_loaded = true;

#define LOAD_VK(function)                                                  \
  vk_table.function = (PFN_vk##function) instance_table.GetDeviceProcAddr( \
      device, "vk" #function);                                             \
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
      device, "vk" #function);                                             \
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
    enc.device->vk_table.CmdPipelineBarrier(enc.vk_command_buffer, src_stages,
                                            dst_stages, 0, 0, nullptr, 1,
                                            &barrier, 0, nullptr);
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
    enc.device->vk_table.CmdPipelineBarrier(enc.vk_command_buffer, src_stages,
                                            dst_stages, 0, 0, nullptr, 0,
                                            nullptr, 1, &barrier);
  }
}

inline void access_compute_bindings(CommandEncoder &enc, gfx::DescriptorSet set)
{
  DescriptorHeap const *heap = (DescriptorHeap *) set.heap;

  for (u32 ibinding = 0; ibinding < heap->set_layout->num_bindings; ibinding++)
  {
    gfx::DescriptorBindingDesc const &binding =
        heap->set_layout->bindings[ibinding];
    switch (binding.type)
    {
      case gfx::DescriptorType::CombinedImageSampler:
      case gfx::DescriptorType::SampledImage:
      {
        Image **images = heap->images + set.index * heap->num_set_images +
                         heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_image(enc, *images[ielement],
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_ACCESS_SHADER_READ_BIT,
                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
      }
      break;

      case gfx::DescriptorType::StorageImage:
      {
        Image **images = heap->images + set.index * heap->num_set_images +
                         heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_image(enc, *images[ielement],
                       VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                       VK_IMAGE_LAYOUT_GENERAL);
        }
      }
      break;

      case gfx::DescriptorType::UniformBuffer:
      case gfx::DescriptorType::DynamicUniformBuffer:
      case gfx::DescriptorType::UniformTexelBuffer:
      {
        Buffer **buffers = heap->buffers + set.index * heap->num_set_buffers +
                           heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_buffer(enc, *buffers[ielement],
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_ACCESS_SHADER_READ_BIT);
        }
      }
      break;

      case gfx::DescriptorType::StorageBuffer:
      case gfx::DescriptorType::DynamicStorageBuffer:
      case gfx::DescriptorType::StorageTexelBuffer:
      {
        Buffer **buffers = heap->buffers + set.index * heap->num_set_buffers +
                           heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_buffer(enc, *buffers[ielement],
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
        }
      }
      break;

      case gfx::DescriptorType::InputAttachment:
        break;

      default:
        UNREACHABLE();
    }
  }
}

inline void access_graphics_bindings(CommandEncoder    &enc,
                                     gfx::DescriptorSet set)
{
  DescriptorHeap const *heap = (DescriptorHeap *) set.heap;
  u32 const             idx  = set.index;

  for (u32 ibinding = 0; ibinding < heap->set_layout->num_bindings; ibinding++)
  {
    gfx::DescriptorBindingDesc const &binding =
        heap->set_layout->bindings[ibinding];
    switch (binding.type)
    {
      case gfx::DescriptorType::CombinedImageSampler:
      case gfx::DescriptorType::SampledImage:
      case gfx::DescriptorType::InputAttachment:
      {
        Image **images = heap->images + idx * heap->num_set_images +
                         heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_image(enc, *images[ielement],
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
      {
        Buffer **buffers = heap->buffers + idx * heap->num_set_buffers +
                           heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_buffer(enc, *buffers[ielement],
                        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_ACCESS_SHADER_READ_BIT);
        }
      }
      break;

      case gfx::DescriptorType::StorageImage:
      {
        Image **images = heap->images + idx * heap->num_set_images +
                         heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_image(enc, *images[ielement],
                       VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                       VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                       VK_IMAGE_LAYOUT_GENERAL);
        }
      }

      case gfx::DescriptorType::StorageTexelBuffer:
      case gfx::DescriptorType::StorageBuffer:
      case gfx::DescriptorType::DynamicStorageBuffer:
      {
        Buffer **buffers = heap->buffers + idx * heap->num_set_buffers +
                           heap->binding_index_map[ibinding];
        for (u32 ielement = 0; ielement < binding.count; ielement++)
        {
          access_buffer(enc, *buffers[ielement],
                        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
        }
      }
      break;
      default:
        UNREACHABLE();
    }
  }
}

inline bool is_render_pass_compatible(RenderPass const      *render_pass,
                                      Span<ImageView *const> color_attachments,
                                      ImageView *depth_stencil_attachment)
{
  // also depends on the formats of the input attachments which can't be
  // determined here
  // our render_passes uses same initial and final layouts
  if (render_pass->num_color_attachments != color_attachments.size())
  {
    return false;
  }

  if ((render_pass->depth_stencil_attachment.format ==
       gfx::Format::Undefined) &&
      (depth_stencil_attachment != nullptr))
  {
    return false;
  }

  if (depth_stencil_attachment != nullptr &&
      (render_pass->depth_stencil_attachment.format !=
       IMAGE_FROM_VIEW(depth_stencil_attachment)->desc.format))
  {
    return false;
  }

  for (usize i = 0; i < render_pass->num_color_attachments; i++)
  {
    if (render_pass->color_attachments[i].format !=
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
      UNREACHABLE();
  }
}

inline bool is_valid_buffer_access(u64 size, u64 access_offset, u64 access_size)
{
  access_size =
      access_size == gfx::WHOLE_SIZE ? (size - access_offset) : access_size;
  return access_offset < size && (access_offset + access_size) <= size &&
         access_size > 0;
}

inline bool is_valid_aligned_buffer_access(u64 size, u64 access_offset,
                                           u64 access_size, u64 alignment)
{
  access_size =
      access_size == gfx::WHOLE_SIZE ? (size - access_offset) : access_size;
  return access_offset < size && (access_offset + access_size) <= size &&
         (access_offset % alignment == 0) && (access_size % alignment == 0) &&
         access_size > 0;
}

inline bool is_valid_image_subresource_access(
    gfx::ImageAspects aspects, u32 num_levels, u32 num_layers,
    gfx::ImageAspects access_aspects, u32 access_level, u32 num_access_levels,
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

Result<gfx::InstanceImpl, Status>
    InstanceInterface::create(AllocatorImpl allocator, Logger *logger,
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

    CHECK_EX(logger, num_read_extensions == num_extensions);
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

    CHECK_EX(logger, num_read_layers == num_layers);
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
      "VK_KHR_xlib_surface",
  };

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

  CHECK_EX(logger, load_instance_table(vk_instance, vkGetInstanceProcAddr,
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

Result<gfx::DeviceImpl, Status> InstanceInterface::create_device(
    gfx::Instance self_, Span<gfx::DeviceType const> preferred_types,
    Span<gfx::Surface const> compatible_surfaces, AllocatorImpl allocator)
{
  Instance *const self               = (Instance *) self_;
  u32 const       num_surfaces       = (u32) compatible_surfaces.size();
  constexpr u32   MAX_QUEUE_FAMILIES = 16;

  u32      num_devices;
  VkResult result = self->vk_table.EnumeratePhysicalDevices(
      self->vk_instance, &num_devices, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  if (num_devices == 0)
  {
    self->logger->trace("No Physical Device Found");
    return Err{Status::DeviceLost};
  }

  VkPhysicalDevice *vk_physical_devices =
      self->allocator.allocate_typed<VkPhysicalDevice>(num_devices);

  if (vk_physical_devices == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  {
    u32 num_read_devices = num_devices;
    result               = self->vk_table.EnumeratePhysicalDevices(
        self->vk_instance, &num_read_devices, vk_physical_devices);

    if (result != VK_SUCCESS)
    {
      self->allocator.deallocate_typed(vk_physical_devices, num_devices);
      return Err{(Status) result};
    }

    CHECK(num_read_devices == num_devices);
  }

  PhysicalDevice *physical_devices =
      self->allocator.allocate_typed<PhysicalDevice>(num_devices);

  if (physical_devices == nullptr)
  {
    self->allocator.deallocate_typed(vk_physical_devices, num_devices);
    return Err{Status::OutOfHostMemory};
  }

  for (u32 i = 0; i < num_devices; i++)
  {
    PhysicalDevice  &dev    = physical_devices[i];
    VkPhysicalDevice vk_dev = vk_physical_devices[i];
    dev.vk_physical_device  = vk_dev;
    self->vk_table.GetPhysicalDeviceFeatures(vk_dev, &dev.features);
    self->vk_table.GetPhysicalDeviceMemoryProperties(vk_dev,
                                                     &dev.memory_properties);
    self->vk_table.GetPhysicalDeviceProperties(vk_dev, &dev.properties);
  }

  self->allocator.deallocate_typed(vk_physical_devices, num_devices);

  self->logger->trace("Available Devices:");
  for (u32 i = 0; i < num_devices; i++)
  {
    PhysicalDevice const             &device     = physical_devices[i];
    VkPhysicalDeviceProperties const &properties = device.properties;
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
        device.vk_physical_device, &num_queue_families, nullptr);

    CHECK(num_queue_families <= MAX_QUEUE_FAMILIES);

    VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

    {
      u32 num_read_queue_families = num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          device.vk_physical_device, &num_queue_families,
          queue_family_properties);
      CHECK(num_read_queue_families == num_queue_families);
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

  u32 selected_device_index = num_devices;
  u32 selected_queue_family = VK_QUEUE_FAMILY_IGNORED;

  for (usize i = 0; i < (u32) preferred_types.size(); i++)
  {
    for (u32 idevice = 0;
         idevice < num_devices && selected_device_index == num_devices;
         idevice++)
    {
      PhysicalDevice const &device = physical_devices[idevice];

      u32 num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          device.vk_physical_device, &num_queue_families, nullptr);

      CHECK(num_queue_families <= MAX_QUEUE_FAMILIES);

      VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

      {
        u32 num_read_queue_families = num_queue_families;
        self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
            device.vk_physical_device, &num_queue_families,
            queue_family_properties);
        CHECK(num_read_queue_families == num_queue_families);
      }

      if (((VkPhysicalDeviceType) preferred_types[i]) ==
          device.properties.deviceType)
      {
        for (u32 iqueue_family = 0; iqueue_family < num_queue_families &&
                                    selected_device_index == num_devices;
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
                  device.vk_physical_device, iqueue_family,
                  (Surface) compatible_surfaces[isurface], &supported);
              if (supported == VK_TRUE)
              {
                num_supported_surfaces++;
              }
            }

            if (num_supported_surfaces == num_surfaces)
            {
              selected_device_index = idevice;
              selected_queue_family = iqueue_family;
              break;
            }
          }
        }
      }
    }
  }

  if (selected_device_index == num_devices)
  {
    self->logger->trace("No Suitable Device Found");
    self->allocator.deallocate_typed(physical_devices, num_devices);
    return Err{Status::DeviceLost};
  }

  PhysicalDevice selected_device = physical_devices[selected_device_index];

  self->allocator.deallocate_typed(physical_devices, num_devices);

  self->logger->trace("Selected Device ", selected_device_index);

  u32 num_extensions;
  result = self->vk_table.EnumerateDeviceExtensionProperties(
      selected_device.vk_physical_device, nullptr, &num_extensions, nullptr);

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

  {
    u32 num_read_extensions = num_extensions;
    result                  = self->vk_table.EnumerateDeviceExtensionProperties(
        selected_device.vk_physical_device, nullptr, &num_read_extensions,
        extensions);
    if (result != VK_SUCCESS)
    {
      self->allocator.deallocate_typed(extensions, num_extensions);
      return Err{(Status) result};
    }
    CHECK(num_extensions == num_read_extensions);
  }

  u32 num_layers;
  result = self->vk_table.EnumerateDeviceLayerProperties(
      selected_device.vk_physical_device, &num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    self->allocator.deallocate_typed(extensions, num_extensions);
    return Err{(Status) result};
  }

  VkLayerProperties *layers =
      self->allocator.allocate_typed<VkLayerProperties>(num_layers);

  if (num_layers > 0 && layers == nullptr)
  {
    self->allocator.deallocate_typed(extensions, num_extensions);
    return Err{Status::OutOfHostMemory};
  }

  {
    u32 num_read_layers = num_layers;
    result              = self->vk_table.EnumerateDeviceLayerProperties(
        selected_device.vk_physical_device, &num_read_layers, layers);
    if (result != VK_SUCCESS)
    {
      self->allocator.deallocate_typed(layers, num_layers);
      self->allocator.deallocate_typed(extensions, num_extensions);
      return Err{(Status) result};
    }
    CHECK(num_read_layers == num_layers);
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

  bool has_swapchain_ext    = false;
  bool has_debug_marker_ext = false;
  bool has_validation_layer = false;

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

    if (has_swapchain_ext && has_debug_marker_ext)
    {
      break;
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

  self->allocator.deallocate_typed(layers, num_layers);
  self->allocator.deallocate_typed(extensions, num_extensions);

  char const *load_extensions[2];
  u32         num_load_extensions = 0;
  char const *load_layers[2];
  u32         num_load_layers = 0;

  // required
  load_extensions[num_load_extensions] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
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
  features.samplerAnisotropy = selected_device.features.samplerAnisotropy;
  features.fillModeNonSolid  = selected_device.features.fillModeNonSolid;
  features.logicOp           = selected_device.features.logicOp;

  VkDeviceCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .queueCreateInfoCount    = 1,
                                 .pQueueCreateInfos       = &queue_create_info,
                                 .enabledLayerCount       = num_load_layers,
                                 .ppEnabledLayerNames     = load_layers,
                                 .enabledExtensionCount   = num_load_extensions,
                                 .ppEnabledExtensionNames = load_extensions,
                                 .pEnabledFeatures        = &features};

  VkDevice vk_device;
  result = self->vk_table.CreateDevice(selected_device.vk_physical_device,
                                       &create_info, nullptr, &vk_device);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  DeviceTable        vk_table;
  VmaVulkanFunctions vma_table;
  CHECK(load_device_table(vk_device, self->vk_table, vk_table));

  load_vma_table(self->vk_table, vk_table, vma_table);

  Device *device = self->allocator.allocate_typed<Device>(1);

  if (device == nullptr)
  {
    vk_table.DestroyDevice(vk_device, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  VkQueue vk_queue;
  vk_table.GetDeviceQueue(vk_device, selected_queue_family, 0, &vk_queue);

  VmaAllocatorCreateInfo vma_create_info{
      .flags          = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
      .physicalDevice = selected_device.vk_physical_device,
      .device         = vk_device,
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
    self->allocator.deallocate_typed(device, 1);
    vk_table.DestroyDevice(vk_device, nullptr);
    return Err{(Status) result};
  }

  new (device) Device{.allocator       = allocator,
                      .logger          = self->logger,
                      .instance        = self,
                      .physical_device = selected_device,
                      .vk_table        = vk_table,
                      .vma_table       = vma_table,
                      .vk_device       = vk_device,
                      .queue_family    = selected_queue_family,
                      .vk_queue        = vk_queue,
                      .vma_allocator   = vma_allocator};

  return Ok{gfx::DeviceImpl{.self      = (gfx::Device) device,
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
  Device *const   device   = (Device *) device_;

  if (device == nullptr)
  {
    return;
  }

  vmaDestroyAllocator(device->vma_allocator);
  device->vk_table.DestroyDevice(device->vk_device, nullptr);
  instance->allocator.deallocate_typed(device, 1);
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
  Device *const                     self = (Device *) self_;
  VkPhysicalDeviceProperties const &vk_properties =
      self->physical_device.properties;

  bool has_uma = false;
  for (u32 i = 0; i < self->physical_device.memory_properties.memoryTypeCount;
       i++)
  {
    if (has_bits(self->physical_device.memory_properties.memoryTypes[i]
                     .propertyFlags,
                 (VkMemoryPropertyFlags) (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)))
    {
      has_uma = true;
      break;
    }
  }

  VkPhysicalDeviceLimits vk_limits = vk_properties.limits;
  gfx::DeviceLimits      limits{
           .max_image_dimension1D     = vk_limits.maxImageDimension1D,
           .max_image_dimension2D     = vk_limits.maxImageDimension2D,
           .max_image_dimension3D     = vk_limits.maxImageDimension3D,
           .max_image_dimension_cube  = vk_limits.maxImageDimensionCube,
           .max_image_array_layers    = vk_limits.maxImageArrayLayers,
           .max_texel_buffer_elements = vk_limits.maxTexelBufferElements,
           .max_uniform_buffer_range  = vk_limits.maxUniformBufferRange,
           .max_storage_buffer_range  = vk_limits.maxStorageBufferRange,
           .max_push_constants_size   = vk_limits.maxPushConstantsSize,
           .max_bound_descriptor_sets = vk_limits.maxBoundDescriptorSets,
           .max_per_stage_descriptor_samplers =
          vk_limits.maxPerStageDescriptorSamplers,
           .max_per_stage_descriptor_uniform_buffers =
          vk_limits.maxPerStageDescriptorUniformBuffers,
           .max_per_stage_descriptor_storage_buffers =
          vk_limits.maxPerStageDescriptorStorageBuffers,
           .max_per_stage_descriptor_sampled_images =
          vk_limits.maxPerStageDescriptorSampledImages,
           .max_per_stage_descriptor_storage_images =
          vk_limits.maxPerStageDescriptorStorageImages,
           .max_per_stage_descriptor_input_attachments =
          vk_limits.maxPerStageDescriptorInputAttachments,
           .max_per_stage_resources     = vk_limits.maxPerStageResources,
           .max_descriptor_set_samplers = vk_limits.maxDescriptorSetSamplers,
           .max_descriptor_set_uniform_buffers =
          vk_limits.maxDescriptorSetUniformBuffers,
           .max_descriptor_set_uniform_buffers_dynamic =
          vk_limits.maxDescriptorSetUniformBuffersDynamic,
           .max_descriptor_set_storage_buffers =
          vk_limits.maxDescriptorSetStorageBuffers,
           .max_descriptor_set_storage_buffers_dynamic =
          vk_limits.maxDescriptorSetStorageBuffersDynamic,
           .max_descriptor_set_sampled_images =
          vk_limits.maxDescriptorSetSampledImages,
           .max_descriptor_set_storage_images =
          vk_limits.maxDescriptorSetStorageImages,
           .max_descriptor_set_input_attachments =
          vk_limits.maxDescriptorSetInputAttachments,
           .max_vertex_input_attributes = vk_limits.maxVertexInputAttributes,
           .max_vertex_input_bindings   = vk_limits.maxVertexInputBindings,
           .max_vertex_input_attribute_offset =
          vk_limits.maxVertexInputAttributeOffset,
           .max_vertex_input_binding_stride = vk_limits.maxVertexInputBindingStride,
           .max_vertex_output_components = vk_limits.maxVertexOutputComponents,
           .max_fragment_input_components = vk_limits.maxFragmentInputComponents,
           .max_fragment_output_attachments = vk_limits.maxFragmentOutputAttachments,
           .max_fragment_dual_src_attachments =
          vk_limits.maxFragmentDualSrcAttachments,
           .max_fragment_combined_output_resources =
          vk_limits.maxFragmentCombinedOutputResources,
           .max_compute_shared_memory_size = vk_limits.maxComputeSharedMemorySize,
           .max_compute_work_group_count = {vk_limits.maxComputeWorkGroupCount[0],
                                            vk_limits.maxComputeWorkGroupCount[1],
                                            vk_limits.maxComputeWorkGroupCount[2]},
           .max_compute_work_group_invocations =
          vk_limits.maxComputeWorkGroupInvocations,
           .max_compute_work_group_size = {vk_limits.maxComputeWorkGroupSize[0],
                                           vk_limits.maxComputeWorkGroupSize[1],
                                           vk_limits.maxComputeWorkGroupSize[2]},
           .max_draw_indexed_index_value = vk_limits.maxDrawIndexedIndexValue,
           .max_draw_indirect_count      = vk_limits.maxDrawIndirectCount,
           .max_sampler_lod_bias         = vk_limits.maxSamplerLodBias,
           .max_sampler_anisotropy       = vk_limits.maxSamplerAnisotropy,
           .max_viewports                = vk_limits.maxViewports,
           .max_viewport_dimensions      = {vk_limits.maxViewportDimensions[0],
                                            vk_limits.maxViewportDimensions[1]},
           .viewport_bounds_range        = {vk_limits.viewportBoundsRange[0],
                                            vk_limits.viewportBoundsRange[1]},
           .min_memory_map_alignment     = vk_limits.minMemoryMapAlignment,
           .min_texel_buffer_offset_alignment =
          vk_limits.minTexelBufferOffsetAlignment,
           .min_uniform_buffer_offset_alignment =
          vk_limits.minUniformBufferOffsetAlignment,
           .min_storage_buffer_offset_alignment =
          vk_limits.minStorageBufferOffsetAlignment,
           .max_framebuffer_width  = vk_limits.maxFramebufferWidth,
           .max_framebuffer_height = vk_limits.maxFramebufferHeight,
           .max_framebuffer_layers = vk_limits.maxFramebufferLayers,
           .framebuffer_color_sample_counts =
          (gfx::SampleCount) vk_limits.framebufferColorSampleCounts,
           .framebuffer_depth_sample_counts =
          (gfx::SampleCount) vk_limits.framebufferDepthSampleCounts,
           .framebuffer_stencil_sample_counts =
          (gfx::SampleCount) vk_limits.framebufferStencilSampleCounts,
           .framebuffer_no_attachments_sample_counts =
          (gfx::SampleCount) vk_limits.framebufferNoAttachmentsSampleCounts,
           .max_color_attachments = vk_limits.maxColorAttachments,
           .sampled_image_color_sample_counts =
          (gfx::SampleCount) vk_limits.sampledImageColorSampleCounts,
           .sampled_image_integer_sample_counts =
          (gfx::SampleCount) vk_limits.sampledImageIntegerSampleCounts,
           .sampled_image_depth_sample_counts =
          (gfx::SampleCount) vk_limits.sampledImageDepthSampleCounts,
           .sampled_image_stencil_sample_counts =
          (gfx::SampleCount) vk_limits.sampledImageStencilSampleCounts,
           .storage_image_sample_counts =
          (gfx::SampleCount) vk_limits.storageImageSampleCounts,
           .max_clip_distances = vk_limits.maxClipDistances,
           .max_cull_distances = vk_limits.maxCullDistances,
           .max_combined_clip_and_cull_distances =
          vk_limits.maxCombinedClipAndCullDistances};

  return gfx::DeviceProperties{
      .api_version        = vk_properties.apiVersion,
      .driver_version     = vk_properties.driverVersion,
      .vendor_id          = vk_properties.vendorID,
      .device_id          = vk_properties.deviceID,
      .api_name           = "vulkan"_span,
      .device_name        = {vk_properties.deviceName,
                             strlen(vk_properties.deviceName)},
      .type               = (gfx::DeviceType) vk_properties.deviceType,
      .has_unified_memory = has_uma,
      .limits             = limits};
}

Result<gfx::FormatProperties, Status>
    DeviceInterface::get_format_properties(gfx::Device self_,
                                           gfx::Format format)
{
  Device *const      self = (Device *) self_;
  VkFormatProperties props;
  self->instance->vk_table.GetPhysicalDeviceFormatProperties(
      self->physical_device.vk_physical_device, (VkFormat) format, &props);
  return Ok(gfx::FormatProperties{
      .linear_tiling_features =
          (gfx::FormatFeatures) props.linearTilingFeatures,
      .optimal_tiling_features =
          (gfx::FormatFeatures) props.optimalTilingFeatures,
      .buffer_features = (gfx::FormatFeatures) props.bufferFeatures});
}

void set_resource_name(Device *device, Span<char const> label,
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
  device->instance->vk_table.SetDebugUtilsObjectNameEXT(device->vk_device,
                                                        &name_info);
  VkDebugMarkerObjectNameInfoEXT debug_info{
      .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
      .pNext       = nullptr,
      .objectType  = debug_type,
      .object      = (u64) resource,
      .pObjectName = buff};
  device->vk_table.DebugMarkerSetObjectNameEXT(device->vk_device, &debug_info);
}

Result<gfx::Buffer, Status>
    DeviceInterface::create_buffer(gfx::Device            self_,
                                   gfx::BufferDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE(desc.size != 0);
  VALIDATE(desc.usage != gfx::BufferUsage::None);

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

  VALIDATE(buffer != nullptr);
  VALIDATE(has_any_bit(buffer->desc.usage,
                       gfx::BufferUsage::UniformTexelBuffer |
                           gfx::BufferUsage::StorageTexelBuffer));
  VALIDATE(desc.format != gfx::Format::Undefined);
  VALIDATE(is_valid_buffer_access(buffer->desc.size, desc.offset, desc.size));

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

  VkResult result = self->vk_table.CreateBufferView(
      self->vk_device, &create_info, nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_view, VK_OBJECT_TYPE_BUFFER_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);

  BufferView *view = self->allocator.allocate_typed<BufferView>(1);

  if (view == nullptr)
  {
    self->vk_table.DestroyBufferView(self->vk_device, vk_view, nullptr);
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

  VALIDATE(desc.format != gfx::Format::Undefined);
  VALIDATE(desc.usage != gfx::ImageUsage::None);
  VALIDATE(desc.aspects != gfx::ImageAspects::None);
  VALIDATE(desc.sample_count != gfx::SampleCount::None);
  VALIDATE(desc.extent.x != 0);
  VALIDATE(desc.extent.y != 0);
  VALIDATE(desc.extent.z != 0);
  VALIDATE(desc.mip_levels > 0);
  VALIDATE(desc.mip_levels <= num_mip_levels(desc.extent));
  VALIDATE(desc.array_layers > 0);
  VALIDATE(!(desc.type == gfx::ImageType::Type2D && desc.extent.z != 1));
  VALIDATE(!(desc.type == gfx::ImageType::Type3D && desc.array_layers != 1));

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

  VALIDATE(desc.image != nullptr);
  VALIDATE(desc.view_format != gfx::Format::Undefined);
  VALIDATE(is_image_view_type_compatible(src_image->desc.type, desc.view_type));
  VALIDATE(is_valid_image_subresource_access(
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
  VkResult    result = self->vk_table.CreateImageView(
      self->vk_device, &create_info, nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_view, VK_OBJECT_TYPE_IMAGE_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);

  ImageView *view = self->allocator.allocate_typed<ImageView>(1);
  if (view == nullptr)
  {
    self->vk_table.DestroyImageView(self->vk_device, vk_view, nullptr);
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
  Device *const       self = (Device *) self_;
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
  VkResult  result = self->vk_table.CreateSampler(self->vk_device, &create_info,
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

  VALIDATE(desc.spirv_code.size_bytes() > 0);

  VkShaderModuleCreateInfo create_info{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0,
      .codeSize = desc.spirv_code.size_bytes(),
      .pCode    = desc.spirv_code.data()};

  VkShaderModule vk_shader;
  VkResult       result = self->vk_table.CreateShaderModule(
      self->vk_device, &create_info, nullptr, &vk_shader);
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

  VALIDATE(desc.color_attachments.size() <= gfx::MAX_COLOR_ATTACHMENTS);
  VALIDATE(desc.input_attachments.size() <= gfx::MAX_INPUT_ATTACHMENTS);

  // render_pass attachment descriptions are packed in the following order:
  // [color_attachments..., depth_stencil_attachment, input_attachments...]
  VkAttachmentDescription vk_attachments[gfx::MAX_COLOR_ATTACHMENTS + 1 +
                                         gfx::MAX_INPUT_ATTACHMENTS];
  VkAttachmentReference   vk_color_attachments[gfx::MAX_COLOR_ATTACHMENTS];
  VkAttachmentReference   vk_depth_stencil_attachment;
  VkAttachmentReference   vk_input_attachments[gfx::MAX_INPUT_ATTACHMENTS];
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

  VkResult result = self->vk_table.CreateRenderPass(
      self->vk_device, &create_info, nullptr, &vk_render_pass);
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
    self->vk_table.DestroyRenderPass(self->vk_device, vk_render_pass, nullptr);
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
  VkImageView vk_attachments[gfx::MAX_COLOR_ATTACHMENTS + 1];

  VALIDATE(desc.extent.x > 0);
  VALIDATE(desc.extent.y > 0);
  VALIDATE(desc.layers > 0);
  VALIDATE((desc.color_attachments.size() > 0) ||
           (desc.depth_stencil_attachment != nullptr));

  for (gfx::ImageView attachment : desc.color_attachments)
  {
    ImageView *const view  = (ImageView *) attachment;
    Image *const     image = IMAGE_FROM_VIEW(attachment);
    gfx::Extent3D    extent =
        mip_down(image->desc.extent, view->desc.first_mip_level);
    VALIDATE(has_bits(image->desc.usage, gfx::ImageUsage::ColorAttachment));
    VALIDATE(has_bits(view->desc.aspects, gfx::ImageAspects::Color));
    VALIDATE(view->desc.num_array_layers >= desc.layers);
    VALIDATE(extent.x >= desc.extent.x);
    VALIDATE(extent.y >= desc.extent.y);
  }

  if (desc.depth_stencil_attachment != nullptr)
  {
    ImageView *const view  = (ImageView *) desc.depth_stencil_attachment;
    Image *const     image = IMAGE_FROM_VIEW(view);
    gfx::Extent3D    extent =
        mip_down(image->desc.extent, view->desc.first_mip_level);
    VALIDATE(
        has_bits(image->desc.usage, gfx::ImageUsage::DepthStencilAttachment));
    VALIDATE(has_any_bit(view->desc.aspects, gfx::ImageAspects::Depth |
                                                 gfx::ImageAspects::Stencil));
    VALIDATE(view->desc.num_array_layers >= desc.layers);
    VALIDATE(extent.x >= desc.extent.x);
    VALIDATE(extent.y >= desc.extent.y);
  }

  VALIDATE(is_render_pass_compatible(
      render_pass,
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

  VkResult result = self->vk_table.CreateFramebuffer(
      self->vk_device, &create_info, nullptr, &vk_framebuffer);
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
    self->vk_table.DestroyFramebuffer(self->vk_device, vk_framebuffer, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (framebuffer) Framebuffer{.extent            = desc.extent,
                                .color_attachments = {},
                                .depth_stencil_attachment =
                                    (ImageView *) desc.depth_stencil_attachment,
                                .layers                = desc.layers,
                                .num_color_attachments = num_color_attachments,
                                .vk_framebuffer        = vk_framebuffer};

  mem::copy(desc.color_attachments,
            (gfx::ImageView *) framebuffer->color_attachments);

  return Ok{(gfx::Framebuffer) framebuffer};
}

Result<gfx::DescriptorSetLayout, Status>
    DeviceInterface::create_descriptor_set_layout(
        gfx::Device self_, gfx::DescriptorSetLayoutDesc const &desc)
{
  Device *const self                = (Device *) self_;
  u32 const     num_bindings        = (u32) desc.bindings.size();
  u32           num_dynamic_buffers = 0;

  for (gfx::DescriptorBindingDesc const &desc : desc.bindings)
  {
    switch (desc.type)
    {
      case gfx::DescriptorType::DynamicStorageBuffer:
      case gfx::DescriptorType::DynamicUniformBuffer:
        num_dynamic_buffers += desc.count;
      default:
        break;
    }
  }

  VALIDATE(num_bindings > 0);
  VALIDATE(num_dynamic_buffers <= gfx::MAX_DESCRIPTOR_DYNAMIC_BUFFERS);
  for (u32 i = 0; i < num_bindings; i++)
  {
    VALIDATE(desc.bindings[i].count > 0);
  }

  VkDescriptorSetLayoutBinding *vk_bindings =
      self->allocator.allocate_typed<VkDescriptorSetLayoutBinding>(
          num_bindings);

  if (vk_bindings == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  gfx::DescriptorBindingDesc *bindings =
      self->allocator.allocate_typed<gfx::DescriptorBindingDesc>(num_bindings);

  if (bindings == nullptr)
  {
    self->allocator.deallocate_typed(vk_bindings, num_bindings);
    return Err{Status::OutOfHostMemory};
  }

  mem::copy(desc.bindings, bindings);

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
  }

  VkDescriptorSetLayoutCreateInfo create_info{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext        = nullptr,
      .flags        = 0,
      .bindingCount = num_bindings,
      .pBindings    = vk_bindings};

  VkDescriptorSetLayout vk_layout;
  VkResult              result = self->vk_table.CreateDescriptorSetLayout(
      self->vk_device, &create_info, nullptr, &vk_layout);

  self->allocator.deallocate_typed(vk_bindings, num_bindings);

  if (result != VK_SUCCESS)
  {
    self->allocator.deallocate_typed(bindings, num_bindings);
    return Err{(Status) result};
  }

  set_resource_name(self, desc.label, vk_layout,
                    VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);

  DescriptorSetLayout *layout =
      self->allocator.allocate_typed<DescriptorSetLayout>(1);
  if (layout == nullptr)
  {
    self->vk_table.DestroyDescriptorSetLayout(self->vk_device, vk_layout,
                                              nullptr);
    self->allocator.deallocate_typed(bindings, num_bindings);
    return Err{Status::OutOfHostMemory};
  }

  new (layout) DescriptorSetLayout{.bindings     = bindings,
                                   .num_bindings = num_bindings,
                                   .vk_layout    = vk_layout};

  return Ok{(gfx::DescriptorSetLayout) layout};
}

Result<gfx::DescriptorHeapImpl, Status>
    DeviceInterface::create_descriptor_heap(gfx::Device self_,
                                            gfx::DescriptorHeapDesc const &desc)
{
  Device *const       self      = (Device *) self_;
  AllocatorImpl const allocator = desc.allocator;

  VALIDATE(desc.num_sets_per_pool > 0);

  DescriptorSetLayout *set_layout = (DescriptorSetLayout *) desc.layout;

  u32 *binding_index_map =
      allocator.allocate_typed<u32>(set_layout->num_bindings);

  if (binding_index_map == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  defer binding_index_map_del{[&] {
    allocator.deallocate_typed(binding_index_map, set_layout->num_bindings);
  }};

  u32 count[11]   = {};
  u32 num_images  = 0;
  u32 num_buffers = 0;
  for (u32 binding = 0; binding < set_layout->num_bindings; binding++)
  {
    gfx::DescriptorBindingDesc desc = set_layout->bindings[binding];

    switch (desc.type)
    {
      case gfx::DescriptorType::Sampler:
      {
        binding_index_map[binding] = U32_MAX;
      }
      break;
      case gfx::DescriptorType::CombinedImageSampler:
      case gfx::DescriptorType::SampledImage:
      case gfx::DescriptorType::StorageImage:
      case gfx::DescriptorType::InputAttachment:
      {
        binding_index_map[binding] = num_images;
        num_images += desc.count;
      }
      break;
      case gfx::DescriptorType::UniformTexelBuffer:
      case gfx::DescriptorType::StorageTexelBuffer:
      case gfx::DescriptorType::UniformBuffer:
      case gfx::DescriptorType::StorageBuffer:
      case gfx::DescriptorType::DynamicUniformBuffer:
      case gfx::DescriptorType::DynamicStorageBuffer:
      {
        binding_index_map[binding] = num_buffers;
        num_buffers += desc.count;
      }
      break;

      default:
        UNREACHABLE();
    }

    u32 type = (u32) desc.type;
    CHECK(type < 11);
    count[type] += desc.count;
  }

  VkDescriptorPoolSize pool_sizes[11] = {};
  u32                  num_pool_sizes = 0;

  for (u32 i = 0; i < size(count); i++)
  {
    if (count[i] > 0)
    {
      pool_sizes[num_pool_sizes] = VkDescriptorPoolSize{
          .type            = (VkDescriptorType) i,
          .descriptorCount = desc.num_sets_per_pool * count[i]};
      num_pool_sizes++;
    }
  }

  u32 num_descriptor_image_infos =
      count[(u32) gfx::DescriptorType::Sampler] +
      count[(u32) gfx::DescriptorType::CombinedImageSampler] +
      count[(u32) gfx::DescriptorType::SampledImage] +
      count[(u32) gfx::DescriptorType::StorageImage] +
      count[(u32) gfx::DescriptorType::InputAttachment];
  u32 num_descriptor_buffer_infos =
      count[(u32) gfx::DescriptorType::UniformBuffer] +
      count[(u32) gfx::DescriptorType::StorageBuffer] +
      count[(u32) gfx::DescriptorType::DynamicUniformBuffer] +
      count[(u32) gfx::DescriptorType::DynamicStorageBuffer];
  u32 num_descriptor_texel_buffer_infos =
      count[(u32) gfx::DescriptorType::UniformTexelBuffer] +
      count[(u32) gfx::DescriptorType::StorageTexelBuffer];

  usize const scratch_size =
      max(max(num_descriptor_image_infos * sizeof(VkDescriptorImageInfo),
              num_descriptor_buffer_infos * sizeof(VkDescriptorBufferInfo)),
          num_descriptor_texel_buffer_infos * sizeof(VkBufferView));

  void *scratch = allocator.allocate(MAX_STANDARD_ALIGNMENT, scratch_size);

  defer scratch_del{[&] {
    allocator.deallocate(MAX_STANDARD_ALIGNMENT, scratch, scratch_size);
  }};

  DescriptorHeap *heap = self->allocator.allocate_typed<DescriptorHeap>(1);

  if (heap == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (heap) DescriptorHeap{.device                  = self,
                            .allocator               = allocator,
                            .logger                  = self->logger,
                            .set_layout              = set_layout,
                            .binding_index_map       = binding_index_map,
                            .pools                   = nullptr,
                            .sets                    = nullptr,
                            .last_use_frame          = nullptr,
                            .released                = nullptr,
                            .free                    = nullptr,
                            .images                  = nullptr,
                            .buffers                 = nullptr,
                            .scratch                 = scratch,
                            .num_set_images          = num_images,
                            .num_set_buffers         = num_buffers,
                            .num_pool_sizes          = num_pool_sizes,
                            .num_pools               = 0,
                            .num_sets_per_pool       = desc.num_sets_per_pool,
                            .num_released            = 0,
                            .num_free                = 0,
                            .pools_capacity          = 0,
                            .sets_capacity           = 0,
                            .last_use_frame_capacity = 0,
                            .released_capacity       = 0,
                            .free_capacity           = 0,
                            .images_capacity         = 0,
                            .buffers_capacity        = 0,
                            .scratch_size            = scratch_size};

  mem::copy(pool_sizes, heap->pool_sizes, num_pool_sizes);

  binding_index_map = nullptr;
  scratch           = nullptr;

  return Ok(gfx::DescriptorHeapImpl{.self      = (gfx::DescriptorHeap) heap,
                                    .interface = &descriptor_heap_interface});
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
      self->vk_device, &create_info, nullptr, &vk_cache);
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

  VALIDATE(num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE(desc.push_constant_size <= gfx::MAX_PUSH_CONSTANT_SIZE);
  VALIDATE(desc.push_constant_size % 4 == 0);
  VALIDATE(desc.compute_shader.entry_point.size() > 0 &&
           desc.compute_shader.entry_point.size() < 256);
  VALIDATE(desc.compute_shader.shader != nullptr);

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

  VkPushConstantRange push_constant_range{.stageFlags =
                                              VK_SHADER_STAGE_COMPUTE_BIT,
                                          .offset = 0,
                                          .size   = desc.push_constant_size};

  VkPipelineLayoutCreateInfo layout_create_info{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0,
      .setLayoutCount         = num_descriptor_sets,
      .pSetLayouts            = vk_descriptor_set_layouts,
      .pushConstantRangeCount = desc.push_constant_size == 0 ? 0U : 1U,
      .pPushConstantRanges =
          desc.push_constant_size == 0 ? nullptr : &push_constant_range};

  VkPipelineLayout vk_layout;
  VkResult         result = self->vk_table.CreatePipelineLayout(
      self->vk_device, &layout_create_info, nullptr, &vk_layout);
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
      self->vk_device,
      desc.cache == nullptr ? nullptr : (PipelineCache) desc.cache, 1,
      &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
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
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_device, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline) ComputePipeline{.vk_pipeline        = vk_pipeline,
                                 .vk_layout          = vk_layout,
                                 .push_constant_size = desc.push_constant_size};

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

  VALIDATE(num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE(desc.push_constant_size <= gfx::MAX_PUSH_CONSTANT_SIZE);
  VALIDATE(desc.push_constant_size % 4 == 0);
  VALIDATE(desc.vertex_shader.entry_point.size() > 0 &&
           desc.vertex_shader.entry_point.size() < 256);
  VALIDATE(desc.fragment_shader.entry_point.size() > 0 &&
           desc.fragment_shader.entry_point.size() < 256);
  VALIDATE(num_attributes <= gfx::MAX_VERTEX_ATTRIBUTES);

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

  VkPushConstantRange push_constant_range{.stageFlags = VK_SHADER_STAGE_ALL,
                                          .offset     = 0,
                                          .size = desc.push_constant_size};

  VkPipelineLayoutCreateInfo layout_create_info{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0,
      .setLayoutCount         = num_descriptor_sets,
      .pSetLayouts            = vk_descriptor_set_layouts,
      .pushConstantRangeCount = desc.push_constant_size == 0 ? 0U : 1U,
      .pPushConstantRanges =
          desc.push_constant_size == 0 ? nullptr : &push_constant_range};

  VkPipelineLayout vk_layout;

  VkResult result = self->vk_table.CreatePipelineLayout(
      self->vk_device, &layout_create_info, nullptr, &vk_layout);
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
      attachment_states[gfx::MAX_COLOR_ATTACHMENTS];

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
      self->vk_device,
      desc.cache == nullptr ? nullptr : (PipelineCache) desc.cache, 1,
      &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
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
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_device, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
      GraphicsPipeline{.vk_pipeline        = vk_pipeline,
                       .vk_layout          = vk_layout,
                       .push_constant_size = desc.push_constant_size};

  return Ok{(gfx::GraphicsPipeline) pipeline};
}

Result<gfx::Fence, Status> DeviceInterface::create_fence(gfx::Device self_,
                                                         bool        signaled)
{
  Device *const self = (Device *) self_;

  VkFenceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = signaled ? (VkFenceCreateFlags) VK_FENCE_CREATE_SIGNALED_BIT :
                          (VkFenceCreateFlags) 0};

  VkFence  vk_fence;
  VkResult result = self->vk_table.CreateFence(self->vk_device, &create_info,
                                               nullptr, &vk_fence);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{(gfx::Fence) vk_fence};
}

Result<gfx::CommandEncoderImpl, Status>
    DeviceInterface::create_command_encoder(gfx::Device   self_,
                                            AllocatorImpl allocator)
{
  Device *const self = (Device *) self_;

  VkCommandPoolCreateInfo command_pool_create_info{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = self->queue_family};

  VkCommandPool vk_command_pool;
  VkResult      result = self->vk_table.CreateCommandPool(
      self->vk_device, &command_pool_create_info, nullptr, &vk_command_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkCommandBufferAllocateInfo allocate_info{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext              = nullptr,
      .commandPool        = vk_command_pool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  VkCommandBuffer vk_command_buffer;
  result = self->vk_table.AllocateCommandBuffers(
      self->vk_device, &allocate_info, &vk_command_buffer);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroyCommandPool(self->vk_device, vk_command_pool,
                                      nullptr);
    return Err{(Status) result};
  }

  CommandEncoder *enc = self->allocator.allocate_typed<CommandEncoder>(1);

  if (enc == nullptr)
  {
    self->vk_table.DestroyCommandPool(self->vk_device, vk_command_pool,
                                      nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (enc) CommandEncoder{.allocator         = allocator,
                           .logger            = self->logger,
                           .device            = self,
                           .vk_command_pool   = vk_command_pool,
                           .vk_command_buffer = vk_command_buffer,
                           .status            = Status::Success,
                           .state             = CommandEncoderState::Reset,
                           .ctx               = {}};

  return Ok{gfx::CommandEncoderImpl{.self      = (gfx::CommandEncoder) enc,
                                    .interface = &command_encoder_interface}};
}

Result<gfx::FrameContext, Status>
    DeviceInterface::create_frame_context(gfx::Device                  self_,
                                          gfx::FrameContextDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE(desc.max_frames_in_flight > 0);

  gfx::CommandEncoderImpl *command_encoders =
      self->allocator.allocate_typed<gfx::CommandEncoderImpl>(
          desc.max_frames_in_flight);

  if (command_encoders == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  {
    Status status   = Status::Success;
    u32    push_end = 0;
    for (; push_end < desc.max_frames_in_flight; push_end++)
    {
      Result result =
          DeviceInterface::create_command_encoder(self_, desc.allocator);
      if (result.is_err())
      {
        status = result.err();
        break;
      }
      command_encoders[push_end] = result.value();
    }

    if (push_end != desc.max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        DeviceInterface::destroy_command_encoder(self_, command_encoders[i]);
      }
      return Err{(Status) status};
    }
  }

  VkSemaphore *acquire_semaphores =
      self->allocator.allocate_typed<VkSemaphore>(desc.max_frames_in_flight);

  {
    VkResult              result   = VK_SUCCESS;
    u32                   push_end = 0;
    VkSemaphoreCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0};
    for (; push_end < desc.max_frames_in_flight; push_end++)
    {
      VkSemaphore semaphore;
      result = self->vk_table.CreateSemaphore(self->vk_device, &create_info,
                                              nullptr, &semaphore);
      if (result != VK_SUCCESS)
      {
        break;
      }
      set_resource_name(self, desc.label, semaphore, VK_OBJECT_TYPE_SEMAPHORE,
                        VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);
      acquire_semaphores[push_end] = semaphore;
    }

    if (push_end != desc.max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                        nullptr);
      }

      for (u32 i = 0; i < desc.max_frames_in_flight; i++)
      {
        DeviceInterface::destroy_command_encoder(self_, command_encoders[i]);
      }

      return Err{(Status) result};
    }
  }

  gfx::Fence *submit_fences =
      self->allocator.allocate_typed<gfx::Fence>(desc.max_frames_in_flight);

  if (submit_fences == nullptr)
  {
    for (u32 i = 0; i < desc.max_frames_in_flight; i++)
    {
      self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                      nullptr);
    }
    return Err{Status::OutOfHostMemory};
  }

  {
    Status status   = Status::Success;
    u32    push_end = 0;
    for (; push_end < desc.max_frames_in_flight; push_end++)
    {
      Result result = DeviceInterface::create_fence(self_, true);
      if (result.is_err())
      {
        status = result.err();
        break;
      }
      submit_fences[push_end] = result.value();
    }

    if (push_end != desc.max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        DeviceInterface::destroy_fence(self_, submit_fences[i]);
      }

      for (u32 i = 0; i < desc.max_frames_in_flight; i++)
      {
        self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                        nullptr);
        DeviceInterface::destroy_command_encoder(self_, command_encoders[i]);
      }

      return Err{(Status) status};
    }
  }

  VkSemaphore *submit_semaphores =
      self->allocator.allocate_typed<VkSemaphore>(desc.max_frames_in_flight);

  if (submit_semaphores == nullptr)
  {
    for (u32 i = 0; i < desc.max_frames_in_flight; i++)
    {
      DeviceInterface::destroy_fence(self_, submit_fences[i]);
      self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                      nullptr);
      DeviceInterface::destroy_command_encoder(self_, command_encoders[i]);
    }

    return Err{Status::OutOfHostMemory};
  }

  {
    VkResult              result   = VK_SUCCESS;
    u32                   push_end = 0;
    VkSemaphoreCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0};
    for (; push_end < desc.max_frames_in_flight; push_end++)
    {
      VkSemaphore semaphore;
      result = self->vk_table.CreateSemaphore(self->vk_device, &create_info,
                                              nullptr, &semaphore);
      if (result != VK_SUCCESS)
      {
        break;
      }
      submit_semaphores[push_end] = semaphore;
    }

    if (push_end != desc.max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        self->vk_table.DestroySemaphore(self->vk_device, submit_semaphores[i],
                                        nullptr);
      }

      for (u32 i = 0; i < desc.max_frames_in_flight; i++)
      {
        DeviceInterface::destroy_fence(self_, submit_fences[i]);
        self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                        nullptr);
        DeviceInterface::destroy_command_encoder(self_, command_encoders[i]);
      }

      return Err{(Status) result};
    }
  }

  FrameContext *ctx = self->allocator.allocate_typed<FrameContext>(1);

  if (ctx == nullptr)
  {
    for (u32 i = 0; i < desc.max_frames_in_flight; i++)
    {
      self->vk_table.DestroySemaphore(self->vk_device, submit_semaphores[i],
                                      nullptr);
      DeviceInterface::destroy_fence(self_, submit_fences[i]);
      self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                      nullptr);
      DeviceInterface::destroy_command_encoder(self_, command_encoders[i]);
    }

    return Err{Status::OutOfHostMemory};
  }

  new (ctx) FrameContext{.tail_frame           = 0,
                         .current_frame        = 0,
                         .ring_index           = 0,
                         .max_frames_in_flight = desc.max_frames_in_flight,
                         .encoders             = command_encoders,
                         .acquire_semaphores   = acquire_semaphores,
                         .submit_fences        = submit_fences,
                         .submit_semaphores    = submit_semaphores};

  return Ok{(gfx::FrameContext) ctx};
}

/// old swapchain will be retired and destroyed irregardless of whether new
/// swapchain recreation fails.
inline VkResult recreate_swapchain(Device *self, Swapchain *swapchain)
{
  VALIDATE(swapchain->desc.preferred_extent.x > 0);
  VALIDATE(swapchain->desc.preferred_extent.y > 0);
  VALIDATE(swapchain->desc.preferred_buffering <= gfx::MAX_SWAPCHAIN_IMAGES);

  VkSurfaceCapabilitiesKHR surface_capabilities;
  VkResult                 result =
      self->instance->vk_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(
          self->physical_device.vk_physical_device, swapchain->vk_surface,
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

  VALIDATE(has_bits(surface_capabilities.supportedUsageFlags,
                    (VkImageUsageFlags) swapchain->desc.usage));
  VALIDATE(has_bits(surface_capabilities.supportedCompositeAlpha,
                    (VkImageUsageFlags) swapchain->desc.composite_alpha));

  // take ownership of internal data for re-use/release
  VkSwapchainKHR old_vk_swapchain = swapchain->vk_swapchain;
  defer          old_vk_swapchain_del{[&] {
    if (old_vk_swapchain != nullptr)
    {
      self->vk_table.DestroySwapchainKHR(self->vk_device, old_vk_swapchain,
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

  result = self->vk_table.CreateSwapchainKHR(self->vk_device, &create_info,
                                             nullptr, &new_vk_swapchain);

  CHECK(result == VK_SUCCESS);

  defer new_vk_swapchain_del{[&] {
    if (new_vk_swapchain != nullptr)
    {
      self->vk_table.DestroySwapchainKHR(self->vk_device, new_vk_swapchain,
                                         nullptr);
    }
  }};

  u32 num_images;
  result = self->vk_table.GetSwapchainImagesKHR(
      self->vk_device, new_vk_swapchain, &num_images, nullptr);

  CHECK(result == VK_SUCCESS);

  CHECK(num_images <= gfx::MAX_SWAPCHAIN_IMAGES);

  result = self->vk_table.GetSwapchainImagesKHR(
      self->vk_device, new_vk_swapchain, &num_images, swapchain->vk_images);

  CHECK(result == VK_SUCCESS);

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

  VALIDATE(desc.preferred_extent.x > 0);
  VALIDATE(desc.preferred_extent.y > 0);

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

  self->vk_table.DestroyBufferView(self->vk_device, buffer_view->vk_view,
                                   nullptr);
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

  VALIDATE(!image->is_swapchain_image);

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

  self->vk_table.DestroyImageView(self->vk_device, image_view->vk_view,
                                  nullptr);
  self->allocator.deallocate_typed(image_view, 1);
}

void DeviceInterface::destroy_sampler(gfx::Device self_, gfx::Sampler sampler_)
{
  Device *const self = (Device *) self_;

  self->vk_table.DestroySampler(self->vk_device, (Sampler) sampler_, nullptr);
}

void DeviceInterface::destroy_shader(gfx::Device self_, gfx::Shader shader_)
{
  Device *const self = (Device *) self_;

  self->vk_table.DestroyShaderModule(self->vk_device, (Shader) shader_,
                                     nullptr);
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

  self->vk_table.DestroyRenderPass(self->vk_device, render_pass->vk_render_pass,
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

  self->vk_table.DestroyFramebuffer(self->vk_device,
                                    framebuffer->vk_framebuffer, nullptr);
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

  self->vk_table.DestroyDescriptorSetLayout(self->vk_device, layout->vk_layout,
                                            nullptr);
  self->allocator.deallocate_typed(layout->bindings, layout->num_bindings);
  self->allocator.deallocate_typed(layout, 1);
}

void DeviceInterface::destroy_descriptor_heap(gfx::Device             self_,
                                              gfx::DescriptorHeapImpl heap_)
{
  Device *const         self = (Device *) self_;
  DescriptorHeap *const heap = (DescriptorHeap *) heap_.self;

  if (heap == nullptr)
  {
    return;
  }

  heap->allocator.deallocate_typed(heap->binding_index_map,
                                   heap->set_layout->num_bindings);
  for (u32 i = 0; i < heap->num_pools; i++)
  {
    self->vk_table.DestroyDescriptorPool(self->vk_device, heap->pools[i],
                                         nullptr);
  }
  heap->allocator.deallocate_typed(heap->pools, heap->pools_capacity);
  heap->allocator.deallocate_typed(heap->sets, heap->sets_capacity);
  heap->allocator.deallocate_typed(heap->last_use_frame,
                                   heap->last_use_frame_capacity);
  heap->allocator.deallocate_typed(heap->released, heap->released_capacity);
  heap->allocator.deallocate_typed(heap->free, heap->free_capacity);
  heap->allocator.deallocate_typed(heap->images, heap->images_capacity);
  heap->allocator.deallocate_typed(heap->buffers, heap->buffers_capacity);
  heap->allocator.deallocate(MAX_STANDARD_ALIGNMENT, heap->scratch,
                             heap->scratch_size);
  self->allocator.deallocate_typed(heap, 1);
}

void DeviceInterface::destroy_pipeline_cache(gfx::Device        self_,
                                             gfx::PipelineCache cache_)
{
  Device *const self = (Device *) self_;
  self->vk_table.DestroyPipelineCache(self->vk_device, (PipelineCache) cache_,
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

  self->vk_table.DestroyPipeline(self->vk_device, pipeline->vk_pipeline,
                                 nullptr);
  self->vk_table.DestroyPipelineLayout(self->vk_device, pipeline->vk_layout,
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

  self->vk_table.DestroyPipeline(self->vk_device, pipeline->vk_pipeline,
                                 nullptr);
  self->vk_table.DestroyPipelineLayout(self->vk_device, pipeline->vk_layout,
                                       nullptr);
  self->allocator.deallocate_typed(pipeline, 1);
}

void DeviceInterface::destroy_fence(gfx::Device self_, gfx::Fence fence_)
{
  Device *const self = (Device *) self_;
  self->vk_table.DestroyFence(self->vk_device, (Fence) fence_, nullptr);
}

void CommandEncoder::reset_context()
{
  switch (state)
  {
    case CommandEncoderState::RenderPass:
      uninit_rp_context();
      break;

    case CommandEncoderState::ComputePass:
      uninit_cp_context();
      break;
    default:
      break;
  }
}

void CommandEncoder::init_rp_context()
{
  new (&ctx.rp) RenderPassContext{.commands = {allocator}};
  state = CommandEncoderState::RenderPass;
}

void CommandEncoder::uninit_rp_context()
{
  ctx.rp.commands.reset();
  state = CommandEncoderState::Begin;
}

void CommandEncoder::init_cp_context()
{
  new (&ctx.cp) ComputePassContext{};
  state = CommandEncoderState::ComputePass;
}

void CommandEncoder::uninit_cp_context()
{
  state = CommandEncoderState::Begin;
}

void DeviceInterface::destroy_command_encoder(gfx::Device             self_,
                                              gfx::CommandEncoderImpl encoder_)
{
  Device *const         self = (Device *) self_;
  CommandEncoder *const enc  = (CommandEncoder *) encoder_.self;

  if (enc == nullptr)
  {
    return;
  }
  enc->reset_context();

  self->vk_table.DestroyCommandPool(self->vk_device, enc->vk_command_pool,
                                    nullptr);
  self->allocator.deallocate_typed(enc, 1);
}

void DeviceInterface::destroy_frame_context(gfx::Device       self_,
                                            gfx::FrameContext frame_context_)
{
  Device *const       self = (Device *) self_;
  FrameContext *const ctx  = (FrameContext *) frame_context_;

  if (ctx == nullptr)
  {
    return;
  }

  for (u32 i = 0; i < ctx->max_frames_in_flight; i++)
  {
    DeviceInterface::destroy_command_encoder(self_, ctx->encoders[i]);
    self->vk_table.DestroySemaphore(self->vk_device, ctx->acquire_semaphores[i],
                                    nullptr);
    self->vk_table.DestroyFence(self->vk_device, (Fence) ctx->submit_fences[i],
                                nullptr);
    self->vk_table.DestroySemaphore(self->vk_device, ctx->submit_semaphores[i],
                                    nullptr);
  }
  self->allocator.deallocate_typed(ctx->acquire_semaphores,
                                   ctx->max_frames_in_flight);
  self->allocator.deallocate_typed(ctx->submit_fences,
                                   ctx->max_frames_in_flight);
  self->allocator.deallocate_typed(ctx->submit_semaphores,
                                   ctx->max_frames_in_flight);
  self->allocator.deallocate_typed(ctx, 1);
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

  self->vk_table.DestroySwapchainKHR(self->vk_device, swapchain->vk_swapchain,
                                     nullptr);
  self->allocator.deallocate_typed(swapchain, 1);
}

Result<void *, Status>
    DeviceInterface::get_buffer_memory_map(gfx::Device self_,
                                           gfx::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE(buffer->desc.host_mapped);

  return Ok{(void *) buffer->host_map};
}

Result<Void, Status> DeviceInterface::invalidate_buffer_memory_map(
    gfx::Device self_, gfx::Buffer buffer_, gfx::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE(buffer->desc.host_mapped);
  VALIDATE(range.offset < buffer->desc.size);
  VALIDATE(range.size == gfx::WHOLE_SIZE ||
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

  VALIDATE(buffer->desc.host_mapped);
  VALIDATE(range.offset < buffer->desc.size);
  VALIDATE(range.size == gfx::WHOLE_SIZE ||
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
      self->vk_device, (PipelineCache) cache, &size, nullptr);
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
      self->vk_device, (PipelineCache) cache, &size, out.data());
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

  VALIDATE(num_srcs > 0);

  VkResult result = self->vk_table.MergePipelineCaches(
      self->vk_device, (PipelineCache) dst, num_srcs,
      (PipelineCache *) srcs.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<Void, Status> DeviceInterface::wait_for_fences(
    gfx::Device self_, Span<gfx::Fence const> fences, bool all, u64 timeout)
{
  Device *const self       = (Device *) self_;
  u32 const     num_fences = (u32) fences.size();

  VALIDATE(num_fences > 0);

  VkResult result = self->vk_table.WaitForFences(self->vk_device, num_fences,
                                                 (Fence *) fences.data(),
                                                 (VkBool32) all, timeout);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<Void, Status>
    DeviceInterface::reset_fences(gfx::Device            self_,
                                  Span<gfx::Fence const> fences)
{
  Device *const self       = (Device *) self_;
  u32 const     num_fences = (u32) fences.size();

  VALIDATE(num_fences > 0);

  VkResult result = self->vk_table.ResetFences(self->vk_device, num_fences,
                                               (Fence *) fences.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<bool, Status> DeviceInterface::get_fence_status(gfx::Device self_,
                                                       gfx::Fence  fence)
{
  Device *const self = (Device *) self_;
  VkResult      result =
      self->vk_table.GetFenceStatus(self->vk_device, (Fence) fence);

  if (result == VK_SUCCESS)
  {
    return Ok{true};
  }

  if (result == VK_NOT_READY)
  {
    return Ok{false};
  }

  return Err{(Status) result};
}

Result<Void, Status> DeviceInterface::wait_idle(gfx::Device self_)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.DeviceWaitIdle(self->vk_device);
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

gfx::FrameInfo DeviceInterface::get_frame_info(gfx::Device,
                                               gfx::FrameContext frame_context_)
{
  FrameContext *const ctx = (FrameContext *) frame_context_;

  return gfx::FrameInfo{.tail    = ctx->tail_frame,
                        .current = ctx->current_frame,
                        .encoders =
                            Span{ctx->encoders, ctx->max_frames_in_flight},
                        .ring_index = ctx->ring_index};
}

Result<u32, Status> DeviceInterface::get_surface_formats(
    gfx::Device self_, gfx::Surface surface_, Span<gfx::SurfaceFormat> formats)
{
  Device *const      self    = (Device *) self_;
  VkSurfaceKHR const surface = (VkSurfaceKHR) surface_;

  u32      num_supported;
  VkResult result = self->instance->vk_table.GetPhysicalDeviceSurfaceFormatsKHR(
      self->physical_device.vk_physical_device, surface, &num_supported,
      nullptr);

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
        self->physical_device.vk_physical_device, surface, &num_supported,
        vk_formats);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      self->allocator.deallocate_typed(vk_formats, num_supported);
      return Err{(Status) result};
    }

    CHECK(num_read == num_supported && result != VK_INCOMPLETE);
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
          self->physical_device.vk_physical_device, surface, &num_supported,
          nullptr);

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
        self->physical_device.vk_physical_device, surface, &num_supported,
        vk_present_modes);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      self->allocator.deallocate_typed(vk_present_modes, num_supported);
      return Err{(Status) result};
    }

    CHECK(num_read == num_supported && result != VK_INCOMPLETE);
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
          self->physical_device.vk_physical_device, surface, &capabilities);

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
  VALIDATE(desc.preferred_extent.x > 0);
  VALIDATE(desc.preferred_extent.y > 0);
  Swapchain *const swapchain = (Swapchain *) swapchain_;
  swapchain->is_optimal      = false;
  swapchain->desc            = desc;
  return Ok{Void{}};
}

Result<Void, Status>
    DeviceInterface::begin_frame(gfx::Device       self_,
                                 gfx::FrameContext frame_context_,
                                 gfx::Swapchain    swapchain_)
{
  Device *const       self      = (Device *) self_;
  FrameContext *const ctx       = (FrameContext *) frame_context_;
  Swapchain *const    swapchain = (Swapchain *) swapchain_;
  VkResult            result    = VK_SUCCESS;
  Fence const submit_fence      = (Fence) ctx->submit_fences[ctx->ring_index];
  CommandEncoder *const enc =
      (CommandEncoder *) ctx->encoders[ctx->ring_index].self;

  VALIDATE(!enc->is_recording());

  result = self->vk_table.WaitForFences(self->vk_device, 1, &submit_fence,
                                        VK_TRUE, U64_MAX);

  CHECK(result == VK_SUCCESS);

  result = self->vk_table.ResetFences(self->vk_device, 1, &submit_fence);

  CHECK(result == VK_SUCCESS);

  if (swapchain->is_out_of_date || !swapchain->is_optimal ||
      swapchain->vk_swapchain == nullptr)
  {
    // await all pending submitted operations on the device possibly using
    // the swapchain, to avoid destroying whilst in use
    result = self->vk_table.DeviceWaitIdle(self->vk_device);
    CHECK(result == VK_SUCCESS);

    result = recreate_swapchain(self, swapchain);
    CHECK(result == VK_SUCCESS);
  }

  if (!swapchain->is_zero_sized)
  {
    u32 next_image;
    result = self->vk_table.AcquireNextImageKHR(
        self->vk_device, swapchain->vk_swapchain, U64_MAX,
        ctx->acquire_semaphores[ctx->ring_index], nullptr, &next_image);

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

  self->vk_table.ResetCommandBuffer(enc->vk_command_buffer, 0);

  enc->reset_context();

  VkCommandBufferBeginInfo info{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext            = nullptr,
      .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr};
  result = self->vk_table.BeginCommandBuffer(enc->vk_command_buffer, &info);
  CHECK(result == VK_SUCCESS);

  return Ok{Void{}};
}

Result<Void, Status>
    DeviceInterface::submit_frame(gfx::Device       self_,
                                  gfx::FrameContext frame_context_,
                                  gfx::Swapchain    swapchain_)
{
  Device *const       self      = (Device *) self_;
  FrameContext *const ctx       = (FrameContext *) frame_context_;
  Swapchain *const    swapchain = (Swapchain *) swapchain_;
  Fence const submit_fence      = (Fence) ctx->submit_fences[ctx->ring_index];
  CommandEncoder *const enc =
      (CommandEncoder *) ctx->encoders[ctx->ring_index].self;
  VkCommandBuffer const command_buffer = enc->vk_command_buffer;
  VkSemaphore const submit_semaphore = ctx->submit_semaphores[ctx->ring_index];
  VkSemaphore const acquire_semaphore =
      ctx->acquire_semaphores[ctx->ring_index];
  bool const was_acquired = !swapchain->is_zero_sized;
  bool const can_present =
      !(swapchain->is_out_of_date || swapchain->is_zero_sized);

  VALIDATE(enc->is_recording());

  if (was_acquired)
  {
    access_image(*enc, swapchain->image_impls[swapchain->current_image],
                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_NONE,
                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }

  VkResult result = self->vk_table.EndCommandBuffer(command_buffer);
  CHECK(result == VK_SUCCESS);
  CHECK(enc->status == gfx::Status::Success);

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

  enc->state = CommandEncoderState::End;

  CHECK(result == VK_SUCCESS);

  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands could have been executed.
  ctx->current_frame++;
  ctx->tail_frame =
      max(ctx->current_frame, (gfx::FrameId) ctx->max_frames_in_flight) -
      ctx->max_frames_in_flight;
  ctx->ring_index = (ctx->ring_index + 1) % ctx->max_frames_in_flight;

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

Result<u32, Status> DescriptorHeapInterface::allocate(gfx::DescriptorHeap self_)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  // if any free, claim
  if (self->num_free > 0)
  {
    u32 set = self->free[self->num_free - 1];
    self->num_free--;
    mem::zero(self->buffers + set * self->num_set_buffers,
              self->num_set_buffers);
    mem::zero(self->images + set * self->num_set_images, self->num_set_images);
    return Ok{(u32) set};
  }

  VkDescriptorPool *pools = self->allocator.reallocate_typed(
      self->pools, self->pools_capacity, self->num_pools + 1);

  if (pools == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  u32 prev_num_sets = self->num_pools * self->num_sets_per_pool;
  u32 num_sets      = (self->num_pools + 1) * self->num_sets_per_pool;

  self->pools          = pools;
  self->pools_capacity = self->num_pools + 1;

  VkDescriptorSet *sets = self->allocator.reallocate_typed(
      self->sets, self->sets_capacity, num_sets);

  if (sets == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->sets          = sets;
  self->sets_capacity = num_sets;

  VkDescriptorSet *new_sets = sets + self->num_pools * self->num_sets_per_pool;

  u64 *last_use_frame = self->allocator.reallocate_typed(
      self->last_use_frame, self->last_use_frame_capacity, num_sets);

  if (last_use_frame == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->last_use_frame          = last_use_frame;
  self->last_use_frame_capacity = num_sets;

  u32 *released = self->allocator.reallocate_typed(
      self->released, self->released_capacity, num_sets);

  if (released == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->released          = released;
  self->released_capacity = num_sets;

  u32 *free = self->allocator.reallocate_typed(self->free, self->free_capacity,
                                               num_sets);

  if (free == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->free          = free;
  self->free_capacity = num_sets;

  Image **images = self->allocator.reallocate_typed(
      self->images, self->images_capacity, num_sets * self->num_set_images);
  if (images == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->images          = images;
  self->images_capacity = num_sets * self->num_set_images;

  mem::zero(self->images + prev_num_sets * self->num_set_images,
            self->num_set_images * self->num_sets_per_pool);

  Buffer **buffers = self->allocator.reallocate_typed(
      self->buffers, self->buffers_capacity, num_sets * self->num_set_buffers);
  if (buffers == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->buffers          = buffers;
  self->buffers_capacity = num_sets * self->num_set_buffers;

  mem::zero(self->buffers + prev_num_sets * self->num_set_buffers,
            self->num_set_buffers * self->num_sets_per_pool);

  VkDescriptorPoolCreateInfo pool_create_info{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0,
      .maxSets       = self->num_sets_per_pool,
      .poolSizeCount = self->num_pool_sizes,
      .pPoolSizes    = self->pool_sizes};

  VkDescriptorPool vk_pool;
  VkResult         result = self->device->vk_table.CreateDescriptorPool(
      self->device->vk_device, &pool_create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  for (u32 i = 0; i < self->num_sets_per_pool; i++)
  {
    VkDescriptorSetAllocateInfo set_alloc_info{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = vk_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &self->set_layout->vk_layout};
    result = self->device->vk_table.AllocateDescriptorSets(
        self->device->vk_device, &set_alloc_info, new_sets + i);

    // must not have these errors
    CHECK(result != VK_ERROR_OUT_OF_POOL_MEMORY &&
          result != VK_ERROR_FRAGMENTED_POOL);

    if (result != VK_SUCCESS)
    {
      self->device->vk_table.DestroyDescriptorPool(self->device->vk_device,
                                                   vk_pool, nullptr);
      return Err{(Status) result};
    }
  }

  u32 const assigned           = prev_num_sets;
  self->pools[self->num_pools] = vk_pool;
  self->num_pools++;

  // fill the free sets in reverse order (i.e. [set 4, set 3, set 2])
  // as reclamation pulls from the end of the free groups. this helps with
  // predictability of indexes of newly allocated groups
  for (u32 free = self->num_pools * self->num_sets_per_pool - 1;
       free > assigned; free--, self->num_free++)
  {
    self->free[self->num_free] = free;
  }

  return Ok{(u32) assigned};
}

void DescriptorHeapInterface::collect(gfx::DescriptorHeap self_,
                                      gfx::FrameId        tail_frame)
{
  // move from released to free for all released groups not in use by the device
  DescriptorHeap *const self         = (DescriptorHeap *) self_;
  u32                   num_released = 0;
  for (u32 i = 0; i < self->num_released; i++)
  {
    if (self->last_use_frame[self->released[i]] < tail_frame)
    {
      self->free[self->num_free] = self->released[i];
      self->num_free++;
    }
    else
    {
      self->released[num_released] = self->released[i];
      num_released++;
    }
  }

  self->num_released = num_released;
}

void DescriptorHeapInterface::mark_in_use(gfx::DescriptorHeap self_, u32 set,
                                          gfx::FrameId current_frame)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(set < (self->num_pools * self->num_sets_per_pool));
  VALIDATE(self->last_use_frame[set] <= current_frame);

  self->last_use_frame[set] = current_frame;
}

bool DescriptorHeapInterface::is_in_use(gfx::DescriptorHeap self_, u32 set,
                                        gfx::FrameId tail_frame)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(set < (self->num_pools * self->num_sets_per_pool));

  return self->last_use_frame[set] >= tail_frame;
}

void DescriptorHeapInterface::release(gfx::DescriptorHeap self_, u32 set)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(set < (self->num_pools * self->num_sets_per_pool));
  VALIDATE((self->num_released + 1) <=
           (self->num_pools * self->num_sets_per_pool));

  self->released[self->num_released] = set;
  self->num_released++;
}

gfx::DescriptorHeapStats
    DescriptorHeapInterface::get_stats(gfx::DescriptorHeap self_)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  return gfx::DescriptorHeapStats{.num_allocated =
                                      self->num_pools * self->num_sets_per_pool,
                                  .num_free     = self->num_free,
                                  .num_released = self->num_released,
                                  .num_pools    = self->num_pools};
}

#define DESCRIPTOR_HEAP_PRELUDE(desc_type)                         \
  VALIDATE(set < (self->num_pools * self->num_sets_per_pool));     \
  VALIDATE(binding < self->set_layout->num_bindings);              \
  VALIDATE(self->set_layout->bindings[binding].type == desc_type); \
  VALIDATE(self->set_layout->bindings[binding].count == elements.size())

void DescriptorHeapInterface::sampler(gfx::DescriptorHeap self_, u32 set,
                                      u32                             binding,
                                      Span<gfx::SamplerBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::Sampler);

  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *) self->scratch;
  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::SamplerBinding const &element = elements[i];
    image_infos[i] =
        VkDescriptorImageInfo{.sampler     = (Sampler) element.sampler,
                              .imageView   = nullptr,
                              .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED};
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet           = self->sets[set],
                                .dstBinding       = binding,
                                .dstArrayElement  = 0,
                                .descriptorCount  = (u32) elements.size(),
                                .descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER,
                                .pImageInfo       = image_infos,
                                .pBufferInfo      = nullptr,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::combined_image_sampler(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::CombinedImageSamplerBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::CombinedImageSampler);

  for (gfx::CombinedImageSamplerBinding const &element : elements)
  {
    VALIDATE(has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::Sampled));
  }

  Image **images = self->images + set * self->num_set_images +
                   self->binding_index_map[binding];
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::CombinedImageSamplerBinding const &element = elements[i];
    image_infos[i]                                  = VkDescriptorImageInfo{
                                         .sampler     = (Sampler) element.sampler,
                                         .imageView   = ((ImageView *) element.image_view)->vk_view,
                                         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    images[i] = (Image *) ((ImageView *) (element.image_view))->desc.image;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                .pImageInfo       = image_infos,
                                .pBufferInfo      = nullptr,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::sampled_image(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::ImageBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::SampledImage);

  for (gfx::ImageBinding const &element : elements)
  {
    VALIDATE(has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::Sampled));
  }

  Image **images = self->images + set * self->num_set_images +
                   self->binding_index_map[binding];
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::ImageBinding const &element = elements[i];
    image_infos[i]                   = VkDescriptorImageInfo{
                          .sampler     = nullptr,
                          .imageView   = ((ImageView *) element.image_view)->vk_view,
                          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    images[i] = (Image *) ((ImageView *) (element.image_view))->desc.image;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                .pImageInfo       = image_infos,
                                .pBufferInfo      = nullptr,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::storage_image(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::ImageBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::StorageImage);

  for (gfx::ImageBinding const &element : elements)
  {
    VALIDATE(has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::Storage));
  }

  Image **images = self->images + set * self->num_set_buffers +
                   self->binding_index_map[binding];
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::ImageBinding const &element = elements[i];
    image_infos[i]                   = VkDescriptorImageInfo{
                          .sampler     = nullptr,
                          .imageView   = ((ImageView *) element.image_view)->vk_view,
                          .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
    images[i] = (Image *) ((ImageView *) (element.image_view))->desc.image;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                .pImageInfo       = image_infos,
                                .pBufferInfo      = nullptr,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::uniform_texel_buffer(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::TexelBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::UniformTexelBuffer);

  for (gfx::TexelBufferBinding const &element : elements)
  {
    // TODO(lamarrr): check alingment of buffer
    VALIDATE(has_bits(BUFFER_FROM_VIEW(element.buffer_view)->desc.usage,
                      gfx::BufferUsage::UniformTexelBuffer));
  }

  Buffer **buffers = self->buffers + set * self->num_set_buffers +
                     self->binding_index_map[binding];
  VkBufferView *buffer_views = (VkBufferView *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::TexelBufferBinding const &element = elements[i];
    buffer_views[i] = ((BufferView *) element.buffer_view)->vk_view;
    buffers[i] = (Buffer *) ((BufferView *) element.buffer_view)->desc.buffer;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
                                .pImageInfo       = nullptr,
                                .pBufferInfo      = nullptr,
                                .pTexelBufferView = buffer_views};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::storage_texel_buffer(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::TexelBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::StorageTexelBuffer);

  for (gfx::TexelBufferBinding const &element : elements)
  {
    VALIDATE(has_bits(BUFFER_FROM_VIEW(element.buffer_view)->desc.usage,
                      gfx::BufferUsage::StorageTexelBuffer));
    // TODO(lamarrr): check alingment of buffer
  }

  Buffer **buffers = self->buffers + set * self->num_set_buffers +
                     self->binding_index_map[binding];
  VkBufferView *buffer_views = (VkBufferView *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::TexelBufferBinding const &element = elements[i];
    buffers[i] = (Buffer *) ((BufferView *) element.buffer_view)->desc.buffer;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
                                .pImageInfo       = nullptr,
                                .pBufferInfo      = nullptr,
                                .pTexelBufferView = buffer_views};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::uniform_buffer(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::BufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::UniformBuffer);

  for (gfx::BufferBinding const &element : elements)
  {
    // TODO(lamarrr): check alingment of buffer
    VALIDATE(has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::UniformBuffer));
  }

  Buffer **buffers = self->buffers + set * self->num_set_buffers +
                     self->binding_index_map[binding];
  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::BufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
    buffers[i] = (Buffer *) element.buffer;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                .pImageInfo       = nullptr,
                                .pBufferInfo      = buffer_infos,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::storage_buffer(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::BufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::StorageBuffer);

  for (gfx::BufferBinding const &element : elements)
  {
    VALIDATE(has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::StorageBuffer));
    // TODO(lamarrr): max uniform buffer range
    VALIDATE((element.offset % self->device->physical_device.properties.limits
                                   .minStorageBufferOffsetAlignment) == 0);
  }

  Buffer **buffers = self->buffers + set * self->num_set_buffers +
                     self->binding_index_map[binding];
  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::BufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
    buffers[i] = (Buffer *) element.buffer;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                .pImageInfo       = nullptr,
                                .pBufferInfo      = buffer_infos,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::dynamic_uniform_buffer(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::BufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::DynamicUniformBuffer);
  for (gfx::BufferBinding const &element : elements)
  {
    // TODO(lamarrr): max uniform buffer range
    VALIDATE(has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::UniformBuffer));
    VALIDATE((element.offset % self->device->physical_device.properties.limits
                                   .minUniformBufferOffsetAlignment) == 0);
  }

  Buffer **buffers = self->buffers + set * self->num_set_buffers +
                     self->binding_index_map[binding];
  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::BufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
    buffers[i] = (Buffer *) element.buffer;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                .pImageInfo       = nullptr,
                                .pBufferInfo      = buffer_infos,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::dynamic_storage_buffer(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::BufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::DynamicStorageBuffer);

  for (gfx::BufferBinding const &element : elements)
  {
    // TODO(lamarrr): max uniform buffer range
    VALIDATE(has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::StorageBuffer));
    VALIDATE((element.offset % self->device->physical_device.properties.limits
                                   .minUniformBufferOffsetAlignment) == 0);
  }

  Buffer **buffers = self->buffers + set * self->num_set_buffers +
                     self->binding_index_map[binding];
  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::BufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
    buffers[i] = (Buffer *) element.buffer;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                .pImageInfo       = nullptr,
                                .pBufferInfo      = buffer_infos,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::input_attachment(
    gfx::DescriptorHeap self_, u32 set, u32 binding,
    Span<gfx::ImageBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  DESCRIPTOR_HEAP_PRELUDE(gfx::DescriptorType::InputAttachment);
  for (gfx::ImageBinding const &element : elements)
  {
    VALIDATE(has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::InputAttachment));
  }

  Image **images = self->images + set * self->num_set_images +
                   self->binding_index_map[binding];
  VkDescriptorImageInfo *image_infos = (VkDescriptorImageInfo *) self->scratch;

  for (u32 i = 0; i < elements.size(); i++)
  {
    gfx::ImageBinding const &element = elements[i];
    image_infos[i]                   = VkDescriptorImageInfo{
                          .sampler     = nullptr,
                          .imageView   = ((ImageView *) element.image_view)->vk_view,
                          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    images[i] = (Image *) ((ImageView *) element.image_view)->desc.image;
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = self->sets[set],
                                .dstBinding      = binding,
                                .dstArrayElement = 0,
                                .descriptorCount = (u32) elements.size(),
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                                .pImageInfo       = image_infos,
                                .pBufferInfo      = nullptr,
                                .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

#define ENCODER_PRELUDE()                                \
  CommandEncoder *const self = (CommandEncoder *) self_; \
  if (self->status != Status::Success)                   \
  {                                                      \
    return;                                              \
  }

void CommandEncoderInterface::begin_debug_marker(gfx::CommandEncoder self_,
                                                 Span<char const> region_name,
                                                 Vec4             color)
{
  ENCODER_PRELUDE();

  VALIDATE(region_name.size() < 256);
  char region_name_cstr[256];
  to_c_str(region_name, to_span(region_name_cstr));

  VkDebugMarkerMarkerInfoEXT info{
      .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
      .pNext       = nullptr,
      .pMarkerName = region_name_cstr,
      .color       = {color.x, color.y, color.z, color.w}};
  self->device->vk_table.CmdDebugMarkerBeginEXT(self->vk_command_buffer, &info);
}

void CommandEncoderInterface::end_debug_marker(gfx::CommandEncoder self_)
{
  ENCODER_PRELUDE();
  self->device->vk_table.CmdDebugMarkerEndEXT(self->vk_command_buffer);
}

void CommandEncoderInterface::fill_buffer(gfx::CommandEncoder self_,
                                          gfx::Buffer dst_, u64 offset,
                                          u64 size, u32 data)
{
  ENCODER_PRELUDE();
  Buffer *const dst = (Buffer *) dst_;

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  VALIDATE(is_valid_aligned_buffer_access(dst->desc.size, offset, size, 4));

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);
  self->device->vk_table.CmdFillBuffer(self->vk_command_buffer, dst->vk_buffer,
                                       offset, size, data);
}

void CommandEncoderInterface::copy_buffer(gfx::CommandEncoder self_,
                                          gfx::Buffer src_, gfx::Buffer dst_,
                                          Span<gfx::BufferCopy const> copies)
{
  ENCODER_PRELUDE();
  Buffer *const src        = (Buffer *) src_;
  Buffer *const dst        = (Buffer *) dst_;
  u32 const     num_copies = (u32) copies.size();

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(has_bits(src->desc.usage, gfx::BufferUsage::TransferSrc));
  VALIDATE(has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  VALIDATE(num_copies > 0);
  for (gfx::BufferCopy const &copy : copies)
  {
    VALIDATE(
        is_valid_buffer_access(src->desc.size, copy.src_offset, copy.size));
    VALIDATE(
        is_valid_buffer_access(dst->desc.size, copy.dst_offset, copy.size));
  }

  VkBufferCopy *vk_copies =
      self->allocator.allocate_typed<VkBufferCopy>(num_copies);

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

  self->device->vk_table.CmdCopyBuffer(self->vk_command_buffer, src->vk_buffer,
                                       dst->vk_buffer, num_copies, vk_copies);

  self->allocator.deallocate_typed(vk_copies, num_copies);
}

void CommandEncoderInterface::update_buffer(gfx::CommandEncoder self_,
                                            Span<u8 const> src, u64 dst_offset,
                                            gfx::Buffer dst_)
{
  ENCODER_PRELUDE();
  Buffer *const dst       = (Buffer *) dst_;
  u64 const     copy_size = src.size_bytes();

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  VALIDATE(
      is_valid_aligned_buffer_access(dst->desc.size, dst_offset, copy_size, 4));

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  self->device->vk_table.CmdUpdateBuffer(self->vk_command_buffer,
                                         dst->vk_buffer, dst_offset,
                                         (u64) src.size(), src.data());
}

void CommandEncoderInterface::clear_color_image(
    gfx::CommandEncoder self_, gfx::Image dst_, gfx::Color clear_color,
    Span<gfx::ImageSubresourceRange const> ranges)

{
  ENCODER_PRELUDE();
  Image *const dst        = (Image *) dst_;
  u32 const    num_ranges = (u32) ranges.size();

  static_assert(sizeof(gfx::Color) == sizeof(VkClearColorValue));
  VALIDATE(!self->is_in_render_pass());
  VALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  VALIDATE(num_ranges > 0);
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    VALIDATE(is_valid_image_subresource_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        range.aspects, range.first_mip_level, range.num_mip_levels,
        range.first_array_layer, range.num_array_layers));
  }

  VkImageSubresourceRange *vk_ranges =
      self->allocator.allocate_typed<VkImageSubresourceRange>(num_ranges);

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

  self->device->vk_table.CmdClearColorImage(
      self->vk_command_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_clear_color, num_ranges,
      vk_ranges);

  self->allocator.deallocate_typed(vk_ranges, num_ranges);
}

void CommandEncoderInterface::clear_depth_stencil_image(
    gfx::CommandEncoder self_, gfx::Image dst_,
    gfx::DepthStencil                      clear_depth_stencil,
    Span<gfx::ImageSubresourceRange const> ranges)

{
  ENCODER_PRELUDE();
  Image *const dst        = (Image *) dst_;
  u32 const    num_ranges = (u32) ranges.size();

  static_assert(sizeof(gfx::DepthStencil) == sizeof(VkClearDepthStencilValue));
  VALIDATE(!self->is_in_render_pass());
  VALIDATE(num_ranges > 0);
  VALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    VALIDATE(is_valid_image_subresource_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        range.aspects, range.first_mip_level, range.num_mip_levels,
        range.first_array_layer, range.num_array_layers));
  }

  VkImageSubresourceRange *vk_ranges =
      self->allocator.allocate_typed<VkImageSubresourceRange>(num_ranges);

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

  self->device->vk_table.CmdClearDepthStencilImage(
      self->vk_command_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_clear_depth_stencil, num_ranges,
      vk_ranges);

  self->allocator.deallocate_typed(vk_ranges, num_ranges);
}

void CommandEncoderInterface::copy_image(gfx::CommandEncoder self_,
                                         gfx::Image src_, gfx::Image dst_,
                                         Span<gfx::ImageCopy const> copies)
{
  ENCODER_PRELUDE();
  Image *const src        = (Image *) src_;
  Image *const dst        = (Image *) dst_;
  u32 const    num_copies = (u32) copies.size();

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(num_copies > 0);
  VALIDATE(has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  VALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::ImageCopy const &copy = copies[i];

    VALIDATE(is_valid_image_subresource_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        copy.src_layers.aspects, copy.src_layers.mip_level, 1,
        copy.src_layers.first_array_layer, copy.src_layers.num_array_layers));
    VALIDATE(is_valid_image_subresource_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        copy.dst_layers.aspects, copy.dst_layers.mip_level, 1,
        copy.dst_layers.first_array_layer, copy.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, copy.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.dst_layers.mip_level);
    VALIDATE(copy.extent.x > 0);
    VALIDATE(copy.extent.y > 0);
    VALIDATE(copy.extent.z > 0);
    VALIDATE(copy.src_offset.x <= src_extent.x);
    VALIDATE(copy.src_offset.y <= src_extent.y);
    VALIDATE(copy.src_offset.z <= src_extent.z);
    VALIDATE((copy.src_offset.x + copy.extent.x) <= src_extent.x);
    VALIDATE((copy.src_offset.y + copy.extent.x) <= src_extent.y);
    VALIDATE((copy.src_offset.z + copy.extent.x) <= src_extent.z);
    VALIDATE(copy.dst_offset.x <= dst_extent.x);
    VALIDATE(copy.dst_offset.y <= dst_extent.y);
    VALIDATE(copy.dst_offset.z <= dst_extent.z);
    VALIDATE((copy.dst_offset.x + copy.extent.x) <= dst_extent.x);
    VALIDATE((copy.dst_offset.y + copy.extent.x) <= dst_extent.y);
    VALIDATE((copy.dst_offset.z + copy.extent.x) <= dst_extent.z);
  }

  VkImageCopy *vk_copies =
      self->allocator.allocate_typed<VkImageCopy>(num_copies);

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

  self->device->vk_table.CmdCopyImage(
      self->vk_command_buffer, src->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);

  self->allocator.deallocate_typed(vk_copies, num_copies);
}

void CommandEncoderInterface::copy_buffer_to_image(
    gfx::CommandEncoder self_, gfx::Buffer src_, gfx::Image dst_,
    Span<gfx::BufferImageCopy const> copies)
{
  ENCODER_PRELUDE();
  Buffer *const src        = (Buffer *) src_;
  Image *const  dst        = (Image *) dst_;
  u32 const     num_copies = (u32) copies.size();

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(num_copies > 0);
  VALIDATE(has_bits(src->desc.usage, gfx::BufferUsage::TransferSrc));
  VALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferImageCopy const &copy = copies[i];
    VALIDATE(is_valid_buffer_access(src->desc.size, copy.buffer_offset,
                                    gfx::WHOLE_SIZE));

    VALIDATE(is_valid_image_subresource_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        copy.image_layers.aspects, copy.image_layers.mip_level, 1,
        copy.image_layers.first_array_layer,
        copy.image_layers.num_array_layers));

    VALIDATE(copy.image_extent.x > 0);
    VALIDATE(copy.image_extent.y > 0);
    VALIDATE(copy.image_extent.z > 0);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.image_layers.mip_level);
    VALIDATE(copy.image_extent.x <= dst_extent.x);
    VALIDATE(copy.image_extent.y <= dst_extent.y);
    VALIDATE(copy.image_extent.z <= dst_extent.z);
    VALIDATE((copy.image_offset.x + copy.image_extent.x) <= dst_extent.x);
    VALIDATE((copy.image_offset.y + copy.image_extent.y) <= dst_extent.y);
    VALIDATE((copy.image_offset.z + copy.image_extent.z) <= dst_extent.z);
  }

  VkBufferImageCopy *vk_copies =
      self->allocator.allocate_typed<VkBufferImageCopy>(num_copies);

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

  self->device->vk_table.CmdCopyBufferToImage(
      self->vk_command_buffer, src->vk_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);

  self->allocator.deallocate_typed(vk_copies, num_copies);
}

void CommandEncoderInterface::blit_image(gfx::CommandEncoder self_,
                                         gfx::Image src_, gfx::Image dst_,
                                         Span<gfx::ImageBlit const> blits,
                                         gfx::Filter                filter)
{
  ENCODER_PRELUDE();
  Image *const src       = (Image *) src_;
  Image *const dst       = (Image *) dst_;
  u32 const    num_blits = (u32) blits.size();

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(num_blits > 0);
  VALIDATE(has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  VALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_blits; i++)
  {
    gfx::ImageBlit const &blit = blits[i];

    VALIDATE(is_valid_image_subresource_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        blit.src_layers.aspects, blit.src_layers.mip_level, 1,
        blit.src_layers.first_array_layer, blit.src_layers.num_array_layers));

    VALIDATE(is_valid_image_subresource_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        blit.dst_layers.aspects, blit.dst_layers.mip_level, 1,
        blit.dst_layers.first_array_layer, blit.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, blit.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, blit.dst_layers.mip_level);
    VALIDATE(blit.src_offsets[0].x <= src_extent.x);
    VALIDATE(blit.src_offsets[0].y <= src_extent.y);
    VALIDATE(blit.src_offsets[0].z <= src_extent.z);
    VALIDATE(blit.src_offsets[1].x <= src_extent.x);
    VALIDATE(blit.src_offsets[1].y <= src_extent.y);
    VALIDATE(blit.src_offsets[1].z <= src_extent.z);
    VALIDATE(blit.dst_offsets[0].x <= dst_extent.x);
    VALIDATE(blit.dst_offsets[0].y <= dst_extent.y);
    VALIDATE(blit.dst_offsets[0].z <= dst_extent.z);
    VALIDATE(blit.dst_offsets[1].x <= dst_extent.x);
    VALIDATE(blit.dst_offsets[1].y <= dst_extent.y);
    VALIDATE(blit.dst_offsets[1].z <= dst_extent.z);
    VALIDATE(!((src->desc.type == gfx::ImageType::Type1D) &&
               (blit.src_offsets[0].y != 0 | blit.src_offsets[1].y != 1)));
    VALIDATE(!((src->desc.type == gfx::ImageType::Type1D ||
                src->desc.type == gfx::ImageType::Type2D) &&
               (blit.src_offsets[0].z != 0 | blit.src_offsets[1].z != 1)));
    VALIDATE(!((dst->desc.type == gfx::ImageType::Type1D) &&
               (blit.dst_offsets[0].y != 0 | blit.dst_offsets[1].y != 1)));
    VALIDATE(!((dst->desc.type == gfx::ImageType::Type1D ||
                dst->desc.type == gfx::ImageType::Type2D) &&
               (blit.src_offsets[0].z != 0 | blit.dst_offsets[1].z != 1)));
  }

  VkImageBlit *vk_blits =
      self->allocator.allocate_typed<VkImageBlit>(num_blits);

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
  self->device->vk_table.CmdBlitImage(self->vk_command_buffer, src->vk_image,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      dst->vk_image,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      num_blits, vk_blits, (VkFilter) filter);

  self->allocator.deallocate_typed(vk_blits, num_blits);
}

void CommandEncoderInterface::resolve_image(
    gfx::CommandEncoder self_, gfx::Image src_, gfx::Image dst_,
    Span<gfx::ImageResolve const> resolves)
{
  ENCODER_PRELUDE();
  Image *const src          = (Image *) src_;
  Image *const dst          = (Image *) dst_;
  u32 const    num_resolves = (u32) resolves.size();

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(num_resolves > 0);
  VALIDATE(has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  VALIDATE(has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  VALIDATE(has_bits(dst->desc.sample_count, gfx::SampleCount::Count1));

  for (u32 i = 0; i < num_resolves; i++)
  {
    gfx::ImageResolve const &resolve = resolves[i];

    VALIDATE(is_valid_image_subresource_access(
        src->desc.aspects, src->desc.mip_levels, src->desc.array_layers,
        resolve.src_layers.aspects, resolve.src_layers.mip_level, 1,
        resolve.src_layers.first_array_layer,
        resolve.src_layers.num_array_layers));
    VALIDATE(is_valid_image_subresource_access(
        dst->desc.aspects, dst->desc.mip_levels, dst->desc.array_layers,
        resolve.dst_layers.aspects, resolve.dst_layers.mip_level, 1,
        resolve.dst_layers.first_array_layer,
        resolve.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, resolve.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, resolve.dst_layers.mip_level);
    VALIDATE(resolve.extent.x > 0);
    VALIDATE(resolve.extent.y > 0);
    VALIDATE(resolve.extent.z > 0);
    VALIDATE(resolve.src_offset.x <= src_extent.x);
    VALIDATE(resolve.src_offset.y <= src_extent.y);
    VALIDATE(resolve.src_offset.z <= src_extent.z);
    VALIDATE((resolve.src_offset.x + resolve.extent.x) <= src_extent.x);
    VALIDATE((resolve.src_offset.y + resolve.extent.x) <= src_extent.y);
    VALIDATE((resolve.src_offset.z + resolve.extent.x) <= src_extent.z);
    VALIDATE(resolve.dst_offset.x <= dst_extent.x);
    VALIDATE(resolve.dst_offset.y <= dst_extent.y);
    VALIDATE(resolve.dst_offset.z <= dst_extent.z);
    VALIDATE((resolve.dst_offset.x + resolve.extent.x) <= dst_extent.x);
    VALIDATE((resolve.dst_offset.y + resolve.extent.x) <= dst_extent.y);
    VALIDATE((resolve.dst_offset.z + resolve.extent.x) <= dst_extent.z);
  }

  VkImageResolve *vk_resolves =
      self->allocator.allocate_typed<VkImageResolve>(num_resolves);

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

  self->device->vk_table.CmdResolveImage(
      self->vk_command_buffer, src->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_resolves, vk_resolves);

  self->allocator.deallocate_typed(vk_resolves, num_resolves);
}

void CommandEncoderInterface::begin_render_pass(
    gfx::CommandEncoder self_, gfx::Framebuffer framebuffer_,
    gfx::RenderPass render_pass_, gfx::Offset render_offset,
    gfx::Extent                   render_extent,
    Span<gfx::Color const>        color_attachments_clear_values,
    Span<gfx::DepthStencil const> depth_stencil_attachment_clear_value)
{
  ENCODER_PRELUDE();
  Framebuffer *const framebuffer = (Framebuffer *) framebuffer_;
  RenderPass *const  render_pass = (RenderPass *) render_pass_;
  u32 const          num_color_clear_values =
      (u32) color_attachments_clear_values.size();
  u32 const num_depth_clear_values =
      (u32) depth_stencil_attachment_clear_value.size();

  VALIDATE(!self->is_in_render_pass());
  VALIDATE(num_depth_clear_values == 0 || num_depth_clear_values == 1);
  VALIDATE(is_render_pass_compatible(
      render_pass,
      Span{framebuffer->color_attachments, framebuffer->num_color_attachments},
      framebuffer->depth_stencil_attachment));
  VALIDATE(color_attachments_clear_values.size() <=
           framebuffer->num_color_attachments);
  VALIDATE(render_extent.x > 0);
  VALIDATE(render_extent.y > 0);
  VALIDATE(render_offset.x <= framebuffer->extent.x);
  VALIDATE(render_offset.y <= framebuffer->extent.y);
  VALIDATE((render_offset.x + render_extent.x) <= framebuffer->extent.x);
  VALIDATE((render_offset.y + render_extent.y) <= framebuffer->extent.y);

  self->init_rp_context();
  self->ctx.rp.offset      = render_offset;
  self->ctx.rp.extent      = render_extent;
  self->ctx.rp.render_pass = render_pass;
  self->ctx.rp.framebuffer = framebuffer;
  mem::copy(color_attachments_clear_values, self->ctx.rp.color_clear_values);
  self->ctx.rp.depth_stencil_clear_value =
      (num_depth_clear_values == 0) ? gfx::DepthStencil{} :
                                      depth_stencil_attachment_clear_value[0];
  self->ctx.rp.num_color_clear_values         = num_color_clear_values;
  self->ctx.rp.num_depth_stencil_clear_values = num_depth_clear_values;
}

void CommandEncoderInterface::end_render_pass(gfx::CommandEncoder self_)
{
  ENCODER_PRELUDE();

  VALIDATE(self->is_in_render_pass());
  RenderPassContext &ctx = self->ctx.rp;

  // TODO(lamarrr): memory consumption for render command is just too much,
  // reduce. we can have SOA? and parameters also specified in another SOA, and
  // indexes to iterate through
  for (RenderCommand const &cmd : ctx.commands)
  {
    switch (cmd.type)
    {
      case RenderCommandType::BindDescriptorSet:
      {
        for (u8 i = 0; i < cmd.set.v2; i++)
        {
          access_graphics_bindings(*self, cmd.set.v0[i]);
        }
      }
      break;
      case RenderCommandType::BindVertexBuffer:
      {
        access_buffer(*self, *cmd.vertex_buffer.v1,
                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
      }
      break;
      case RenderCommandType::BindIndexBuffer:
      {
        access_buffer(*self, *cmd.index_buffer.v0,
                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_INDEX_READ_BIT);
      }
      break;
      case RenderCommandType::DrawIndirect:
      case RenderCommandType::DrawIndexedIndirect:
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
    access_image(*self, *IMAGE_FROM_VIEW(ctx.framebuffer->color_attachments[i]),
                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                 color_attachment_image_access(
                     self->ctx.rp.render_pass->color_attachments[i]),
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
    VkClearValue vk_clear_values[gfx::MAX_COLOR_ATTACHMENTS + 1];

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

    self->device->vk_table.CmdBeginRenderPass(
        self->vk_command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  }

  GraphicsPipeline *pipeline = nullptr;

  for (RenderCommand const &cmd : self->ctx.rp.commands)
  {
    switch (cmd.type)
    {
      case RenderCommandType::BindDescriptorSet:
      {
        VkDescriptorSet vk_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
        for (u32 iset = 0; iset < cmd.set.v2; iset++)
        {
          gfx::DescriptorSet set  = cmd.set.v0[iset];
          DescriptorHeap    *heap = (DescriptorHeap *) set.heap;
          vk_sets[iset]           = heap->sets[set.index];
        }

        self->device->vk_table.CmdBindDescriptorSets(
            self->vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->vk_layout, 0, cmd.set.v2, vk_sets, cmd.set.v3,
            cmd.set.v1);
      }
      break;
      case RenderCommandType::BindPipeline:
      {
        pipeline = cmd.pipeline;
        self->device->vk_table.CmdBindPipeline(self->vk_command_buffer,
                                               VK_PIPELINE_BIND_POINT_GRAPHICS,
                                               pipeline->vk_pipeline);
      }
      break;
      case RenderCommandType::PushConstants:
      {
        self->device->vk_table.CmdPushConstants(
            self->vk_command_buffer, pipeline->vk_layout, VK_SHADER_STAGE_ALL,
            0, pipeline->push_constant_size, cmd.push_constant);
      }
      break;
      case RenderCommandType::SetViewport:
      {
        VkViewport vk_viewport{.x        = cmd.viewport.offset.x,
                               .y        = cmd.viewport.offset.y,
                               .width    = cmd.viewport.extent.x,
                               .height   = cmd.viewport.extent.y,
                               .minDepth = cmd.viewport.min_depth,
                               .maxDepth = cmd.viewport.max_depth};
        self->device->vk_table.CmdSetViewport(self->vk_command_buffer, 0, 1,
                                              &vk_viewport);
      }
      break;
      case RenderCommandType::SetScissor:
      {
        VkRect2D vk_scissor{
            .offset =
                VkOffset2D{(i32) cmd.scissor.v0.x, (i32) cmd.scissor.v0.y},
            .extent = VkExtent2D{cmd.scissor.v1.x, cmd.scissor.v1.y}};
        self->device->vk_table.CmdSetScissor(self->vk_command_buffer, 0, 1,
                                             &vk_scissor);
      }
      break;
      case RenderCommandType::SetBlendConstant:
      {
        f32 vk_constant[4] = {cmd.blend_constant.x, cmd.blend_constant.y,
                              cmd.blend_constant.z, cmd.blend_constant.w};
        self->device->vk_table.CmdSetBlendConstants(self->vk_command_buffer,
                                                    vk_constant);
      }
      break;
      case RenderCommandType::SetStencilCompareMask:
      {
        self->device->vk_table.CmdSetStencilCompareMask(
            self->vk_command_buffer, (VkStencilFaceFlags) cmd.stencil.v0,
            cmd.stencil.v1);
      }
      break;
      case RenderCommandType::SetStencilReference:
      {
        self->device->vk_table.CmdSetStencilReference(
            self->vk_command_buffer, (VkStencilFaceFlags) cmd.stencil.v0,
            cmd.stencil.v1);
      }
      break;
      case RenderCommandType::SetStencilWriteMask:
      {
        self->device->vk_table.CmdSetStencilWriteMask(
            self->vk_command_buffer, (VkStencilFaceFlags) cmd.stencil.v0,
            cmd.stencil.v1);
      }
      break;
      case RenderCommandType::BindVertexBuffer:
      {
        self->device->vk_table.CmdBindVertexBuffers(
            self->vk_command_buffer, cmd.vertex_buffer.v0, 1,
            &cmd.vertex_buffer.v1->vk_buffer, &cmd.vertex_buffer.v2);
      }
      break;
      case RenderCommandType::BindIndexBuffer:
      {
        self->device->vk_table.CmdBindIndexBuffer(
            self->vk_command_buffer, cmd.index_buffer.v0->vk_buffer,
            cmd.index_buffer.v1, (VkIndexType) cmd.index_buffer.v2);
      }
      break;
      case RenderCommandType::Draw:
      {
        self->device->vk_table.CmdDraw(
            self->vk_command_buffer, cmd.draw_indexed.v0, cmd.draw_indexed.v1,
            cmd.draw_indexed.v2, cmd.draw_indexed.v3);
      }
      break;
      case RenderCommandType::DrawIndexed:
      {
        self->device->vk_table.CmdDrawIndexed(
            self->vk_command_buffer, cmd.draw_indexed.v0, cmd.draw_indexed.v1,
            cmd.draw_indexed.v2, cmd.draw_indexed.v3, cmd.draw_indexed.v4);
      }
      break;
      case RenderCommandType::DrawIndirect:
      {
        self->device->vk_table.CmdDrawIndirect(
            self->vk_command_buffer, cmd.draw_indirect.v0->vk_buffer,
            cmd.draw_indirect.v1, cmd.draw_indirect.v2, cmd.draw_indirect.v3);
      }
      break;
      case RenderCommandType::DrawIndexedIndirect:
      {
        self->device->vk_table.CmdDrawIndexedIndirect(
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

  self->device->vk_table.CmdEndRenderPass(self->vk_command_buffer);
  self->reset_context();
}

void CommandEncoderInterface::bind_compute_pipeline(
    gfx::CommandEncoder self_, gfx::ComputePipeline pipeline)
{
  ENCODER_PRELUDE();
  VALIDATE(!self->is_in_render_pass());
  self->init_cp_context();
  self->ctx.cp.pipeline = (ComputePipeline *) pipeline;

  self->device->vk_table.CmdBindPipeline(self->vk_command_buffer,
                                         VK_PIPELINE_BIND_POINT_COMPUTE,
                                         self->ctx.cp.pipeline->vk_pipeline);
}

void CommandEncoderInterface::bind_graphics_pipeline(
    gfx::CommandEncoder self_, gfx::GraphicsPipeline pipeline_)
{
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());
  GraphicsPipeline *pipeline = (GraphicsPipeline *) pipeline_;
  self->ctx.rp.pipeline      = pipeline;
  if (!self->ctx.rp.commands.push(RenderCommand{
          .type = RenderCommandType::BindPipeline, .pipeline = pipeline}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::bind_descriptor_sets(
    gfx::CommandEncoder self_, Span<gfx::DescriptorSet const> descriptor_sets,
    Span<u32 const> dynamic_offsets)
{
  ENCODER_PRELUDE();
  u32 const num_sets            = (u32) descriptor_sets.size();
  u32 const num_dynamic_offsets = (u32) dynamic_offsets.size();
  VALIDATE((self->is_in_render_pass() && self->ctx.rp.pipeline != nullptr) ||
           (self->is_in_compute_pass() && self->ctx.cp.pipeline != nullptr));
  VALIDATE(num_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);

  // TODO(lamarrr): check for dynamic storage, uniform
  for (u32 offset : dynamic_offsets)
  {
    VALIDATE((offset % self->device->physical_device.properties.limits
                           .minUniformBufferOffsetAlignment) == 0);
  }

  for (gfx::DescriptorSet set : descriptor_sets)
  {
    DescriptorHeap *heap = (DescriptorHeap *) set.heap;
    VALIDATE(set.index < heap->num_pools * heap->num_sets_per_pool);
    for (u32 i = 0; i < heap->set_layout->num_bindings; i++)
    {
      gfx::DescriptorType binding_type = heap->set_layout->bindings[i].type;
      if (self->is_in_compute_pass())
      {
        VALIDATE(binding_type != gfx::DescriptorType::InputAttachment);
      }
    }
  }

  if (self->is_in_compute_pass())
  {
    VkDescriptorSet vk_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
    for (u32 iset = 0; iset < num_sets; iset++)
    {
      gfx::DescriptorSet set  = descriptor_sets[iset];
      DescriptorHeap    *heap = (DescriptorHeap *) set.heap;
      vk_sets[iset]           = heap->sets[set.index];
      self->ctx.cp.sets[iset] = set;
    }
    self->ctx.cp.num_sets = num_sets;

    self->device->vk_table.CmdBindDescriptorSets(
        self->vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        self->ctx.cp.pipeline->vk_layout, 0, num_sets, vk_sets,
        num_dynamic_offsets, dynamic_offsets.data());
  }
  else if (self->is_in_render_pass())
  {
    RenderCommand cmd{.type = RenderCommandType::BindDescriptorSet};
    mem::copy(descriptor_sets, cmd.set.v0);
    mem::copy(dynamic_offsets, cmd.set.v1);
    cmd.set.v2 = num_sets;
    cmd.set.v3 = num_dynamic_offsets;
    if (!self->ctx.rp.commands.push(cmd))
    {
      self->status = Status::OutOfHostMemory;
      return;
    }
  }
  else
  {
    UNREACHABLE();
  }
}

void CommandEncoderInterface::push_constants(gfx::CommandEncoder self_,
                                             Span<u8 const> push_constants_data)
{
  ENCODER_PRELUDE();
  VALIDATE((self->is_in_render_pass() && self->ctx.rp.pipeline != nullptr) ||
           (self->is_in_compute_pass() && self->ctx.cp.pipeline != nullptr));
  VALIDATE(push_constants_data.size_bytes() <= gfx::MAX_PUSH_CONSTANT_SIZE);
  VALIDATE(push_constants_data.size_bytes() % 4 == 0);

  if (self->is_in_compute_pass())
  {
    VALIDATE(push_constants_data.size() ==
             self->ctx.cp.pipeline->push_constant_size);
    self->device->vk_table.CmdPushConstants(
        self->vk_command_buffer, self->ctx.cp.pipeline->vk_layout,
        VK_SHADER_STAGE_ALL, 0, self->ctx.rp.pipeline->push_constant_size,
        push_constants_data.data());
  }
  else if (self->is_in_render_pass())
  {
    VALIDATE(push_constants_data.size() ==
             self->ctx.rp.pipeline->push_constant_size);
    RenderCommand cmd{.type = RenderCommandType::PushConstants};
    mem::copy(push_constants_data, cmd.push_constant);
    if (!self->ctx.rp.commands.push(cmd))
    {
      self->status = Status::OutOfHostMemory;
      return;
    }
  }
  else
  {
    UNREACHABLE();
  }
}

void CommandEncoderInterface::dispatch(gfx::CommandEncoder self_,
                                       u32 group_count_x, u32 group_count_y,
                                       u32 group_count_z)
{
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_compute_pass() && self->ctx.cp.pipeline != nullptr);
  VALIDATE(group_count_x <= gfx::MAX_COMPUTE_GROUP_COUNT_X);
  VALIDATE(group_count_y <= gfx::MAX_COMPUTE_GROUP_COUNT_Y);
  VALIDATE(group_count_z <= gfx::MAX_COMPUTE_GROUP_COUNT_Z);

  for (u32 i = 0; i < self->ctx.cp.num_sets; i++)
  {
    access_compute_bindings(*self, self->ctx.cp.sets[i]);
  }

  self->device->vk_table.CmdDispatch(self->vk_command_buffer, group_count_x,
                                     group_count_y, group_count_z);
}

void CommandEncoderInterface::dispatch_indirect(gfx::CommandEncoder self_,
                                                gfx::Buffer buffer_, u64 offset)
{
  ENCODER_PRELUDE();
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE(self->is_in_compute_pass() && self->ctx.cp.pipeline != nullptr);
  VALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  VALIDATE(is_valid_aligned_buffer_access(buffer->desc.size, offset,
                                          sizeof(gfx::DispatchCommand), 4));

  for (u32 i = 0; i < self->ctx.cp.num_sets; i++)
  {
    access_compute_bindings(*self, self->ctx.cp.sets[i]);
  }

  self->device->vk_table.CmdDispatchIndirect(self->vk_command_buffer,
                                             buffer->vk_buffer, offset);
}

void CommandEncoderInterface::set_viewport(gfx::CommandEncoder  self_,
                                           gfx::Viewport const &viewport)
{
  ENCODER_PRELUDE();

  VALIDATE(self->is_in_render_pass());
  VALIDATE(viewport.min_depth >= 0.0F);
  VALIDATE(viewport.max_depth <= 1.0F);

  if (!self->ctx.rp.commands.push(RenderCommand{
          .type = RenderCommandType::SetViewport, .viewport = viewport}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_scissor(gfx::CommandEncoder self_,
                                          gfx::Offset         scissor_offset,
                                          gfx::Extent         scissor_extent)
{
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type    = RenderCommandType::SetScissor,
                        .scissor = {scissor_offset, scissor_extent}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_blend_constants(gfx::CommandEncoder self_,
                                                  Vec4 blend_constant)
{
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type           = RenderCommandType::SetBlendConstant,
                        .blend_constant = blend_constant}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoderInterface::set_stencil_compare_mask(
    gfx::CommandEncoder self_, gfx::StencilFaces faces, u32 mask)
{
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type    = RenderCommandType::SetStencilCompareMask,
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type    = RenderCommandType::SetStencilReference,
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type    = RenderCommandType::SetStencilWriteMask,
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());
  u32 const num_vertex_buffers = (u32) vertex_buffers.size();
  VALIDATE(num_vertex_buffers > 0);
  VALIDATE(num_vertex_buffers <= gfx::MAX_VERTEX_ATTRIBUTES);
  VALIDATE(offsets.size() == vertex_buffers.size());
  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    u64 const     offset = offsets[i];
    Buffer *const buffer = (Buffer *) vertex_buffers[i];
    VALIDATE(offset < buffer->desc.size);
    VALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::VertexBuffer));
  }

  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    if (!self->ctx.rp.commands.push(RenderCommand{
            .type          = RenderCommandType::BindVertexBuffer,
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());
  Buffer *const index_buffer = (Buffer *) index_buffer_;
  u64 const     index_size   = index_type_size(index_type);

  VALIDATE(offset < index_buffer->desc.size);
  VALIDATE((offset % index_size) == 0);
  VALIDATE(has_bits(index_buffer->desc.usage, gfx::BufferUsage::IndexBuffer));

  self->ctx.rp.index_buffer        = index_buffer;
  self->ctx.rp.index_type          = index_type;
  self->ctx.rp.index_buffer_offset = offset;
  if (!self->ctx.rp.commands.push(
          RenderCommand{.type         = RenderCommandType::BindIndexBuffer,
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());
  VALIDATE(self->ctx.rp.pipeline != nullptr);

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type = RenderCommandType::Draw,
                        .draw = {vertex_count, instance_count, first_vertex_id,
                                 first_instance_id}}))
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());
  VALIDATE(self->ctx.rp.pipeline != nullptr);
  VALIDATE(self->ctx.rp.index_buffer != nullptr);
  u64 const index_size = index_type_size(self->ctx.rp.index_type);
  VALIDATE((self->ctx.rp.index_buffer_offset + first_index * index_size) <
           self->ctx.rp.index_buffer->desc.size);
  VALIDATE((self->ctx.rp.index_buffer_offset +
            (first_index + num_indices) * index_size) <=
           self->ctx.rp.index_buffer->desc.size);

  if (!self->ctx.rp.commands.push(RenderCommand{
          .type         = RenderCommandType::DrawIndexed,
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE(self->ctx.rp.pipeline != nullptr);
  VALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  VALIDATE(offset < buffer->desc.size);
  VALIDATE((offset + (u64) draw_count * stride) <= buffer->desc.size);
  VALIDATE(stride % 4 == 0);
  VALIDATE(stride >= sizeof(gfx::DrawCommand));

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type          = RenderCommandType::DrawIndirect,
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
  ENCODER_PRELUDE();
  VALIDATE(self->is_in_render_pass());
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE(self->ctx.rp.pipeline != nullptr);
  VALIDATE(self->ctx.rp.index_buffer != nullptr);
  VALIDATE(has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  VALIDATE(offset < buffer->desc.size);
  VALIDATE((offset + (u64) draw_count * stride) <= buffer->desc.size);
  VALIDATE(stride % 4 == 0);
  VALIDATE(stride >= sizeof(gfx::DrawIndexedCommand));

  if (!self->ctx.rp.commands.push(
          RenderCommand{.type          = RenderCommandType::DrawIndexedIndirect,
                        .draw_indirect = {buffer, offset, draw_count, stride}}))
  {
    self->status = Status::OutOfHostMemory;
    return;
  }
}

}        // namespace vk
}        // namespace ash
