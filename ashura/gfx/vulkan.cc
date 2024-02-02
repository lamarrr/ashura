#include "ashura/gfx/vulkan.h"
#include "ashura/std/algorithms.h"
#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/source_location.h"
#include "vulkan/vulkan.h"
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

#define CHECK_WITH(logger, description, ...) \
  PANIC_IF(logger, "Check: " description " Failed", __VA_ARGS__)

// TODO(lamarrr): tracer for vma memory allocations
// trace u64, i64, f64 etc
#define VALIDATE(obj, description, ...) \
  PANIC_IF((obj)->logger, "Validation: " description " Failed", __VA_ARGS__)

#define CHECK(obj, description, ...) \
  PANIC_IF((obj)->logger, "Check: " description " Failed", __VA_ARGS__)

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
  }

#undef LOAD_VK

  return all_loaded;
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

inline void access_buffer(CommandEncoder const &encoder, Buffer &buffer,
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
    encoder.device->vk_table.CmdPipelineBarrier(
        encoder.vk_command_buffer, src_stages, dst_stages, 0, 0, nullptr, 1,
        &barrier, 0, nullptr);
  }
}

inline void access_image(CommandEncoder const &encoder, Image &image,
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
    encoder.device->vk_table.CmdPipelineBarrier(
        encoder.vk_command_buffer, src_stages, dst_stages, 0, 0, nullptr, 0,
        nullptr, 1, &barrier);
  }
}

inline void access_compute_bindings(CommandEncoder &encoder)
{
  for (u32 i = 0; i < encoder.num_bound_descriptor_sets; i++)
  {
    DescriptorHeap const *heap  = encoder.bound_descriptor_set_heaps[i];
    u32 const             set   = encoder.bound_descriptor_sets[i];
    u32 const             group = encoder.bound_descriptor_set_groups[i];
    DescriptorSetLayout const *const layout = heap->set_layouts[set];

    for (u32 ibinding = 0; ibinding < layout->num_bindings; ibinding++)
    {
      gfx::DescriptorBindingDesc const &binding = layout->bindings[ibinding];
      switch (binding.type)
      {
        case gfx::DescriptorType::CombinedImageSampler:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::CombinedImageSamplerBinding const *bindings =
              (gfx::CombinedImageSamplerBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_image(
                encoder, *IMAGE_FROM_VIEW(bindings[ielement].image_view),
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

        case gfx::DescriptorType::SampledImage:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::SampledImageBinding const *bindings =
              (gfx::SampledImageBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_image(
                encoder, *IMAGE_FROM_VIEW(bindings[ielement].image_view),
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

        case gfx::DescriptorType::StorageImage:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::StorageImageBinding const *bindings =
              (gfx::StorageImageBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_image(encoder,
                         *IMAGE_FROM_VIEW(bindings[ielement].image_view),
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
                         VK_IMAGE_LAYOUT_GENERAL);
          }
        }
        break;

        case gfx::DescriptorType::UniformTexelBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::UniformTexelBufferBinding const *bindings =
              (gfx::UniformTexelBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder,
                          *BUFFER_FROM_VIEW(bindings[ielement].buffer_view),
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::StorageTexelBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::StorageTexelBufferBinding const *bindings =
              (gfx::StorageTexelBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(
                encoder, *BUFFER_FROM_VIEW(bindings[ielement].buffer_view),
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::UniformBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::UniformBufferBinding const *bindings =
              (gfx::UniformBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder, *((Buffer *) bindings[ielement].buffer),
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::StorageBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::StorageBufferBinding const *bindings =
              (gfx::StorageBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder, *((Buffer *) bindings[ielement].buffer),
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_WRITE_BIT |
                              VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::DynamicUniformBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::DynamicUniformBufferBinding const *bindings =
              (gfx::DynamicUniformBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder, *((Buffer *) bindings[ielement].buffer),
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::DynamicStorageBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::DynamicStorageBufferBinding const *bindings =
              (gfx::DynamicStorageBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder, *((Buffer *) bindings[ielement].buffer),
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                          VK_ACCESS_SHADER_WRITE_BIT |
                              VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        default:
          break;
      }
    }
  }
}

inline void access_graphics_bindings(CommandEncoder &encoder)
{
  for (u32 i = 0; i < encoder.num_bound_descriptor_sets; i++)
  {
    DescriptorHeap const *heap  = encoder.bound_descriptor_set_heaps[i];
    u32 const             set   = encoder.bound_descriptor_sets[i];
    u32 const             group = encoder.bound_descriptor_set_groups[i];
    DescriptorSetLayout const *const layout = heap->set_layouts[set];

    for (u32 ibinding = 0; ibinding < layout->num_bindings; ibinding++)
    {
      gfx::DescriptorBindingDesc const &binding = layout->bindings[ibinding];
      switch (binding.type)
      {
        case gfx::DescriptorType::CombinedImageSampler:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::CombinedImageSamplerBinding const *bindings =
              (gfx::CombinedImageSamplerBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_image(encoder,
                         *IMAGE_FROM_VIEW(bindings[ielement].image_view),
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

        case gfx::DescriptorType::SampledImage:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::SampledImageBinding const *bindings =
              (gfx::SampledImageBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_image(encoder,
                         *IMAGE_FROM_VIEW(bindings[ielement].image_view),
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

        case gfx::DescriptorType::UniformTexelBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::UniformTexelBufferBinding const *bindings =
              (gfx::UniformTexelBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder,
                          *BUFFER_FROM_VIEW(bindings[ielement].buffer_view),
                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::UniformBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::UniformBufferBinding const *bindings =
              (gfx::UniformBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder, *((Buffer *) bindings[ielement].buffer),
                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::DynamicUniformBuffer:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::DynamicUniformBufferBinding const *bindings =
              (gfx::DynamicUniformBufferBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_buffer(encoder, *((Buffer *) bindings[ielement].buffer),
                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                              VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_ACCESS_SHADER_READ_BIT);
          }
        }
        break;

        case gfx::DescriptorType::InputAttachment:
        {
          u32 offset = heap->binding_offsets[set][ibinding];
          gfx::InputAttachmentBinding const *bindings =
              (gfx::InputAttachmentBinding const
                   *) (heap->bindings +
                       (usize) heap->group_binding_stride * group +
                       (usize) offset);
          for (u32 ielement = 0; ielement < binding.count; ielement++)
          {
            access_image(encoder,
                         *(IMAGE_FROM_VIEW(bindings[ielement].image_view)),
                         VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          }
        }
        break;

        default:
          break;
      }
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
  if (render_pass->num_color_attachments != color_attachments.size)
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

    CHECK_WITH(logger, "", num_read_extensions == num_extensions);
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

    CHECK_WITH(logger, "", num_read_layers == num_layers);
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

  char const *load_extensions[4];
  char const *load_layers[4];
  u32         num_load_extensions = 0;
  u32         num_load_layers     = 0;

  CHECK_WITH(
      logger,
      "Required Vulkan Extension: " VK_KHR_SURFACE_EXTENSION_NAME
      " is not supported",
      !alg::find(
           Span<VkExtensionProperties const>{extensions, num_extensions},
           VK_KHR_SURFACE_EXTENSION_NAME,
           [](VkExtensionProperties const &property, char const *find_name) {
             return strcmp(property.extensionName, find_name) == 0;
           })
           .is_empty());

  load_extensions[num_load_extensions] = VK_KHR_SURFACE_EXTENSION_NAME;
  num_load_extensions++;

  if (enable_validation_layer)
  {
    CHECK_WITH(
        logger,
        "Required Vulkan Validation Layer: " VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        " is not supported",
        !alg::find(
             Span<VkExtensionProperties const>{extensions, num_extensions},
             VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
             [](VkExtensionProperties const &property, char const *find_name) {
               return strcmp(property.extensionName, find_name) == 0;
             })
             .is_empty());
    load_extensions[num_load_extensions] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    num_load_extensions++;
  }

  if (enable_validation_layer)
  {
    if (alg::find(Span<VkLayerProperties const>{layers, num_layers},
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

  new (instance) Instance{.refcount                 = 1,
                          .allocator                = allocator,
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

  CHECK_WITH(logger, "Unable to load all required vulkan procedure address",
             load_instance_table(vk_instance, vkGetInstanceProcAddr, vk_table,
                                 enable_validation_layer));

  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;

  if (enable_validation_layer)
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

void InstanceInterface::ref(gfx::Instance instance_)
{
  ((Instance *) instance_)->refcount++;
}

void InstanceInterface::unref(gfx::Instance instance_)
{
  Instance *const instance = (Instance *) instance_;

  if (--instance->refcount == 0)
  {
    if (instance->validation_layer_enabled)
    {
      instance->vk_table.DestroyDebugUtilsMessengerEXT(
          instance->vk_instance, instance->vk_debug_messenger, nullptr);
    }
    instance->vk_table.DestroyInstance(instance->vk_instance, nullptr);
    instance->allocator.deallocate_typed(instance, 1);
  }
}

Result<gfx::DeviceImpl, Status> InstanceInterface::create_device(
    gfx::Instance self_, Span<gfx::DeviceType const> preferred_types,
    Span<gfx::Surface const> compatible_surfaces, AllocatorImpl allocator)
{
  Instance *const self               = (Instance *) self_;
  u32 const       num_surfaces       = (u32) compatible_surfaces.size;
  constexpr u32   MAX_QUEUE_FAMILIES = 16;

  u32      num_devices;
  VkResult result = self->vk_table.EnumeratePhysicalDevices(
      self->vk_instance, &num_devices, nullptr);

  if (num_devices == 0)
  {
    self->logger->trace("No Physical Device Found");
    return Err{Status::DeviceLost};
  }

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
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

    CHECK(self, "", num_read_devices == num_devices);
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

    CHECK(self, "", num_queue_families <= MAX_QUEUE_FAMILIES);

    VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

    {
      u32 num_read_queue_families = num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          device.vk_physical_device, &num_queue_families,
          queue_family_properties);
      CHECK(self, "", num_read_queue_families == num_queue_families);
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

  for (usize i = 0; i < (u32) preferred_types.size; i++)
  {
    for (u32 idevice = 0;
         idevice < num_devices && selected_device_index == num_devices;
         idevice++)
    {
      PhysicalDevice const &device = physical_devices[idevice];

      u32 num_queue_families;
      self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
          device.vk_physical_device, &num_queue_families, nullptr);

      CHECK(self, "", num_queue_families <= MAX_QUEUE_FAMILIES);

      VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

      {
        u32 num_read_queue_families = num_queue_families;
        self->vk_table.GetPhysicalDeviceQueueFamilyProperties(
            device.vk_physical_device, &num_queue_families,
            queue_family_properties);
        CHECK(self, "", num_read_queue_families == num_queue_families);
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
                  compatible_surfaces[isurface], &supported);
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
    CHECK(self, "", num_extensions == num_read_extensions);
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
    CHECK(self, "", num_read_layers == num_layers);
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
  CHECK(self, "Failed To Load Vulkan Device Functions",
        load_device_table(vk_device, self->vk_table, vk_table));

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

  new (device) Device{.refcount        = 1,
                      .allocator       = allocator,
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

void InstanceInterface::ref_device(gfx::Instance, gfx::Device device_)
{
  Device *const device = (Device *) device_;
  device->refcount++;
}

void InstanceInterface::unref_device(gfx::Instance instance_,
                                     gfx::Device   device_)
{
  Instance *const instance = (Instance *) instance_;
  Device *const   device   = (Device *) device_;

  if (device == nullptr)
  {
    return;
  }

  if (--device->refcount == 0)
  {
    vmaDestroyAllocator(device->vma_allocator);
    device->vk_table.DestroyDevice(device->vk_device, nullptr);
    instance->allocator.deallocate_typed(device, 1);
  }
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

  return gfx::DeviceProperties{.api_version    = vk_properties.apiVersion,
                               .driver_version = vk_properties.driverVersion,
                               .vendor_id      = vk_properties.vendorID,
                               .device_id      = vk_properties.deviceID,
                               .api_name       = "vulkan",
                               .device_name    = vk_properties.deviceName,
                               .type =
                                   (gfx::DeviceType) vk_properties.deviceType,
                               .has_unified_memory = has_uma,
                               .features = gfx::DeviceFeatures::Basic};
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

Result<gfx::Buffer, Status>
    DeviceInterface::create_buffer(gfx::Device            self_,
                                   gfx::BufferDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE(self, "", desc.size != 0);
  VALIDATE(self, "", desc.usage != gfx::BufferUsage::None);

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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
        .object      = (u64) vk_buffer,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  Buffer *buffer = self->allocator.allocate_typed<Buffer>(1);
  if (buffer == nullptr)
  {
    vmaDestroyBuffer(self->vma_allocator, vk_buffer, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  new (buffer) Buffer{.refcount            = 1,
                      .desc                = desc,
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

  VALIDATE(self, "", buffer != nullptr);
  VALIDATE(self, "",
           has_any_bit(buffer->desc.usage,
                       gfx::BufferUsage::UniformTexelBuffer |
                           gfx::BufferUsage::StorageTexelBuffer));
  VALIDATE(self, "", desc.format != gfx::Format::Undefined);
  VALIDATE(self, "",
           is_valid_buffer_access(buffer->desc.size, desc.offset, desc.size));

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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
        .object      = (u64) vk_view,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  BufferView *view = self->allocator.allocate_typed<BufferView>(1);

  if (view == nullptr)
  {
    self->vk_table.DestroyBufferView(self->vk_device, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) BufferView{.refcount = 1, .desc = desc, .vk_view = vk_view};

  view->desc.size = view_size;

  return Ok{(gfx::BufferView) view};
}

Result<gfx::Image, Status>
    DeviceInterface::create_image(gfx::Device self_, gfx::ImageDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE(self, "", desc.format != gfx::Format::Undefined);
  VALIDATE(self, "", desc.usage != gfx::ImageUsage::None);
  VALIDATE(self, "", desc.aspects != gfx::ImageAspects::None);
  VALIDATE(self, "", desc.extent.x != 0);
  VALIDATE(self, "", desc.extent.y != 0);
  VALIDATE(self, "", desc.extent.z != 0);
  VALIDATE(self, "", desc.mip_levels > 0);
  VALIDATE(self, "", desc.mip_levels <= num_mip_levels(desc.extent));
  VALIDATE(self, "", desc.array_layers > 0);
  VALIDATE(self, "",
           !(desc.type == gfx::ImageType::Type2D && desc.extent.z != 1));
  VALIDATE(self, "",
           !(desc.type == gfx::ImageType::Type3D && desc.array_layers != 1));

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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
        .object      = (u64) vk_image,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  Image *image = self->allocator.allocate_typed<Image>(1);

  if (image == nullptr)
  {
    vmaDestroyImage(self->vma_allocator, vk_image, vma_allocation);
    return Err{Status::OutOfHostMemory};
  }

  new (image) Image{.refcount            = 1,
                    .desc                = desc,
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

  VALIDATE(self, "", desc.image != nullptr);
  VALIDATE(self, "", desc.view_format != gfx::Format::Undefined);
  VALIDATE(self, "",
           is_image_view_type_compatible(src_image->desc.type, desc.view_type));
  VALIDATE(self, "",
           is_valid_image_subresource_access(
               src_image->desc.aspects, src_image->desc.mip_levels,
               src_image->desc.array_layers, desc.aspects, desc.first_mip_level,
               desc.num_mip_levels, desc.first_array_layer,
               desc.num_array_layers));

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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
        .object      = (u64) vk_view,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  ImageView *view = self->allocator.allocate_typed<ImageView>(1);
  if (view == nullptr)
  {
    self->vk_table.DestroyImageView(self->vk_device, vk_view, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) ImageView{.refcount = 1, .desc = desc, .vk_view = vk_view};

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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,
        .object      = (u64) vk_sampler,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  Sampler *sampler = self->allocator.allocate_typed<Sampler>(1);
  if (sampler == nullptr)
  {
    self->vk_table.DestroySampler(self->vk_device, vk_sampler, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (sampler) Sampler{.refcount = 1, .vk_sampler = vk_sampler};

  return Ok{(gfx::Sampler) sampler};
}

Result<gfx::Shader, Status>
    DeviceInterface::create_shader(gfx::Device            self_,
                                   gfx::ShaderDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE(self, "", desc.spirv_code.size_bytes() > 0);

  VkShaderModuleCreateInfo create_info{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0,
      .codeSize = desc.spirv_code.size_bytes(),
      .pCode    = desc.spirv_code.data};

  VkShaderModule vk_shader;
  VkResult       result = self->vk_table.CreateShaderModule(
      self->vk_device, &create_info, nullptr, &vk_shader);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
        .object      = (u64) vk_shader,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  Shader *shader = self->allocator.allocate_typed<Shader>(1);
  if (shader == nullptr)
  {
    self->vk_table.DestroyShaderModule(self->vk_device, vk_shader, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (shader) Shader{.refcount = 1, .vk_shader = vk_shader};

  return Ok{(gfx::Shader) shader};
}

Result<gfx::RenderPass, Status>
    DeviceInterface::create_render_pass(gfx::Device                self_,
                                        gfx::RenderPassDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE(self, "", desc.color_attachments.size <= gfx::MAX_COLOR_ATTACHMENTS);
  VALIDATE(self, "", desc.input_attachments.size <= gfx::MAX_INPUT_ATTACHMENTS);

  // render_pass attachment descriptions are packed in the following order:
  // [color_attachments..., depth_stencil_attachment, input_attachments...]
  VkAttachmentDescription vk_attachments[gfx::MAX_COLOR_ATTACHMENTS + 1 +
                                         gfx::MAX_INPUT_ATTACHMENTS];
  VkAttachmentReference   vk_color_attachments[gfx::MAX_COLOR_ATTACHMENTS];
  VkAttachmentReference   vk_depth_stencil_attachment;
  VkAttachmentReference   vk_input_attachments[gfx::MAX_INPUT_ATTACHMENTS];
  u32 const  num_color_attachments = (u32) desc.color_attachments.size;
  bool const has_depth_stencil_attachment =
      desc.depth_stencil_attachment.format != gfx::Format::Undefined;
  u32 const num_input_attachments = (u32) desc.input_attachments.size;
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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
        .object      = (u64) vk_render_pass,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  RenderPass *render_pass = self->allocator.allocate_typed<RenderPass>(1);
  if (render_pass == nullptr)
  {
    self->vk_table.DestroyRenderPass(self->vk_device, vk_render_pass, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (render_pass)
      RenderPass{.refcount                 = 1,
                 .color_attachments        = {},
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
  u32 const         num_color_attachments = (u32) desc.color_attachments.size;
  bool const        has_depth_stencil_attachment =
      desc.depth_stencil_attachment != nullptr;
  u32 const num_attachments =
      num_color_attachments + (has_depth_stencil_attachment ? 1U : 0U);
  VkImageView vk_attachments[gfx::MAX_COLOR_ATTACHMENTS + 1];

  VALIDATE(self, "", desc.extent.x > 0);
  VALIDATE(self, "", desc.extent.y > 0);
  VALIDATE(self, "", desc.layers > 0);

  for (gfx::ImageView attachment : desc.color_attachments)
  {
    ImageView *const view  = (ImageView *) attachment;
    Image *const     image = IMAGE_FROM_VIEW(attachment);
    gfx::Extent3D    extent =
        mip_down(image->desc.extent, view->desc.first_mip_level);
    VALIDATE(self, "",
             has_bits(image->desc.usage, gfx::ImageUsage::ColorAttachment));
    VALIDATE(self, "", has_bits(view->desc.aspects, gfx::ImageAspects::Color));
    VALIDATE(self, "", view->desc.num_array_layers >= desc.layers);
    VALIDATE(self, "", extent.x >= desc.extent.x);
    VALIDATE(self, "", extent.y >= desc.extent.y);
  }

  if (desc.depth_stencil_attachment != nullptr)
  {
    ImageView *const view  = (ImageView *) desc.depth_stencil_attachment;
    Image *const     image = IMAGE_FROM_VIEW(view);
    gfx::Extent3D    extent =
        mip_down(image->desc.extent, view->desc.first_mip_level);
    VALIDATE(
        self, "",
        has_bits(image->desc.usage, gfx::ImageUsage::DepthStencilAttachment));
    VALIDATE(self, "",
             has_any_bit(view->desc.aspects, gfx::ImageAspects::Depth |
                                                 gfx::ImageAspects::Stencil));
    VALIDATE(self, "", view->desc.num_array_layers >= desc.layers);
    VALIDATE(self, "", extent.x >= desc.extent.x);
    VALIDATE(self, "", extent.y >= desc.extent.y);
  }

  VALIDATE(self, "Framebuffer and Renderpass are not compatible",
           is_render_pass_compatible(
               render_pass,
               Span{(ImageView *const *) desc.color_attachments.data,
                    desc.color_attachments.size},
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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,
        .object      = (u64) vk_framebuffer,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  Framebuffer *framebuffer = self->allocator.allocate_typed<Framebuffer>(1);
  if (framebuffer == nullptr)
  {
    self->vk_table.DestroyFramebuffer(self->vk_device, vk_framebuffer, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (framebuffer) Framebuffer{.refcount          = 1,
                                .extent            = desc.extent,
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
  Device *const self         = (Device *) self_;
  u32 const     num_bindings = (u32) desc.bindings.size;

  VALIDATE(self, "", num_bindings > 0);
  for (u32 i = 0; i < num_bindings; i++)
  {
    VALIDATE(self, "", desc.bindings[i].count > 0);
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

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
        .object      = (u64) vk_layout,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  DescriptorSetLayout *layout =
      self->allocator.allocate_typed<DescriptorSetLayout>(1);
  if (layout == nullptr)
  {
    self->vk_table.DestroyDescriptorSetLayout(self->vk_device, vk_layout,
                                              nullptr);
    self->allocator.deallocate_typed(bindings, num_bindings);
    return Err{Status::OutOfHostMemory};
  }

  new (layout) DescriptorSetLayout{.refcount     = 1,
                                   .bindings     = bindings,
                                   .num_bindings = num_bindings,
                                   .vk_layout    = vk_layout};

  return Ok{(gfx::DescriptorSetLayout) layout};
}

Result<gfx::DescriptorHeapImpl, Status> DeviceInterface::create_descriptor_heap(
    gfx::Device                          self_,
    Span<gfx::DescriptorSetLayout const> descriptor_set_layouts,
    u32 groups_per_pool, AllocatorImpl allocator)
{
  Device *const self     = (Device *) self_;
  u32 const     num_sets = (u32) descriptor_set_layouts.size;

  VALIDATE(self, "", groups_per_pool > 0);
  VALIDATE(self, "", num_sets > 0);

  DescriptorSetLayout **set_layouts =
      self->allocator.allocate_typed<DescriptorSetLayout *>(num_sets);

  if (set_layouts == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  mem::copy(descriptor_set_layouts, (gfx::DescriptorSetLayout *) set_layouts);

  u32 **binding_offsets = self->allocator.allocate_typed<u32 *>(num_sets);

  if (binding_offsets == nullptr)
  {
    self->allocator.deallocate_typed(set_layouts, num_sets);
    return Err{Status::OutOfHostMemory};
  }

  {
    u32 push_end = 0;
    for (; push_end < num_sets; push_end++)
    {
      u32 *binding_offset = self->allocator.allocate_typed<u32>(
          set_layouts[push_end]->num_bindings);
      if (binding_offset == nullptr)
      {
        break;
      }
      binding_offsets[push_end] = binding_offset;
    }

    if (push_end != num_sets)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        self->allocator.deallocate_typed(binding_offsets[i],
                                         set_layouts[i]->num_bindings);
      }
      self->allocator.deallocate_typed(binding_offsets, num_sets);
      self->allocator.deallocate_typed(set_layouts, num_sets);
      return Err{Status::OutOfHostMemory};
    }
  }

  u32 group_binding_stride = 0;
  u32 num_image_infos      = 0;
  u32 num_buffer_infos     = 0;
  u32 num_buffer_views     = 0;
  {
    u32 offset = 0;
    for (u32 set = 0; set < num_sets; set++)
    {
      for (u32 binding = 0; binding < set_layouts[set]->num_bindings; binding++)
      {
        gfx::DescriptorBindingDesc desc = set_layouts[set]->bindings[binding];

        switch (desc.type)
        {
          case gfx::DescriptorType::Sampler:
            offset =
                (u32) mem::align_offset(alignof(gfx::SamplerBinding), offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::SamplerBinding) * desc.count);
            num_image_infos = max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::CombinedImageSampler:
            offset = (u32) mem::align_offset(
                alignof(gfx::CombinedImageSamplerBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::CombinedImageSamplerBinding) * desc.count);
            num_image_infos = max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::SampledImage:
            offset = (u32) mem::align_offset(alignof(gfx::SampledImageBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::SampledImageBinding) * desc.count);
            num_image_infos = max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::StorageImage:
            offset = (u32) mem::align_offset(alignof(gfx::StorageImageBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::StorageImageBinding) * desc.count);
            num_image_infos = max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::UniformTexelBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::UniformTexelBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::UniformTexelBufferBinding) * desc.count);
            num_buffer_views = max(num_buffer_views, desc.count);
            break;
          case gfx::DescriptorType::StorageTexelBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::StorageTexelBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::StorageTexelBufferBinding) * desc.count);
            num_buffer_views = max(num_buffer_views, desc.count);
            break;
          case gfx::DescriptorType::UniformBuffer:
            offset = (u32) mem::align_offset(alignof(gfx::UniformBufferBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::UniformBufferBinding) * desc.count);
            num_buffer_infos = max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::StorageBuffer:
            offset = (u32) mem::align_offset(alignof(gfx::StorageBufferBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::StorageBufferBinding) * desc.count);
            num_buffer_infos = max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::DynamicUniformBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::DynamicUniformBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::DynamicUniformBufferBinding) * desc.count);
            num_buffer_infos = max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::DynamicStorageBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::DynamicStorageBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::DynamicStorageBufferBinding) * desc.count);
            num_buffer_infos = max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::InputAttachment:
            offset = (u32) mem::align_offset(
                alignof(gfx::InputAttachmentBinding), offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::InputAttachmentBinding) * desc.count);
            num_image_infos = max(num_image_infos, desc.count);
            break;
          default:
            break;
        }
      }
    }
    group_binding_stride = offset;
  }

  usize scratch_memory_size =
      max(max(num_image_infos * sizeof(VkDescriptorImageInfo),
              num_buffer_infos * sizeof(VkDescriptorBufferInfo)),
          num_buffer_views * sizeof(VkBufferView));

  void *scratch_memory =
      self->allocator.allocate(MAX_STANDARD_ALIGNMENT, scratch_memory_size);
  if (scratch_memory == nullptr)
  {
    for (u32 i = 0; i < num_sets; i++)
    {
      self->allocator.deallocate_typed(binding_offsets[i],
                                       set_layouts[i]->num_bindings);
    }
    self->allocator.deallocate_typed(binding_offsets, num_sets);
    self->allocator.deallocate_typed(set_layouts, num_sets);
    return Err{Status::OutOfHostMemory};
  }

  DescriptorHeap *heap = self->allocator.allocate_typed<DescriptorHeap>(1);

  if (heap == nullptr)
  {
    self->allocator.deallocate(MAX_STANDARD_ALIGNMENT, scratch_memory,
                               scratch_memory_size);
    for (u32 i = 0; i < num_sets; i++)
    {
      self->allocator.deallocate_typed(binding_offsets[i],
                                       set_layouts[i]->num_bindings);
    }
    self->allocator.deallocate_typed(binding_offsets, num_sets);
    self->allocator.deallocate_typed(set_layouts, num_sets);
    return Err{Status::OutOfHostMemory};
  }

  new (heap) DescriptorHeap{.refcount                    = 1,
                            .device                      = self,
                            .allocator                   = allocator,
                            .logger                      = self->logger,
                            .set_layouts                 = set_layouts,
                            .binding_offsets             = binding_offsets,
                            .vk_pools                    = nullptr,
                            .vk_descriptor_sets          = nullptr,
                            .last_use_frame              = nullptr,
                            .released_groups             = nullptr,
                            .free_groups                 = nullptr,
                            .bindings                    = nullptr,
                            .scratch_memory              = scratch_memory,
                            .num_sets_per_group          = num_sets,
                            .num_pools                   = 0,
                            .num_groups_per_pool         = groups_per_pool,
                            .num_released_groups         = 0,
                            .num_free_groups             = 0,
                            .group_binding_stride        = group_binding_stride,
                            .vk_pools_capacity           = 0,
                            .vk_descriptor_sets_capacity = 0,
                            .last_use_frame_capacity     = 0,
                            .released_groups_capacity    = 0,
                            .free_groups_capacity        = 0,
                            .bindings_capacity           = 0,
                            .scratch_memory_size         = scratch_memory_size};

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
      .pInitialData    = desc.initial_data.data};

  VkPipelineCache vk_cache;
  VkResult        result = self->vk_table.CreatePipelineCache(
      self->vk_device, &create_info, nullptr, &vk_cache);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT,
        .object      = (u64) vk_cache,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  PipelineCache *cache = self->allocator.allocate_typed<PipelineCache>(1);
  if (cache == nullptr)
  {
    self->vk_table.DestroyPipelineCache(self->vk_device, vk_cache, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (cache) PipelineCache{.refcount = 1, .vk_cache = vk_cache};

  return Ok{(gfx::PipelineCache) cache};
}

Result<gfx::ComputePipeline, Status> DeviceInterface::create_compute_pipeline(
    gfx::Device self_, gfx::ComputePipelineDesc const &desc)
{
  Device *const self                = (Device *) self_;
  u32 const     num_descriptor_sets = (u32) desc.descriptor_set_layouts.size;

  VALIDATE(self, "", num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE(self, "", desc.push_constant_size <= gfx::MAX_PUSH_CONSTANT_SIZE);
  VALIDATE(self, "", desc.push_constant_size % 4 == 0);
  VALIDATE(self, "", desc.compute_shader.entry_point != nullptr);
  VALIDATE(self, "", desc.compute_shader.shader != nullptr);

  VkDescriptorSetLayout
      vk_descriptor_set_layouts[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
  for (u32 i = 0; i < num_descriptor_sets; i++)
  {
    vk_descriptor_set_layouts[i] =
        ((DescriptorSetLayout *) desc.descriptor_set_layouts[i])->vk_layout;
  }

  VkSpecializationInfo vk_specialization{
      .mapEntryCount = (u32) desc.compute_shader.specialization_constants.size,
      .pMapEntries   = (VkSpecializationMapEntry const *)
                         desc.compute_shader.specialization_constants.data,
      .dataSize =
          desc.compute_shader.specialization_constants_data.size_bytes(),
      .pData = desc.compute_shader.specialization_constants_data.data};

  VkPipelineShaderStageCreateInfo vk_stage{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext  = nullptr,
      .flags  = 0,
      .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = ((Shader *) desc.compute_shader.shader)->vk_shader,
      .pName  = desc.compute_shader.entry_point,
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
      .pushConstantRangeCount = 1,
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
      desc.cache == nullptr ? nullptr :
                              ((PipelineCache *) desc.cache)->vk_cache,
      1, &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
    return Err{(Status) result};
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
        .object      = (u64) vk_pipeline,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  ComputePipeline *pipeline =
      self->allocator.allocate_typed<ComputePipeline>(1);
  if (pipeline == nullptr)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_device, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline) ComputePipeline{
      .refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

  return Ok{(gfx::ComputePipeline) pipeline};
}

Result<gfx::GraphicsPipeline, Status> DeviceInterface::create_graphics_pipeline(
    gfx::Device self_, gfx::GraphicsPipelineDesc const &desc)
{
  Device *const self                        = (Device *) self_;
  constexpr u32 NUM_PIPELINE_DYNAMIC_STATES = 6U;
  u32 const     num_descriptor_sets = (u32) desc.descriptor_set_layouts.size;
  u32 const     num_input_bindings  = (u32) desc.vertex_input_bindings.size;
  u32 const     num_attributes      = (u32) desc.vertex_attributes.size;
  u32 const     num_color_attachments =
      (u32) desc.color_blend_state.attachments.size;

  VALIDATE(self,
           "number of descriptor set layouts exceed maximum pipeline "
           "descriptor sets size",
           num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE(self, "", desc.push_constant_size <= gfx::MAX_PUSH_CONSTANT_SIZE);
  VALIDATE(self, "", desc.vertex_shader.entry_point != nullptr);
  VALIDATE(self, "", desc.fragment_shader.entry_point != nullptr);
  VALIDATE(self, "", num_attributes <= gfx::MAX_VERTEX_ATTRIBUTES);
  VALIDATE(self, "", num_attributes <= num_input_bindings);

  VkDescriptorSetLayout
      vk_descriptor_set_layouts[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];
  for (u32 i = 0; i < num_descriptor_sets; i++)
  {
    vk_descriptor_set_layouts[i] =
        ((DescriptorSetLayout *) desc.descriptor_set_layouts[i])->vk_layout;
  }

  VkSpecializationInfo vk_vs_specialization{
      .mapEntryCount = (u32) desc.vertex_shader.specialization_constants.size,
      .pMapEntries   = (VkSpecializationMapEntry const *)
                         desc.vertex_shader.specialization_constants.data,
      .dataSize = desc.vertex_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.vertex_shader.specialization_constants_data.data};

  VkSpecializationInfo vk_fs_specialization{
      .mapEntryCount = (u32) desc.fragment_shader.specialization_constants.size,
      .pMapEntries   = (VkSpecializationMapEntry const *)
                         desc.fragment_shader.specialization_constants.data,
      .dataSize =
          desc.fragment_shader.specialization_constants_data.size_bytes(),
      .pData = desc.fragment_shader.specialization_constants_data.data};

  VkPipelineShaderStageCreateInfo vk_stages[2] = {
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_VERTEX_BIT,
       .module = ((Shader *) desc.vertex_shader.shader)->vk_shader,
       .pName  = desc.vertex_shader.entry_point,
       .pSpecializationInfo = &vk_vs_specialization},
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = ((Shader *) desc.fragment_shader.shader)->vk_shader,
       .pName  = desc.fragment_shader.entry_point,
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
      .pushConstantRangeCount = 1,
      .pPushConstantRanges    = &push_constant_range};

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

  VkPipelineViewportStateCreateInfo viewport_state{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0,
      .viewportCount = 1,
      .pViewports    = nullptr,
      .scissorCount  = 1,
      .pScissors     = nullptr};

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
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable   = (VkBool32) false,
      .minSampleShading      = 1,
      .pSampleMask           = nullptr,
      .alphaToCoverageEnable = (VkBool32) false,
      .alphaToOneEnable      = (VkBool32) false};

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0,
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
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0,
      .logicOpEnable   = (VkBool32) desc.color_blend_state.logic_op_enable,
      .logicOp         = (VkLogicOp) desc.color_blend_state.logic_op,
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
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .dynamicStateCount = NUM_PIPELINE_DYNAMIC_STATES,
      .pDynamicStates    = dynamic_states};

  VkGraphicsPipelineCreateInfo create_info{
      .sType               = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
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
      desc.cache == nullptr ? nullptr :
                              ((PipelineCache *) desc.cache)->vk_cache,
      1, &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
    return Err{(Status) result};
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
        .object      = (u64) vk_pipeline,
        .pObjectName = desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device, &debug_info);
  }

  GraphicsPipeline *pipeline =
      self->allocator.allocate_typed<GraphicsPipeline>(1);
  if (pipeline == nullptr)
  {
    self->vk_table.DestroyPipelineLayout(self->vk_device, vk_layout, nullptr);
    self->vk_table.DestroyPipeline(self->vk_device, vk_pipeline, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline) GraphicsPipeline{
      .refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

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

  Fence *fence = self->allocator.allocate_typed<Fence>(1);

  if (fence == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (fence) Fence{.refcount = 1, .vk_fence = vk_fence};

  return Ok{(gfx::Fence) fence};
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

  CommandEncoder *encoder = self->allocator.allocate_typed<CommandEncoder>(1);

  if (encoder == nullptr)
  {
    self->vk_table.DestroyCommandPool(self->vk_device, vk_command_pool,
                                      nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (encoder) CommandEncoder{.refcount                 = 1,
                               .allocator                = allocator,
                               .logger                   = self->logger,
                               .device                   = self,
                               .vk_command_pool          = vk_command_pool,
                               .vk_command_buffer        = vk_command_buffer,
                               .bound_compute_pipeline   = nullptr,
                               .bound_graphics_pipeline  = nullptr,
                               .bound_render_pass        = nullptr,
                               .bound_framebuffer        = nullptr,
                               .bound_vertex_buffers     = {},
                               .bound_index_buffer       = nullptr,
                               .num_bound_vertex_buffers = 0,
                               .bound_index_type = gfx::IndexType::Uint16,
                               .bound_index_buffer_offset   = 0,
                               .bound_descriptor_set_heaps  = {},
                               .bound_descriptor_set_groups = {},
                               .num_bound_descriptor_sets   = 0,
                               .status                      = Status::Success};

  return Ok{gfx::CommandEncoderImpl{.self      = (gfx::CommandEncoder) encoder,
                                    .interface = &command_encoder_interface}};
}

Result<gfx::FrameContext, Status> DeviceInterface::create_frame_context(
    gfx::Device self_, u32 max_frames_in_flight,
    Span<AllocatorImpl const> command_encoder_allocators)
{
  Device *const self = (Device *) self_;

  VALIDATE(self, "", max_frames_in_flight > 0);

  gfx::CommandEncoderImpl *command_encoders =
      self->allocator.allocate_typed<gfx::CommandEncoderImpl>(
          max_frames_in_flight);

  if (command_encoders == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  {
    Status status   = Status::Success;
    u32    push_end = 0;
    for (; push_end < max_frames_in_flight; push_end++)
    {
      Result result = DeviceInterface::create_command_encoder(
          self_, command_encoder_allocators[push_end]);
      if (result.is_err())
      {
        status = result.err();
        break;
      }
      command_encoders[push_end] = result.value();
    }

    if (push_end != max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        DeviceInterface::unref_command_encoder(self_, command_encoders[i]);
      }
      return Err{(Status) status};
    }
  }

  VkSemaphore *acquire_semaphores =
      self->allocator.allocate_typed<VkSemaphore>(max_frames_in_flight);

  {
    VkResult              result   = VK_SUCCESS;
    u32                   push_end = 0;
    VkSemaphoreCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0};
    for (; push_end < max_frames_in_flight; push_end++)
    {
      VkSemaphore semaphore;
      result = self->vk_table.CreateSemaphore(self->vk_device, &create_info,
                                              nullptr, &semaphore);
      if (result != VK_SUCCESS)
      {
        break;
      }
      acquire_semaphores[push_end] = semaphore;
    }

    if (push_end != max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                        nullptr);
      }

      for (u32 i = 0; i < max_frames_in_flight; i++)
      {
        DeviceInterface::unref_command_encoder(self_, command_encoders[i]);
      }

      return Err{(Status) result};
    }
  }

  gfx::Fence *submit_fences =
      self->allocator.allocate_typed<gfx::Fence>(max_frames_in_flight);

  if (submit_fences == nullptr)
  {
    for (u32 i = 0; i < max_frames_in_flight; i++)
    {
      self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                      nullptr);
    }
    return Err{Status::OutOfHostMemory};
  }

  {
    Status status   = Status::Success;
    u32    push_end = 0;
    for (; push_end < max_frames_in_flight; push_end++)
    {
      Result result = DeviceInterface::create_fence(self_, true);
      if (result.is_err())
      {
        status = result.err();
        break;
      }
      submit_fences[push_end] = result.value();
    }

    if (push_end != max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        DeviceInterface::unref_fence(self_, submit_fences[i]);
      }

      for (u32 i = 0; i < max_frames_in_flight; i++)
      {
        self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                        nullptr);
        DeviceInterface::unref_command_encoder(self_, command_encoders[i]);
      }

      return Err{(Status) status};
    }
  }

  VkSemaphore *submit_semaphores =
      self->allocator.allocate_typed<VkSemaphore>(max_frames_in_flight);

  if (submit_semaphores == nullptr)
  {
    for (u32 i = 0; i < max_frames_in_flight; i++)
    {
      DeviceInterface::unref_fence(self_, submit_fences[i]);
      self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                      nullptr);
      DeviceInterface::unref_command_encoder(self_, command_encoders[i]);
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
    for (; push_end < max_frames_in_flight; push_end++)
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

    if (push_end != max_frames_in_flight)
    {
      for (u32 i = 0; i < push_end; i++)
      {
        self->vk_table.DestroySemaphore(self->vk_device, submit_semaphores[i],
                                        nullptr);
      }

      for (u32 i = 0; i < max_frames_in_flight; i++)
      {
        DeviceInterface::unref_fence(self_, submit_fences[i]);
        self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                        nullptr);
        DeviceInterface::unref_command_encoder(self_, command_encoders[i]);
      }

      return Err{(Status) result};
    }
  }

  FrameContext *frame_context = self->allocator.allocate_typed<FrameContext>(1);

  if (frame_context == nullptr)
  {
    for (u32 i = 0; i < max_frames_in_flight; i++)
    {
      self->vk_table.DestroySemaphore(self->vk_device, submit_semaphores[i],
                                      nullptr);
      DeviceInterface::unref_fence(self_, submit_fences[i]);
      self->vk_table.DestroySemaphore(self->vk_device, acquire_semaphores[i],
                                      nullptr);
      DeviceInterface::unref_command_encoder(self_, command_encoders[i]);
    }

    return Err{Status::OutOfHostMemory};
  }

  new (frame_context) FrameContext{.refcount                = 1,
                                   .trailing_frame          = 0,
                                   .current_frame           = 0,
                                   .current_command_encoder = 0,
                                   .max_frames_in_flight = max_frames_in_flight,
                                   .command_encoders     = command_encoders,
                                   .acquire_semaphores   = acquire_semaphores,
                                   .submit_fences        = submit_fences,
                                   .submit_semaphores    = submit_semaphores};

  return Ok{(gfx::FrameContext) frame_context};
}

/// old swapchain will be retired and destroyed irregardless of whether new
/// swapchain recreation fails.
inline VkResult recreate_swapchain(Device const *self, Swapchain *swapchain)
{
  VALIDATE(self, "", swapchain->desc.preferred_extent.x > 0);
  VALIDATE(self, "", swapchain->desc.preferred_extent.y > 0);
  VALIDATE(self, "",
           swapchain->desc.preferred_buffering <= gfx::MAX_SWAPCHAIN_IMAGES);

  // take ownership of internal data for re-use/release
  VkSwapchainKHR old_vk_swapchain = swapchain->vk_swapchain;
  swapchain->is_valid             = false;
  swapchain->is_optimal           = false;
  swapchain->format               = gfx::SurfaceFormat{};
  swapchain->usage                = gfx::ImageUsage::None;
  swapchain->present_mode         = gfx::PresentMode::Immediate;
  swapchain->extent               = gfx::Extent{};
  swapchain->composite_alpha      = gfx::CompositeAlpha::None;
  swapchain->num_images           = 0;
  swapchain->current_image        = 0;
  swapchain->vk_swapchain         = nullptr;

  VkSurfaceCapabilitiesKHR surface_capabilities;
  VkResult                 result =
      self->instance->vk_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(
          self->physical_device.vk_physical_device, swapchain->vk_surface,
          &surface_capabilities);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroySwapchainKHR(self->vk_device, old_vk_swapchain,
                                       nullptr);
    return result;
  }

  VALIDATE(self, "",
           has_bits(surface_capabilities.supportedUsageFlags,
                    (VkImageUsageFlags) swapchain->desc.usage));
  VALIDATE(self, "",
           has_bits(surface_capabilities.supportedCompositeAlpha,
                    (VkImageUsageFlags) swapchain->desc.composite_alpha));

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

  if (old_vk_swapchain != nullptr)
  {
    self->vk_table.DestroySwapchainKHR(self->vk_device, old_vk_swapchain,
                                       nullptr);
    old_vk_swapchain = nullptr;
  }

  if (result != VK_SUCCESS)
  {
    return result;
  }

  u32 num_images;
  result = self->vk_table.GetSwapchainImagesKHR(
      self->vk_device, new_vk_swapchain, &num_images, nullptr);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroySwapchainKHR(self->vk_device, new_vk_swapchain,
                                       nullptr);
    return result;
  }

  CHECK(self, "", num_images <= gfx::MAX_SWAPCHAIN_IMAGES);

  result = self->vk_table.GetSwapchainImagesKHR(
      self->vk_device, new_vk_swapchain, &num_images, swapchain->vk_images);

  if (result != VK_SUCCESS)
  {
    self->vk_table.DestroySwapchainKHR(self->vk_device, new_vk_swapchain,
                                       nullptr);
    return result;
  }

  for (u32 i = 0; i < num_images; i++)
  {
    swapchain->image_impls[i] = Image{
        .refcount            = 1,
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
  }

  if (swapchain->desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT swapchain_debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,
        .object      = (u64) new_vk_swapchain,
        .pObjectName = swapchain->desc.label};
    self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device,
                                               &swapchain_debug_info);

    for (u32 i = 0; i < num_images; i++)
    {
      VkDebugMarkerObjectNameInfoEXT image_debug_info{
          .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
          .pNext       = nullptr,
          .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
          .object      = (u64) swapchain->vk_images[i],
          .pObjectName = swapchain->desc.label};
      self->vk_table.DebugMarkerSetObjectNameEXT(self->vk_device,
                                                 &image_debug_info);
    }
  }

  swapchain->generation++;
  swapchain->is_valid        = true;
  swapchain->is_optimal      = true;
  swapchain->format          = swapchain->desc.format;
  swapchain->usage           = swapchain->desc.usage;
  swapchain->present_mode    = swapchain->desc.present_mode;
  swapchain->extent.x        = vk_extent.width;
  swapchain->extent.y        = vk_extent.height;
  swapchain->composite_alpha = swapchain->desc.composite_alpha;
  swapchain->num_images      = num_images;
  swapchain->current_image   = 0;
  swapchain->vk_swapchain    = new_vk_swapchain;

  return VK_SUCCESS;
}

Result<gfx::Swapchain, Status>
    DeviceInterface::create_swapchain(gfx::Device self_, gfx::Surface surface,
                                      gfx::SwapchainDesc const &desc)
{
  Device *const self      = (Device *) self_;
  Swapchain    *swapchain = self->allocator.allocate_typed<Swapchain>(1);
  if (swapchain == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  new (swapchain) Swapchain{.generation      = 0,
                            .desc            = desc,
                            .is_valid        = false,
                            .is_optimal      = false,
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
                            .vk_surface      = surface};

  return Ok{(gfx::Swapchain) swapchain};
}

void DeviceInterface::ref_buffer(gfx::Device, gfx::Buffer buffer_)
{
  ((Buffer *) buffer_)->refcount++;
}

void DeviceInterface::ref_buffer_view(gfx::Device, gfx::BufferView buffer_view_)
{
  ((BufferView *) buffer_view_)->refcount++;
}

void DeviceInterface::ref_image(gfx::Device self_, gfx::Image image_)
{
  Device *self  = (Device *) self_;
  Image  *image = (Image *) image_;
  VALIDATE(self, "", !image->is_swapchain_image);
  image->refcount++;
}

void DeviceInterface::ref_image_view(gfx::Device, gfx::ImageView image_view_)
{
  ((ImageView *) image_view_)->refcount++;
}

void DeviceInterface::ref_sampler(gfx::Device, gfx::Sampler sampler_)
{
  ((Sampler *) sampler_)->refcount++;
}

void DeviceInterface::ref_shader(gfx::Device, gfx::Shader shader_)
{
  ((Shader *) shader_)->refcount++;
}

void DeviceInterface::ref_render_pass(gfx::Device, gfx::RenderPass render_pass_)
{
  ((RenderPass *) render_pass_)->refcount++;
}

void DeviceInterface::ref_framebuffer(gfx::Device,
                                      gfx::Framebuffer framebuffer_)
{
  ((Framebuffer *) framebuffer_)->refcount++;
}

void DeviceInterface::ref_descriptor_set_layout(
    gfx::Device, gfx::DescriptorSetLayout layout_)
{
  ((DescriptorSetLayout *) layout_)->refcount++;
}

void DeviceInterface::ref_descriptor_heap(gfx::Device,
                                          gfx::DescriptorHeapImpl heap_)
{
  ((DescriptorHeap *) heap_.self)->refcount++;
}

void DeviceInterface::ref_pipeline_cache(gfx::Device, gfx::PipelineCache cache_)
{
  ((PipelineCache *) cache_)->refcount++;
}

void DeviceInterface::ref_compute_pipeline(gfx::Device,
                                           gfx::ComputePipeline pipeline_)
{
  ((ComputePipeline *) pipeline_)->refcount++;
}

void DeviceInterface::ref_graphics_pipeline(gfx::Device,
                                            gfx::GraphicsPipeline pipeline_)
{
  ((GraphicsPipeline *) pipeline_)->refcount++;
}

void DeviceInterface::ref_fence(gfx::Device, gfx::Fence fence_)
{
  ((Fence *) fence_)->refcount++;
}

void DeviceInterface::ref_command_encoder(gfx::Device,
                                          gfx::CommandEncoderImpl encoder_)
{
  ((CommandEncoder *) encoder_.self)->refcount++;
}

void DeviceInterface::ref_frame_context(gfx::Device,
                                        gfx::FrameContext frame_context_)
{
  ((FrameContext *) frame_context_)->refcount++;
}

void DeviceInterface::unref_buffer(gfx::Device self_, gfx::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  if (buffer == nullptr)
  {
    return;
  }

  if (--buffer->refcount == 0)
  {
    vmaDestroyBuffer(self->vma_allocator, buffer->vk_buffer,
                     buffer->vma_allocation);
    self->allocator.deallocate_typed(buffer, 1);
  }
}

void DeviceInterface::unref_buffer_view(gfx::Device     self_,
                                        gfx::BufferView buffer_view_)
{
  Device *const     self        = (Device *) self_;
  BufferView *const buffer_view = (BufferView *) buffer_view_;

  if (buffer_view == nullptr)
  {
    return;
  }

  if (--buffer_view->refcount == 0)
  {
    self->vk_table.DestroyBufferView(self->vk_device, buffer_view->vk_view,
                                     nullptr);
    self->allocator.deallocate_typed(buffer_view, 1);
  }
}

void DeviceInterface::unref_image(gfx::Device self_, gfx::Image image_)
{
  Device *const self  = (Device *) self_;
  Image *const  image = (Image *) image_;

  if (image == nullptr)
  {
    return;
  }

  VALIDATE(self, "", !image->is_swapchain_image);

  if (--image->refcount == 0)
  {
    vmaDestroyImage(self->vma_allocator, image->vk_image,
                    image->vma_allocation);
    self->allocator.deallocate_typed(image, 1);
  }
}

void DeviceInterface::unref_image_view(gfx::Device    self_,
                                       gfx::ImageView image_view_)
{
  Device *const    self       = (Device *) self_;
  ImageView *const image_view = (ImageView *) image_view_;

  if (image_view == nullptr)
  {
    return;
  }

  if (--image_view->refcount == 0)
  {
    self->vk_table.DestroyImageView(self->vk_device, image_view->vk_view,
                                    nullptr);
    self->allocator.deallocate_typed(image_view, 1);
  }
}

void DeviceInterface::unref_sampler(gfx::Device self_, gfx::Sampler sampler_)
{
  Device *const  self    = (Device *) self_;
  Sampler *const sampler = (Sampler *) sampler_;

  if (sampler == nullptr)
  {
    return;
  }

  if (--sampler->refcount == 0)
  {
    self->vk_table.DestroySampler(self->vk_device, sampler->vk_sampler,
                                  nullptr);
    self->allocator.deallocate_typed(sampler, 1);
  }
}

void DeviceInterface::unref_shader(gfx::Device self_, gfx::Shader shader_)
{
  Device *const self   = (Device *) self_;
  Shader *const shader = (Shader *) shader_;

  if (shader == nullptr)
  {
    return;
  }

  if (--shader->refcount == 0)
  {
    self->vk_table.DestroyShaderModule(self->vk_device, shader->vk_shader,
                                       nullptr);
    self->allocator.deallocate_typed(shader, 1);
  }
}

void DeviceInterface::unref_render_pass(gfx::Device     self_,
                                        gfx::RenderPass render_pass_)
{
  Device *const     self        = (Device *) self_;
  RenderPass *const render_pass = (RenderPass *) render_pass_;

  if (render_pass == nullptr)
  {
    return;
  }

  if (--render_pass->refcount == 0)
  {
    self->vk_table.DestroyRenderPass(self->vk_device,
                                     render_pass->vk_render_pass, nullptr);
    self->allocator.deallocate_typed(render_pass, 1);
  }
}

void DeviceInterface::unref_framebuffer(gfx::Device      self_,
                                        gfx::Framebuffer framebuffer_)
{
  Device *const      self        = (Device *) self_;
  Framebuffer *const framebuffer = (Framebuffer *) framebuffer_;

  if (framebuffer == nullptr)
  {
    return;
  }

  if (--framebuffer->refcount == 0)
  {
    self->vk_table.DestroyFramebuffer(self->vk_device,
                                      framebuffer->vk_framebuffer, nullptr);
    self->allocator.deallocate_typed(framebuffer, 1);
  }
}

void DeviceInterface::unref_descriptor_set_layout(
    gfx::Device self_, gfx::DescriptorSetLayout layout_)
{
  Device *const              self   = (Device *) self_;
  DescriptorSetLayout *const layout = (DescriptorSetLayout *) layout_;

  if (layout == nullptr)
  {
    return;
  }

  if (--layout->refcount == 0)
  {
    self->vk_table.DestroyDescriptorSetLayout(self->vk_device,
                                              layout->vk_layout, nullptr);
    self->allocator.deallocate_typed(layout->bindings, layout->num_bindings);
    self->allocator.deallocate_typed(layout, 1);
  }
}

void DeviceInterface::unref_descriptor_heap(gfx::Device             self_,
                                            gfx::DescriptorHeapImpl heap_)
{
  Device *const         self = (Device *) self_;
  DescriptorHeap *const heap = (DescriptorHeap *) heap_.self;

  if (heap == nullptr)
  {
    return;
  }

  if (--heap->refcount == 0)
  {
    for (u32 i = 0; i < heap->num_sets_per_group; i++)
    {
      self->allocator.deallocate_typed(heap->binding_offsets[i],
                                       heap->set_layouts[i]->num_bindings);
    }
    self->allocator.deallocate_typed(heap->set_layouts,
                                     heap->num_sets_per_group);
    self->allocator.deallocate_typed(heap->binding_offsets,
                                     heap->num_sets_per_group);
    for (u32 i = 0; i < heap->num_pools; i++)
    {
      self->vk_table.DestroyDescriptorPool(self->vk_device, heap->vk_pools[i],
                                           nullptr);
    }
    heap->allocator.deallocate_typed(heap->vk_pools, heap->vk_pools_capacity);
    heap->allocator.deallocate_typed(heap->vk_descriptor_sets,
                                     heap->vk_descriptor_sets_capacity);
    heap->allocator.deallocate_typed(heap->last_use_frame,
                                     heap->last_use_frame_capacity);
    heap->allocator.deallocate_typed(heap->released_groups,
                                     heap->released_groups_capacity);
    heap->allocator.deallocate_typed(heap->free_groups,
                                     heap->free_groups_capacity);
    heap->allocator.deallocate(MAX_STANDARD_ALIGNMENT, heap->bindings,
                               heap->bindings_capacity);
    heap->allocator.deallocate(MAX_STANDARD_ALIGNMENT, heap->scratch_memory,
                               heap->scratch_memory_size);
    self->allocator.deallocate_typed(heap, 1);
  }
}

void DeviceInterface::unref_pipeline_cache(gfx::Device        self_,
                                           gfx::PipelineCache cache_)
{
  Device *const        self  = (Device *) self_;
  PipelineCache *const cache = (PipelineCache *) cache_;

  if (cache == nullptr)
  {
    return;
  }

  if (--cache->refcount == 0)
  {
    self->vk_table.DestroyPipelineCache(self->vk_device, cache->vk_cache,
                                        nullptr);
    self->allocator.deallocate_typed(cache, 1);
  }
}

void DeviceInterface::unref_compute_pipeline(gfx::Device          self_,
                                             gfx::ComputePipeline pipeline_)
{
  Device *const          self     = (Device *) self_;
  ComputePipeline *const pipeline = (ComputePipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  if (--pipeline->refcount == 0)
  {
    self->vk_table.DestroyPipeline(self->vk_device, pipeline->vk_pipeline,
                                   nullptr);
    self->vk_table.DestroyPipelineLayout(self->vk_device, pipeline->vk_layout,
                                         nullptr);
    self->allocator.deallocate_typed(pipeline, 1);
  }
}

void DeviceInterface::unref_graphics_pipeline(gfx::Device           self_,
                                              gfx::GraphicsPipeline pipeline_)
{
  Device *const           self     = (Device *) self_;
  GraphicsPipeline *const pipeline = (GraphicsPipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  if (--pipeline->refcount == 0)
  {
    self->vk_table.DestroyPipeline(self->vk_device, pipeline->vk_pipeline,
                                   nullptr);
    self->vk_table.DestroyPipelineLayout(self->vk_device, pipeline->vk_layout,
                                         nullptr);
    self->allocator.deallocate_typed(pipeline, 1);
  }
}

void DeviceInterface::unref_fence(gfx::Device self_, gfx::Fence fence_)
{
  Device *const self  = (Device *) self_;
  Fence *const  fence = (Fence *) fence_;

  if (fence == nullptr)
  {
    return;
  }

  if (--fence->refcount == 0)
  {
    self->vk_table.DestroyFence(self->vk_device, fence->vk_fence, nullptr);
    self->allocator.deallocate_typed(fence, 1);
  }
}

void DeviceInterface::unref_command_encoder(gfx::Device             self_,
                                            gfx::CommandEncoderImpl encoder_)
{
  Device *const         self    = (Device *) self_;
  CommandEncoder *const encoder = (CommandEncoder *) encoder_.self;

  if (encoder == nullptr)
  {
    return;
  }

  if (--encoder->refcount == 0)
  {
    self->vk_table.DestroyCommandPool(self->vk_device, encoder->vk_command_pool,
                                      nullptr);
    self->allocator.deallocate_typed(encoder, 1);
  }
}

void DeviceInterface::unref_frame_context(gfx::Device       self_,
                                          gfx::FrameContext frame_context_)
{
  Device *const       self          = (Device *) self_;
  FrameContext *const frame_context = (FrameContext *) frame_context_;

  if (frame_context == nullptr)
  {
    return;
  }

  if (--frame_context->refcount == 0)
  {
    for (u32 i = 0; i < frame_context->max_frames_in_flight; i++)
    {
      DeviceInterface::unref_command_encoder(
          self_, frame_context->command_encoders[i]);
      self->vk_table.DestroySemaphore(
          self->vk_device, frame_context->acquire_semaphores[i], nullptr);
      self->vk_table.DestroyFence(
          self->vk_device,
          ((Fence *) frame_context->submit_fences[i])->vk_fence, nullptr);
      self->vk_table.DestroySemaphore(
          self->vk_device, frame_context->submit_semaphores[i], nullptr);
    }
    self->allocator.deallocate_typed(frame_context->acquire_semaphores,
                                     frame_context->max_frames_in_flight);
    self->allocator.deallocate_typed(frame_context->submit_fences,
                                     frame_context->max_frames_in_flight);
    self->allocator.deallocate_typed(frame_context->submit_semaphores,
                                     frame_context->max_frames_in_flight);
    self->allocator.deallocate_typed(frame_context, 1);
  }
}

Result<void *, Status>
    DeviceInterface::get_buffer_memory_map(gfx::Device self_,
                                           gfx::Buffer buffer_)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE(self, "", buffer->desc.host_mapped);

  return Ok{(void *) buffer->host_map};
}

Result<Void, Status> DeviceInterface::invalidate_buffer_memory_map(
    gfx::Device self_, gfx::Buffer buffer_, gfx::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE(self, "", buffer->desc.host_mapped);
  VALIDATE(self, "", range.offset < buffer->desc.size);
  VALIDATE(self, "",
           range.size == gfx::WHOLE_SIZE ||
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

  VALIDATE(self, "", buffer->desc.host_mapped);
  VALIDATE(self, "", range.offset < buffer->desc.size);
  VALIDATE(self, "",
           range.size == gfx::WHOLE_SIZE ||
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
      self->vk_device, ((PipelineCache *) cache)->vk_cache, &size, nullptr);
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
      self->vk_device, ((PipelineCache *) cache)->vk_cache, &size, out.data);
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
  u32 const     num_srcs = (u32) srcs.size;

  VALIDATE(self, "", num_srcs > 0);

  VkPipelineCache *vk_caches =
      self->allocator.allocate_typed<VkPipelineCache>(num_srcs);
  if (vk_caches == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  for (u32 i = 0; i < num_srcs; i++)
  {
    vk_caches[i] = ((PipelineCache *) srcs[i])->vk_cache;
  }

  VkResult result = self->vk_table.MergePipelineCaches(
      self->vk_device, ((PipelineCache *) dst)->vk_cache, num_srcs, vk_caches);

  self->allocator.deallocate_typed(vk_caches, num_srcs);
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
  u32 const     num_fences = (u32) fences.size;

  VALIDATE(self, "", num_fences > 0);

  VkFence *vk_fences = self->allocator.allocate_typed<VkFence>(num_fences);
  if (vk_fences == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  for (u32 i = 0; i < num_fences; i++)
  {
    vk_fences[i] = ((Fence *) fences[i])->vk_fence;
  }

  VkResult result = self->vk_table.WaitForFences(
      self->vk_device, num_fences, vk_fences, (VkBool32) all, timeout);

  self->allocator.deallocate_typed(vk_fences, num_fences);

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
  u32 const     num_fences = (u32) fences.size;

  VALIDATE(self, "", num_fences > 0);

  VkFence *vk_fences = self->allocator.allocate_typed<VkFence>(num_fences);
  if (vk_fences == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  for (u32 i = 0; i < num_fences; i++)
  {
    vk_fences[i] = ((Fence *) fences[i])->vk_fence;
  }

  VkResult result =
      self->vk_table.ResetFences(self->vk_device, num_fences, vk_fences);

  self->allocator.deallocate_typed(vk_fences, num_fences);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<bool, Status> DeviceInterface::get_fence_status(gfx::Device self_,
                                                       gfx::Fence  fence)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.GetFenceStatus(self->vk_device,
                                                       ((Fence *) fence)->vk_fence);

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

Result<Void, Status> DeviceInterface::submit(gfx::Device         self_,
                                             gfx::CommandEncoder encoder,
                                             gfx::Fence          signal_fence)
{
  Device *const self = (Device *) self_;

  VkSubmitInfo info{.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .pNext              = nullptr,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores    = nullptr,
                    .pWaitDstStageMask  = nullptr,
                    .commandBufferCount = 1,
                    .pCommandBuffers =
                        &((CommandEncoder *) encoder)->vk_command_buffer,
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores    = nullptr};

  VkResult result = self->vk_table.QueueSubmit(
      self->vk_queue, 1, &info, ((Fence *) signal_fence)->vk_fence);

  if (result == VK_SUCCESS)
  {
    return Ok{Void{}};
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
  FrameContext *const frame_context = (FrameContext *) frame_context_;

  return gfx::FrameInfo{
      .trailing                = frame_context->trailing_frame,
      .current                 = frame_context->current_frame,
      .command_encoders        = Span{frame_context->command_encoders,
                               frame_context->max_frames_in_flight},
      .current_command_encoder = frame_context->current_command_encoder};
}

Result<u32, Status> DeviceInterface::get_surface_formats(
    gfx::Device self_, gfx::Surface surface_, Span<gfx::SurfaceFormat> formats)
{
  Device *const      self    = (Device *) self_;
  VkSurfaceKHR const surface = surface_;

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

    if (result != VK_SUCCESS)
    {
      self->allocator.deallocate_typed(vk_formats, num_supported);
      return Err{(Status) result};
    }

    CHECK(self, "", num_read == num_supported);
  }

  u32 num_copies = min(num_supported, (u32) formats.size);

  for (u32 i = 0; i < num_copies; i++)
  {
    formats.data[i].format      = (gfx::Format) vk_formats[i].format;
    formats.data[i].color_space = (gfx::ColorSpace) vk_formats[i].colorSpace;
  }

  self->allocator.deallocate_typed(vk_formats, num_supported);

  return Ok{(u32) num_supported};
}

Result<u32, Status> DeviceInterface::get_surface_present_modes(
    gfx::Device self_, gfx::Surface surface_, Span<gfx::PresentMode> modes)
{
  Device *const      self    = (Device *) self_;
  VkSurfaceKHR const surface = surface_;

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

    if (result != VK_SUCCESS)
    {
      self->allocator.deallocate_typed(vk_present_modes, num_supported);
      return Err{(Status) result};
    }

    CHECK(self, "", num_read == num_supported);
  }

  u32 num_copies = min(num_supported, (u32) modes.size);

  for (u32 i = 0; i < num_copies; i++)
  {
    modes.data[i] = (gfx::PresentMode) vk_present_modes[i];
  }

  self->allocator.deallocate_typed(vk_present_modes, num_supported);

  return Ok{(u32) num_supported};
}

Result<gfx::SurfaceCapabilities, Status>
    DeviceInterface::get_surface_capabilities(gfx::Device  self_,
                                              gfx::Surface surface_)
{
  Device *const            self    = (Device *) self_;
  VkSurfaceKHR const       surface = surface_;
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

Result<gfx::SwapchainInfo, Status>
    DeviceInterface::get_swapchain_info(gfx::Device, gfx::Swapchain swapchain_)
{
  Swapchain *const swapchain = (Swapchain *) swapchain_;

  return Ok{gfx::SwapchainInfo{
      .generation    = swapchain->generation,
      .extent        = swapchain->extent,
      .format        = swapchain->desc.format,
      .images        = Span{swapchain->images, swapchain->num_images},
      .current_image = swapchain->current_image}};
}

Result<Void, Status> DeviceInterface::invalidate_swapchain(
    gfx::Device, gfx::Swapchain swapchain_, gfx::SwapchainDesc const &desc)
{
  Swapchain *const swapchain = (Swapchain *) swapchain_;
  swapchain->is_optimal      = false;
  swapchain->desc            = desc;
  return Ok{Void{}};
}

Result<Void, Status>
    DeviceInterface::begin_frame(gfx::Device self_, gfx::Swapchain swapchain_,
                                 gfx::FrameContext frame_context_)
{
  Device *const       self          = (Device *) self_;
  FrameContext *const frame_context = (FrameContext *) frame_context_;
  Swapchain *const    swapchain     = (Swapchain *) swapchain_;
  VkResult            result        = VK_SUCCESS;

  if (!swapchain->is_valid)
  {
    if (swapchain->vk_swapchain != nullptr)
    {
      // await all pending submitted operations on the device possibly using
      // the swapchain, to avoid destroying whilst in use
      result = self->vk_table.DeviceWaitIdle(self->vk_device);
      if (result != VK_SUCCESS)
      {
        return Err{(Status) result};
      }
    }

    result = recreate_swapchain(self, swapchain);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
  }

  u32 next_image;
  result = self->vk_table.AcquireNextImageKHR(
      self->vk_device, swapchain->vk_swapchain, U64_MAX,
      frame_context->acquire_semaphores[frame_context->current_command_encoder],
      nullptr, &next_image);

  if (result == VK_SUBOPTIMAL_KHR)
  {
    swapchain->is_optimal = false;
  }
  else if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  swapchain->current_image = next_image;
  return Ok{Void{}};
}

Result<Void, Status>
    DeviceInterface::submit_frame(gfx::Device self_, gfx::Swapchain swapchain_,
                                  gfx::FrameContext frame_context_)
{
  Device *const       self          = (Device *) self_;
  FrameContext *const frame_context = (FrameContext *) frame_context_;
  Swapchain *const    swapchain     = (Swapchain *) swapchain_;
  Fence *const        submit_fence =
      (Fence *)
          frame_context->submit_fences[frame_context->current_command_encoder];
  CommandEncoder *const command_encoder =
      (CommandEncoder *) frame_context
          ->command_encoders[frame_context->current_command_encoder]
          .self;
  VkCommandBuffer const command_buffer = command_encoder->vk_command_buffer;
  VkSemaphore const     acquire_semaphore =
      frame_context->acquire_semaphores[frame_context->current_command_encoder];
  VkSemaphore const submit_semaphore =
      frame_context->submit_semaphores[frame_context->current_command_encoder];

  VALIDATE(self, "", swapchain->is_valid);

  VkResult result = self->vk_table.WaitForFences(
      self->vk_device, 1, &submit_fence->vk_fence, VK_TRUE, U64_MAX / 2);

  // there's not really any way to preserve state here and allow for re-entrancy
  CHECK(self, "", result == VK_SUCCESS);

  result =
      self->vk_table.ResetFences(self->vk_device, 1, &submit_fence->vk_fence);

  CHECK(self, "", result == VK_SUCCESS);

  VkSubmitInfo submit_info{.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                           .pNext              = nullptr,
                           .waitSemaphoreCount = 1,
                           .pWaitSemaphores    = &acquire_semaphore,
                           .pWaitDstStageMask  = nullptr,
                           .commandBufferCount = 1,
                           .pCommandBuffers    = &command_buffer,
                           .signalSemaphoreCount = 1,
                           .pSignalSemaphores    = &submit_semaphore};

  result = self->vk_table.QueueSubmit(self->vk_queue, 1, &submit_info,
                                      submit_fence->vk_fence);

  CHECK(self, "", result == VK_SUCCESS);

  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands could have been executed.
  frame_context->current_frame++;
  frame_context->trailing_frame =
      max(frame_context->current_frame,
          (gfx::FrameId) frame_context->max_frames_in_flight) -
      frame_context->max_frames_in_flight;
  frame_context->current_command_encoder =
      (frame_context->current_command_encoder + 1) %
      frame_context->max_frames_in_flight;

  VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                .pNext = nullptr,
                                .waitSemaphoreCount = 1,
                                .pWaitSemaphores    = &submit_semaphore,
                                .swapchainCount     = 1,
                                .pSwapchains        = &swapchain->vk_swapchain,
                                .pImageIndices      = &swapchain->current_image,
                                .pResults           = nullptr};
  result = self->vk_table.QueuePresentKHR(self->vk_queue, &present_info);

  // if swapchain redundant, mark as redundant for next frame
  // and let them do the work again. acquire next image at start of frame and
  // recreate if necessary, not at end of loop
  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    swapchain->is_valid = false;
  }
  else
  {
    CHECK(self, "", result == VK_SUCCESS);
  }

  return Ok{Void{}};
}

Result<u32, Status>
    DescriptorHeapInterface::add_group(gfx::DescriptorHeap self_,
                                       gfx::FrameId        trailing_frame)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;
  u32 const             num_sets_per_pool =
      self->num_sets_per_group * self->num_groups_per_pool;
  usize const pool_bindings_size =
      self->num_groups_per_pool * (usize) self->group_binding_stride;
  u32 num_bindings_per_group = 0;
  for (u32 i = 0; i < self->num_sets_per_group; i++)
  {
    num_bindings_per_group += self->set_layouts[i]->num_bindings;
  }

  // move from released to free for all released groups not in use by the device
  if (self->num_released_groups > 0)
  {
    u32 num_released_groups = 0;
    for (u32 i = 0; i < self->num_released_groups; i++)
    {
      if (self->last_use_frame[self->released_groups[i]] < trailing_frame)
      {
        self->free_groups[self->num_free_groups] = self->released_groups[i];
        self->num_free_groups++;
      }
      else
      {
        self->released_groups[num_released_groups] = self->released_groups[i];
        num_released_groups++;
      }
    }

    self->num_released_groups = num_released_groups;
  }

  // if any free, claim
  if (self->num_free_groups > 0)
  {
    u32 group = self->free_groups[self->num_free_groups - 1];
    self->num_free_groups--;
    mem::zero(self->bindings + group * (usize) self->group_binding_stride,
              self->group_binding_stride);
    return Ok{(u32) group};
  }

  VkDescriptorPool *pools = self->allocator.reallocate_typed(
      self->vk_pools, self->vk_pools_capacity, self->num_pools + 1);

  if (pools == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->vk_pools          = pools;
  self->vk_pools_capacity = self->num_pools + 1;

  VkDescriptorSet *descriptor_sets = self->allocator.reallocate_typed(
      self->vk_descriptor_sets, self->vk_descriptor_sets_capacity,
      (self->num_pools + 1) * num_sets_per_pool);

  if (descriptor_sets == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->vk_descriptor_sets          = descriptor_sets;
  self->vk_descriptor_sets_capacity = (self->num_pools + 1) * num_sets_per_pool;

  VkDescriptorSet *new_descriptor_sets =
      descriptor_sets + self->num_pools * num_sets_per_pool;

  u64 *last_use_frame = self->allocator.reallocate_typed(
      self->last_use_frame, self->last_use_frame_capacity,
      (self->num_pools + 1) * self->num_groups_per_pool);

  if (last_use_frame == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->last_use_frame = last_use_frame;
  self->last_use_frame_capacity =
      (self->num_pools + 1) * self->num_groups_per_pool;

  u32 *released_groups = self->allocator.reallocate_typed(
      self->released_groups, self->released_groups_capacity,
      (self->num_pools + 1) * self->num_groups_per_pool);

  if (released_groups == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->released_groups = released_groups;
  self->released_groups_capacity =
      (self->num_pools + 1) * self->num_groups_per_pool;

  u32 *free_groups = self->allocator.reallocate_typed(
      self->free_groups, self->free_groups_capacity,
      (self->num_pools + 1) * self->num_groups_per_pool);

  if (free_groups == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->free_groups = free_groups;
  self->free_groups_capacity =
      (self->num_pools + 1) * self->num_groups_per_pool;

  u8 *bindings = (u8 *) self->allocator.reallocate(
      MAX_STANDARD_ALIGNMENT, self->bindings, self->bindings_capacity,
      (self->num_pools + 1) * pool_bindings_size);

  if (bindings == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  self->bindings          = bindings;
  self->bindings_capacity = (self->num_pools + 1) * pool_bindings_size;

  mem::zero(self->bindings + self->num_pools * pool_bindings_size,
            pool_bindings_size);

  VkDescriptorPoolSize *pool_sizes =
      self->allocator.allocate_typed<VkDescriptorPoolSize>(
          num_bindings_per_group);

  if (pool_sizes == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  for (u32 iset = 0, ibinding = 0; iset < self->num_sets_per_group; iset++)
  {
    for (u32 iset_binding = 0;
         iset_binding < self->set_layouts[iset]->num_bindings;
         iset_binding++, ibinding++)
    {
      gfx::DescriptorBindingDesc desc =
          self->set_layouts[iset]->bindings[iset_binding];
      pool_sizes[ibinding] = VkDescriptorPoolSize{
          .type            = (VkDescriptorType) desc.type,
          .descriptorCount = desc.count * self->num_groups_per_pool};
    }
  }

  VkDescriptorPoolCreateInfo pool_create_info{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0,
      .maxSets       = self->num_sets_per_group * self->num_groups_per_pool,
      .poolSizeCount = num_bindings_per_group,
      .pPoolSizes    = pool_sizes};

  VkDescriptorPool vk_pool;
  VkResult         result = self->device->vk_table.CreateDescriptorPool(
      self->device->vk_device, &pool_create_info, nullptr, &vk_pool);

  self->allocator.deallocate_typed(pool_sizes, num_bindings_per_group);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkDescriptorSetLayout *set_layouts =
      self->allocator.allocate_typed<VkDescriptorSetLayout>(
          self->num_sets_per_group * self->num_groups_per_pool);

  if (set_layouts == nullptr)
  {
    return Err{Status::OutOfHostMemory};
  }

  for (u32 igroup = 0; igroup < self->num_groups_per_pool; igroup++)
  {
    for (u32 iset = 0; iset < self->num_sets_per_group; iset++)
    {
      set_layouts[igroup * self->num_sets_per_group + iset] =
          self->set_layouts[iset]->vk_layout;
    }
  }

  VkDescriptorSetAllocateInfo set_alloc_info{
      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .pNext              = nullptr,
      .descriptorPool     = vk_pool,
      .descriptorSetCount = self->num_groups_per_pool,
      .pSetLayouts        = set_layouts};
  result = self->device->vk_table.AllocateDescriptorSets(
      self->device->vk_device, &set_alloc_info, new_descriptor_sets);

  self->allocator.deallocate_typed(set_layouts, self->num_sets_per_group *
                                                    self->num_groups_per_pool);

  // must not have these errors
  CHECK(self, "Descriptor set allocation logic error",
        result != VK_ERROR_OUT_OF_POOL_MEMORY &&
            result != VK_ERROR_FRAGMENTED_POOL);

  if (result != VK_SUCCESS)
  {
    self->device->vk_table.DestroyDescriptorPool(self->device->vk_device,
                                                 vk_pool, nullptr);
    return Err{(Status) result};
  }

  u32 const assigned_group        = self->num_pools * self->num_groups_per_pool;
  self->vk_pools[self->num_pools] = vk_pool;
  self->num_pools++;
  // fill the free groups in reverse order (i.e. [set 4, set 3, set 2])
  // as reclamation pulls from the end of the free groups. this helps make with
  // predictability of indexes of newly allocated groups
  for (u32 free_group = self->num_pools * self->num_groups_per_pool - 1;
       free_group > assigned_group; free_group--, self->num_free_groups++)
  {
    self->free_groups[self->num_free_groups] = free_group;
  }

  return Ok{(u32) assigned_group};
}

void DescriptorHeapInterface::sampler(gfx::DescriptorHeap self_, u32 group,
                                      u32 set, u32 binding,
                                      Span<gfx::SamplerBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::Sampler);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);

  gfx::SamplerBinding *bindings =
      (gfx::SamplerBinding *) (self->bindings +
                               (usize) self->group_binding_stride * group +
                               self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorImageInfo *image_infos =
      (VkDescriptorImageInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::SamplerBinding const &element = elements[i];
    image_infos[i]                     = VkDescriptorImageInfo{
                            .sampler     = ((Sampler *) element.sampler)->vk_sampler,
                            .imageView   = nullptr,
                            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLER,
      .pImageInfo       = image_infos,
      .pBufferInfo      = nullptr,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::combined_image_sampler(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::CombinedImageSamplerBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::CombinedImageSampler);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::CombinedImageSamplerBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::Sampled));
  }

  gfx::CombinedImageSamplerBinding *bindings =
      (gfx::CombinedImageSamplerBinding
           *) (self->bindings + (usize) self->group_binding_stride * group +
               self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorImageInfo *image_infos =
      (VkDescriptorImageInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::CombinedImageSamplerBinding const &element = elements[i];
    image_infos[i]                                  = VkDescriptorImageInfo{
                                         .sampler     = ((Sampler *) element.sampler)->vk_sampler,
                                         .imageView   = ((ImageView *) element.image_view)->vk_view,
                                         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .pImageInfo       = image_infos,
      .pBufferInfo      = nullptr,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::sampled_image(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::SampledImageBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::SampledImage);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::SampledImageBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::Sampled));
  }

  gfx::SampledImageBinding *bindings =
      (gfx::SampledImageBinding *) (self->bindings +
                                    (usize) self->group_binding_stride * group +
                                    self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorImageInfo *image_infos =
      (VkDescriptorImageInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::SampledImageBinding const &element = elements[i];
    image_infos[i]                          = VkDescriptorImageInfo{
                                 .sampler     = nullptr,
                                 .imageView   = ((ImageView *) element.image_view)->vk_view,
                                 .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      .pImageInfo       = image_infos,
      .pBufferInfo      = nullptr,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::storage_image(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::StorageImageBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::StorageImage);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::StorageImageBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::Storage));
  }

  gfx::StorageImageBinding *bindings =
      (gfx::StorageImageBinding *) (self->bindings +
                                    (usize) self->group_binding_stride * group +
                                    self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorImageInfo *image_infos =
      (VkDescriptorImageInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::StorageImageBinding const &element = elements[i];
    image_infos[i]                          = VkDescriptorImageInfo{
                                 .sampler     = nullptr,
                                 .imageView   = ((ImageView *) element.image_view)->vk_view,
                                 .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      .pImageInfo       = image_infos,
      .pBufferInfo      = nullptr,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::uniform_texel_buffer(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::UniformTexelBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::UniformTexelBuffer);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::UniformTexelBufferBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(BUFFER_FROM_VIEW(element.buffer_view)->desc.usage,
                      gfx::BufferUsage::UniformTexelBuffer));
  }

  gfx::UniformTexelBufferBinding *bindings =
      (gfx::UniformTexelBufferBinding *) (self->bindings +
                                          (usize) self->group_binding_stride *
                                              group +
                                          self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkBufferView *buffer_views = (VkBufferView *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::UniformTexelBufferBinding const &element = elements[i];
    buffer_views[i] = ((BufferView *) element.buffer_view)->vk_view;
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
      .pImageInfo       = nullptr,
      .pBufferInfo      = nullptr,
      .pTexelBufferView = buffer_views};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::storage_texel_buffer(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::StorageTexelBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::StorageTexelBuffer);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::StorageTexelBufferBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(BUFFER_FROM_VIEW(element.buffer_view)->desc.usage,
                      gfx::BufferUsage::StorageTexelBuffer));
  }

  gfx::StorageTexelBufferBinding *bindings =
      (gfx::StorageTexelBufferBinding *) (self->bindings +
                                          (usize) self->group_binding_stride *
                                              group +
                                          self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkBufferView *buffer_views = (VkBufferView *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::StorageTexelBufferBinding const &element = elements[i];
    buffer_views[i] = ((BufferView *) element.buffer_view)->vk_view;
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
      .pImageInfo       = nullptr,
      .pBufferInfo      = nullptr,
      .pTexelBufferView = buffer_views};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::uniform_buffer(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::UniformBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::UniformBuffer);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::UniformBufferBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::UniformBuffer));
  }

  gfx::UniformBufferBinding *bindings =
      (gfx::UniformBufferBinding *) (self->bindings +
                                     (usize) self->group_binding_stride *
                                         group +
                                     self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::UniformBufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pImageInfo       = nullptr,
      .pBufferInfo      = buffer_infos,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::storage_buffer(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::StorageBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::StorageBuffer);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::StorageBufferBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::StorageBuffer));
  }

  gfx::StorageBufferBinding *bindings =
      (gfx::StorageBufferBinding *) (self->bindings +
                                     (usize) self->group_binding_stride *
                                         group +
                                     self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::StorageBufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .pImageInfo       = nullptr,
      .pBufferInfo      = buffer_infos,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::dynamic_uniform_buffer(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::DynamicUniformBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::DynamicUniformBuffer);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::DynamicUniformBufferBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::UniformBuffer));
  }

  gfx::DynamicUniformBufferBinding *bindings =
      (gfx::DynamicUniformBufferBinding
           *) (self->bindings + (usize) self->group_binding_stride * group +
               self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::DynamicUniformBufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
      .pImageInfo       = nullptr,
      .pBufferInfo      = buffer_infos,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::dynamic_storage_buffer(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::DynamicStorageBufferBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::StorageBuffer);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::DynamicStorageBufferBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(((Buffer *) element.buffer)->desc.usage,
                      gfx::BufferUsage::StorageBuffer));
  }

  gfx::DynamicStorageBufferBinding *bindings =
      (gfx::DynamicStorageBufferBinding
           *) (self->bindings + (usize) self->group_binding_stride * group +
               self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorBufferInfo *buffer_infos =
      (VkDescriptorBufferInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::DynamicStorageBufferBinding const &element = elements[i];
    buffer_infos[i] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) element.buffer)->vk_buffer,
                               .offset = element.offset,
                               .range  = element.size};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
      .pImageInfo       = nullptr,
      .pBufferInfo      = buffer_infos,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::input_attachment(
    gfx::DescriptorHeap self_, u32 group, u32 set, u32 binding,
    Span<gfx::InputAttachmentBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", set < self->num_sets_per_group);
  VALIDATE(self, "", binding < self->set_layouts[set]->num_bindings);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].type ==
               gfx::DescriptorType::InputAttachment);
  VALIDATE(self, "",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::InputAttachmentBinding const &element : elements)
  {
    VALIDATE(self, "",
             has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
                      gfx::ImageUsage::InputAttachment));
  }

  gfx::InputAttachmentBinding *bindings =
      (gfx::InputAttachmentBinding *) (self->bindings +
                                       (usize) self->group_binding_stride *
                                           group +
                                       self->binding_offsets[set][binding]);
  mem::copy(elements, bindings);

  VkDescriptorImageInfo *image_infos =
      (VkDescriptorImageInfo *) self->scratch_memory;
  for (u32 i = 0; i < elements.size; i++)
  {
    gfx::InputAttachmentBinding const &element = elements[i];
    image_infos[i]                             = VkDescriptorImageInfo{
                                    .sampler     = nullptr,
                                    .imageView   = ((ImageView *) element.image_view)->vk_view,
                                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  }

  VkWriteDescriptorSet vk_write{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .pNext = nullptr,
      .dstSet =
          self->vk_descriptor_sets[self->num_sets_per_group * group + set],
      .dstBinding       = binding,
      .dstArrayElement  = 0,
      .descriptorCount  = (u32) elements.size,
      .descriptorType   = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
      .pImageInfo       = image_infos,
      .pBufferInfo      = nullptr,
      .pTexelBufferView = nullptr};

  self->device->vk_table.UpdateDescriptorSets(self->device->vk_device, 1,
                                              &vk_write, 0, nullptr);
}

void DescriptorHeapInterface::mark_in_use(gfx::DescriptorHeap self_, u32 group,
                                          gfx::FrameId current_frame)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "", self->last_use_frame[group] <= current_frame);

  self->last_use_frame[group] = current_frame;
}

bool DescriptorHeapInterface::is_in_use(gfx::DescriptorHeap self_, u32 group,
                                        gfx::FrameId trailing_frame)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));

  return self->last_use_frame[group] >= trailing_frame;
}

void DescriptorHeapInterface::release(gfx::DescriptorHeap self_, u32 group)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE(self, "", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE(self, "multiple descriptor group release detected",
           (self->num_released_groups + 1) <=
               (self->num_pools * self->num_groups_per_pool));

  self->released_groups[self->num_released_groups] = group;
  self->num_released_groups++;
}

gfx::DescriptorHeapStats
    DescriptorHeapInterface::get_stats(gfx::DescriptorHeap self_)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  return gfx::DescriptorHeapStats{
      .num_allocated_groups = self->num_pools * self->num_groups_per_pool,
      .num_free_groups      = self->num_free_groups,
      .num_released_groups  = self->num_released_groups,
      .num_pools            = self->num_pools};
}

void CommandEncoderInterface::begin(gfx::CommandEncoder self_)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  if (self->status != Status::Success)
  {
    return;
  }

  VkCommandBufferBeginInfo info{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext            = nullptr,
      .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr};
  self->status = (Status) self->device->vk_table.BeginCommandBuffer(
      self->vk_command_buffer, &info);
}

Result<Void, Status> CommandEncoderInterface::end(gfx::CommandEncoder self_)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  if (self->status != Status::Success)
  {
    return Err{(Status) self->status};
  }

  VkResult result =
      self->device->vk_table.EndCommandBuffer(self->vk_command_buffer);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

void CommandEncoderInterface::reset(gfx::CommandEncoder self_)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  self->device->vk_table.ResetCommandBuffer(self->vk_command_buffer, 0);
  self->bound_compute_pipeline    = nullptr;
  self->bound_graphics_pipeline   = nullptr;
  self->bound_render_pass         = nullptr;
  self->bound_framebuffer         = nullptr;
  self->num_bound_vertex_buffers  = 0;
  self->bound_index_buffer        = nullptr;
  self->num_bound_descriptor_sets = 0;
  self->status                    = Status::Success;
}

void CommandEncoderInterface::begin_debug_marker(gfx::CommandEncoder self_,
                                                 char const *region_name,
                                                 Vec4        color)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  if (self->status != Status::Success)
  {
    return;
  }

  VkDebugMarkerMarkerInfoEXT info{
      .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
      .pNext       = nullptr,
      .pMarkerName = region_name,
      .color       = {color.x, color.y, color.z, color.w}};
  self->device->vk_table.CmdDebugMarkerBeginEXT(self->vk_command_buffer, &info);
}

void CommandEncoderInterface::end_debug_marker(gfx::CommandEncoder self_)
{
  CommandEncoder *const self = (CommandEncoder *) self_;
  if (self->status != Status::Success)
  {
    return;
  }
  self->device->vk_table.CmdDebugMarkerEndEXT(self->vk_command_buffer);
}

void CommandEncoderInterface::fill_buffer(gfx::CommandEncoder self_,
                                          gfx::Buffer dst_, u64 offset,
                                          u64 size, u32 data)
{
  CommandEncoder *const self = (CommandEncoder *) self_;
  Buffer *const         dst  = (Buffer *) dst_;

  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  VALIDATE(self, "",
           is_valid_aligned_buffer_access(dst->desc.size, offset, size, 4));

  if (self->status != Status::Success)
  {
    return;
  }

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);
  self->device->vk_table.CmdFillBuffer(self->vk_command_buffer, dst->vk_buffer,
                                       offset, size, data);
}

void CommandEncoderInterface::copy_buffer(gfx::CommandEncoder self_,
                                          gfx::Buffer src_, gfx::Buffer dst_,
                                          Span<gfx::BufferCopy const> copies)
{
  CommandEncoder *const self       = (CommandEncoder *) self_;
  Buffer *const         src        = (Buffer *) src_;
  Buffer *const         dst        = (Buffer *) dst_;
  u32 const             num_copies = (u32) copies.size;

  VALIDATE(self, "", has_bits(src->desc.usage, gfx::BufferUsage::TransferSrc));
  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  VALIDATE(self, "", num_copies > 0);
  for (gfx::BufferCopy const &copy : copies)
  {
    VALIDATE(
        self, "",
        is_valid_buffer_access(src->desc.size, copy.src_offset, copy.size));
    VALIDATE(
        self, "",
        is_valid_buffer_access(dst->desc.size, copy.dst_offset, copy.size));
  }

  if (self->status != Status::Success)
  {
    return;
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
  CommandEncoder *const self      = (CommandEncoder *) self_;
  Buffer *const         dst       = (Buffer *) dst_;
  u64 const             copy_size = src.size_bytes();

  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::BufferUsage::TransferDst));
  VALIDATE(
      self, "",
      is_valid_aligned_buffer_access(dst->desc.size, dst_offset, copy_size, 4));

  if (self->status != Status::Success)
  {
    return;
  }

  access_buffer(*self, *dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  self->device->vk_table.CmdUpdateBuffer(self->vk_command_buffer,
                                         dst->vk_buffer, dst_offset,
                                         (u64) src.size, src.data);
}

void CommandEncoderInterface::clear_color_image(
    gfx::CommandEncoder self_, gfx::Image dst_, gfx::Color clear_color,
    Span<gfx::ImageSubresourceRange const> ranges)

{
  CommandEncoder *const self       = (CommandEncoder *) self_;
  Image *const          dst        = (Image *) dst_;
  u32 const             num_ranges = (u32) ranges.size;

  static_assert(sizeof(gfx::Color) == sizeof(VkClearColorValue));
  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  VALIDATE(self, "", num_ranges > 0);
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    VALIDATE(self, "",
             is_valid_image_subresource_access(
                 dst->desc.aspects, dst->desc.mip_levels,
                 dst->desc.array_layers, range.aspects, range.first_mip_level,
                 range.num_mip_levels, range.first_array_layer,
                 range.num_array_layers));
  }

  if (self->status != Status::Success)
  {
    return;
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
  CommandEncoder *const self       = (CommandEncoder *) self_;
  Image *const          dst        = (Image *) dst_;
  u32 const             num_ranges = (u32) ranges.size;

  static_assert(sizeof(gfx::DepthStencil) == sizeof(VkClearDepthStencilValue));
  VALIDATE(self, "", num_ranges > 0);
  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    VALIDATE(self, "",
             is_valid_image_subresource_access(
                 dst->desc.aspects, dst->desc.mip_levels,
                 dst->desc.array_layers, range.aspects, range.first_mip_level,
                 range.num_mip_levels, range.first_array_layer,
                 range.num_array_layers));
  }

  if (self->status != Status::Success)
  {
    return;
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
  CommandEncoder *const self       = (CommandEncoder *) self_;
  Image *const          src        = (Image *) src_;
  Image *const          dst        = (Image *) dst_;
  u32 const             num_copies = (u32) copies.size;

  VALIDATE(self, "", num_copies > 0);
  VALIDATE(self, "", has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::ImageCopy const &copy = copies[i];

    VALIDATE(self, "",
             is_valid_image_subresource_access(
                 src->desc.aspects, src->desc.mip_levels,
                 src->desc.array_layers, copy.src_layers.aspects,
                 copy.src_layers.mip_level, 1,
                 copy.src_layers.first_array_layer,
                 copy.src_layers.num_array_layers));
    VALIDATE(self, "",
             is_valid_image_subresource_access(
                 dst->desc.aspects, dst->desc.mip_levels,
                 dst->desc.array_layers, copy.dst_layers.aspects,
                 copy.dst_layers.mip_level, 1,
                 copy.dst_layers.first_array_layer,
                 copy.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, copy.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.dst_layers.mip_level);
    VALIDATE(self, "", copy.extent.x > 0);
    VALIDATE(self, "", copy.extent.y > 0);
    VALIDATE(self, "", copy.extent.z > 0);
    VALIDATE(self, "", copy.src_offset.x <= src_extent.x);
    VALIDATE(self, "", copy.src_offset.y <= src_extent.y);
    VALIDATE(self, "", copy.src_offset.z <= src_extent.z);
    VALIDATE(self, "", (copy.src_offset.x + copy.extent.x) <= src_extent.x);
    VALIDATE(self, "", (copy.src_offset.y + copy.extent.x) <= src_extent.y);
    VALIDATE(self, "", (copy.src_offset.z + copy.extent.x) <= src_extent.z);
    VALIDATE(self, "", copy.dst_offset.x <= dst_extent.x);
    VALIDATE(self, "", copy.dst_offset.y <= dst_extent.y);
    VALIDATE(self, "", copy.dst_offset.z <= dst_extent.z);
    VALIDATE(self, "", (copy.dst_offset.x + copy.extent.x) <= dst_extent.x);
    VALIDATE(self, "", (copy.dst_offset.y + copy.extent.x) <= dst_extent.y);
    VALIDATE(self, "", (copy.dst_offset.z + copy.extent.x) <= dst_extent.z);
  }

  if (self->status != Status::Success)
  {
    return;
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
  CommandEncoder *const self       = (CommandEncoder *) self_;
  Buffer *const         src        = (Buffer *) src_;
  Image *const          dst        = (Image *) dst_;
  u32 const             num_copies = (u32) copies.size;

  VALIDATE(self, "", num_copies > 0);
  VALIDATE(self, "", has_bits(src->desc.usage, gfx::BufferUsage::TransferSrc));
  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferImageCopy const &copy = copies[i];
    VALIDATE(self, "",
             is_valid_buffer_access(src->desc.size, copy.buffer_offset,
                                    gfx::WHOLE_SIZE));

    VALIDATE(self, "",
             is_valid_image_subresource_access(
                 dst->desc.aspects, dst->desc.mip_levels,
                 dst->desc.array_layers, copy.image_layers.aspects,
                 copy.image_layers.mip_level, 1,
                 copy.image_layers.first_array_layer,
                 copy.image_layers.num_array_layers));

    VALIDATE(self, "", copy.image_extent.x > 0);
    VALIDATE(self, "", copy.image_extent.y > 0);
    VALIDATE(self, "", copy.image_extent.z > 0);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, copy.image_layers.mip_level);
    VALIDATE(self, "", copy.image_extent.x <= dst_extent.x);
    VALIDATE(self, "", copy.image_extent.y <= dst_extent.y);
    VALIDATE(self, "", copy.image_extent.z <= dst_extent.z);
    VALIDATE(self, "",
             (copy.image_offset.x + copy.image_extent.x) <= dst_extent.x);
    VALIDATE(self, "",
             (copy.image_offset.y + copy.image_extent.y) <= dst_extent.y);
    VALIDATE(self, "",
             (copy.image_offset.z + copy.image_extent.z) <= dst_extent.z);
  }

  if (self->status != Status::Success)
  {
    return;
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
  CommandEncoder *const self      = (CommandEncoder *) self_;
  Image *const          src       = (Image *) src_;
  Image *const          dst       = (Image *) dst_;
  u32 const             num_blits = (u32) blits.size;

  VALIDATE(self, "", num_blits > 0);
  VALIDATE(self, "", has_bits(src->desc.usage, gfx::ImageUsage::TransferSrc));
  VALIDATE(self, "", has_bits(dst->desc.usage, gfx::ImageUsage::TransferDst));
  for (u32 i = 0; i < num_blits; i++)
  {
    gfx::ImageBlit const &blit = blits[i];

    VALIDATE(self, "",
             is_valid_image_subresource_access(
                 src->desc.aspects, src->desc.mip_levels,
                 src->desc.array_layers, blit.src_layers.aspects,
                 blit.src_layers.mip_level, 1,
                 blit.src_layers.first_array_layer,
                 blit.src_layers.num_array_layers));

    VALIDATE(self, "",
             is_valid_image_subresource_access(
                 dst->desc.aspects, dst->desc.mip_levels,
                 dst->desc.array_layers, blit.dst_layers.aspects,
                 blit.dst_layers.mip_level, 1,
                 blit.dst_layers.first_array_layer,
                 blit.dst_layers.num_array_layers));

    gfx::Extent3D src_extent =
        mip_down(src->desc.extent, blit.src_layers.mip_level);
    gfx::Extent3D dst_extent =
        mip_down(dst->desc.extent, blit.dst_layers.mip_level);
    VALIDATE(self, "", blit.src_offsets[0].x <= src_extent.x);
    VALIDATE(self, "", blit.src_offsets[0].y <= src_extent.y);
    VALIDATE(self, "", blit.src_offsets[0].z <= src_extent.z);
    VALIDATE(self, "", blit.src_offsets[1].x <= src_extent.x);
    VALIDATE(self, "", blit.src_offsets[1].y <= src_extent.y);
    VALIDATE(self, "", blit.src_offsets[1].z <= src_extent.z);
    VALIDATE(self, "", blit.dst_offsets[0].x <= dst_extent.x);
    VALIDATE(self, "", blit.dst_offsets[0].y <= dst_extent.y);
    VALIDATE(self, "", blit.dst_offsets[0].z <= dst_extent.z);
    VALIDATE(self, "", blit.dst_offsets[1].x <= dst_extent.x);
    VALIDATE(self, "", blit.dst_offsets[1].y <= dst_extent.y);
    VALIDATE(self, "", blit.dst_offsets[1].z <= dst_extent.z);
  }

  if (self->status != Status::Success)
  {
    return;
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
                                      src->vk_image,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      num_blits, vk_blits, (VkFilter) filter);

  self->allocator.deallocate_typed(vk_blits, num_blits);
}

void CommandEncoderInterface::begin_render_pass(
    gfx::CommandEncoder self_, gfx::Framebuffer framebuffer_,
    gfx::RenderPass render_pass_, gfx::Offset render_offset,
    gfx::Extent              render_extent,
    Span<gfx::Color const>   color_attachments_clear_values,
    gfx::DepthStencil const &depth_stencil_attachment_clear_value)
{
  CommandEncoder *const self        = (CommandEncoder *) self_;
  Framebuffer *const    framebuffer = (Framebuffer *) framebuffer_;
  RenderPass *const     render_pass = (RenderPass *) render_pass_;
  u32 const  num_color_clear_values = (u32) color_attachments_clear_values.size;
  bool const has_depth_stencil_attachment =
      framebuffer->depth_stencil_attachment != nullptr;
  u32 const num_vk_clear_values =
      num_color_clear_values + (has_depth_stencil_attachment ? 1U : 0U);

  VALIDATE(self, "",
           is_render_pass_compatible(render_pass,
                                     Span{framebuffer->color_attachments,
                                          framebuffer->num_color_attachments},
                                     framebuffer->depth_stencil_attachment));
  VALIDATE(self, "",
           color_attachments_clear_values.size ==
               framebuffer->num_color_attachments);
  VALIDATE(self, "", render_extent.x > 0);
  VALIDATE(self, "", render_extent.y > 0);
  VALIDATE(self, "", render_offset.x <= framebuffer->extent.x);
  VALIDATE(self, "", render_offset.y <= framebuffer->extent.y);
  VALIDATE(self, "",
           (render_offset.x + render_extent.x) <= framebuffer->extent.x);
  VALIDATE(self, "",
           (render_offset.y + render_extent.y) <= framebuffer->extent.y);

  if (self->status != Status::Success)
  {
    return;
  }

  VkClearValue vk_clear_values[gfx::MAX_COLOR_ATTACHMENTS + 1];

  {
    u32 ivk_clear_value = 0;
    for (u32 icolor_clear_value = 0;
         icolor_clear_value < num_color_clear_values;
         icolor_clear_value++, ivk_clear_value++)
    {
      gfx::Color const &color =
          color_attachments_clear_values[icolor_clear_value];
      memcpy(&vk_clear_values[ivk_clear_value].color, &color,
             sizeof(gfx::Color));
    }

    if (has_depth_stencil_attachment)
    {
      vk_clear_values[ivk_clear_value].depthStencil.depth =
          depth_stencil_attachment_clear_value.depth;
      vk_clear_values[ivk_clear_value].depthStencil.stencil =
          depth_stencil_attachment_clear_value.stencil;
    }
  }

  self->bound_render_pass = render_pass;
  self->bound_framebuffer = framebuffer;

  for (u32 i = 0; i < framebuffer->num_color_attachments; i++)
  {
    access_image(
        *self, *IMAGE_FROM_VIEW(framebuffer->color_attachments[i]),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        color_attachment_image_access(render_pass->color_attachments[i]),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }

  if (has_depth_stencil_attachment)
  {
    VkAccessFlags access = depth_stencil_attachment_image_access(
        render_pass->depth_stencil_attachment);
    access_image(
        *self,
        *IMAGE_FROM_VIEW(self->bound_framebuffer->depth_stencil_attachment),
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        access,
        has_write_access(access) ?
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
  }

  VkRect2D vk_render_area{.offset = VkOffset2D{.x = (i32) render_offset.x,
                                               .y = (i32) render_offset.y},
                          .extent = VkExtent2D{.width  = render_extent.x,
                                               .height = render_extent.y}};
  VkRenderPassBeginInfo begin_info{.sType =
                                       VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                   .pNext       = nullptr,
                                   .renderPass  = render_pass->vk_render_pass,
                                   .framebuffer = framebuffer->vk_framebuffer,
                                   .renderArea  = vk_render_area,
                                   .clearValueCount = num_vk_clear_values,
                                   .pClearValues    = vk_clear_values};

  self->device->vk_table.CmdBeginRenderPass(
      self->vk_command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandEncoderInterface::end_render_pass(gfx::CommandEncoder self_)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_render_pass != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdEndRenderPass(self->vk_command_buffer);
}

void CommandEncoderInterface::bind_compute_pipeline(
    gfx::CommandEncoder self_, gfx::ComputePipeline pipeline)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  if (self->status != Status::Success)
  {
    return;
  }

  self->bound_compute_pipeline  = (ComputePipeline *) pipeline;
  self->bound_graphics_pipeline = nullptr;

  self->device->vk_table.CmdBindPipeline(
      self->vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      self->bound_compute_pipeline->vk_pipeline);
}

void CommandEncoderInterface::bind_graphics_pipeline(
    gfx::CommandEncoder self_, gfx::GraphicsPipeline pipeline)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  if (self->status != Status::Success)
  {
    return;
  }

  self->bound_graphics_pipeline = (GraphicsPipeline *) pipeline;
  self->bound_compute_pipeline  = nullptr;

  self->device->vk_table.CmdBindPipeline(
      self->vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      self->bound_graphics_pipeline->vk_pipeline);
}

void CommandEncoderInterface::bind_descriptor_sets(
    gfx::CommandEncoder self_, Span<gfx::DescriptorHeap const> descriptor_heaps,
    Span<u32 const> groups, Span<u32 const> sets,
    Span<u32 const> dynamic_offsets)
{
  CommandEncoder *const self                = (CommandEncoder *) self_;
  u32 const             num_sets            = (u32) sets.size;
  u32 const             num_dynamic_offsets = (u32) dynamic_offsets.size;

  VALIDATE(self, "", num_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE(self, "", descriptor_heaps.size == groups.size);
  VALIDATE(self, "", groups.size == sets.size);
  VALIDATE(self, "", num_dynamic_offsets <= num_sets);
  for (u32 iset = 0; iset < num_sets; iset++)
  {
    DescriptorHeap *heap = (DescriptorHeap *) descriptor_heaps[iset];
    VALIDATE(self, "",
             groups[iset] < heap->num_pools * heap->num_groups_per_pool);
    VALIDATE(self, "", sets[iset] < heap->num_sets_per_group);
  }

  VkDescriptorSet vk_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];

  for (u32 iset = 0; iset < num_sets; iset++)
  {
    DescriptorHeap *heap = (DescriptorHeap *) descriptor_heaps[iset];
    vk_sets[iset] =
        heap->vk_descriptor_sets[heap->num_sets_per_group * groups[iset] +
                                 sets[iset]];
    self->bound_descriptor_set_heaps[iset]  = heap;
    self->bound_descriptor_set_groups[iset] = groups[iset];
    self->bound_descriptor_sets[iset]       = sets[iset];
  }
  self->num_bound_descriptor_sets = num_sets;

  VkPipelineBindPoint vk_bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
  VkPipelineLayout    vk_layout     = nullptr;

  if (self->bound_compute_pipeline != nullptr)
  {
    vk_bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
    vk_layout     = self->bound_compute_pipeline->vk_layout;
  }
  else if (self->bound_graphics_pipeline != nullptr)
  {
    vk_bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vk_layout     = self->bound_graphics_pipeline->vk_layout;
  }
  else
  {
    UNREACHABLE();
  }

  self->device->vk_table.CmdBindDescriptorSets(
      self->vk_command_buffer, vk_bind_point, vk_layout, 0, num_sets, vk_sets,
      num_dynamic_offsets, dynamic_offsets.data);
}

void CommandEncoderInterface::push_constants(gfx::CommandEncoder self_,
                                             Span<u8 const> push_constants_data)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "",
           !(self->bound_compute_pipeline == nullptr &&
             self->bound_graphics_pipeline == nullptr));
  VALIDATE(self, "",
           push_constants_data.size_bytes() <= gfx::MAX_PUSH_CONSTANT_SIZE);
  VALIDATE(self, "", push_constants_data.size_bytes() % 4 == 0);

  if (self->status != Status::Success)
  {
    return;
  }

  VkPipelineLayout vk_layout = nullptr;

  if (self->bound_compute_pipeline != nullptr)
  {
    vk_layout = self->bound_compute_pipeline->vk_layout;
  }
  else if (self->bound_graphics_pipeline != nullptr)
  {
    vk_layout = self->bound_graphics_pipeline->vk_layout;
  }
  else
  {
    UNREACHABLE();
  }

  self->device->vk_table.CmdPushConstants(
      self->vk_command_buffer, vk_layout, VK_SHADER_STAGE_ALL, 0,
      (u32) push_constants_data.size_bytes(), push_constants_data.data);
}

void CommandEncoderInterface::dispatch(gfx::CommandEncoder self_,
                                       u32 group_count_x, u32 group_count_y,
                                       u32 group_count_z)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_compute_pipeline != nullptr);
  VALIDATE(self, "", group_count_x <= gfx::MAX_COMPUTE_GROUP_COUNT_X);
  VALIDATE(self, "", group_count_y <= gfx::MAX_COMPUTE_GROUP_COUNT_Y);
  VALIDATE(self, "", group_count_z <= gfx::MAX_COMPUTE_GROUP_COUNT_Z);

  access_compute_bindings(*self);

  self->device->vk_table.CmdDispatch(self->vk_command_buffer, group_count_x,
                                     group_count_y, group_count_z);
}

void CommandEncoderInterface::dispatch_indirect(gfx::CommandEncoder self_,
                                                gfx::Buffer buffer_, u64 offset)
{
  CommandEncoder *const self   = (CommandEncoder *) self_;
  Buffer *const         buffer = (Buffer *) buffer_;

  VALIDATE(self, "", self->bound_compute_pipeline != nullptr);
  VALIDATE(self, "",
           has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  VALIDATE(self, "",
           is_valid_aligned_buffer_access(buffer->desc.size, offset,
                                          sizeof(gfx::IndirectDispatchCommand),
                                          4));

  access_compute_bindings(*self);

  self->device->vk_table.CmdDispatchIndirect(self->vk_command_buffer,
                                             buffer->vk_buffer, offset);
}

void CommandEncoderInterface::set_viewport(gfx::CommandEncoder  self_,
                                           gfx::Viewport const &viewport)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);
  VALIDATE(self, "", viewport.min_depth >= 0.0F);
  VALIDATE(self, "", viewport.max_depth <= 1.0F);

  if (self->status != Status::Success)
  {
    return;
  }

  VkViewport vk_viewport{.x        = viewport.offset.x,
                         .y        = viewport.offset.y,
                         .width    = viewport.extent.x,
                         .height   = viewport.extent.y,
                         .minDepth = viewport.min_depth,
                         .maxDepth = viewport.max_depth};
  self->device->vk_table.CmdSetViewport(self->vk_command_buffer, 0, 1,
                                        &vk_viewport);
}

void CommandEncoderInterface::set_scissor(gfx::CommandEncoder self_,
                                          gfx::Offset         scissor_offset,
                                          gfx::Extent         scissor_extent)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  VkRect2D vk_scissor{
      .offset = VkOffset2D{(i32) scissor_offset.x, (i32) scissor_offset.y},
      .extent = VkExtent2D{scissor_extent.x, scissor_extent.y}};
  self->device->vk_table.CmdSetScissor(self->vk_command_buffer, 0, 1,
                                       &vk_scissor);
}

void CommandEncoderInterface::set_blend_constants(gfx::CommandEncoder self_,
                                                  Vec4 blend_constant)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  f32 vk_constants[4] = {blend_constant.x, blend_constant.y, blend_constant.z,
                         blend_constant.w};
  self->device->vk_table.CmdSetBlendConstants(self->vk_command_buffer,
                                              vk_constants);
}

void CommandEncoderInterface::set_stencil_compare_mask(
    gfx::CommandEncoder self_, gfx::StencilFaces faces, u32 mask)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdSetStencilCompareMask(
      self->vk_command_buffer, (VkStencilFaceFlags) faces, mask);
}

void CommandEncoderInterface::set_stencil_reference(gfx::CommandEncoder self_,
                                                    gfx::StencilFaces   faces,
                                                    u32 reference)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdSetStencilReference(
      self->vk_command_buffer, (VkStencilFaceFlags) faces, reference);
}

void CommandEncoderInterface::set_stencil_write_mask(gfx::CommandEncoder self_,
                                                     gfx::StencilFaces   faces,
                                                     u32                 mask)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdSetStencilWriteMask(
      self->vk_command_buffer, (VkStencilFaceFlags) faces, mask);
}

void CommandEncoderInterface::bind_vertex_buffers(
    gfx::CommandEncoder self_, Span<gfx::Buffer const> vertex_buffers,
    Span<u64 const> offsets)
{
  CommandEncoder *const self               = (CommandEncoder *) self_;
  u32 const             num_vertex_buffers = (u32) vertex_buffers.size;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);
  VALIDATE(self, "", num_vertex_buffers > 0);
  VALIDATE(self, "", num_vertex_buffers <= gfx::MAX_VERTEX_ATTRIBUTES);
  VALIDATE(self, "", offsets.size == vertex_buffers.size);
  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    u64 const     offset = offsets[i];
    Buffer *const buffer = (Buffer *) vertex_buffers[i];
    VALIDATE(self, "", offset < buffer->desc.size);
    VALIDATE(self, "",
             has_bits(buffer->desc.usage, gfx::BufferUsage::VertexBuffer));
  }

  if (self->status != Status::Success)
  {
    return;
  }

  VkBuffer vk_buffers[gfx::MAX_VERTEX_ATTRIBUTES];

  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    Buffer *buffer                = (Buffer *) vertex_buffers[i];
    vk_buffers[i]                 = buffer->vk_buffer;
    self->bound_vertex_buffers[i] = buffer;
  }
  self->num_bound_vertex_buffers = num_vertex_buffers;

  self->device->vk_table.CmdBindVertexBuffers(
      self->vk_command_buffer, 0, num_vertex_buffers, vk_buffers, offsets.data);
}

void CommandEncoderInterface::bind_index_buffer(gfx::CommandEncoder self_,
                                                gfx::Buffer    index_buffer_,
                                                u64            offset,
                                                gfx::IndexType index_type)
{
  CommandEncoder *const self         = (CommandEncoder *) self_;
  Buffer *const         index_buffer = (Buffer *) index_buffer_;
  u64 const             index_size   = index_type_size(index_type);

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);
  VALIDATE(self, "", offset < index_buffer->desc.size);
  VALIDATE(self, "", (offset % index_size) == 0);
  VALIDATE(self, "",
           has_bits(index_buffer->desc.usage, gfx::BufferUsage::IndexBuffer));

  if (self->status != Status::Success)
  {
    return;
  }

  self->bound_index_buffer        = index_buffer;
  self->bound_index_type          = index_type;
  self->bound_index_buffer_offset = offset;

  self->device->vk_table.CmdBindIndexBuffer(self->vk_command_buffer,
                                            index_buffer->vk_buffer, offset,
                                            (VkIndexType) index_type);
}

void CommandEncoderInterface::draw(gfx::CommandEncoder self_, u32 first_index,
                                   u32 num_indices, i32 vertex_offset,
                                   u32 first_instance, u32 num_instances)
{
  CommandEncoder *const self       = (CommandEncoder *) self_;
  u64 const             index_size = index_type_size(self->bound_index_type);

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);
  VALIDATE(self, "", self->bound_render_pass != nullptr);
  VALIDATE(self, "", self->bound_framebuffer != nullptr);
  VALIDATE(self, "", self->bound_index_buffer != nullptr);
  VALIDATE(self, "",
           (self->bound_index_buffer_offset + first_index * index_size) <
               self->bound_index_buffer->desc.size);
  VALIDATE(self, "",
           (self->bound_index_buffer_offset +
            (first_index + num_indices) * index_size) <=
               self->bound_index_buffer->desc.size);

  if (self->status != Status::Success)
  {
    return;
  }

  for (u32 i = 0; i < self->num_bound_vertex_buffers; i++)
  {
    access_buffer(*self, *self->bound_vertex_buffers[i],
                  VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                  VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
  }

  access_buffer(*self, *self->bound_index_buffer,
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT);

  access_graphics_bindings(*self);

  self->device->vk_table.CmdDrawIndexed(self->vk_command_buffer, num_indices,
                                        num_instances, first_index,
                                        vertex_offset, first_instance);
}

void CommandEncoderInterface::draw_indirect(gfx::CommandEncoder self_,
                                            gfx::Buffer buffer_, u64 offset,
                                            u32 draw_count, u32 stride)
{
  CommandEncoder *const self   = (CommandEncoder *) self_;
  Buffer *const         buffer = (Buffer *) buffer_;

  VALIDATE(self, "", self->bound_graphics_pipeline != nullptr);
  VALIDATE(self, "", self->bound_render_pass != nullptr);
  VALIDATE(self, "", self->bound_framebuffer != nullptr);
  VALIDATE(self, "", self->bound_index_buffer != nullptr);
  VALIDATE(self, "",
           has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  VALIDATE(self, "", offset < buffer->desc.size);
  VALIDATE(self, "", (offset + (u64) draw_count * stride) <= buffer->desc.size);
  VALIDATE(self, "", stride % 4 == 0);
  VALIDATE(self, "", stride >= sizeof(gfx::IndirectDrawCommand));

  if (self->status != Status::Success)
  {
    return;
  }

  for (u32 i = 0; i < self->num_bound_vertex_buffers; i++)
  {
    access_buffer(*self, *self->bound_vertex_buffers[i],
                  VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                  VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
  }

  access_buffer(*self, *self->bound_index_buffer,
                VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT);

  access_graphics_bindings(*self);

  self->device->vk_table.CmdDrawIndexedIndirect(
      self->vk_command_buffer, buffer->vk_buffer, offset, draw_count, stride);
}

void CommandEncoderInterface::present_image(gfx::CommandEncoder self_,
                                            gfx::Image          image_)
{
  CommandEncoder *const self  = (CommandEncoder *) self_;
  Image *const          image = (Image *) image_;

  if (self->status != Status::Success)
  {
    return;
  }

  access_image(*self, *image, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
               VK_ACCESS_NONE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

}        // namespace vk
}        // namespace ash
