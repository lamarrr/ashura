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

#define RETURN_IF_ERR(...)              \
  do                                    \
  {                                     \
    status = __VA_ARGS__;               \
    if (status != gfx::Status::Success) \
    {                                   \
      return;                           \
    }                                   \
  } while (false)

// todo(lamarrr): define macros for checks, use debug name

#define ALLOC_NUM(allocator, type, num) \
  (type *) allocator.allocate(allocator.data, (usize) sizeof(type) * num, alignof(type))

#define ALLOC_ARRAY(allocator, type, num)
#define ALLOC_OBJECT(allocator, type)
#define ALLOC_OBJECTS(allocator, type, num)

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

constexpr VkAccessFlags color_attachment_image_access(gfx::RenderPassAttachment const &attachment)
{
  VkAccessFlags access = VK_ACCESS_NONE;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
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

constexpr VkAccessFlags
    depth_stencil_attachment_image_access(gfx::RenderPassAttachment const &attachment)
{
  VkAccessFlags access = VK_ACCESS_NONE;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store || attachment.store_op == gfx::StoreOp::DontCare ||
      attachment.stencil_load_op == gfx::LoadOp::Clear ||
      attachment.stencil_load_op == gfx::LoadOp::DontCare ||
      attachment.stencil_store_op == gfx::StoreOp::Store ||
      attachment.stencil_store_op == gfx::StoreOp::DontCare)
  {
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  }

  if (attachment.load_op == gfx::LoadOp::Load || attachment.stencil_load_op == gfx::LoadOp::Load)
  {
    access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  }

  return access;
}

constexpr bool has_read_access(VkAccessFlags access)
{
  return has_any_bit(
      access,
      (VkAccessFlags) (VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
                       VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT |
                       VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
                       VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT |
                       VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT |
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
      (VkAccessFlags) (VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT |
                       VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT |
                       VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT |
                       VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT |
                       VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR |
                       VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV |
                       VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV));
}

bool sync(BufferState &state, BufferAccess request, VkBufferMemoryBarrier &barrier,
          VkPipelineStageFlags &src_stages, VkPipelineStageFlags &dst_stages)
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
        state.sequence  = AccessSequence::Write;
        state.access[0] = BufferAccess{.stages = request.stages, .access = request.access};
        return false;
      }
      else if (has_read)
      {
        state.sequence  = AccessSequence::Reads;
        state.access[0] = BufferAccess{.stages = request.stages, .access = request.access};
        return false;
      }
    }
    break;
    case AccessSequence::Reads:
    {
      if (has_write)
      {
        // wait till done reading before modifying
        // reset access sequence since all stages following this write need to wait on this write
        state.sequence                    = AccessSequence::Write;
        BufferAccess const previous_reads = state.access[0];
        state.access[0]       = BufferAccess{.stages = request.stages, .access = request.access};
        state.access[1]       = BufferAccess{};
        src_stages            = previous_reads.stages;
        barrier.srcAccessMask = previous_reads.access;
        dst_stages            = request.stages;
        barrier.dstAccessMask = request.access;

        return true;
      }
      else if (has_read)
      {
        // combine all subsequent reads, so the next writer knows to wait on all combined reads to
        // complete
        state.sequence                    = AccessSequence::Reads;
        BufferAccess const previous_reads = state.access[0];
        state.access[0] = BufferAccess{.stages = previous_reads.stages | request.stages,
                                       .access = previous_reads.access | request.access};
        return false;
      }
    }
    break;
    case AccessSequence::Write:
    {
      if (has_write)
      {
        // wait till done writing before modifying
        // remove previous write since this access already waits on another access to complete
        // and the next access will have to wait on this access
        state.sequence                    = AccessSequence::Write;
        BufferAccess const previous_write = state.access[0];
        state.access[0]       = BufferAccess{.stages = request.stages, .access = request.access};
        state.access[1]       = BufferAccess{};
        src_stages            = previous_write.stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = previous_write.access;
        barrier.dstAccessMask = request.access;
        return true;
      }
      else if (has_read)
      {
        // wait till all write stages are done
        state.sequence        = AccessSequence::ReadAfterWrite;
        state.access[1]       = BufferAccess{.stages = request.stages, .access = request.access};
        src_stages            = state.access[0].stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = state.access[0].access;
        barrier.dstAccessMask = request.access;
        return true;
      }
    }
    break;
    case AccessSequence::ReadAfterWrite:
    {
      if (has_write)
      {
        // wait for all reading stages only
        // stages can be reset and point only to the latest write stage, since they all need to wait
        // for this write anyway.
        state.sequence                    = AccessSequence::Write;
        BufferAccess const previous_reads = state.access[1];
        state.access[0]       = BufferAccess{.stages = request.stages, .access = request.access};
        state.access[1]       = BufferAccess{};
        src_stages            = previous_reads.stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = previous_reads.access;
        barrier.dstAccessMask = request.access;
        return true;
      }
      else if (has_read)
      {
        // wait for all write stages to be done
        // no need to wait on other reads since we are only performing a read
        // mask all subsequent reads so next writer knows to wait on all reads to complete

        // if stage and access intersects previous barrier, no need to add new one

        if (has_any_bit(state.access[1].stages, request.stages) &&
            has_any_bit(state.access[1].access, request.access))
        {
          return false;
        }
        else
        {
          state.sequence = AccessSequence::ReadAfterWrite;
          state.access[1].stages |= request.stages;
          state.access[1].access |= request.access;
          src_stages            = state.access[0].stages;
          dst_stages            = request.stages;
          barrier.srcAccessMask = state.access[0].access;
          barrier.dstAccessMask = request.access;
          return true;
        }
      }
    }
    break;
    default:
      return false;
      break;
  }

  return false;
}

// layout transitions are considered write operations even if only a read happens so multiple ones
// can't happen at the same time
//
// we'll kind of be waiting on a barrier operation which doesn't make sense cos the barrier might have already taken care of us
// even when they both only perform reads
//
// if their scopes don't line-up, they won't observe the effects same

bool sync(ImageState &state, ImageAccess request, VkImageMemoryBarrier &barrier,
          VkPipelineStageFlags &src_stages, VkPipelineStageFlags &dst_stages)
{
  // TODO(lamarrr): make sure aspects are filled
  VkImageLayout const current_layout          = state.access[0].layout;
  bool const          needs_layout_transition = current_layout != request.layout;
  bool const          has_write = has_write_access(request.access) || needs_layout_transition;
  bool const          has_read  = has_read_access(request.access);
  barrier.oldLayout             = current_layout;
  barrier.newLayout             = request.layout;

  switch (state.sequence)
  {
      // no sync needed, no accessor before this
    case AccessSequence::None:
    {
      if (has_write)
      {
        state.sequence  = AccessSequence::Write;
        state.access[0] = ImageAccess{
            .stages = request.stages, .access = request.access, .layout = request.layout};

        if (needs_layout_transition)
        {
          src_stages            = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
          dst_stages            = request.stages;
          barrier.srcAccessMask = VK_ACCESS_NONE;
          barrier.dstAccessMask = request.access;
          return true;
        }
        else
        {
          return false;
        }
      }
      else if (has_read)
      {
        state.sequence  = AccessSequence::Reads;
        state.access[0] = ImageAccess{
            .stages = request.stages, .access = request.access, .layout = request.layout};
        return false;
      }
    }
    break;
    case AccessSequence::Reads:
    {
      if (has_write)
      {
        // wait till done reading before modifying
        // reset access sequence since all stages following this write need to wait on this write
        state.sequence                   = AccessSequence::Write;
        ImageAccess const previous_reads = state.access[0];
        state.access[0]                  = ImageAccess{
                             .stages = request.stages, .access = request.access, .layout = request.layout};
        state.access[1]       = ImageAccess{};
        src_stages            = previous_reads.stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = previous_reads.access;
        barrier.dstAccessMask = request.access;
        return true;
      }
      else if (has_read)
      {
        // combine all subsequent reads, so the next writer knows to wait on all combined reads to
        // complete
        state.sequence                   = AccessSequence::Reads;
        ImageAccess const previous_reads = state.access[0];
        state.access[0] = ImageAccess{.stages = previous_reads.stages | request.stages,
                                      .access = previous_reads.access | request.access,
                                      .layout = request.layout};
        return false;
      }
    }
    break;
    case AccessSequence::Write:
    {
      if (has_write)
      {
        // wait till done writing before modifying
        // remove previous write since this access already waits on another access to complete
        // and the next access will have to wait on this access
        state.sequence                   = AccessSequence::Write;
        ImageAccess const previous_write = state.access[0];
        state.access[0]                  = ImageAccess{
                             .stages = request.stages, .access = request.access, .layout = request.layout};
        state.access[1]       = ImageAccess{};
        src_stages            = previous_write.stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = previous_write.access;
        barrier.dstAccessMask = request.access;
        return true;
      }
      else if (has_read)
      {
        // wait till all write stages are done
        state.sequence  = AccessSequence::ReadAfterWrite;
        state.access[1] = ImageAccess{
            .stages = request.stages, .access = request.access, .layout = request.layout};
        src_stages            = state.access[0].stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = state.access[0].access;
        barrier.dstAccessMask = request.access;
        return true;
      }
    }
    break;
    case AccessSequence::ReadAfterWrite:
    {
      if (has_write)
      {
        // wait for all reading stages only
        // stages can be reset and point only to the latest write stage, since they all need to wait
        // for this write anyway.
        state.sequence                   = AccessSequence::Write;
        ImageAccess const previous_reads = state.access[1];
        state.access[0]                  = ImageAccess{
                             .stages = request.stages, .access = request.access, .layout = request.layout};
        state.access[1]       = ImageAccess{};
        src_stages            = previous_reads.stages;
        dst_stages            = request.stages;
        barrier.srcAccessMask = previous_reads.access;
        barrier.dstAccessMask = request.access;
        return true;
      }
      else if (has_read)
      {
        // wait for all write stages to be done
        // no need to wait on other reads since we are only performing a read
        // mask all subsequent reads so next writer knows to wait on all reads to complete
        //
        // if stage and access intersects previous barrier, no need to add new one as we'll observe the effect
        state.sequence = AccessSequence::ReadAfterWrite;

        if (has_any_bit(state.access[1].stages, request.stages) &&
            has_any_bit(state.access[1].access, request.access))
        {
          return false;
        }
        else
        {
          state.access[1].stages |= request.stages;
          state.access[1].access |= request.access;
          src_stages            = state.access[0].stages;
          dst_stages            = request.stages;
          barrier.srcAccessMask = state.access[0].access;
          barrier.dstAccessMask = request.access;
          return true;
        }
      }
    }
    break;
    default:
      return false;
  }
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
  // check size

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

  Buffer *buffer = ALLOC_NUM(allocator, Buffer, 1);
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
  // check range
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

  BufferView *view = ALLOC_NUM(allocator, BufferView, 1);

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
  VkImage                 vk_image;                                                               \
  VmaAllocation           vma_allocation;                                                         \
  VmaAllocationInfo       vma_allocation_info;                                                    \
  VmaAllocationCreateInfo vma_allocation_create_info = {.usage = VMA_MEMORY_USAGE_GPU_ONLY};      \
  VK_ERR(vmaCreateImage(vma_allocator, &create_info, &vma_allocation_create_info, &vk_image,      \
                        &vma_allocation, &vma_allocation_info));                                  \
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
  Image *image = ALLOC_NUM(allocator, Image, 1);                                                  \
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
  // check size
  // check aspects
  // check format support against usage
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

  VkBufferImageCopy *vk_copies = ALLOC_NUM(allocator, VkBufferImageCopy, (u32) copies.size());

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
  // check aspect match
  // check subresource range
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

  ImageView *view = ALLOC_NUM(allocator, ImageView, 1);
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

  Sampler *sampler = ALLOC_NUM(allocator, Sampler, 1);
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
  // check size
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

  Shader *shader = ALLOC_NUM(allocator, Shader, 1);
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
      desc.depth_stencil_attachment.format == gfx::Format::Undefined ? 0U : 1U;
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

  RenderPass *render_pass = ALLOC_NUM(allocator, RenderPass, 1);
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

  Framebuffer *framebuffer = ALLOC_NUM(allocator, Framebuffer, 1);
  if (framebuffer == nullptr)
  {
    vk_table.DestroyFramebuffer(vk_device, vk_framebuffer, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (framebuffer) Framebuffer{.refcount = 1, .desc = desc, .vk_framebuffer = vk_framebuffer};

  return stx::Ok((gfx::Framebuffer) framebuffer);
}

stx::Result<gfx::DescriptorLayout, gfx::Status>
    DeviceImpl::create_descriptor_layout(gfx::DescriptorLayoutDesc const &desc)
{
  u32                           num_bindings = (u32) desc.bindings.size();
  VkDescriptorSetLayoutBinding *vk_bindings =
      ALLOC_NUM(allocator, VkDescriptorSetLayoutBinding, num_bindings);

  if (vk_bindings == nullptr)
  {
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  gfx::DescriptorCount count;

  for (usize i = 0; i < num_bindings; i++)
  {
    gfx::DescriptorBindingDesc const &binding = desc.bindings.data()[i];
    vk_bindings[i] = VkDescriptorSetLayoutBinding{.binding        = binding.binding,
                                                  .descriptorType = (VkDescriptorType) binding.type,
                                                  .descriptorCount    = binding.count,
                                                  .stageFlags         = VK_SHADER_STAGE_ALL,
                                                  .pImmutableSamplers = nullptr};

    switch (binding.type)
    {
      case gfx::DescriptorType::Sampler:
        count.samplers += binding.count;
        break;
      case gfx::DescriptorType::CombinedImageSampler:
        count.combined_image_samplers += binding.count;
        break;
      case gfx::DescriptorType::SampledImage:
        count.sampled_images += binding.count;
        break;
      case gfx::DescriptorType::StorageImage:
        count.storage_images += binding.count;
        break;
      case gfx::DescriptorType::UniformTexelBuffer:
        count.uniform_texel_buffers += binding.count;
        break;
      case gfx::DescriptorType::StorageTexelBuffer:
        count.storage_texel_buffers += binding.count;
        break;
      case gfx::DescriptorType::UniformBuffer:
        count.uniform_buffers += binding.count;
        break;
      case gfx::DescriptorType::StorageBuffer:
        count.storage_buffers += binding.count;
        break;
      case gfx::DescriptorType::DynamicUniformBuffer:
        count.dynamic_uniform_buffers += binding.count;
        break;
      case gfx::DescriptorType::DynamicStorageBuffer:
        count.dynamic_storage_buffers += binding.count;
        break;
      case gfx::DescriptorType::InputAttachment:
        count.input_attachments += binding.count;
        break;
    }
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

  DescriptorLayout *layout = ALLOC_NUM(allocator, DescriptorLayout, 1);
  if (layout == nullptr)
  {
    vk_table.DestroyDescriptorSetLayout(vk_device, vk_layout, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (layout) DescriptorLayout{.refcount = 1, .count = count, .vk_layout = vk_layout};

  return stx::Ok((gfx::DescriptorLayout) layout);
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

  PipelineCache *cache = ALLOC_NUM(allocator, PipelineCache, 1);
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
      .mapEntryCount = (u32) desc.compute_shader.specialization_constants.size(),
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
      .pSetLayouts            = &((DescriptorLayout *) desc.descriptor_layout)->vk_layout,
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

  ComputePipeline *pipeline = ALLOC_NUM(allocator, ComputePipeline, 1);
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
      .mapEntryCount = (u32) desc.vertex_shader.specialization_constants.size(),
      .pMapEntries =
          (VkSpecializationMapEntry const *) desc.vertex_shader.specialization_constants.data(),
      .dataSize = desc.vertex_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.vertex_shader.specialization_constants_data.data()};

  VkSpecializationInfo vk_fs_specialization{
      .mapEntryCount = (u32) desc.fragment_shader.specialization_constants.size(),
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
      .pSetLayouts            = &((DescriptorLayout *) desc.descriptor_layout)->vk_layout,
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
      .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext                   = nullptr,
      .flags                   = 0,
      .depthClampEnable        = (VkBool32) desc.rasterization_state.depth_clamp_enable,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode             = (VkPolygonMode) desc.rasterization_state.polygon_mode,
      .cullMode                = (VkCullModeFlags) desc.rasterization_state.cull_mode,
      .frontFace               = (VkFrontFace) desc.rasterization_state.front_face,
      .depthBiasEnable         = (VkBool32) desc.rasterization_state.depth_bias_enable,
      .depthBiasConstantFactor = desc.rasterization_state.depth_bias_constant_factor,
      .depthBiasClamp          = desc.rasterization_state.depth_bias_clamp,
      .depthBiasSlopeFactor    = desc.rasterization_state.depth_bias_slope_factor,
      .lineWidth               = 1.0F};

  VkPipelineMultisampleStateCreateInfo multisample_state{
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext                 = nullptr,
      .flags                 = 0,
      .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable   = (VkBool32) false,
      .minSampleShading      = 1,
      .pSampleMask           = nullptr,
      .alphaToCoverageEnable = (VkBool32) false,
      .alphaToOneEnable      = (VkBool32) false};

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext                 = nullptr,
      .flags                 = 0,
      .depthTestEnable       = (VkBool32) desc.depth_stencil_state.depth_test_enable,
      .depthWriteEnable      = (VkBool32) desc.depth_stencil_state.depth_write_enable,
      .depthCompareOp        = (VkCompareOp) desc.depth_stencil_state.depth_compare_op,
      .depthBoundsTestEnable = (VkBool32) desc.depth_stencil_state.depth_bounds_test_enable,
      .stencilTestEnable     = (VkBool32) desc.depth_stencil_state.stencil_test_enable,
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

  GraphicsPipeline *pipeline = ALLOC_NUM(allocator, GraphicsPipeline, 1);
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

stx::Result<usize, gfx::Status> DeviceImpl::get_pipeline_cache_size(gfx::PipelineCache cache)
{
  usize    size;
  VkResult result =
      vk_table.GetPipelineCacheData(vk_device, ((PipelineCache *) cache)->vk_cache, &size, nullptr);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }
  return stx::Ok((usize) size);
}

stx::Result<usize, gfx::Status> DeviceImpl::get_pipeline_cache_data(gfx::PipelineCache cache,
                                                                    stx::Span<u8>      out)
{
  usize    size   = out.size_bytes();
  VkResult result = vk_table.GetPipelineCacheData(vk_device, ((PipelineCache *) cache)->vk_cache,
                                                  &size, out.data());
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }
  return stx::Ok((usize) size);
}

gfx::Status DeviceImpl::merge_pipeline_cache(gfx::PipelineCache                  dst,
                                             stx::Span<gfx::PipelineCache const> srcs)
{
  u32              num_srcs  = (u32) srcs.size();
  VkPipelineCache *vk_caches = ALLOC_NUM(allocator, VkPipelineCache, num_srcs);
  if (vk_caches == nullptr)
  {
    return gfx::Status::OutOfHostMemory;
  }

  for (u32 i = 0; i < num_srcs; i++)
  {
    vk_caches[i] = ((PipelineCache *) srcs.data()[i])->vk_cache;
  }

  VkResult result = vk_table.MergePipelineCaches(vk_device, ((PipelineCache *) dst)->vk_cache,
                                                 num_srcs, vk_caches);

  allocator.deallocate(allocator.data, vk_caches);

  return (gfx::Status) result;
}

gfx::Status DeviceImpl::wait_for_fences(stx::Span<gfx::Fence const> fences, bool all, u64 timeout)
{
  u32      num_fences = (u32) fences.size();
  VkFence *vk_fences  = ALLOC_NUM(allocator, VkFence, num_fences);
  ASH_CHECK(vk_fences != nullptr);
  for (u32 i = 0; i < num_fences; i++)
  {
    vk_fences[i] = ((Fence *) fences.data()[i])->vk_fence;
  }

  VkResult result =
      vk_table.WaitForFences(vk_device, num_fences, vk_fences, (VkBool32) all, timeout);

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

void DeviceImpl::submit(gfx::CommandEncoder &encoder, gfx::Fence signal_fence)
{
  CommandEncoderImpl *impl = (CommandEncoderImpl *) encoder.to_impl();
  VkSubmitInfo        info{.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                           .pNext                = nullptr,
                           .waitSemaphoreCount   = 1,
                           .pWaitSemaphores      = nullptr,
                           .pWaitDstStageMask    = nullptr,
                           .commandBufferCount   = 1,
                           .pCommandBuffers      = &impl->vk_command_buffer,
                           .signalSemaphoreCount = 1,
                           .pSignalSemaphores    = nullptr};
  ASH_CHECK(vk_table.QueueSubmit(vk_queue, 1, &info, ((Fence *) signal_fence)->vk_fence) ==
            VK_SUCCESS);
  // TODO(LAMARRR): handle, return error if any
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
  if (status != gfx::Status::Success)
  {
    return;
  }
  VkCommandBufferBeginInfo info{.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                .pNext            = nullptr,
                                .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                .pInheritanceInfo = nullptr};
  status = (gfx::Status) device->vk_table.BeginCommandBuffer(vk_command_buffer, &info);
}

gfx::Status CommandEncoderImpl::end()
{
  if (status != gfx::Status::Success)
  {
    return status;
  }
  device->vk_table.EndCommandBuffer(vk_command_buffer);
}

void CommandEncoderImpl::reset()
{
  device->vk_table.ResetCommandPool(device->vk_device, vk_command_pool, 0);
  status = gfx::Status::Success;
  // reset status
  // preserve buffers
}

void CommandEncoderImpl::begin_debug_marker(char const *region_name, Vec4 color)
{
  if (status != gfx::Status::Success)
  {
    return;
  }
  VkDebugMarkerMarkerInfoEXT info{.sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
                                  .pNext       = nullptr,
                                  .pMarkerName = region_name,
                                  .color       = {color.x, color.y, color.z, color.w}};
  device->vk_table.CmdDebugMarkerBeginEXT(vk_command_buffer, &info);
}

void CommandEncoderImpl::end_debug_marker()
{
  if (status != gfx::Status::Success)
  {
    return;
  }
  device->vk_table.CmdDebugMarkerEndEXT(vk_command_buffer);
}

void CommandEncoderImpl::fill_buffer(gfx::Buffer dst, u64 offset, u64 size, u32 data)
{
  if (status != gfx::Status::Success)
  {
    return;
  }
  // check sizes
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
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32           num_copies = (u32) copies.size();
  VkBufferCopy *vk_copies  = ALLOC_NUM(allocator, VkBufferCopy, num_copies);

  if (vk_copies == nullptr)
  {
    status = gfx::Status::OutOfHostMemory;
    return;
  }

  Buffer     *src_impl = (Buffer *) src;
  Buffer     *dst_impl = (Buffer *) dst;
  BufferScope scope    = transfer_buffer_scope(dst_impl->desc.usage);

  sync_acquire_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);

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
  if (status != gfx::Status::Success)
  {
    return;
  }
  // check sizes
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
  if (status != gfx::Status::Success)
  {
    return;
  }
  u32                      num_ranges = (u32) ranges.size();
  VkImageSubresourceRange *vk_ranges  = ALLOC_NUM(allocator, VkImageSubresourceRange, num_ranges);

  if (vk_ranges == nullptr)
  {
    status = gfx::Status::OutOfHostMemory;
    return;
  }

  Image     *dst_impl = (Image *) dst;
  ImageScope scope    = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     scope, (VkImageAspectFlags) dst_impl->desc.aspects);

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
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32                      num_ranges = (u32) ranges.size();
  VkImageSubresourceRange *vk_ranges  = ALLOC_NUM(allocator, VkImageSubresourceRange, num_ranges);

  if (vk_ranges == nullptr)
  {
    status = gfx::Status::OutOfHostMemory;
    return;
  }

  Image     *dst_impl = (Image *) dst;
  ImageScope scope    = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     scope, (VkImageAspectFlags) dst_impl->desc.aspects);

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
  // check copies > 0
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32          num_copies = (u32) copies.size();
  VkImageCopy *vk_copies  = ALLOC_NUM(allocator, VkImageCopy, num_copies);

  if (vk_copies == nullptr)
  {
    status = gfx::Status::OutOfHostMemory;
    return;
  }

  Image     *src_impl  = (Image *) src;
  Image     *dst_impl  = (Image *) dst;
  ImageScope src_scope = transfer_image_scope(src_impl->desc.usage);
  ImageScope dst_scope = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, src_impl->vk_image, src_impl->desc.usage,
                     src_scope, (VkImageAspectFlags) dst_impl->desc.aspects);
  if (src != dst)
  {
    sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image,
                       dst_impl->desc.usage, dst_scope,
                       (VkImageAspectFlags) dst_impl->desc.aspects);
  }

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

  if (src != dst)
  {
    sync_release_image(device->vk_table, vk_command_buffer, dst_impl->vk_image,
                       dst_impl->desc.usage, dst_scope,
                       (VkImageAspectFlags) dst_impl->desc.aspects);
  }

  allocator.deallocate(allocator.data, vk_copies);
}

void CommandEncoderImpl::copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                              stx::Span<gfx::BufferImageCopy const> copies)
{
  // check copies > 0
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32                num_copies = (u32) copies.size();
  VkBufferImageCopy *vk_copies  = ALLOC_NUM(allocator, VkBufferImageCopy, num_copies);

  if (vk_copies == nullptr)
  {
    status = gfx::Status::OutOfHostMemory;
    return;
  }

  Image     *dst_impl  = (Image *) dst;
  ImageScope dst_scope = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image, dst_impl->desc.usage,
                     dst_scope, (VkImageAspectFlags) dst_impl->desc.aspects);

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
  // check blits >0

  u32          num_blits = (u32) blits.size();
  VkImageBlit *vk_blits  = ALLOC_NUM(allocator, VkImageBlit, num_blits);

  if (vk_blits == nullptr)
  {
    status = gfx::Status::OutOfHostMemory;
    return;
  }

  Image     *src_impl  = (Image *) src;
  Image     *dst_impl  = (Image *) dst;
  ImageScope src_scope = transfer_image_scope(src_impl->desc.usage);
  ImageScope dst_scope = transfer_image_scope(dst_impl->desc.usage);

  sync_acquire_image(device->vk_table, vk_command_buffer, src_impl->vk_image, src_impl->desc.usage,
                     src_scope, (VkImageAspectFlags) dst_impl->desc.aspects);
  if (src != dst)
  {
    sync_acquire_image(device->vk_table, vk_command_buffer, dst_impl->vk_image,
                       dst_impl->desc.usage, dst_scope,
                       (VkImageAspectFlags) dst_impl->desc.aspects);
  }

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
  if (src != dst)
  {
    sync_release_image(device->vk_table, vk_command_buffer, dst_impl->vk_image,
                       dst_impl->desc.usage, dst_scope,
                       (VkImageAspectFlags) dst_impl->desc.aspects);
  }

  allocator.deallocate(allocator.data, vk_blits);
}

void CommandEncoderImpl::begin_descriptor_pass()
{
}

u32 CommandEncoderImpl::push_descriptors(gfx::DescriptorLayout layout, u32 count)
{
  //if(   ){}
  // check if in descriptor pass
  ASH_CHECK(count > 0, "Push descriptor count is 0");
  u32 const descriptor = vk_descriptor_layouts.count;

  RETURN_IF_ERR(descriptor_storage_bindings.ends.grow_count(allocator, count));
  RETURN_IF_ERR(vk_descriptor_layouts.grow_count(allocator, count));

  // if we use ordered-pushing we can spec writes here correctly
  // and only allocate as much as needed and not need to keep track of writes
  // e can store as many as counts as well

  descriptor_storage_bindings.ends.fill({.buffer = 0, .image = 0}, descriptor, count);

  DescriptorLayout *layout_impl = (DescriptorLayout *) layout;
  vk_descriptor_layouts.fill(layout_impl->vk_layout, descriptor, count);

  descriptors_size_target.samplers += count * layout_impl->count.samplers;
  descriptors_size_target.combined_image_samplers +=
      count * layout_impl->count.combined_image_samplers;
  descriptors_size_target.sampled_images += count * layout_impl->count.sampled_images;
  descriptors_size_target.storage_images += count * layout_impl->count.storage_images;
  descriptors_size_target.uniform_texel_buffers += count * layout_impl->count.uniform_texel_buffers;
  descriptors_size_target.storage_texel_buffers += count * layout_impl->count.storage_texel_buffers;
  descriptors_size_target.uniform_buffers += count * layout_impl->count.uniform_buffers;
  descriptors_size_target.storage_buffers += count * layout_impl->count.storage_buffers;
  descriptors_size_target.dynamic_uniform_buffers +=
      count * layout_impl->count.dynamic_uniform_buffers;
  descriptors_size_target.dynamic_storage_buffers +=
      count * layout_impl->count.dynamic_storage_buffers;
  descriptors_size_target.input_attachments += count * layout_impl->count.input_attachments;

  return descriptor;
}

VkImageLayout get_image_binding_layout(gfx::PipelineBindPoint bind_point, gfx::ImageUsage usage)
{
  switch (bind_point)
  {
    case gfx::PipelineBindPoint::Graphics:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case gfx::PipelineBindPoint::Compute:
      if (has_bits(usage, gfx::ImageUsage::ComputeShaderStorage))
      {
        return VK_IMAGE_LAYOUT_GENERAL;
      }
      else
      {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      }
      break;
    default:
      ASH_UNREACHABLE();
  }
}

// todo(lamarrr): end_descriptor_pass allocate sets and free memory for them
void CommandEncoderImpl::push_bindings(u32 descriptor, gfx::PipelineBindPoint bind_point,
                                       gfx::DescriptorBindings const &bindings)
{
  // check status
  ASH_CHECK(descriptor < descriptor_storage_bindings.ends.count, "invalid descriptor index");
  ASH_CHECK(!(bindings.samplers.is_empty() && bindings.combined_image_samplers.is_empty() &&
              bindings.sampled_images.is_empty() && bindings.storage_images.is_empty() &&
              bindings.uniform_texel_buffers.is_empty() &&
              bindings.storage_texel_buffers.is_empty() && bindings.uniform_buffers.is_empty() &&
              bindings.storage_buffers.is_empty() && bindings.input_attachments.is_empty()),
            "descriptor bindings must be non-empty");
  // check index
  // check status
  // build descriptor write here
  u32       storage_buffers_offset = descriptor_storage_bindings.buffers.count;
  u32       storage_images_offset  = descriptor_storage_bindings.images.count;
  u32 const num_storage_buffers =
      (u32) bindings.storage_buffers.size() + (u32) bindings.storage_texel_buffers.size();
  u32 const num_storage_images              = (u32) bindings.storage_images.size();
  u32       buffer_writes_offset            = vk_descriptor_buffers.count;
  u32       texel_buffer_view_writes_offset = vk_descriptor_texel_buffer_views.count;
  u32       image_writes_offset             = vk_descriptor_images.count;
  u32       writes_offset                   = vk_descriptor_writes.count;
  u32 const num_buffer_writes =
      (u32) bindings.uniform_buffers.size() + (u32) bindings.storage_buffers.size();
  u32 const num_texel_buffer_view_writes =
      (u32) bindings.uniform_texel_buffers.size() + (u32) bindings.storage_texel_buffers.size();
  u32 const num_image_writes =
      (u32) bindings.samplers.size() + (u32) bindings.combined_image_samplers.size() +
      (u32) bindings.sampled_images.size() + (u32) bindings.storage_images.size() +
      (u32) bindings.input_attachments.size();
  u32       writes_offset = vk_descriptor_writes.count;
  u32 const num_writes    = num_buffer_writes + num_texel_buffer_view_writes + num_image_writes;

  RETURN_IF_ERR(descriptor_storage_bindings.buffers.grow_count(allocator, num_storage_buffers));
  RETURN_IF_ERR(descriptor_storage_bindings.images.grow_count(allocator, num_storage_images));
  RETURN_IF_ERR(vk_descriptor_buffers.grow_count(allocator, num_buffer_writes));
  RETURN_IF_ERR(
      vk_descriptor_texel_buffer_views.grow_count(allocator, num_texel_buffer_view_writes));
  RETURN_IF_ERR(vk_descriptor_images.grow_count(allocator, num_image_writes));
  RETURN_IF_ERR(vk_descriptor_writes.grow_count(allocator, num_writes));

  for (u32 i = 0; i < (u32) bindings.storage_buffers.size(); i++, storage_buffers_offset++)
  {
    descriptor_storage_bindings.buffers.data[storage_buffers_offset] =
        ((Buffer *) bindings.storage_buffers.data()[i].buffer);
  }

  for (u32 i = 0; i < (u32) bindings.storage_texel_buffers.size(); i++, storage_buffers_offset++)
  {
    descriptor_storage_bindings.buffers.data[storage_buffers_offset] =
        (Buffer *) (((BufferView *) bindings.storage_texel_buffers.data()[i].buffer_view)
                        ->desc.buffer);
  }

  for (u32 i = 0; i < (u32) bindings.storage_images.size(); i++, storage_images_offset++)
  {
    descriptor_storage_bindings.images.data[storage_images_offset] =
        (Image *) (((ImageView *) bindings.storage_images.data()[i].image_view)->desc.image);
  }

  descriptor_storage_bindings.ends.data[descriptor] = {
      .buffer = descriptor_storage_bindings.buffers.count,
      .image  = descriptor_storage_bindings.images.count};

  for (gfx::UniformBufferBinding const &binding : bindings.uniform_buffers)
  {
    vk_descriptor_buffers.data[buffer_writes_offset] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) binding.buffer)->vk_buffer,
                               .offset = binding.offset,
                               .range  = binding.size};
    buffer_writes_offset++;
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    vk_descriptor_buffers.data[buffer_writes_offset] =
        VkDescriptorBufferInfo{.buffer = ((Buffer *) binding.buffer)->vk_buffer,
                               .offset = binding.offset,
                               .range  = binding.size};
    buffer_writes_offset++;
  }

  for (gfx::UniformTexelBufferBinding const &binding : bindings.uniform_texel_buffers)
  {
    vk_descriptor_texel_buffer_views.data[texel_buffer_view_writes_offset] =
        ((BufferView *) binding.buffer_view)->vk_view;
    texel_buffer_view_writes_offset++;
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    vk_descriptor_texel_buffer_views.data[texel_buffer_view_writes_offset] =
        ((BufferView *) binding.buffer_view)->vk_view;
    texel_buffer_view_writes_offset++;
  }

  // todo(lamarrr): need stage to determine image layout
  // add to vk descriptor write?

  for (gfx::SamplerBinding const &binding : bindings.samplers)
  {
    vk_descriptor_images.data[image_writes_offset] =
        VkDescriptorImageInfo{.sampler     = ((Sampler *) binding.sampler)->vk_sampler,
                              .imageView   = nullptr,
                              .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED};
    image_writes_offset++;
  }

  for (gfx::CombinedImageSamplerBinding const &binding : bindings.combined_image_samplers)
  {
    ImageView *view_impl                           = (ImageView *) binding.image_view;
    Image     *image                               = (Image *) view_impl->desc.image;
    vk_descriptor_images.data[image_writes_offset] = VkDescriptorImageInfo{
        .sampler     = ((Sampler *) binding.sampler)->vk_sampler,
        .imageView   = view_impl->vk_view,
        .imageLayout = get_image_binding_layout(bind_point, image->desc.usage)};
    image_writes_offset++;
  }

  for (gfx::SampledImageBinding const &binding : bindings.sampled_images)
  {
    ImageView *view_impl                           = (ImageView *) binding.image_view;
    Image     *image                               = (Image *) view_impl->desc.image;
    vk_descriptor_images.data[image_writes_offset] = VkDescriptorImageInfo{
        .sampler     = nullptr,
        .imageView   = view_impl->vk_view,
        .imageLayout = get_image_binding_layout(bind_point, image->desc.usage)};
    image_writes_offset++;
  }

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    ImageView *view_impl = (ImageView *) binding.image_view;
    Image     *image     = (Image *) view_impl->desc.image;
    // always in general layout
    vk_descriptor_images.data[image_writes_offset] = VkDescriptorImageInfo{
        .sampler     = nullptr,
        .imageView   = view_impl->vk_view,
        .imageLayout = get_image_binding_layout(bind_point, image->desc.usage)};
    image_writes_offset++;
  }

  for (gfx::InputAttachmentBinding const &binding : bindings.input_attachments)
  {
    ImageView *view_impl                           = (ImageView *) binding.image_view;
    Image     *image                               = (Image *) view_impl->desc.image;
    vk_descriptor_images.data[image_writes_offset] = VkDescriptorImageInfo{
        .sampler     = nullptr,
        .imageView   = view_impl->vk_view,
        .imageLayout = get_image_binding_layout(bind_point, image->desc.usage)};
    image_writes_offset++;
  }

  // write vk_descriptor_writes
  // write to array

  u32 samplers_offset = 0;
  for (u32 i = 0; i < num_writes; i++)
  {
    vk_descriptor_writes.data[i] = VkWriteDescriptorSet{
        .sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext  = nullptr,
        .dstSet = vk_descriptors.data[i],
    };
  }
}

void CommandEncoderImpl::end_descriptor_pass()
{
  // handle error
  // todo(lamarrr): what does the spec say about zero-sized?
  // what if the new capacity is still 0
  // todo lamarrr: handle descriptor capacity 0
  bool                 need_pool_resize = false;
  gfx::DescriptorCount new_capacity;

#define CHECK_DESCPOOL_RESIZE_NEEDED(type)                                                        \
  if (descriptors_size_target.##type > descriptors_capacity.##type)                               \
  {                                                                                               \
    new_capacity.##type = descriptors_size_target.##type + (descriptors_size_target.##type << 1); \
    need_pool_resize    = true;                                                                   \
  }

  CHECK_DESCPOOL_RESIZE_NEEDED(samplers);
  CHECK_DESCPOOL_RESIZE_NEEDED(combined_image_samplers);
  CHECK_DESCPOOL_RESIZE_NEEDED(sampled_images);
  CHECK_DESCPOOL_RESIZE_NEEDED(storage_images);
  CHECK_DESCPOOL_RESIZE_NEEDED(uniform_texel_buffers);
  CHECK_DESCPOOL_RESIZE_NEEDED(storage_texel_buffers);
  CHECK_DESCPOOL_RESIZE_NEEDED(uniform_buffers);
  CHECK_DESCPOOL_RESIZE_NEEDED(storage_buffers);
  CHECK_DESCPOOL_RESIZE_NEEDED(dynamic_uniform_buffers);
  CHECK_DESCPOOL_RESIZE_NEEDED(dynamic_storage_buffers);
  CHECK_DESCPOOL_RESIZE_NEEDED(input_attachments);

  if (need_pool_resize)
  {
    if (vk_descriptor_pool != nullptr)
    {
      device->vk_table.DestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
      vk_descriptor_pool = nullptr;
    }

    VkDescriptorPoolSize vk_pool_sizes[gfx::NUM_DESCRIPTOR_TYPES] = {
        {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = new_capacity.samplers},
        {.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = new_capacity.combined_image_samplers},
        {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = new_capacity.sampled_images},
        {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = new_capacity.storage_images},
        {.type            = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
         .descriptorCount = new_capacity.uniform_texel_buffers},
        {.type            = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
         .descriptorCount = new_capacity.storage_texel_buffers},
        {.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = new_capacity.uniform_buffers},
        {.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .descriptorCount = new_capacity.storage_buffers},
        {.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
         .descriptorCount = new_capacity.dynamic_uniform_buffers},
        {.type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
         .descriptorCount = new_capacity.dynamic_storage_buffers},
        {.type            = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
         .descriptorCount = new_capacity.input_attachments}};

    u32 max_descriptors = new_capacity.samplers + new_capacity.combined_image_samplers +
                          new_capacity.sampled_images + new_capacity.storage_images +
                          new_capacity.uniform_texel_buffers + new_capacity.storage_texel_buffers +
                          new_capacity.uniform_buffers + new_capacity.storage_buffers +
                          new_capacity.dynamic_uniform_buffers +
                          new_capacity.dynamic_storage_buffers + new_capacity.input_attachments;

    VkDescriptorPoolCreateInfo create_info{.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                           .pNext   = nullptr,
                                           .flags   = 0,
                                           .maxSets = max_descriptors,
                                           .poolSizeCount = gfx::NUM_DESCRIPTOR_TYPES,
                                           .pPoolSizes    = vk_pool_sizes};

    device->vk_table.CreateDescriptorPool(device->vk_device, &create_info, nullptr,
                                          &vk_descriptor_pool);
    // check
    descriptors_capacity = new_capacity;
  }

  device->vk_table.ResetDescriptorPool(device->vk_device, vk_descriptor_pool, 0);
  // check success

  VkDescriptorSetAllocateInfo allocate_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                            .pNext = nullptr,
                                            .descriptorPool     = vk_descriptor_pool,
                                            .descriptorSetCount = vk_descriptor_layouts.count,
                                            .pSetLayouts        = vk_descriptor_layouts.data};

  // resize vk_descriptors
  // device oom
  device->vk_table.AllocateDescriptorSets(device->vk_device, &allocate_info, vk_descriptors.data);

  // this  pass must be heavily guarded and occur first
  // check that this is called only once
  // resolve to vk descriptor sets
  for (VkWriteDescriptorSet &set : vk_descriptor_writes)
  {
    set.dstSet = vk_descriptors.data[(u32) set.dstSet];        // OH MY!
  }

  // count is incorrect
  device->vk_table.UpdateDescriptorSets(device->vk_device, vk_descriptor_writes.count,
                                        vk_descriptor_writes.data, 0, nullptr);
  // free non-storage binding descriptors
}

void CommandEncoderImpl::bind_descriptor(u32 index)
{
  // checks
  // bind to render/compute
  // check index
}

void CommandEncoderImpl::bind_next_descriptor()
{
  // advance
  // bind to render/compute
  // checks
  // check index
}

void CommandEncoderImpl::begin_render_pass(
    gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
    stx::Span<gfx::Color const>        color_attachments_clear_values,
    stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values)
{
  this->framebuffer = (Framebuffer *) framebuffer;

  for (gfx::ImageView view : ((Framebuffer *) framebuffer)->desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }

    Image     *impl  = (Image *) (((ImageView *) view)->desc.image);
    ImageScope scope = color_attachment_image_scope(impl->desc.usage);
    sync_acquire_image(device->vk_table, vk_command_buffer, impl->vk_image, impl->desc.usage, scope,
                       (VkImageAspectFlags) impl->desc.aspects);
  }

  {
    gfx::ImageView view = ((Framebuffer *) framebuffer)->desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      Image     *impl  = (Image *) (((ImageView *) view)->desc.image);
      ImageScope scope = depth_stencil_attachment_image_scope(impl->desc.usage);
      sync_acquire_image(device->vk_table, vk_command_buffer, impl->vk_image, impl->desc.usage,
                         scope, (VkImageAspectFlags) impl->desc.aspects);
    }
  }

  VkRenderPassBeginInfo begin_info{.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                   .pNext           = nullptr,
                                   .renderPass      = ((RenderPass *) render_pass)->vk_render_pass,
                                   .framebuffer     = ((Framebuffer *) framebuffer)->vk_framebuffer,
                                   .renderArea      = VkRect2D{},
                                   .clearValueCount = 1,
                                   .pClearValues    = nullptr};

  device->vk_table.CmdBeginRenderPass(vk_command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandEncoderImpl::end_render_pass()
{
  // check in renderpass
  for (gfx::ImageView view : ((Framebuffer *) framebuffer)->desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }

    Image     *impl  = (Image *) (((ImageView *) view)->desc.image);
    ImageScope scope = color_attachment_image_scope(impl->desc.usage);
    sync_release_image(device->vk_table, vk_command_buffer, impl->vk_image, impl->desc.usage, scope,
                       (VkImageAspectFlags) impl->desc.aspects);
  }

  {
    gfx::ImageView view = ((Framebuffer *) framebuffer)->desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      Image     *impl  = (Image *) (((ImageView *) view)->desc.image);
      ImageScope scope = depth_stencil_attachment_image_scope(impl->desc.usage);
      sync_release_image(device->vk_table, vk_command_buffer, impl->vk_image, impl->desc.usage,
                         scope, (VkImageAspectFlags) impl->desc.aspects);
    }
  }

  device->vk_table.CmdEndRenderPass(vk_command_buffer);
}

void CommandEncoderImpl::bind_pipeline(gfx::ComputePipeline pipeline)
{
  // check status
  this->compute_pipeline = (ComputePipeline *) pipeline;
  device->vk_table.CmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                   this->compute_pipeline->vk_pipeline);
}

void CommandEncoderImpl::bind_pipeline(gfx::GraphicsPipeline pipeline)
{
  this->graphics_pipeline = (GraphicsPipeline *) pipeline;
  device->vk_table.CmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   this->graphics_pipeline->vk_pipeline);
}

void CommandEncoderImpl::push_constants(stx::Span<u8 const> push_constants_data)
{
  // check status
  if (compute_pipeline)
  {
    device->vk_table.CmdPushConstants(
        vk_command_buffer, compute_pipeline->vk_layout, VK_SHADER_STAGE_ALL, 0,
        (u32) push_constants_data.size_bytes(), push_constants_data.data());
  }
  else if (graphics_pipeline)
  {
    device->vk_table.CmdPushConstants(
        vk_command_buffer, graphics_pipeline->vk_layout, VK_SHADER_STAGE_ALL, 0,
        (u32) push_constants_data.size_bytes(), push_constants_data.data());
  }
  else
  {
    ASH_PANIC("Not in renderpass or compute pass");
  }
}

void CommandEncoderImpl::dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z)
{
}

void CommandEncoderImpl::dispatch_indirect(gfx::Buffer buffer, u64 offset)
{
  // pre
  device->vk_table;
  // post
}

void CommandEncoderImpl::set_viewport(gfx::Viewport const &viewport)
{
  VkViewport vk_viewport{.x        = viewport.area.offset.x,
                         .y        = viewport.area.offset.y,
                         .width    = viewport.area.extent.x,
                         .height   = viewport.area.extent.y,
                         .minDepth = viewport.min_depth,
                         .maxDepth = viewport.max_depth};
  device->vk_table.CmdSetViewport(vk_command_buffer, 0, 1, &vk_viewport);
}

void CommandEncoderImpl::set_scissor(IRect scissor)
{
  VkRect2D vk_scissor{.offset = VkOffset2D{scissor.offset.x, scissor.offset.y},
                      .extent = VkExtent2D{scissor.extent.width, scissor.extent.height}};
  device->vk_table.CmdSetScissor(vk_command_buffer, 0, 1, &vk_scissor);
}

void CommandEncoderImpl::set_blend_constants(Vec4 blend_constants)
{
  f32 vk_constants[4] = {blend_constants.x, blend_constants.y, blend_constants.z,
                         blend_constants.w};
  device->vk_table.CmdSetBlendConstants(vk_command_buffer, vk_constants);
}

void CommandEncoderImpl::set_stencil_compare_mask(gfx::StencilFaces faces, u32 mask)
{
  device->vk_table.CmdSetStencilCompareMask(vk_command_buffer, (VkStencilFaceFlags) faces, mask);
}

void CommandEncoderImpl::set_stencil_reference(gfx::StencilFaces faces, u32 reference)
{
  device->vk_table.CmdSetStencilReference(vk_command_buffer, (VkStencilFaceFlags) faces, reference);
}

void CommandEncoderImpl::set_stencil_write_mask(gfx::StencilFaces faces, u32 mask)
{
  device->vk_table.CmdSetStencilWriteMask(vk_command_buffer, (VkStencilFaceFlags) faces, mask);
}

void CommandEncoderImpl::set_vertex_buffers(stx::Span<gfx::Buffer const> vertex_buffers,
                                            stx::Span<u64 const>         offsets)
{
  u32       num_buffers = (u32) vertex_buffers.size();
  VkBuffer *vk_buffers  = ALLOC_NUM(allocator, VkBuffer, num_buffers);

  if (vk_buffers == nullptr)
  {
    status = gfx::Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_buffers; i++)
  {
    vk_buffers[i] = ((Buffer *) vertex_buffers.data()[i])->vk_buffer;
  }

  device->vk_table.CmdBindVertexBuffers(vk_command_buffer, 0, num_buffers, vk_buffers,
                                        offsets.data());

  allocator.deallocate(allocator.data, vk_buffers);
}

void CommandEncoderImpl::set_index_buffer(gfx::Buffer index_buffer, u64 offset)
{
  device->vk_table.CmdBindIndexBuffer(vk_command_buffer, ((Buffer *) index_buffer)->vk_buffer,
                                      offset, VK_INDEX_TYPE_UINT32);
}

void CommandEncoderImpl::draw(u32 first_index, u32 num_indices, i32 vertex_offset,
                              u32 first_instance, u32 num_instances)
{
  device->vk_table.CmdDrawIndexed(vk_command_buffer, num_indices, num_instances, first_index,
                                  vertex_offset, first_instance);
}

void CommandEncoderImpl::draw_indirect(gfx::Buffer buffer, u64 offset, u32 draw_count, u32 stride)
{
  device->vk_table.CmdDrawIndexedIndirect(vk_command_buffer, ((Buffer *) buffer)->vk_buffer, offset,
                                          draw_count, stride);
}

void CommandEncoderImpl::on_execution_complete(stx::UniqueFn<void()> &&fn)
{
  completion_tasks.push(std::move(fn)).unwrap();
}

}        // namespace vk
}        // namespace ash
