#include "ashura/vulkan_gfx.h"
#include "stx/span.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

// todo(lamarrr): define macros for checks, use debug name

#define ALLOC_N(allocator, type, num) \
  (type *) (allocator).allocate((usize) sizeof(type) * (num), alignof(type))

#define ALLOC_OBJECT(allocator, type) (type *) (allocator).allocate(sizeof(type), alignof(type))

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

bool load_device_table(VkDevice device, DeviceTable &vk_table, VmaVulkanFunctions &vma_table)
{
  bool all_loaded = true;

#define LOAD_VK(function)                                                             \
  vk_table.function = (PFN_vk##function) vkGetDeviceProcAddr(device, "vk" #function); \
  all_loaded        = all_loaded && (vk_table.function != nullptr)

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
  LOAD_VK(CreateDevice);
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
#undef LOAD_VK

#define LOAD_VKEXT(function)                                                          \
  vk_table.function = (PFN_vk##function) vkGetDeviceProcAddr(device, "vk" #function); \
  vk_table.function = (vk_table.function != nullptr) ? vk_table.function : function##_Stub;

  LOAD_VKEXT(DebugMarkerSetObjectTagEXT);
  LOAD_VKEXT(DebugMarkerSetObjectNameEXT);

  LOAD_VKEXT(CmdDebugMarkerBeginEXT);
  LOAD_VKEXT(CmdDebugMarkerEndEXT);
  LOAD_VKEXT(CmdDebugMarkerInsertEXT);

#undef LOAD_VKEXT

#define SET_VMA(function) vma_table.vk##function = vk_table.function
  // TODO(lamarrr): phy dev functions
  SET_VMA(AllocateMemory);
  SET_VMA(FreeMemory);
  SET_VMA(UnmapMemory);
  SET_VMA(FlushMappedMemoryRanges);
  SET_VMA(InvalidateMappedMemoryRanges);
  SET_VMA(BindBufferMemory);
  SET_VMA(BindImageMemory);
  SET_VMA(GetBufferMemoryRequirements);
  SET_VMA(GetImageMemoryRequirements);
  SET_VMA(CreateBuffer);
  SET_VMA(DestroyBuffer);
  SET_VMA(CreateImage);
  SET_VMA(DestroyImage);
  SET_VMA(CmdCopyBuffer);

#undef SET_VMA

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
// we'll kind of be waiting on a barrier operation which doesn't make sense cos the barrier might
// have already taken care of us even when they both only perform reads
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
        // if stage and access intersects previous barrier, no need to add new one as we'll observe
        // the effect
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

Result<gfx::FormatProperties, gfx::Status>
    DeviceInterface::get_format_properties(gfx::Device self, gfx::Format format)
{
  Device            *impl = (Device *) self;
  VkFormatProperties props;
  // TODOD(Lamarrr): fix
  vkGetPhysicalDeviceFormatProperties(impl->vk_phy_device, (VkFormat) format, &props);
  return stx::Ok(gfx::FormatProperties{
      .linear_tiling_features  = (gfx::FormatFeatures) props.linearTilingFeatures,
      .optimal_tiling_features = (gfx::FormatFeatures) props.optimalTilingFeatures,
      .buffer_features         = (gfx::FormatFeatures) props.bufferFeatures});
}

Result<gfx::Buffer, gfx::Status> DeviceInterface::create_buffer(gfx::Device            self,
                                                                gfx::BufferDesc const &desc)
{
  // check size
  Device                 *impl = (Device *) self;
  VkBufferCreateInfo      create_info{.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                      .pNext                 = nullptr,
                                      .flags                 = 0,
                                      .size                  = desc.size,
                                      .usage                 = (VkBufferUsageFlags) desc.usage,
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
  VkResult result = vmaCreateBuffer(impl->vma_allocator, &create_info, &alloc_create_info,
                                    &vk_buffer, &vma_allocation, &vma_allocation_info);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  void *host_map = nullptr;
  if (has_any_bit(desc.properties, gfx::MemoryProperties::HostVisible |
                                       gfx::MemoryProperties::HostCoherent |
                                       gfx::MemoryProperties::HostCached))
  {
    result = impl->vk_table.MapMemory(impl->vk_device, vma_allocation_info.deviceMemory, 0,
                                      VK_WHOLE_SIZE, 0, &host_map);
    // TODO(lamarrr): check result
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
        .object      = (u64) vk_buffer,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  Buffer *buffer = ALLOC_OBJECT(impl->allocator, Buffer);
  if (buffer == nullptr)
  {
    vmaDestroyBuffer(impl->vma_allocator, vk_buffer, vma_allocation);
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

Result<gfx::BufferView, gfx::Status>
    DeviceInterface::create_buffer_view(gfx::Device self, gfx::BufferViewDesc const &desc)
{
  // check range
  Device                *impl = (Device *) self;
  VkBufferViewCreateInfo create_info{.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                                     .pNext  = nullptr,
                                     .flags  = 0,
                                     .buffer = ((Buffer *) desc.buffer)->vk_buffer,
                                     .format = (VkFormat) desc.format,
                                     .offset = desc.offset,
                                     .range  = desc.size};

  VkBufferView vk_view;

  VkResult result =
      impl->vk_table.CreateBufferView(impl->vk_device, &create_info, nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
        .object      = (u64) vk_view,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  BufferView *view = ALLOC_OBJECT(impl->allocator, BufferView);

  if (view == nullptr)
  {
    impl->vk_table.DestroyBufferView(impl->vk_device, vk_view, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (view) BufferView{.refcount = 1, .desc = desc, .vk_view = vk_view};

  return stx::Ok((gfx::BufferView) view);
}

Result<gfx::Image, gfx::Status> DeviceInterface::create_image(gfx::Device           self,
                                                              gfx::ImageDesc const &desc)
{
  // check size
  // check aspects
  // check format support against usage
  Device           *impl = (Device *) self;
  VkImageCreateInfo create_info{.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                .pNext                 = nullptr,
                                .flags                 = 0,
                                .imageType             = (VkImageType) desc.type,
                                .format                = (VkFormat) desc.format,
                                .extent                = VkExtent3D{.width  = desc.extent.width,
                                                                    .height = desc.extent.height,
                                                                    .depth  = desc.extent.depth},
                                .mipLevels             = desc.mip_levels,
                                .arrayLayers           = desc.array_layers,
                                .samples               = VK_SAMPLE_COUNT_1_BIT,
                                .tiling                = VK_IMAGE_TILING_OPTIMAL,
                                .usage                 = (VkImageUsageFlags) desc.usage,
                                .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
                                .queueFamilyIndexCount = 0,
                                .pQueueFamilyIndices   = nullptr,
                                .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};

  VkImage                 vk_image;
  VmaAllocation           vma_allocation;
  VmaAllocationInfo       vma_allocation_info;
  VmaAllocationCreateInfo vma_allocation_create_info = {.usage = VMA_MEMORY_USAGE_GPU_ONLY};
  VkResult result = vmaCreateImage(impl->vma_allocator, &create_info, &vma_allocation_create_info,
                                   &vk_image, &vma_allocation, &vma_allocation_info);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
        .object      = (u64) vk_image,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  Image *image = ALLOC_OBJECT(impl->allocator, Image);

  if (image == nullptr)
  {
    vmaDestroyImage(impl->vma_allocator, vk_image, vma_allocation);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (image) Image{.refcount            = 1,
                    .desc                = desc,
                    .is_weak             = false,
                    .vk_image            = vk_image,
                    .vma_allocation      = vma_allocation,
                    .vma_allocation_info = vma_allocation_info};

  return stx::Ok((gfx::Image) image);
}

Result<gfx::ImageView, gfx::Status>
    DeviceInterface::create_image_view(gfx::Device self, gfx::ImageViewDesc const &desc)
{
  // check aspect match
  // check subresource range
  Device               *impl = (Device *) self;
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
  VkResult    result =
      impl->vk_table.CreateImageView(impl->vk_device, &create_info, nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
        .object      = (u64) vk_view,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  ImageView *view = ALLOC_OBJECT(impl->allocator, ImageView);
  if (view == nullptr)
  {
    impl->vk_table.DestroyImageView(impl->vk_device, vk_view, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (view) ImageView{.refcount = 1, .desc = desc, .vk_view = vk_view};

  return stx::Ok((gfx::ImageView) view);
}

Result<gfx::Sampler, gfx::Status> DeviceInterface::create_sampler(gfx::Device             self,
                                                                  gfx::SamplerDesc const &desc)
{
  Device             *impl = (Device *) self;
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
  VkResult  result =
      impl->vk_table.CreateSampler(impl->vk_device, &create_info, nullptr, &vk_sampler);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,
        .object      = (u64) vk_sampler,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  Sampler *sampler = ALLOC_OBJECT(impl->allocator, Sampler);
  if (sampler == nullptr)
  {
    impl->vk_table.DestroySampler(impl->vk_device, vk_sampler, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (sampler) Sampler{.refcount = 1, .vk_sampler = vk_sampler};

  return stx::Ok((gfx::Sampler) sampler);
}

Result<gfx::Shader, gfx::Status> DeviceInterface::create_shader(gfx::Device            self,
                                                                gfx::ShaderDesc const &desc)
{
  // check size
  Device                  *impl = (Device *) self;
  VkShaderModuleCreateInfo create_info{.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                       .pNext    = nullptr,
                                       .flags    = 0,
                                       .codeSize = desc.spirv_code.size_bytes(),
                                       .pCode    = desc.spirv_code.data};

  VkShaderModule vk_shader;
  VkResult       result =
      impl->vk_table.CreateShaderModule(impl->vk_device, &create_info, nullptr, &vk_shader);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
        .object      = (u64) vk_shader,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  Shader *shader = ALLOC_OBJECT(impl->allocator, Shader);
  if (shader == nullptr)
  {
    impl->vk_table.DestroyShaderModule(impl->vk_device, vk_shader, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (shader) Shader{.refcount = 1, .vk_shader = vk_shader};

  return stx::Ok((gfx::Shader) shader);
}

Result<gfx::RenderPass, gfx::Status>
    DeviceInterface::create_render_pass(gfx::Device self, gfx::RenderPassDesc const &desc)
{
  Device                 *impl = (Device *) self;
  VkAttachmentDescription vk_attachments[gfx::MAX_COLOR_ATTACHMENTS * 2 + 1];
  VkAttachmentReference   vk_color_attachments[gfx::MAX_COLOR_ATTACHMENTS];
  VkAttachmentReference   vk_input_attachments[gfx::MAX_COLOR_ATTACHMENTS];
  VkAttachmentReference   vk_depth_stencil_attachment;
  u32                     num_attachments       = 0;
  u32                     num_color_attachments = (u32) desc.color_attachments.size;
  u32                     num_input_attachments = (u32) desc.input_attachments.size;
  u32                     num_depth_stencil_attachments =
      desc.depth_stencil_attachment.format == gfx::Format::Undefined ? 0U : 1U;
  for (u32 icolor_attachment = 0; icolor_attachment < (u32) desc.color_attachments.size;
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

  for (u32 iinput_attachment = 0; iinput_attachment < (u32) desc.input_attachments.size;
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
  VkResult               result =
      impl->vk_table.CreateRenderPass(impl->vk_device, &create_info, nullptr, &vk_render_pass);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
        .object      = (u64) vk_render_pass,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  RenderPass *render_pass = ALLOC_OBJECT(impl->allocator, RenderPass);
  if (render_pass == nullptr)
  {
    impl->vk_table.DestroyRenderPass(impl->vk_device, vk_render_pass, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (render_pass) RenderPass{.refcount = 1, .desc = desc, .vk_render_pass = vk_render_pass};

  return stx::Ok((gfx::RenderPass) render_pass);
}

Result<gfx::Framebuffer, gfx::Status>
    DeviceInterface::create_framebuffer(gfx::Device self, gfx::FramebufferDesc const &desc)
{
  Device     *impl                          = (Device *) self;
  u32         num_color_attachments         = (u32) desc.color_attachments.size;
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

  VkResult result =
      impl->vk_table.CreateFramebuffer(impl->vk_device, &create_info, nullptr, &vk_framebuffer);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,
        .object      = (u64) vk_framebuffer,
        .pObjectName = desc.label};
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  Framebuffer *framebuffer = ALLOC_OBJECT(impl->allocator, Framebuffer);
  if (framebuffer == nullptr)
  {
    impl->vk_table.DestroyFramebuffer(impl->vk_device, vk_framebuffer, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (framebuffer) Framebuffer{.refcount = 1, .desc = desc, .vk_framebuffer = vk_framebuffer};

  return stx::Ok((gfx::Framebuffer) framebuffer);
}

Result<gfx::DescriptorSetLayout, gfx::Status>
    DeviceInterface::create_descriptor_set_layout(gfx::Device                         self,
                                                  gfx::DescriptorSetLayoutDesc const &desc)
{
  Device                       *impl         = (Device *) self;
  u32                           num_bindings = (u32) desc.bindings.size;
  VkDescriptorSetLayoutBinding *vk_bindings =
      ALLOC_N(impl->allocator, VkDescriptorSetLayoutBinding, num_bindings);

  if (vk_bindings == nullptr)
  {
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  gfx::DescriptorCount count;

  for (u32 i = 0; i < num_bindings; i++)
  {
    gfx::DescriptorBindingDesc const &binding = desc.bindings.data[i];
    vk_bindings[i]                            = VkDescriptorSetLayoutBinding{.binding        = i,
                                                                             .descriptorType = (VkDescriptorType) binding.type,
                                                                             .descriptorCount    = binding.count,
                                                                             .stageFlags         = VK_SHADER_STAGE_ALL,
                                                                             .pImmutableSamplers = nullptr};
    // TODO(lamarrr): if ray tracing is not enabled
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
      case gfx::DescriptorType::AccelerationStructure:
        count.acceleration_structures += binding.count;
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
      impl->vk_table.CreateDescriptorSetLayout(impl->vk_device, &create_info, nullptr, &vk_layout);

  impl->allocator.deallocate(vk_bindings);

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
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  DescriptorSetLayout *layout = ALLOC_OBJECT(impl->allocator, DescriptorSetLayout);
  if (layout == nullptr)
  {
    impl->vk_table.DestroyDescriptorSetLayout(impl->vk_device, vk_layout, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (layout) DescriptorSetLayout{.refcount = 1, .count = count, .vk_layout = vk_layout};

  return stx::Ok((gfx::DescriptorSetLayout) layout);
}

Result<gfx::PipelineCache, gfx::Status>
    DeviceInterface::create_pipeline_cache(gfx::Device self, gfx::PipelineCacheDesc const &desc)
{
  Device                   *impl = (Device *) self;
  VkPipelineCacheCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
                                        .pNext = nullptr,
                                        .flags = 0,
                                        .initialDataSize = desc.initial_data.size_bytes(),
                                        .pInitialData    = desc.initial_data.data};

  VkPipelineCache vk_cache;
  VkResult        result =
      impl->vk_table.CreatePipelineCache(impl->vk_device, &create_info, nullptr, &vk_cache);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  PipelineCache *cache = ALLOC_OBJECT(impl->allocator, PipelineCache);
  if (cache == nullptr)
  {
    impl->vk_table.DestroyPipelineCache(impl->vk_device, vk_cache, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (cache) PipelineCache{.refcount = 1, .vk_cache = vk_cache};

  return stx::Ok((gfx::PipelineCache) cache);
}

Result<gfx::ComputePipeline, gfx::Status>
    DeviceInterface::create_compute_pipeline(gfx::Device self, gfx::ComputePipelineDesc const &desc)
{
  Device              *impl = (Device *) self;
  VkSpecializationInfo vk_specialization{
      .mapEntryCount = (u32) desc.compute_shader.specialization_constants.size,
      .pMapEntries =
          (VkSpecializationMapEntry const *) desc.compute_shader.specialization_constants.data,
      .dataSize = desc.compute_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.compute_shader.specialization_constants_data.data};

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
  VkResult result = impl->vk_table.CreatePipelineLayout(impl->vk_device, &layout_create_info,
                                                        nullptr, &vk_layout);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  VkComputePipelineCreateInfo create_info{.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                          .pNext  = nullptr,
                                          .flags  = 0,
                                          .stage  = vk_stage,
                                          .layout = vk_layout,
                                          .basePipelineHandle = nullptr,
                                          .basePipelineIndex  = 0};

  VkPipeline vk_pipeline;
  result = impl->vk_table.CreateComputePipelines(
      impl->vk_device, desc.cache == nullptr ? nullptr : ((PipelineCache *) desc.cache)->vk_cache,
      1, &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    impl->vk_table.DestroyPipelineLayout(impl->vk_device, vk_layout, nullptr);
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
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  ComputePipeline *pipeline = ALLOC_OBJECT(impl->allocator, ComputePipeline);
  if (pipeline == nullptr)
  {
    impl->vk_table.DestroyPipelineLayout(impl->vk_device, vk_layout, nullptr);
    impl->vk_table.DestroyPipeline(impl->vk_device, vk_pipeline, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (pipeline) ComputePipeline{.refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

  return stx::Ok((gfx::ComputePipeline) pipeline);
}

Result<gfx::GraphicsPipeline, gfx::Status>
    DeviceInterface::create_graphics_pipeline(gfx::Device                      self,
                                              gfx::GraphicsPipelineDesc const &desc)
{
  Device              *impl = (Device *) self;
  VkSpecializationInfo vk_vs_specialization{
      .mapEntryCount = (u32) desc.vertex_shader.specialization_constants.size,
      .pMapEntries =
          (VkSpecializationMapEntry const *) desc.vertex_shader.specialization_constants.data,
      .dataSize = desc.vertex_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.vertex_shader.specialization_constants_data.data};

  VkSpecializationInfo vk_fs_specialization{
      .mapEntryCount = (u32) desc.fragment_shader.specialization_constants.size,
      .pMapEntries =
          (VkSpecializationMapEntry const *) desc.fragment_shader.specialization_constants.data,
      .dataSize = desc.fragment_shader.specialization_constants_data.size_bytes(),
      .pData    = desc.fragment_shader.specialization_constants_data.data};

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
  VkResult result = impl->vk_table.CreatePipelineLayout(impl->vk_device, &layout_create_info,
                                                        nullptr, &vk_layout);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }

  VkVertexInputBindingDescription input_bindings[gfx::MAX_VERTEX_ATTRIBUTES];
  u32                             num_input_bindings = 0;
  for (; num_input_bindings < desc.vertex_input_bindings.size; num_input_bindings++)
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
  for (; num_attributes < desc.vertex_attributes.size; num_attributes++)
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

  for (; num_color_attachments < (u32) desc.color_blend_state.attachments.size;
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
  result = impl->vk_table.CreateGraphicsPipelines(
      impl->vk_device, desc.cache == nullptr ? nullptr : ((PipelineCache *) desc.cache)->vk_cache,
      1, &create_info, nullptr, &vk_pipeline);

  if (result != VK_SUCCESS)
  {
    impl->vk_table.DestroyPipelineLayout(impl->vk_device, vk_layout, nullptr);
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
    impl->vk_table.DebugMarkerSetObjectNameEXT(impl->vk_device, &debug_info);
  }

  GraphicsPipeline *pipeline = ALLOC_OBJECT(impl->allocator, GraphicsPipeline);
  if (pipeline == nullptr)
  {
    impl->vk_table.DestroyPipelineLayout(impl->vk_device, vk_layout, nullptr);
    impl->vk_table.DestroyPipeline(impl->vk_device, vk_pipeline, nullptr);
    return stx::Err(gfx::Status::OutOfHostMemory);
  }

  new (pipeline)
      GraphicsPipeline{.refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

  return stx::Ok((gfx::GraphicsPipeline) pipeline);
}

Result<gfx::CommandEncoderImpl, gfx::Status>
    DeviceInterface::create_command_encoder(gfx::Device self)
{
  Device *impl = (Device *) self;
}

Result<void *, gfx::Status> DeviceInterface::get_buffer_memory_map(gfx::Device self,
                                                                   gfx::Buffer buffer)
{
  (void) self;
  Buffer *buffer_impl = (Buffer *) buffer;
  // check has_any_bit(buffer_impl->desc.properties, gfx::MemoryProperties::HostCached|
  // gfx::MemoryProperties::HostCoherent| gfx::MemoryProperties::HostVisible);
  return stx::Ok((void *) buffer_impl->host_map);
}

gfx::Status DeviceInterface::invalidate_buffer_memory_map(gfx::Device self, gfx::Buffer buffer,
                                                          gfx::MemoryRange range)
{
  // TODO(lamarrr): check is host accessible
  Device             *impl        = (Device *) self;
  Buffer             *buffer_impl = (Buffer *) buffer;
  VkMappedMemoryRange vk_range{.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                               .pNext  = nullptr,
                               .memory = buffer_impl->vma_allocation_info.deviceMemory,
                               .offset = range.offset,
                               .size   = range.size};
  return (gfx::Status) impl->vk_table.InvalidateMappedMemoryRanges(impl->vk_device, 1, &vk_range);
}

gfx::Status DeviceInterface::flush_buffer_memory_map(gfx::Device self, gfx::Buffer buffer,
                                                     gfx::MemoryRange range)
{
  // TODO(lamarrr): check is host accessible
  Device             *impl        = (Device *) self;
  Buffer             *buffer_impl = (Buffer *) buffer;
  VkMappedMemoryRange vk_range{.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                               .pNext  = nullptr,
                               .memory = buffer_impl->vma_allocation_info.deviceMemory,
                               .offset = range.offset,
                               .size   = range.size};
  return (gfx::Status) impl->vk_table.FlushMappedMemoryRanges(impl->vk_device, 1, &vk_range);
}

Result<usize, gfx::Status> DeviceInterface::get_pipeline_cache_size(gfx::Device        self,
                                                                    gfx::PipelineCache cache)
{
  Device  *impl = (Device *) self;
  usize    size;
  VkResult result = impl->vk_table.GetPipelineCacheData(
      impl->vk_device, ((PipelineCache *) cache)->vk_cache, &size, nullptr);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }
  return stx::Ok((usize) size);
}

Result<usize, gfx::Status> DeviceInterface::get_pipeline_cache_data(gfx::Device        self,
                                                                    gfx::PipelineCache cache,
                                                                    Span<u8>           out)
{
  Device  *impl   = (Device *) self;
  usize    size   = out.size_bytes();
  VkResult result = impl->vk_table.GetPipelineCacheData(
      impl->vk_device, ((PipelineCache *) cache)->vk_cache, &size, out.data);
  if (result != VK_SUCCESS)
  {
    return stx::Err((gfx::Status) result);
  }
  return stx::Ok((usize) size);
}

gfx::Status DeviceInterface::merge_pipeline_cache(gfx::Device self, gfx::PipelineCache dst,
                                                  Span<gfx::PipelineCache const> srcs)
{
  Device          *impl      = (Device *) self;
  u32              num_srcs  = (u32) srcs.size;
  VkPipelineCache *vk_caches = ALLOC_N(impl->allocator, VkPipelineCache, num_srcs);
  if (vk_caches == nullptr)
  {
    return gfx::Status::OutOfHostMemory;
  }

  for (u32 i = 0; i < num_srcs; i++)
  {
    vk_caches[i] = ((PipelineCache *) srcs.data[i])->vk_cache;
  }

  VkResult result = impl->vk_table.MergePipelineCaches(
      impl->vk_device, ((PipelineCache *) dst)->vk_cache, num_srcs, vk_caches);

  impl->allocator.deallocate(vk_caches);

  return (gfx::Status) result;
}

gfx::Status DeviceInterface::wait_for_fences(gfx::Device self, Span<gfx::Fence const> fences,
                                             bool all, nanoseconds timeout)
{
  Device  *impl       = (Device *) self;
  u32      num_fences = (u32) fences.size;
  VkFence *vk_fences  = ALLOC_N(impl->allocator, VkFence, num_fences);

  if (vk_fences == nullptr)
  {
    return gfx::Status::OutOfHostMemory;
  }

  for (u32 i = 0; i < num_fences; i++)
  {
    vk_fences[i] = ((Fence *) fences.data[i])->vk_fence;
  }

  VkResult result = impl->vk_table.WaitForFences(impl->vk_device, num_fences, vk_fences,
                                                 (VkBool32) all, (u64) timeout.count());

  impl->allocator.deallocate(vk_fences);

  return (gfx::Status) result;
}

gfx::Status DeviceInterface::reset_fences(gfx::Device self, Span<gfx::Fence const> fences)
{
  Device  *impl       = (Device *) self;
  u32      num_fences = (u32) fences.size;
  VkFence *vk_fences  = ALLOC_N(impl->allocator, VkFence, num_fences);
  if (vk_fences == nullptr)
  {
    return gfx::Status::OutOfHostMemory;
  }

  for (u32 i = 0; i < num_fences; i++)
  {
    vk_fences[i] = ((Fence *) fences.data[i])->vk_fence;
  }

  VkResult result = impl->vk_table.ResetFences(impl->vk_device, num_fences, vk_fences);

  impl->allocator.deallocate(vk_fences);

  return (gfx::Status) result;
}

gfx::Status DeviceInterface::get_fence_status(gfx::Device self, gfx::Fence fence)
{
  Device *impl = (Device *) self;
  return (gfx::Status) impl->vk_table.GetFenceStatus(impl->vk_device, ((Fence *) fence)->vk_fence);
}

void DeviceInterface::submit(gfx::Device self, gfx::CommandEncoder &encoder,
                             gfx::Fence signal_fence)
{
  Device             *impl = (Device *) self;
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

void DeviceInterface::wait_idle(gfx::Device self)
{
  vk_table.DeviceWaitIdle(vk_device);
  // todo(lamarrr):check
}

void DeviceInterface::wait_queue_idle(gfx::Device self)
{
  vk_table.QueueWaitIdle(vk_queue);
  // todo(lamarrr):check
}

void CommandEncoderInterface::begin(gfx::CommandEncoder self)
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

gfx::Status CommandEncoderInterface::end(gfx::CommandEncoder self)
{
  if (status != gfx::Status::Success)
  {
    return status;
  }
  device->vk_table.EndCommandBuffer(vk_command_buffer);
}

void CommandEncoderInterface::reset(gfx::CommandEncoder self)
{
  device->vk_table.ResetCommandPool(device->vk_device, vk_command_pool, 0);
  status = gfx::Status::Success;
  // reset status
  // preserve buffers
}

void CommandEncoderInterface::begin_debug_marker(gfx::CommandEncoder self, char const *region_name,
                                                 Vec4 color)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  VkDebugMarkerMarkerInfoEXT info{.sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
                                  .pNext       = nullptr,
                                  .pMarkerName = region_name,
                                  .color       = {color.x, color.y, color.z, color.w}};
  impl->device->vk_table.CmdDebugMarkerBeginEXT(impl->vk_command_buffer, &info);
}

void CommandEncoderInterface::end_debug_marker(gfx::CommandEncoder self)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->device->vk_table.CmdDebugMarkerEndEXT(impl->vk_command_buffer);
}

void CommandEncoderInterface::fill_buffer(gfx::CommandEncoder self, gfx::Buffer dst, u64 offset,
                                          u64 size, u32 data)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
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

void CommandEncoderInterface::copy_buffer(gfx::CommandEncoder self, gfx::Buffer src,
                                          gfx::Buffer dst, Span<gfx::BufferCopy const> copies)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32           num_copies = (u32) copies.size;
  VkBufferCopy *vk_copies  = ALLOC_N(allocator, VkBufferCopy, num_copies);

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

void CommandEncoderInterface::update_buffer(gfx::CommandEncoder self, Span<u8 const> src,
                                            u64 dst_offset, gfx::Buffer dst)
{
  CommandEncoder *impl = (CommandEncoder *) self;
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
                                   (u64) src.size, src.data());

  sync_release_buffer(device->vk_table, vk_command_buffer, dst_impl->vk_buffer,
                      dst_impl->desc.usage, scope);
}

void CommandEncoderInterface::clear_color_image(gfx::CommandEncoder self, gfx::Image dst,
                                                gfx::Color                             clear_color,
                                                Span<gfx::ImageSubresourceRange const> ranges)

{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (status != gfx::Status::Success)
  {
    return;
  }
  u32                      num_ranges = (u32) ranges.size;
  VkImageSubresourceRange *vk_ranges  = ALLOC_N(allocator, VkImageSubresourceRange, num_ranges);

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

void CommandEncoderInterface::clear_depth_stencil_image(
    gfx::CommandEncoder self, gfx::Image dst, gfx::DepthStencil clear_depth_stencil,
    Span<gfx::ImageSubresourceRange const> ranges)

{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32                      num_ranges = (u32) ranges.size;
  VkImageSubresourceRange *vk_ranges  = ALLOC_N(allocator, VkImageSubresourceRange, num_ranges);

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

void CommandEncoderInterface::copy_image(gfx::CommandEncoder self, gfx::Image src, gfx::Image dst,
                                         Span<gfx::ImageCopy const> copies)
{
  // check copies > 0
  CommandEncoder *impl = (CommandEncoder *) self;
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32          num_copies = (u32) copies.size;
  VkImageCopy *vk_copies  = ALLOC_N(allocator, VkImageCopy, num_copies);

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

void CommandEncoderInterface::copy_buffer_to_image(gfx::CommandEncoder self, gfx::Buffer src,
                                                   gfx::Image                       dst,
                                                   Span<gfx::BufferImageCopy const> copies)
{
  // check copies > 0
  CommandEncoder *impl = (CommandEncoder *) self;
  if (status != gfx::Status::Success)
  {
    return;
  }

  u32                num_copies = (u32) copies.size;
  VkBufferImageCopy *vk_copies  = ALLOC_N(allocator, VkBufferImageCopy, num_copies);

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

void CommandEncoderInterface::blit_image(gfx::CommandEncoder self, gfx::Image src, gfx::Image dst,
                                         Span<gfx::ImageBlit const> blits, gfx::Filter filter)
{
  // check blits >0
  CommandEncoder *impl = (CommandEncoder *) self;

  u32          num_blits = (u32) blits.size;
  VkImageBlit *vk_blits  = ALLOC_N(allocator, VkImageBlit, num_blits);

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

u32 CommandEncoderInterface::push_descriptors(gfx::CommandEncoder   self,
                                              gfx::DescriptorLayout layout, u32 count)
{
  // if(   ){}
  //  check if in descriptor pass
  ASH_CHECK(count > 0, "Push descriptor count is 0");
  u32 const descriptor = vk_descriptor_layouts.count;

  impl->status = descriptor_storage_bindings.ends.grow_size(allocator, count);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };
  impl->status = vk_descriptor_layouts.grow_size(allocator, count);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };

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
void CommandEncoderInterface::push_bindings(gfx::CommandEncoder self, u32 descriptor,
                                            gfx::PipelineBindPoint         bind_point,
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
      (u32) bindings.storage_buffers.size + (u32) bindings.storage_texel_buffers.size;
  u32 const num_storage_images              = (u32) bindings.storage_images.size;
  u32       buffer_writes_offset            = vk_descriptor_buffers.count;
  u32       texel_buffer_view_writes_offset = vk_descriptor_texel_buffer_views.count;
  u32       image_writes_offset             = vk_descriptor_images.count;
  u32       writes_offset                   = vk_descriptor_writes.count;
  u32 const num_buffer_writes =
      (u32) bindings.uniform_buffers.size + (u32) bindings.storage_buffers.size;
  u32 const num_texel_buffer_view_writes =
      (u32) bindings.uniform_texel_buffers.size + (u32) bindings.storage_texel_buffers.size;
  u32 const num_image_writes =
      (u32) bindings.samplers.size + (u32) bindings.combined_image_samplers.size +
      (u32) bindings.sampled_images.size + (u32) bindings.storage_images.size +
      (u32) bindings.input_attachments.size;
  u32       writes_offset = vk_descriptor_writes.count;
  u32 const num_writes    = num_buffer_writes + num_texel_buffer_view_writes + num_image_writes;

  impl->status = descriptor_storage_bindings.buffers.grow_size(allocator, num_storage_buffers);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };
  impl->status = descriptor_storage_bindings.images.grow_size(allocator, num_storage_images);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };
  impl->status = vk_descriptor_buffers.grow_size(allocator, num_buffer_writes);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };
  impl->status =
      vk_descriptor_texel_buffer_views.grow_size(allocator, num_texel_buffer_view_writes);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };
  impl->status = vk_descriptor_images.grow_size(allocator, num_image_writes);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };
  impl->status = vk_descriptor_writes.grow_size(allocator, num_writes);
  if (impl->status != gfx::Status::Success)
  {
    return;
  };

  for (u32 i = 0; i < (u32) bindings.storage_buffers.size; i++, storage_buffers_offset++)
  {
    descriptor_storage_bindings.buffers.data[storage_buffers_offset] =
        ((Buffer *) bindings.storage_buffers.data()[i].buffer);
  }

  for (u32 i = 0; i < (u32) bindings.storage_texel_buffers.size; i++, storage_buffers_offset++)
  {
    descriptor_storage_bindings.buffers.data[storage_buffers_offset] =
        (Buffer *) (((BufferView *) bindings.storage_texel_buffers.data()[i].buffer_view)
                        ->desc.buffer);
  }

  for (u32 i = 0; i < (u32) bindings.storage_images.size; i++, storage_images_offset++)
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

void CommandEncoderInterface::end_descriptor_pass(gfx::CommandEncoder self)
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

void CommandEncoderInterface::bind_descriptor(gfx::CommandEncoder self, u32 index)
{
  // checks
  // bind to render/compute
  // check index
}

void CommandEncoderInterface::bind_next_descriptor(gfx::CommandEncoder self, )
{
  // advance
  // bind to render/compute
  // checks
  // check index
}

void CommandEncoderInterface::begin_render_pass(
    gfx::CommandEncoder self, gfx::Framebuffer framebuffer, gfx::RenderPass render_pass,
    IRect render_area, Span<gfx::Color const> color_attachments_clear_values,
    Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values)
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

void CommandEncoderInterface::end_render_pass(gfx::CommandEncoder self, )
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

void CommandEncoderInterface::bind_pipeline(gfx::CommandEncoder self, gfx::ComputePipeline pipeline)
{
  // check status
  this->compute_pipeline = (ComputePipeline *) pipeline;
  device->vk_table.CmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                   this->compute_pipeline->vk_pipeline);
}

void CommandEncoderInterface::bind_pipeline(gfx::CommandEncoder   self,
                                            gfx::GraphicsPipeline pipeline)
{
  this->graphics_pipeline = (GraphicsPipeline *) pipeline;
  device->vk_table.CmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   this->graphics_pipeline->vk_pipeline);
}

void CommandEncoderInterface::push_constants(gfx::CommandEncoder self,
                                             Span<u8 const>      push_constants_data)
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

void CommandEncoderInterface::dispatch(gfx::CommandEncoder self, u32 group_count_x,
                                       u32 group_count_y, u32 group_count_z)
{
}

void CommandEncoderInterface::dispatch_indirect(gfx::CommandEncoder self, gfx::Buffer buffer,
                                                u64 offset)
{
  // pre
  device->vk_table;
  // post
}

void CommandEncoderInterface::set_viewport(gfx::CommandEncoder self, gfx::Viewport const &viewport)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  VkViewport vk_viewport{.x        = viewport.area.offset.x,
                         .y        = viewport.area.offset.y,
                         .width    = viewport.area.extent.x,
                         .height   = viewport.area.extent.y,
                         .minDepth = viewport.min_depth,
                         .maxDepth = viewport.max_depth};
  impl->device->vk_table.CmdSetViewport(impl->vk_command_buffer, 0, 1, &vk_viewport);
}

void CommandEncoderInterface::set_scissor(gfx::CommandEncoder self, IRect scissor)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  VkRect2D vk_scissor{.offset = VkOffset2D{scissor.offset.x, scissor.offset.y},
                      .extent = VkExtent2D{scissor.extent.width, scissor.extent.height}};
  impl->device->vk_table.CmdSetScissor(impl->vk_command_buffer, 0, 1, &vk_scissor);
}

void CommandEncoderInterface::set_blend_constants(gfx::CommandEncoder self, Vec4 blend_constants)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  f32 vk_constants[4] = {blend_constants.x, blend_constants.y, blend_constants.z,
                         blend_constants.w};
  impl->device->vk_table.CmdSetBlendConstants(impl->vk_command_buffer, vk_constants);
}

void CommandEncoderInterface::set_stencil_compare_mask(gfx::CommandEncoder self,
                                                       gfx::StencilFaces faces, u32 mask)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->device->vk_table.CmdSetStencilCompareMask(impl->vk_command_buffer,
                                                  (VkStencilFaceFlags) faces, mask);
}

void CommandEncoderInterface::set_stencil_reference(gfx::CommandEncoder self,
                                                    gfx::StencilFaces faces, u32 reference)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->device->vk_table.CmdSetStencilReference(impl->vk_command_buffer, (VkStencilFaceFlags) faces,
                                                reference);
}

void CommandEncoderInterface::set_stencil_write_mask(gfx::CommandEncoder self,
                                                     gfx::StencilFaces faces, u32 mask)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->device->vk_table.CmdSetStencilWriteMask(impl->vk_command_buffer, (VkStencilFaceFlags) faces,
                                                mask);
}

void CommandEncoderInterface::set_vertex_buffers(gfx::CommandEncoder     self,
                                                 Span<gfx::Buffer const> vertex_buffers,
                                                 Span<u64 const>         offsets)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  u32       num_buffers = (u32) vertex_buffers.size;
  VkBuffer *vk_buffers  = ALLOC_N(impl->allocator, VkBuffer, num_buffers);

  if (vk_buffers == nullptr)
  {
    impl->status = gfx::Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_buffers; i++)
  {
    vk_buffers[i] = ((Buffer *) vertex_buffers.data[i])->vk_buffer;
  }

  impl->device->vk_table.CmdBindVertexBuffers(impl->vk_command_buffer, 0, num_buffers, vk_buffers,
                                              offsets.data);

  impl->allocator.deallocate(vk_buffers);
}

void CommandEncoderInterface::set_index_buffer(gfx::CommandEncoder self, gfx::Buffer index_buffer,
                                               u64 offset)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->device->vk_table.CmdBindIndexBuffer(
      impl->vk_command_buffer, ((Buffer *) index_buffer)->vk_buffer, offset, VK_INDEX_TYPE_UINT32);
}

void CommandEncoderInterface::draw(gfx::CommandEncoder self, u32 first_index, u32 num_indices,
                                   i32 vertex_offset, u32 first_instance, u32 num_instances)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->device->vk_table.CmdDrawIndexed(impl->vk_command_buffer, num_indices, num_instances,
                                        first_index, vertex_offset, first_instance);
}

void CommandEncoderInterface::draw_indirect(gfx::CommandEncoder self, gfx::Buffer buffer,
                                            u64 offset, u32 draw_count, u32 stride)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->device->vk_table.CmdDrawIndexedIndirect(
      impl->vk_command_buffer, ((Buffer *) buffer)->vk_buffer, offset, draw_count, stride);
}

// TODO(lamarrr): instead of this we can have tasks use the trailing frame indices to execute tasks
void CommandEncoderInterface::on_execution_complete(gfx::CommandEncoder     self,
                                                    stx::UniqueFn<void()> &&fn)
{
  CommandEncoder *impl = (CommandEncoder *) self;
  if (impl->status != gfx::Status::Success)
  {
    return;
  }
  impl->completion_tasks.push(std::move(fn)).unwrap();
}

}        // namespace vk
}        // namespace ash
