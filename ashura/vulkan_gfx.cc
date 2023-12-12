#include "ashura/vulkan_gfx.h"
#include "ashura/mem.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

#define VALIDATE(desc, ...)           \
  do                                  \
  {                                   \
    if (!(__VA_ARGS__))               \
    {                                 \
      stx::panic(desc, #__VA_ARGS__); \
    }                                 \
  } while (false)

#define CHECK(desc, ...)              \
  do                                  \
  {                                   \
    if (!(__VA_ARGS__))               \
    {                                 \
      stx::panic(desc, #__VA_ARGS__); \
    }                                 \
  } while (false)

#define UNREACHABLE() stx::panic("Expected Unreachable")

#define BUFFER_FROM_VIEW(buffer_view) \
  ((Buffer *) (((BufferView *) (buffer_view))->desc.buffer))
#define IMAGE_FROM_VIEW(image_view) \
  ((Image *) (((ImageView *) (image_view))->desc.image))

// todo(lamarrr): define macros for checks, use debug name
static gfx::DeviceInterface const device_interface{
    .ref                   = DeviceInterface::ref,
    .unref                 = DeviceInterface::unref,
    .get_device_info       = DeviceInterface::get_device_info,
    .get_format_properties = DeviceInterface::get_format_properties,
    .create_buffer         = DeviceInterface::create_buffer,
    .create_buffer_view    = DeviceInterface::create_buffer_view,
    .create_image          = DeviceInterface::create_image,
    .create_image_view     = DeviceInterface::create_image_view,
    .create_sampler        = DeviceInterface::create_sampler,
    .create_shader         = DeviceInterface::create_shader,
    .create_render_pass    = DeviceInterface::create_render_pass,
    .create_framebuffer    = DeviceInterface::create_framebuffer,
    .create_descriptor_set_layout =
        DeviceInterface::create_descriptor_set_layout,
    .create_descriptor_heap      = DeviceInterface::create_descriptor_heap,
    .create_pipeline_cache       = DeviceInterface::create_pipeline_cache,
    .create_compute_pipeline     = DeviceInterface::create_compute_pipeline,
    .create_fence                = DeviceInterface::create_fence,
    .create_frame_context        = DeviceInterface::create_frame_context,
    .create_swapchain            = DeviceInterface::create_swapchain,
    .ref_buffer                  = DeviceInterface::ref_buffer,
    .ref_buffer_view             = DeviceInterface::ref_buffer_view,
    .ref_image                   = DeviceInterface::ref_image,
    .ref_image_view              = DeviceInterface::ref_image_view,
    .ref_sampler                 = DeviceInterface::ref_sampler,
    .ref_shader                  = DeviceInterface::ref_shader,
    .ref_render_pass             = DeviceInterface::ref_render_pass,
    .ref_framebuffer             = DeviceInterface::ref_framebuffer,
    .ref_descriptor_set_layout   = DeviceInterface::ref_descriptor_set_layout,
    .ref_descriptor_heap         = DeviceInterface::ref_descriptor_heap,
    .ref_pipeline_cache          = DeviceInterface::ref_pipeline_cache,
    .ref_compute_pipeline        = DeviceInterface::ref_compute_pipeline,
    .ref_fence                   = DeviceInterface::ref_fence,
    .ref_command_encoder         = DeviceInterface::ref_command_encoder,
    .ref_frame_context           = DeviceInterface::ref_frame_context,
    .unref_buffer                = DeviceInterface::unref_buffer,
    .unref_buffer_view           = DeviceInterface::unref_buffer_view,
    .unref_image                 = DeviceInterface::unref_image,
    .unref_image_view            = DeviceInterface::unref_image_view,
    .unref_sampler               = DeviceInterface::unref_sampler,
    .unref_shader                = DeviceInterface::unref_shader,
    .unref_render_pass           = DeviceInterface::unref_render_pass,
    .unref_framebuffer           = DeviceInterface::unref_framebuffer,
    .unref_descriptor_set_layout = DeviceInterface::unref_descriptor_set_layout,
    .unref_descriptor_heap       = DeviceInterface::unref_descriptor_heap,
    .unref_pipeline_cache        = DeviceInterface::unref_pipeline_cache,
    .unref_compute_pipeline      = DeviceInterface::unref_compute_pipeline,
    .unref_fence                 = DeviceInterface::unref_fence,
    .unref_command_encoder       = DeviceInterface::unref_command_encoder,
    .unref_frame_context         = DeviceInterface::unref_frame_context,
    .get_buffer_memory_map       = DeviceInterface::get_buffer_memory_map,
    .invalidate_buffer_memory_map =
        DeviceInterface::invalidate_buffer_memory_map,
    .flush_buffer_memory_map   = DeviceInterface::flush_buffer_memory_map,
    .get_pipeline_cache_size   = DeviceInterface::get_pipeline_cache_size,
    .get_pipeline_cache_data   = DeviceInterface::get_pipeline_cache_data,
    .merge_pipeline_cache      = DeviceInterface::merge_pipeline_cache,
    .wait_for_fences           = DeviceInterface::wait_for_fences,
    .reset_fences              = DeviceInterface::reset_fences,
    .get_fence_status          = DeviceInterface::get_fence_status,
    .submit                    = DeviceInterface::submit,
    .wait_idle                 = DeviceInterface::wait_idle,
    .wait_queue_idle           = DeviceInterface::wait_queue_idle,
    .get_frame_info            = DeviceInterface::get_frame_info,
    .get_surface_formats       = DeviceInterface::get_surface_formats,
    .get_surface_present_modes = DeviceInterface::get_surface_present_modes,
    .get_swapchain_info        = DeviceInterface::get_swapchain_info,
    .invalidate_swapchain      = DeviceInterface::invalidate_swapchain,
    .begin_frame               = DeviceInterface::begin_frame,
    .submit_frame              = DeviceInterface::submit_frame};

static gfx::DescriptorHeapInterface const descriptor_heap_interface{
    .add_group              = DescriptorHeapInterface::add_group,
    .sampler                = DescriptorHeapInterface::sampler,
    .combined_image_sampler = DescriptorHeapInterface::combined_image_sampler,
    .sampled_image          = DescriptorHeapInterface::sampled_image,
    .storage_image          = DescriptorHeapInterface::storage_image,
    .uniform_texel_buffer   = DescriptorHeapInterface::uniform_texel_buffer,
    .storage_texel_buffer   = DescriptorHeapInterface::storage_texel_buffer,
    .uniform_buffer         = DescriptorHeapInterface::uniform_buffer,
    .storage_buffer         = DescriptorHeapInterface::storage_buffer,
    .dynamic_uniform_buffer = DescriptorHeapInterface::dynamic_uniform_buffer,
    .dynamic_storage_buffer = DescriptorHeapInterface::dynamic_storage_buffer,
    .input_attachment       = DescriptorHeapInterface::input_attachment,
    .mark_in_use            = DescriptorHeapInterface::mark_in_use,
    .is_in_use              = DescriptorHeapInterface::is_in_use,
    .release                = DescriptorHeapInterface::release,
    .get_stats              = DescriptorHeapInterface::get_stats};

static gfx::CommandEncoderInterface const command_encoder_interface{
    .begin              = CommandEncoderInterface::begin,
    .end                = CommandEncoderInterface::end,
    .begin_debug_marker = CommandEncoderInterface::begin_debug_marker,
    .end_debug_marker   = CommandEncoderInterface::end_debug_marker,
    .fill_buffer        = CommandEncoderInterface::fill_buffer,
    .copy_buffer        = CommandEncoderInterface::copy_buffer,
    .update_buffer      = CommandEncoderInterface::update_buffer,
    .clear_color_image  = CommandEncoderInterface::clear_color_image,
    .clear_depth_stencil_image =
        CommandEncoderInterface::clear_depth_stencil_image,
    .copy_image             = CommandEncoderInterface::copy_image,
    .copy_buffer_to_image   = CommandEncoderInterface::copy_buffer_to_image,
    .blit_image             = CommandEncoderInterface::blit_image,
    .begin_render_pass      = CommandEncoderInterface::begin_render_pass,
    .end_render_pass        = CommandEncoderInterface::end_render_pass,
    .bind_compute_pipeline  = CommandEncoderInterface::bind_compute_pipeline,
    .bind_graphics_pipeline = CommandEncoderInterface::bind_graphics_pipeline,
    .bind_descriptor_sets   = CommandEncoderInterface::bind_descriptor_sets,
    .push_constants         = CommandEncoderInterface::push_constants,
    .dispatch               = CommandEncoderInterface::dispatch,
    .dispatch_indirect      = CommandEncoderInterface::dispatch_indirect,
    .set_viewport           = CommandEncoderInterface::set_viewport,
    .set_scissor            = CommandEncoderInterface::set_scissor,
    .set_blend_constants    = CommandEncoderInterface::set_blend_constants,
    .set_stencil_compare_mask =
        CommandEncoderInterface::set_stencil_compare_mask,
    .set_stencil_reference  = CommandEncoderInterface::set_stencil_reference,
    .set_stencil_write_mask = CommandEncoderInterface::set_stencil_write_mask,
    .set_vertex_buffers     = CommandEncoderInterface::set_vertex_buffers,
    .set_index_buffer       = CommandEncoderInterface::set_index_buffer,
    .draw                   = CommandEncoderInterface::draw,
    .draw_indirect          = CommandEncoderInterface::draw_indirect};

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

bool load_instance_table(VkInstance instance, InstanceTable &vk_instance_table)
{
  bool all_loaded = true;

#define LOAD_VK(function)                                                 \
  vk_instance_table.function =                                            \
      (PFN_vk##function) vkGetInstanceProcAddr(instance, "vk" #function); \
  all_loaded = all_loaded && (vk_instance_table.function != nullptr)

  LOAD_VK(CreateDebugReportCallbackEXT);
  LOAD_VK(CreateDebugUtilsMessengerEXT);
  LOAD_VK(CreateInstance);
  LOAD_VK(DestroyDebugReportCallbackEXT);
  LOAD_VK(DestroyDebugUtilsMessengerEXT);
  LOAD_VK(DestroyInstance);
  LOAD_VK(DestroySurfaceKHR);
  LOAD_VK(EnumeratePhysicalDevices);
  LOAD_VK(GetInstanceProcAddr);
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
#undef LOAD_VK

  return all_loaded;
}

bool load_device_table(VkDevice device, DeviceTable &vk_table,
                       VmaVulkanFunctions &vma_table)
{
  bool all_loaded = true;

#define LOAD_VK(function)                                             \
  vk_table.function =                                                 \
      (PFN_vk##function) vkGetDeviceProcAddr(device, "vk" #function); \
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

  LOAD_VK(CreateSwapchainKHR);
  LOAD_VK(DestroySwapchainKHR);
  LOAD_VK(GetSwapchainImagesKHR);
  LOAD_VK(AcquireNextImageKHR);
  LOAD_VK(QueuePresentKHR);

#undef LOAD_VK

#define LOAD_VK_STUBBED(function)                                     \
  vk_table.function =                                                 \
      (PFN_vk##function) vkGetDeviceProcAddr(device, "vk" #function); \
  vk_table.function =                                                 \
      (vk_table.function != nullptr) ? vk_table.function : function##_Stub;

  LOAD_VK_STUBBED(DebugMarkerSetObjectTagEXT);
  LOAD_VK_STUBBED(DebugMarkerSetObjectNameEXT);

  LOAD_VK_STUBBED(CmdDebugMarkerBeginEXT);
  LOAD_VK_STUBBED(CmdDebugMarkerEndEXT);
  LOAD_VK_STUBBED(CmdDebugMarkerInsertEXT);

#undef LOAD_VK_STUBBED

#define SET_VMA(function) vma_table.vk##function = vk_table.function
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

inline void access_buffer(CommandEncoder const *encoder, Buffer *buffer,
                          VkPipelineStageFlags stages, VkAccessFlags access)
{
  VkBufferMemoryBarrier barrier;
  VkPipelineStageFlags  src_stages;
  VkPipelineStageFlags  dst_stages;
  if (sync_buffer(buffer->state,
                  BufferAccess{.stages = stages, .access = access}, barrier,
                  src_stages, dst_stages))
  {
    barrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer              = buffer->vk_buffer;
    barrier.offset              = 0;
    barrier.size                = VK_WHOLE_SIZE;
    encoder->device->vk_table.CmdPipelineBarrier(
        encoder->vk_command_buffer, src_stages, dst_stages, 0, 0, nullptr, 1,
        &barrier, 0, nullptr);
  }
}

inline void access_image(CommandEncoder const *encoder, Image *image,
                         VkPipelineStageFlags stages, VkAccessFlags access,
                         VkImageLayout layout)
{
  VkImageMemoryBarrier barrier;
  VkPipelineStageFlags src_stages;
  VkPipelineStageFlags dst_stages;
  if (sync_image(
          image->state,
          ImageAccess{.stages = stages, .access = access, .layout = layout},
          barrier, src_stages, dst_stages))
  {
    barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = image->vk_image;
    barrier.subresourceRange.aspectMask =
        (VkImageAspectFlags) image->desc.aspects;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
    encoder->device->vk_table.CmdPipelineBarrier(
        encoder->vk_command_buffer, src_stages, dst_stages, 0, 0, nullptr, 0,
        nullptr, 1, &barrier);
  }
}

inline bool is_renderpass_compatible(RenderPass const           *render_pass,
                                     gfx::FramebufferDesc const &desc)
{
  // also depends on the formats of the input attachments which can't be
  // determined here
  // our renderpasses uses same initial and final layouts
  if (render_pass->desc.color_attachments.size != desc.color_attachments.size)
  {
    return false;
  }

  if ((render_pass->desc.depth_stencil_attachment.format ==
       gfx::Format::Undefined) &&
      (desc.depth_stencil_attachment != nullptr))
  {
    return false;
  }

  if (desc.depth_stencil_attachment != nullptr &&
      (render_pass->desc.depth_stencil_attachment.format !=
       IMAGE_FROM_VIEW(desc.depth_stencil_attachment)->desc.format))
  {
    return false;
  }

  for (usize i = 0; i < render_pass->desc.color_attachments.size; i++)
  {
    if (render_pass->desc.color_attachments[i].format !=
        IMAGE_FROM_VIEW(desc.color_attachments[i])->desc.format)
    {
      return false;
    }
  }

  return true;
}

Result<gfx::FormatProperties, Status>
    DeviceInterface::get_format_properties(gfx::Device self_,
                                           gfx::Format format)
{
  Device *const      self = (Device *) self_;
  VkFormatProperties props;
  vkGetPhysicalDeviceFormatProperties(self->vk_phy_device, (VkFormat) format,
                                      &props);
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
  VALIDATE("", desc.size > 0);
  VALIDATE("", desc.usage != gfx::BufferUsage::None);

  Device *const      self = (Device *) self_;
  VkBufferCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size  = desc.size,
                                 .usage = (VkBufferUsageFlags) desc.usage,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 1,
                                 .pQueueFamilyIndices   = nullptr};
  VmaAllocationCreateInfo alloc_create_info{
      .flags          = 0,
      .usage          = VMA_MEMORY_USAGE_AUTO,
      .requiredFlags  = (VkMemoryPropertyFlags) desc.properties,
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
    return Err((Status) result);
  }

  void *host_map = nullptr;
  if (has_any_bit(desc.properties, gfx::MemoryProperties::HostVisible |
                                       gfx::MemoryProperties::HostCoherent |
                                       gfx::MemoryProperties::HostCached))
  {
    result = self->vk_table.MapMemory(self->vk_device,
                                      vma_allocation_info.deviceMemory, 0,
                                      VK_WHOLE_SIZE, 0, &host_map);
  }

  if (result != VK_SUCCESS)
  {
    vmaDestroyBuffer(self->vma_allocator, vk_buffer, vma_allocation);
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (buffer) Buffer{.refcount            = 1,
                      .desc                = desc,
                      .vk_buffer           = vk_buffer,
                      .vma_allocation      = vma_allocation,
                      .vma_allocation_info = vma_allocation_info,
                      .host_map            = host_map};

  return Ok((gfx::Buffer) buffer);
}

Result<gfx::BufferView, Status>
    DeviceInterface::create_buffer_view(gfx::Device                self_,
                                        gfx::BufferViewDesc const &desc)
{
  VALIDATE("", desc.buffer != nullptr);
  VALIDATE("", desc.format != gfx::Format::Undefined);
  VALIDATE("", desc.offset < ((Buffer *) desc.buffer)->desc.size);
  VALIDATE("",
           (desc.offset + desc.size) <= ((Buffer *) desc.buffer)->desc.size);

  Device *const self = (Device *) self_;

  VkBufferViewCreateInfo create_info{
      .sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
      .pNext  = nullptr,
      .flags  = 0,
      .buffer = ((Buffer *) desc.buffer)->vk_buffer,
      .format = (VkFormat) desc.format,
      .offset = desc.offset,
      .range  = desc.size};

  VkBufferView vk_view;

  VkResult result = self->vk_table.CreateBufferView(
      self->vk_device, &create_info, nullptr, &vk_view);
  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (view) BufferView{.refcount = 1, .desc = desc, .vk_view = vk_view};

  return Ok((gfx::BufferView) view);
}

Result<gfx::Image, Status>
    DeviceInterface::create_image(gfx::Device self_, gfx::ImageDesc const &desc)
{
  VALIDATE("", desc.format != gfx::Format::Undefined);
  VALIDATE("", desc.usage != gfx::ImageUsage::None);
  VALIDATE("", desc.aspects != gfx::ImageAspects::None);
  VALIDATE("", desc.extent.is_visible());
  VALIDATE("", desc.mip_levels > 0);
  VALIDATE("", desc.array_layers > 0);

  Device *const     self = (Device *) self_;
  VkImageCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = 0,
                                .imageType = (VkImageType) desc.type,
                                .format    = (VkFormat) desc.format,
                                .extent =
                                    VkExtent3D{.width  = desc.extent.width,
                                               .height = desc.extent.height,
                                               .depth  = desc.extent.depth},
                                .mipLevels   = desc.mip_levels,
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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (image) Image{.refcount            = 1,
                    .desc                = desc,
                    .is_swapchain_image  = false,
                    .vk_image            = vk_image,
                    .vma_allocation      = vma_allocation,
                    .vma_allocation_info = vma_allocation_info,
                    .state               = {}};

  return Ok((gfx::Image) image);
}

Result<gfx::ImageView, Status>
    DeviceInterface::create_image_view(gfx::Device               self_,
                                       gfx::ImageViewDesc const &desc)
{
  VALIDATE("", desc.image != nullptr);
  VALIDATE("", desc.view_format != gfx::Format::Undefined);
  VALIDATE("", desc.aspects != gfx::ImageAspects::None);
  VALIDATE("", has_bits(((Image *) desc.image)->desc.aspects,
                        gfx::ImageAspects::None));
  VALIDATE("", desc.first_mip_level < ((Image *) desc.image)->desc.mip_levels);
  VALIDATE("", (desc.first_mip_level + desc.num_mip_levels) <=
                   ((Image *) desc.image)->desc.mip_levels);
  VALIDATE("",
           desc.first_array_layer < ((Image *) desc.image)->desc.array_layers);
  VALIDATE("", (desc.first_array_layer + desc.num_array_layers) <=
                   ((Image *) desc.image)->desc.array_layers);

  Device *const         self = (Device *) self_;
  VkImageViewCreateInfo create_info{
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0,
      .image    = ((Image *) desc.image)->vk_image,
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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (view) ImageView{.refcount = 1, .desc = desc, .vk_view = vk_view};

  return Ok((gfx::ImageView) view);
}

Result<gfx::Sampler, Status>
    DeviceInterface::create_sampler(gfx::Device             self_,
                                    gfx::SamplerDesc const &desc)
{
  Device *const       self = (Device *) self_;
  VkSamplerCreateInfo create_info{
      .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .pNext                   = nullptr,
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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (sampler) Sampler{.refcount = 1, .vk_sampler = vk_sampler};

  return Ok((gfx::Sampler) sampler);
}

Result<gfx::Shader, Status>
    DeviceInterface::create_shader(gfx::Device            self_,
                                   gfx::ShaderDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE("", desc.spirv_code.size_bytes() > 0);

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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (shader) Shader{.refcount = 1, .vk_shader = vk_shader};

  return Ok((gfx::Shader) shader);
}

Result<gfx::RenderPass, Status>
    DeviceInterface::create_render_pass(gfx::Device                self_,
                                        gfx::RenderPassDesc const &desc)
{
  Device *const self = (Device *) self_;

  VALIDATE("", desc.color_attachments.size <= gfx::MAX_COLOR_ATTACHMENTS);
  VALIDATE("", desc.input_attachments.size <= gfx::MAX_INPUT_ATTACHMENTS);

  // renderpass attachment descriptions are packed in the following order:
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

  gfx::RenderPassAttachment *color_attachments =
      self->allocator.allocate_typed<gfx::RenderPassAttachment>(
          num_color_attachments);

  if (color_attachments == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  gfx::RenderPassAttachment *input_attachments =
      self->allocator.allocate_typed<gfx::RenderPassAttachment>(
          num_input_attachments);

  if (color_attachments == nullptr)
  {
    self->allocator.deallocate_typed(color_attachments, num_color_attachments);
    return Err(Status::OutOfHostMemory);
  }

  mem::copy(desc.color_attachments, color_attachments);
  mem::copy(desc.input_attachments, input_attachments);

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
    self->allocator.deallocate_typed(color_attachments, num_color_attachments);
    self->allocator.deallocate_typed(input_attachments, num_input_attachments);
    return Err((Status) result);
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
    self->allocator.deallocate_typed(color_attachments, num_color_attachments);
    self->allocator.deallocate_typed(input_attachments, num_input_attachments);
    return Err(Status::OutOfHostMemory);
  }

  new (render_pass) RenderPass{
      .refcount = 1,
      .desc =
          gfx::RenderPassDesc{
              .label             = desc.label,
              .color_attachments = {color_attachments, num_color_attachments},
              .input_attachments = {input_attachments, num_input_attachments},
              .depth_stencil_attachment = desc.depth_stencil_attachment},
      .vk_render_pass = vk_render_pass};

  return Ok((gfx::RenderPass) render_pass);
}

Result<gfx::Framebuffer, Status>
    DeviceInterface::create_framebuffer(gfx::Device                 self_,
                                        gfx::FramebufferDesc const &desc)
{
  Device *const self                  = (Device *) self_;
  u32 const     num_color_attachments = (u32) desc.color_attachments.size;
  bool const    has_depth_stencil_attachment =
      desc.depth_stencil_attachment != nullptr;
  u32 const num_attachments =
      num_color_attachments + (has_depth_stencil_attachment ? 1U : 0U);
  VkImageView vk_attachments[gfx::MAX_COLOR_ATTACHMENTS + 1];

  VALIDATE("Framebuffer and Renderpass are not compatible",
           is_renderpass_compatible((RenderPass *) desc.renderpass, desc));

  gfx::ImageView *color_attachments =
      self->allocator.allocate_typed<gfx::ImageView>(num_color_attachments);

  if (color_attachments == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  mem::copy(desc.color_attachments, color_attachments);

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
      .attachmentCount = num_attachments,
      .pAttachments    = vk_attachments,
      .width           = desc.extent.width,
      .height          = desc.extent.height,
      .layers          = desc.layers};

  VkFramebuffer vk_framebuffer;

  VkResult result = self->vk_table.CreateFramebuffer(
      self->vk_device, &create_info, nullptr, &vk_framebuffer);
  if (result != VK_SUCCESS)
  {
    self->allocator.deallocate_typed(color_attachments, num_color_attachments);
    return Err((Status) result);
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
    self->allocator.deallocate_typed(color_attachments, num_color_attachments);
    return Err(Status::OutOfHostMemory);
  }

  new (framebuffer) Framebuffer{
      .refcount = 1,
      .desc =
          gfx::FramebufferDesc{
              .label             = desc.label,
              .renderpass        = desc.renderpass,
              .extent            = desc.extent,
              .layers            = desc.layers,
              .color_attachments = {color_attachments, num_color_attachments},
              .depth_stencil_attachment = desc.depth_stencil_attachment},
      .vk_framebuffer = vk_framebuffer};

  return Ok((gfx::Framebuffer) framebuffer);
}

Result<gfx::DescriptorSetLayout, Status>
    DeviceInterface::create_descriptor_set_layout(
        gfx::Device self_, gfx::DescriptorSetLayoutDesc const &desc)
{
  Device *const self         = (Device *) self_;
  u32 const     num_bindings = (u32) desc.bindings.size;

  VALIDATE("", num_bindings > 0);
  for (u32 i = 0; i < num_bindings; i++)
  {
    VALIDATE("", desc.bindings[i].count > 0);
  }

  VkDescriptorSetLayoutBinding *vk_bindings =
      self->allocator.allocate_typed<VkDescriptorSetLayoutBinding>(
          num_bindings);

  if (vk_bindings == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  gfx::DescriptorBindingDesc *bindings =
      self->allocator.allocate_typed<gfx::DescriptorBindingDesc>(num_bindings);

  if (bindings == nullptr)
  {
    self->allocator.deallocate_typed(vk_bindings, num_bindings);
    return Err(Status::OutOfHostMemory);
  }

  mem::copy(desc.bindings, bindings);

  for (u32 i = 0; i < num_bindings; i++)
  {
    gfx::DescriptorBindingDesc const &binding = desc.bindings[i];
    vk_bindings[i]                            = VkDescriptorSetLayoutBinding{
                                   .binding            = i,
                                   .descriptorType     = (VkDescriptorType) binding.type,
                                   .descriptorCount    = binding.count,
                                   .stageFlags         = VK_SHADER_STAGE_ALL,
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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (layout) DescriptorSetLayout{.refcount     = 1,
                                   .bindings     = bindings,
                                   .num_bindings = num_bindings,
                                   .vk_layout    = vk_layout};

  return Ok((gfx::DescriptorSetLayout) layout);
}

Result<gfx::DescriptorHeapImpl, Status> DeviceInterface::create_descriptor_heap(
    gfx::Device                          self_,
    Span<gfx::DescriptorSetLayout const> descriptor_set_layouts,
    u32 groups_per_pool, AllocatorImpl allocator)
{
  Device *const self     = (Device *) self_;
  u32 const     num_sets = (u32) descriptor_set_layouts.size;

  VALIDATE("", groups_per_pool > 0);
  VALIDATE("", num_sets > 0);
  for (gfx::DescriptorSetLayout layout : descriptor_set_layouts)
  {
    DescriptorSetLayout *layout_impl = (DescriptorSetLayout *) layout;
    VALIDATE("", layout_impl->num_bindings > 0);
    for (u32 i = 0; i < layout_impl->num_bindings; i++)
    {
      VALIDATE("", layout_impl->bindings[i].count > 0);
    }
  }

  DescriptorSetLayout **set_layouts =
      self->allocator.allocate_typed<DescriptorSetLayout *>(num_sets);

  if (set_layouts == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  mem::copy(descriptor_set_layouts, (gfx::DescriptorSetLayout *) set_layouts);

  u32 **binding_offsets = self->allocator.allocate_typed<u32 *>(num_sets);

  if (binding_offsets == nullptr)
  {
    self->allocator.deallocate_typed(set_layouts, num_sets);
    return Err(Status::OutOfHostMemory);
  }

  {
    u32 iset = 0;
    for (; iset < num_sets; iset++)
    {
      u32 *binding_offset =
          self->allocator.allocate_typed<u32>(set_layouts[iset]->num_bindings);
      if (binding_offset == nullptr)
      {
        break;
      }
      binding_offsets[iset] = binding_offset;
    }

    if (iset != num_sets)
    {
      for (u32 ifree = 0; ifree < iset; ifree++)
      {
        self->allocator.deallocate_typed(binding_offsets[ifree],
                                         set_layouts[ifree]->num_bindings);
      }
      self->allocator.deallocate_typed(binding_offsets, num_sets);
      self->allocator.deallocate_typed(set_layouts, num_sets);
      return Err(Status::OutOfHostMemory);
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
            num_image_infos = std::max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::CombinedImageSampler:
            offset = (u32) mem::align_offset(
                alignof(gfx::CombinedImageSamplerBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::CombinedImageSamplerBinding) * desc.count);
            num_image_infos = std::max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::SampledImage:
            offset = (u32) mem::align_offset(alignof(gfx::SampledImageBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::SampledImageBinding) * desc.count);
            num_image_infos = std::max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::StorageImage:
            offset = (u32) mem::align_offset(alignof(gfx::StorageImageBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::StorageImageBinding) * desc.count);
            num_image_infos = std::max(num_image_infos, desc.count);
            break;
          case gfx::DescriptorType::UniformTexelBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::UniformTexelBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::UniformTexelBufferBinding) * desc.count);
            num_buffer_views = std::max(num_buffer_views, desc.count);
            break;
          case gfx::DescriptorType::StorageTexelBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::StorageTexelBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::StorageTexelBufferBinding) * desc.count);
            num_buffer_views = std::max(num_buffer_views, desc.count);
            break;
          case gfx::DescriptorType::UniformBuffer:
            offset = (u32) mem::align_offset(alignof(gfx::UniformBufferBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::UniformBufferBinding) * desc.count);
            num_buffer_infos = std::max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::StorageBuffer:
            offset = (u32) mem::align_offset(alignof(gfx::StorageBufferBinding),
                                             offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::StorageBufferBinding) * desc.count);
            num_buffer_infos = std::max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::DynamicUniformBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::DynamicUniformBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::DynamicUniformBufferBinding) * desc.count);
            num_buffer_infos = std::max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::DynamicStorageBuffer:
            offset = (u32) mem::align_offset(
                alignof(gfx::DynamicStorageBufferBinding), offset);
            binding_offsets[set][binding] = offset;
            offset +=
                (u32) (sizeof(gfx::DynamicStorageBufferBinding) * desc.count);
            num_buffer_infos = std::max(num_buffer_infos, desc.count);
            break;
          case gfx::DescriptorType::InputAttachment:
            offset = (u32) mem::align_offset(
                alignof(gfx::InputAttachmentBinding), offset);
            binding_offsets[set][binding] = offset;
            offset += (u32) (sizeof(gfx::InputAttachmentBinding) * desc.count);
            num_image_infos = std::max(num_image_infos, desc.count);
            break;
          default:
            break;
        }
      }
    }
    group_binding_stride = offset;
  }

  usize scratch_size =
      std::max({num_image_infos * sizeof(VkDescriptorImageInfo),
                num_buffer_infos * sizeof(VkDescriptorBufferInfo),
                num_buffer_views * sizeof(VkBufferView)});

  void *scratch_memory =
      self->allocator.allocate(MAX_STANDARD_ALIGNMENT, scratch_size);
  if (scratch_memory == nullptr)
  {
    for (u32 ifree = 0; ifree < num_sets; ifree++)
    {
      self->allocator.deallocate_typed(binding_offsets[ifree],
                                       set_layouts[ifree]->num_bindings);
    }
    self->allocator.deallocate_typed(binding_offsets, num_sets);
    self->allocator.deallocate_typed(set_layouts, num_sets);
    return Err(Status::OutOfHostMemory);
  }

  DescriptorHeap *descriptor_heap =
      self->allocator.allocate_typed<DescriptorHeap>(1);

  if (descriptor_heap == nullptr)
  {
    self->allocator.deallocate(MAX_STANDARD_ALIGNMENT, scratch_memory,
                               scratch_size);
    for (u32 ifree = 0; ifree < num_sets; ifree++)
    {
      self->allocator.deallocate_typed(binding_offsets[ifree],
                                       set_layouts[ifree]->num_bindings);
    }
    self->allocator.deallocate_typed(binding_offsets, num_sets);
    self->allocator.deallocate_typed(set_layouts, num_sets);
    return Err(Status::OutOfHostMemory);
  }

  new (descriptor_heap)
      DescriptorHeap{.refcount                    = 1,
                     .device                      = self,
                     .allocator                   = allocator,
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
                     .scratch_memory_size         = 0};

  return Ok(
      gfx::DescriptorHeapImpl{.self = (gfx::DescriptorHeap) descriptor_heap,
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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (cache) PipelineCache{.refcount = 1, .vk_cache = vk_cache};

  return Ok((gfx::PipelineCache) cache);
}

Result<gfx::ComputePipeline, Status> DeviceInterface::create_compute_pipeline(
    gfx::Device self_, gfx::ComputePipelineDesc const &desc)
{
  Device *const self                = (Device *) self_;
  u32 const     num_descriptor_sets = (u32) desc.descriptor_set_layouts.size;

  VALIDATE("", num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE("", desc.push_constant_size <= gfx::MAX_PUSH_CONSTANT_SIZE);

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
      .pName  = desc.compute_shader.entry_point == nullptr ?
                    "main" :
                    desc.compute_shader.entry_point,
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
      .pPushConstantRanges    = &push_constant_range};

  VkPipelineLayout vk_layout;
  VkResult         result = self->vk_table.CreatePipelineLayout(
      self->vk_device, &layout_create_info, nullptr, &vk_layout);
  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (pipeline) ComputePipeline{
      .refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

  return Ok((gfx::ComputePipeline) pipeline);
}

Result<gfx::GraphicsPipeline, Status> DeviceInterface::create_graphics_pipeline(
    gfx::Device self_, gfx::GraphicsPipelineDesc const &desc)
{
  Device *const self                = (Device *) self_;
  u32 const     num_descriptor_sets = (u32) desc.descriptor_set_layouts.size;

  VALIDATE("number of descriptor set layouts exceed maximum pipeline "
           "descriptor set size",
           num_descriptor_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE("", desc.push_constant_size <= gfx::MAX_PUSH_CONSTANT_SIZE);

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
       .pName  = desc.vertex_shader.entry_point == nullptr ?
                     "main" :
                     desc.vertex_shader.entry_point,
       .pSpecializationInfo = &vk_vs_specialization},
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = ((Shader *) desc.fragment_shader.shader)->vk_shader,
       .pName  = desc.fragment_shader.entry_point == nullptr ?
                     "main" :
                     desc.fragment_shader.entry_point,
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
    return Err((Status) result);
  }

  VkVertexInputBindingDescription input_bindings[gfx::MAX_VERTEX_ATTRIBUTES];
  u32 const num_input_bindings = (u32) desc.vertex_input_bindings.size;
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
  u32 const num_attributes = (u32) desc.vertex_attributes.size;
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
  u32 const num_color_attachments =
      (u32) desc.color_blend_state.attachments.size;

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
      .blendConstants  = {desc.color_blend_state.blend_constants.x,
                          desc.color_blend_state.blend_constants.y,
                          desc.color_blend_state.blend_constants.z,
                          desc.color_blend_state.blend_constants.w}};

  constexpr u32  NUM_PIPELINE_DYNAMIC_STATES                 = 6U;
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
    return Err((Status) result);
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
    return Err(Status::OutOfHostMemory);
  }

  new (pipeline) GraphicsPipeline{
      .refcount = 1, .vk_pipeline = vk_pipeline, .vk_layout = vk_layout};

  return Ok((gfx::GraphicsPipeline) pipeline);
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
    return Err((Status) result);
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
    return Err((Status) result);
  }

  CommandEncoder *encoder = self->allocator.allocate_typed<CommandEncoder>(1);

  if (encoder == nullptr)
  {
    self->vk_table.DestroyCommandPool(self->vk_device, vk_command_pool,
                                      nullptr);
    return Err(Status::OutOfHostMemory);
  }

  new (encoder) CommandEncoder{.refcount                    = 1,
                               .allocator                   = allocator,
                               .device                      = self,
                               .vk_command_pool             = vk_command_pool,
                               .vk_command_buffer           = vk_command_buffer,
                               .bound_compute_pipeline      = nullptr,
                               .bound_graphics_pipeline     = nullptr,
                               .bound_render_pass           = nullptr,
                               .bound_framebuffer           = nullptr,
                               .bound_descriptor_set_heaps  = {},
                               .bound_descriptor_set_groups = {},
                               .num_bound_descriptor_sets   = 0,
                               .status                      = Status::Success};

  return Ok(gfx::CommandEncoderImpl{.self      = (gfx::CommandEncoder) encoder,
                                    .interface = &command_encoder_interface});
}

Result<gfx::FrameContext, Status> DeviceInterface::create_frame_context(
    gfx::Device self_, u32 max_frames_in_flight,
    Span<AllocatorImpl const> command_encoder_allocators)
{
  Device *const self = (Device *) self_;

  gfx::CommandEncoderImpl *command_encoders =
      self->allocator.allocate_typed<gfx::CommandEncoderImpl>(
          max_frames_in_flight);

  if (command_encoders == nullptr)
  {
    return Err(Status::OutOfHostMemory);
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
      for (u32 ifree = 0; ifree < push_end; ifree++)
      {
        DeviceInterface::unref_command_encoder(self_, command_encoders[ifree]);
      }
      return Err((Status) status);
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
      for (u32 ifree = 0; ifree < push_end; ifree++)
      {
        self->vk_table.DestroySemaphore(self->vk_device,
                                        acquire_semaphores[ifree], nullptr);
      }

      for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
      {
        DeviceInterface::unref_command_encoder(self_, command_encoders[ifree]);
      }

      return Err((Status) result);
    }
  }

  gfx::Fence *submit_fences =
      self->allocator.allocate_typed<gfx::Fence>(max_frames_in_flight);

  if (submit_fences == nullptr)
  {
    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      self->vk_table.DestroySemaphore(self->vk_device,
                                      acquire_semaphores[ifree], nullptr);
    }
    return Err(Status::OutOfHostMemory);
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
      for (u32 ifree = 0; ifree < push_end; ifree++)
      {
        DeviceInterface::unref_fence(self_, submit_fences[ifree]);
      }

      for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
      {
        self->vk_table.DestroySemaphore(self->vk_device,
                                        acquire_semaphores[ifree], nullptr);
      }

      for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
      {
        DeviceInterface::unref_command_encoder(self_, command_encoders[ifree]);
      }
      return Err((Status) status);
    }
  }

  VkSemaphore *submit_semaphores =
      self->allocator.allocate_typed<VkSemaphore>(max_frames_in_flight);

  if (submit_semaphores == nullptr)
  {
    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      DeviceInterface::unref_fence(self_, submit_fences[ifree]);
    }

    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      self->vk_table.DestroySemaphore(self->vk_device,
                                      acquire_semaphores[ifree], nullptr);
    }

    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      DeviceInterface::unref_command_encoder(self_, command_encoders[ifree]);
    }

    return Err(Status::OutOfHostMemory);
  }

  FrameContext *frame_context = self->allocator.allocate_typed<FrameContext>(1);

  if (frame_context == nullptr)
  {
    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      self->vk_table.DestroySemaphore(self->vk_device, submit_semaphores[ifree],
                                      nullptr);
    }

    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      DeviceInterface::unref_fence(self_, submit_fences[ifree]);
    }

    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      self->vk_table.DestroySemaphore(self->vk_device,
                                      acquire_semaphores[ifree], nullptr);
    }

    for (u32 ifree = 0; ifree < max_frames_in_flight; ifree++)
    {
      DeviceInterface::unref_command_encoder(self_, command_encoders[ifree]);
    }

    return Err(Status::OutOfHostMemory);
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

  return Ok((gfx::FrameContext) frame_context);
}

/// old swapchain will be retired and destroyed irregardless of whether new
/// swapchain recreation fails.
inline VkResult recreate_swapchain(Device const *device, Swapchain *swapchain)
{
  VALIDATE("", swapchain->desc.preferred_extent.is_visible());
  VALIDATE("",
           swapchain->desc.preferred_buffering <= gfx::MAX_SWAPCHAIN_IMAGES);

  // take ownership of internal data for re-use/release
  VkSwapchainKHR old_vk_swapchain = swapchain->vk_swapchain;
  swapchain->is_valid             = false;
  swapchain->is_optimal           = false;
  swapchain->extent               = Extent{};
  swapchain->num_images           = 0;
  swapchain->current_image        = 0;
  swapchain->vk_swapchain         = nullptr;

  VkSurfaceCapabilitiesKHR surface_capabilities;
  VkResult                 result =
      device->vk_instance_table.GetPhysicalDeviceSurfaceCapabilitiesKHR(
          device->vk_phy_device, swapchain->vk_surface, &surface_capabilities);

  if (result != VK_SUCCESS)
  {
    device->vk_table.DestroySwapchainKHR(device->vk_device, old_vk_swapchain,
                                         nullptr);
    return result;
  }

  VALIDATE("", has_bits(surface_capabilities.supportedUsageFlags,
                        (VkImageUsageFlags) swapchain->desc.usage));

  VkExtent2D vk_extent;

  if (surface_capabilities.currentExtent.width == 0xFFFFFFFFU &&
      surface_capabilities.currentExtent.height == 0xFFFFFFFFU)
  {
    vk_extent.width  = std::clamp(swapchain->desc.preferred_extent.width,
                                  surface_capabilities.minImageExtent.width,
                                  surface_capabilities.maxImageExtent.width);
    vk_extent.height = std::clamp(swapchain->desc.preferred_extent.height,
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
    min_image_count = std::clamp(swapchain->desc.preferred_buffering,
                                 surface_capabilities.minImageCount,
                                 surface_capabilities.maxImageCount);
  }
  else
  {
    min_image_count =
        std::max(min_image_count, surface_capabilities.minImageCount);
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
      .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode           = (VkPresentModeKHR) swapchain->desc.present_mode,
      .clipped               = VK_TRUE,
      .oldSwapchain          = old_vk_swapchain};

  VkSwapchainKHR new_vk_swapchain;
  result = device->vk_table.CreateSwapchainKHR(device->vk_device, &create_info,
                                               nullptr, &new_vk_swapchain);

  if (old_vk_swapchain != nullptr)
  {
    device->vk_table.DestroySwapchainKHR(device->vk_device, old_vk_swapchain,
                                         nullptr);
    old_vk_swapchain = nullptr;
  }

  if (result != VK_SUCCESS)
  {
    return result;
  }

  u32 num_images;
  result = device->vk_table.GetSwapchainImagesKHR(
      device->vk_device, new_vk_swapchain, &num_images, nullptr);

  if (result != VK_SUCCESS)
  {
    device->vk_table.DestroySwapchainKHR(device->vk_device, new_vk_swapchain,
                                         nullptr);
    return result;
  }

  CHECK("", num_images <= gfx::MAX_SWAPCHAIN_IMAGES);

  result = device->vk_table.GetSwapchainImagesKHR(
      device->vk_device, new_vk_swapchain, &num_images, swapchain->vk_images);

  if (result != VK_SUCCESS)
  {
    device->vk_table.DestroySwapchainKHR(device->vk_device, new_vk_swapchain,
                                         nullptr);
    return result;
  }

  for (u32 i = 0; i < num_images; i++)
  {
    swapchain->image_impls[i] =
        Image{.refcount = 1,
              .desc     = gfx::ImageDesc{.type    = gfx::ImageType::Type2D,
                                         .format  = swapchain->desc.format.format,
                                         .usage   = swapchain->desc.usage,
                                         .aspects = gfx::ImageAspects::Color,
                                         .extent  = Extent3D{vk_extent.width,
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
    device->vk_table.DebugMarkerSetObjectNameEXT(device->vk_device,
                                                 &swapchain_debug_info);

    for (u32 i = 0; i < num_images; i++)
    {
      VkDebugMarkerObjectNameInfoEXT image_debug_info{
          .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
          .pNext       = nullptr,
          .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
          .object      = (u64) swapchain->vk_images[i],
          .pObjectName = swapchain->desc.label};
      device->vk_table.DebugMarkerSetObjectNameEXT(device->vk_device,
                                                   &image_debug_info);
    }
  }

  swapchain->generation++;
  swapchain->is_valid      = true;
  swapchain->is_optimal    = true;
  swapchain->extent.width  = vk_extent.width;
  swapchain->extent.height = vk_extent.height;
  swapchain->num_images    = num_images;
  swapchain->current_image = 0;
  swapchain->vk_swapchain  = new_vk_swapchain;

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
    return Err(Status::OutOfHostMemory);
  }

  new (swapchain) Swapchain{.generation    = 0,
                            .desc          = desc,
                            .is_valid      = false,
                            .is_optimal    = false,
                            .extent        = {},
                            .image_impls   = {},
                            .images        = {},
                            .vk_images     = {},
                            .num_images    = 0,
                            .current_image = 0,
                            .vk_swapchain  = nullptr,
                            .vk_surface    = (VkSurfaceKHR) surface};

  return Ok((gfx::Swapchain) swapchain);
}

Result<void *, Status>
    DeviceInterface::get_buffer_memory_map(gfx::Device self_,
                                           gfx::Buffer buffer_)
{
  (void) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE("", has_any_bit(buffer->desc.properties,
                           gfx::MemoryProperties::HostVisible |
                               gfx::MemoryProperties::HostCoherent |
                               gfx::MemoryProperties::HostCached));
  return Ok((void *) buffer->host_map);
}

Result<Void, Status> DeviceInterface::invalidate_buffer_memory_map(
    gfx::Device self_, gfx::Buffer buffer_, gfx::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE("", has_any_bit(buffer->desc.properties,
                           gfx::MemoryProperties::HostVisible |
                               gfx::MemoryProperties::HostCoherent |
                               gfx::MemoryProperties::HostCached));

  VkMappedMemoryRange vk_range{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                               .pNext = nullptr,
                               .memory =
                                   buffer->vma_allocation_info.deviceMemory,
                               .offset = range.offset,
                               .size   = range.size};
  VkResult result = self->vk_table.InvalidateMappedMemoryRanges(self->vk_device,
                                                                1, &vk_range);
  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }
  return Ok(Void{});
}

Result<Void, Status> DeviceInterface::flush_buffer_memory_map(
    gfx::Device self_, gfx::Buffer buffer_, gfx::MemoryRange range)
{
  Device *const self   = (Device *) self_;
  Buffer *const buffer = (Buffer *) buffer_;

  VALIDATE("", has_any_bit(buffer->desc.properties,
                           gfx::MemoryProperties::HostVisible |
                               gfx::MemoryProperties::HostCoherent |
                               gfx::MemoryProperties::HostCached));

  VkMappedMemoryRange vk_range{.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                               .pNext = nullptr,
                               .memory =
                                   buffer->vma_allocation_info.deviceMemory,
                               .offset = range.offset,
                               .size   = range.size};

  VkResult result =
      self->vk_table.FlushMappedMemoryRanges(self->vk_device, 1, &vk_range);
  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }
  return Ok(Void{});
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
    return Err((Status) result);
  }
  return Ok((usize) size);
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
    return Err((Status) result);
  }
  return Ok((usize) size);
}

Result<Void, Status>
    DeviceInterface::merge_pipeline_cache(gfx::Device                    self_,
                                          gfx::PipelineCache             dst,
                                          Span<gfx::PipelineCache const> srcs)
{
  Device *const self     = (Device *) self_;
  u32 const     num_srcs = (u32) srcs.size;

  VkPipelineCache *vk_caches =
      self->allocator.allocate_typed<VkPipelineCache>(num_srcs);
  if (vk_caches == nullptr)
  {
    return Err(Status::OutOfHostMemory);
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
    return Err((Status) result);
  }
  return Ok(Void{});
}

Result<Void, Status> DeviceInterface::wait_for_fences(
    gfx::Device self_, Span<gfx::Fence const> fences, bool all, u64 timeout)
{
  Device *const self       = (Device *) self_;
  u32 const     num_fences = (u32) fences.size;

  VALIDATE("", num_fences > 0);

  VkFence *vk_fences = self->allocator.allocate_typed<VkFence>(num_fences);
  if (vk_fences == nullptr)
  {
    return Err(Status::OutOfHostMemory);
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
    return Err((Status) result);
  }
  return Ok(Void{});
}

Result<Void, Status>
    DeviceInterface::reset_fences(gfx::Device            self_,
                                  Span<gfx::Fence const> fences)
{
  Device *const self       = (Device *) self_;
  u32 const     num_fences = (u32) fences.size;

  VALIDATE("", num_fences > 0);

  VkFence *vk_fences = self->allocator.allocate_typed<VkFence>(num_fences);
  if (vk_fences == nullptr)
  {
    return Err(Status::OutOfHostMemory);
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
    return Err((Status) result);
  }
  return Ok(Void{});
}

Result<bool, Status> DeviceInterface::get_fence_status(gfx::Device self_,
                                                       gfx::Fence  fence)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.GetFenceStatus(self->vk_device,
                                                       ((Fence *) fence)->vk_fence);

  if (result == VK_SUCCESS)
  {
    return Ok(true);
  }

  if (result == VK_NOT_READY)
  {
    return Ok(false);
  }

  return Err((Status) result);
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
    return Ok(Void{});
  }

  return Err((Status) result);
}

Result<Void, Status> DeviceInterface::wait_idle(gfx::Device self_)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.DeviceWaitIdle(self->vk_device);
  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }

  return Ok(Void{});
}

Result<Void, Status> DeviceInterface::wait_queue_idle(gfx::Device self_)
{
  Device *const self   = (Device *) self_;
  VkResult      result = self->vk_table.QueueWaitIdle(self->vk_queue);
  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }

  return Ok(Void{});
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
        return Err((Status) result);
      }
    }

    result = recreate_swapchain(self, swapchain);
    if (result != VK_SUCCESS)
    {
      return Err((Status) result);
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
    return Err((Status) result);
  }

  swapchain->current_image = next_image;
  return Ok(Void{});
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
  VkCommandBuffer const command_buffer =
      ((CommandEncoder *) frame_context
           ->command_encoders[frame_context->current_command_encoder]
           .self)
          ->vk_command_buffer;
  VkSemaphore const acquire_semaphore =
      frame_context->acquire_semaphores[frame_context->current_command_encoder];
  VkSemaphore const submit_semaphore =
      frame_context->submit_semaphores[frame_context->current_command_encoder];

  VALIDATE("", swapchain->is_valid);
  VALIDATE("", swapchain->extent.is_visible());

  VkResult result = self->vk_table.WaitForFences(
      self->vk_device, 1, &submit_fence->vk_fence, VK_TRUE, U64_MAX);

  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }

  result =
      self->vk_table.ResetFences(self->vk_device, 1, &submit_fence->vk_fence);

  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }

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

  if (result != VK_SUCCESS)
  {
    // handle error
    // there's not really any way to preserve state here and allow for re-call?
    return Err((Status) result);
  }

  // TODO(lamarrr): is it possible by any means that the number of maximum in
  // flight frames changes? if the user decides to change the buffering for
  // example how will this affect the program state as they depend on it
  frame_context->current_frame++;
  frame_context->trailing_frame =
      std::max(frame_context->current_frame,
               (gfx::FrameId) frame_context->max_frames_in_flight) -
      frame_context->max_frames_in_flight;
  frame_context->current_command_encoder =
      (frame_context->current_command_encoder + 1) %
      frame_context->max_frames_in_flight;

  //
  // - submit commands
  // - present swapchain images, if error, invalidate.
  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands would have been executed.
  // - repeat.
  //
  //
  //
  // at what point is image invalidation handled? recording commands need to
  // check if images are invalidated or not
  //
  // acquire semaphores needs to be done for each swapchain

  VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                .pNext = nullptr,
                                .waitSemaphoreCount = 1,
                                .pWaitSemaphores    = &submit_semaphore,
                                .pSwapchains        = &swapchain->vk_swapchain,
                                .pImageIndices      = &swapchain->current_image,
                                .pResults           = nullptr};
  result = self->vk_table.QueuePresentKHR(self->vk_queue, &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    swapchain->is_valid = false;
  }
  else if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }

  return Ok(Void{});
}

Result<u32, Status>
    DescriptorHeapInterface::add_group(gfx::DescriptorHeap self_,
                                       gfx::FrameId        trailing_frame)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

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
    mem::zero(self->bindings + group * (usize) self->group_binding_stride +
                  self->binding_offsets[0][0],
              self->group_binding_stride);
    return Ok((u32) group);
  }

  VkDescriptorPool *pools =
      self->allocator.grow_typed(self->vk_pools, self->num_pools, 1);

  if (pools == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  self->vk_pools = pools;

  VkDescriptorSet *descriptor_sets = self->allocator.grow_typed(
      self->vk_descriptor_sets,
      self->num_sets_per_group * self->num_pools * self->num_groups_per_pool,
      self->num_sets_per_group * self->num_groups_per_pool);

  if (descriptor_sets == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  self->vk_descriptor_sets = descriptor_sets;
  VkDescriptorSet *new_descriptor_sets =
      descriptor_sets +
      self->num_sets_per_group * self->num_groups_per_pool * self->num_pools;

  u64 *last_use_frame = self->allocator.grow_typed(
      self->last_use_frame, self->num_pools * self->num_groups_per_pool,
      self->num_groups_per_pool);

  if (last_use_frame == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  self->last_use_frame = last_use_frame;
  self->last_use_frame_capacity += self->num_groups_per_pool;

  u32 *released_groups = self->allocator.grow_typed(
      self->released_groups, self->num_pools * self->num_groups_per_pool,
      self->num_groups_per_pool);

  if (released_groups == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  self->released_groups = released_groups;
  self->released_groups_capacity += self->num_groups_per_pool;

  u32 *free_groups = self->allocator.grow_typed(
      self->free_groups, self->num_pools * self->num_groups_per_pool,
      self->num_groups_per_pool);

  if (free_groups == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  self->free_groups = free_groups;
  self->free_groups_capacity += self->num_groups_per_pool;

  usize const pool_bindings_size =
      self->num_groups_per_pool * (usize) self->group_binding_stride;
  u8 *bindings = (u8 *) self->allocator.grow(
      MAX_STANDARD_ALIGNMENT, self->bindings,
      self->num_pools * pool_bindings_size, pool_bindings_size);

  if (bindings == nullptr)
  {
    return Err(Status::OutOfHostMemory);
  }

  self->bindings = bindings;
  self->bindings_capacity += pool_bindings_size;

  mem::zero(self->bindings + self->num_pools * pool_bindings_size,
            pool_bindings_size);

  u32 num_bindings_per_group = 0;
  for (u32 i = 0; i < self->num_sets_per_group; i++)
  {
    num_bindings_per_group += self->set_layouts[i]->num_bindings;
  }

  VkDescriptorPoolSize *pool_sizes =
      self->allocator.allocate_typed<VkDescriptorPoolSize>(
          num_bindings_per_group);

  if (pool_sizes == nullptr)
  {
    return Err(Status::OutOfHostMemory);
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
          .type = (VkDescriptorType) desc.type, .descriptorCount = desc.count};
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
    return Err((Status) result);
  }

  VkDescriptorSetLayout *set_layouts =
      self->allocator.allocate_typed<VkDescriptorSetLayout>(
          self->num_sets_per_group * self->num_groups_per_pool);

  if (set_layouts == nullptr)
  {
    return Err(Status::OutOfHostMemory);
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
  CHECK("Descriptor set allocation logic error",
        result != VK_ERROR_OUT_OF_POOL_MEMORY &&
            result != VK_ERROR_FRAGMENTED_POOL);

  if (result != VK_SUCCESS)
  {
    self->device->vk_table.DestroyDescriptorPool(self->device->vk_device,
                                                 vk_pool, nullptr);
    return Err((Status) result);
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
  self->num_free_groups += (self->num_groups_per_pool - 1);

  return Ok((u32) assigned_group);
}

void DescriptorHeapInterface::sampler(gfx::DescriptorHeap self_, u32 group,
                                      u32 set, u32 binding,
                                      Span<gfx::SamplerBinding const> elements)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::Sampler);
  VALIDATE("",
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::CombinedImageSampler);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::CombinedImageSamplerBinding const &element : elements)
  {
    VALIDATE("", has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::SampledImage);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::SampledImageBinding const &element : elements)
  {
    VALIDATE("", has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::StorageImage);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::StorageImageBinding const &element : elements)
  {
    VALIDATE("", has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::UniformTexelBuffer);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::UniformTexelBufferBinding const &element : elements)
  {
    VALIDATE("", has_bits(BUFFER_FROM_VIEW(element.buffer_view)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::StorageTexelBuffer);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::StorageTexelBufferBinding const &element : elements)
  {
    VALIDATE("", has_bits(BUFFER_FROM_VIEW(element.buffer_view)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::UniformBuffer);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::UniformBufferBinding const &element : elements)
  {
    VALIDATE("", has_bits(((Buffer *) element.buffer)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::StorageBuffer);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::StorageBufferBinding const &element : elements)
  {
    VALIDATE("", has_bits(((Buffer *) element.buffer)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::DynamicUniformBuffer);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::DynamicUniformBufferBinding const &element : elements)
  {
    VALIDATE("", has_bits(((Buffer *) element.buffer)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::StorageBuffer);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::DynamicStorageBufferBinding const &element : elements)
  {
    VALIDATE("", has_bits(((Buffer *) element.buffer)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", set < self->num_sets_per_group);
  VALIDATE("", binding < self->set_layouts[set]->num_bindings);
  VALIDATE("", self->set_layouts[set]->bindings[binding].type ==
                   gfx::DescriptorType::InputAttachment);
  VALIDATE("",
           self->set_layouts[set]->bindings[binding].count == elements.size);
  for (gfx::InputAttachmentBinding const &element : elements)
  {
    VALIDATE("", has_bits(IMAGE_FROM_VIEW(element.image_view)->desc.usage,
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

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("", self->last_use_frame[group] <= current_frame);

  self->last_use_frame[group] = current_frame;
}

bool DescriptorHeapInterface::is_in_use(gfx::DescriptorHeap self_, u32 group,
                                        gfx::FrameId trailing_frame)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));

  return self->last_use_frame[group] >= trailing_frame;
}

void DescriptorHeapInterface::release(gfx::DescriptorHeap self_, u32 group)
{
  DescriptorHeap *const self = (DescriptorHeap *) self_;

  VALIDATE("", group < (self->num_pools * self->num_groups_per_pool));
  VALIDATE("multiple descriptor group release detected",
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
    return Err((Status) self->status);
  }

  VkResult result =
      self->device->vk_table.EndCommandBuffer(self->vk_command_buffer);
  if (result != VK_SUCCESS)
  {
    return Err((Status) result);
  }

  return Ok(Void{});
}

void CommandEncoderInterface::reset(gfx::CommandEncoder self_)
{
  // device->vk_table.ResetCommandPool(device->vk_device, vk_command_pool, 0);
  // status = Status::Success;
  // reset status
  // preserve buffers
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

  VALIDATE("", offset < dst->desc.size);
  VALIDATE("", (offset + size) <= dst->desc.size);
  VALIDATE("", (offset % 4) == 0);
  VALIDATE("", (size % 4) == 0);
  VALIDATE("", size > 0);

  if (self->status != Status::Success)
  {
    return;
  }

  access_buffer(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
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

  VALIDATE("", num_copies > 0);
  for (gfx::BufferCopy const &copy : copies)
  {
    VALIDATE("", copy.src_offset < src->desc.size);
    VALIDATE("", (copy.src_offset + copy.size) <= src->desc.size);
    VALIDATE("", copy.dst_offset < dst->desc.size);
    VALIDATE("", (copy.dst_offset + copy.size) <= dst->desc.size);
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

  access_buffer(self, src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT);
  access_buffer(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT);

  self->device->vk_table.CmdCopyBuffer(self->vk_command_buffer, src->vk_buffer,
                                       dst->vk_buffer, num_copies, vk_copies);

  self->allocator.deallocate_typed(vk_copies, num_copies);
}

void CommandEncoderInterface::update_buffer(gfx::CommandEncoder self_,
                                            Span<u8 const> src, u64 dst_offset,
                                            gfx::Buffer dst_)
{
  CommandEncoder *const self = (CommandEncoder *) self_;
  Buffer *const         dst  = (Buffer *) dst_;

  VALIDATE("", dst_offset < dst->desc.size);
  VALIDATE("", (dst_offset + src.size_bytes()) <= dst->desc.size);
  VALIDATE("", (dst_offset % 4) == 0);
  VALIDATE("", (src.size_bytes() % 4) == 0);
  VALIDATE("", src.size_bytes() > 0);

  if (self->status != Status::Success)
  {
    return;
  }

  access_buffer(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
  static_assert(alignof(gfx::Color) == alignof(VkClearColorValue));
  VALIDATE("", num_ranges > 0);
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    VALIDATE("", has_bits(dst->desc.aspects, range.aspects));
    VALIDATE("", range.first_mip_level < dst->desc.mip_levels);
    VALIDATE("", range.first_array_layer < dst->desc.array_layers);
    VALIDATE("", (range.first_mip_level + range.num_mip_levels) <=
                     dst->desc.mip_levels);
    VALIDATE("", (range.first_array_layer + range.num_array_layers) <=
                     dst->desc.array_layers);
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

  access_image(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_WRITE_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  self->device->vk_table.CmdClearColorImage(
      self->vk_command_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (VkClearColorValue *) &clear_color,
      num_ranges, vk_ranges);

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
  static_assert(alignof(gfx::DepthStencil) ==
                alignof(VkClearDepthStencilValue));
  VALIDATE("", num_ranges > 0);
  for (u32 i = 0; i < num_ranges; i++)
  {
    gfx::ImageSubresourceRange const &range = ranges[i];
    VALIDATE("", has_bits(dst->desc.aspects, range.aspects));
    VALIDATE("", range.first_mip_level < dst->desc.mip_levels);
    VALIDATE("", range.first_array_layer < dst->desc.array_layers);
    VALIDATE("", (range.first_mip_level + range.num_mip_levels) <=
                     dst->desc.mip_levels);
    VALIDATE("", (range.first_array_layer + range.num_array_layers) <=
                     dst->desc.array_layers);
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

  access_image(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_WRITE_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  self->device->vk_table.CmdClearDepthStencilImage(
      self->vk_command_buffer, dst->vk_image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      (VkClearDepthStencilValue *) &clear_depth_stencil, num_ranges, vk_ranges);

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

  VALIDATE("", num_copies > 0);
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::ImageCopy const &copy = copies[i];
    VALIDATE("", URect3D{{}, src->desc.extent}.contains(
                     URect3D{copy.src_offset, copy.extent}));
    VALIDATE("", URect3D{{}, dst->desc.extent}.contains(
                     URect3D{copy.dst_offset, copy.extent}));

    VALIDATE("", has_bits(src->desc.aspects, copy.src_layers.aspects));
    VALIDATE("", copy.src_layers.mip_level < src->desc.mip_levels);
    VALIDATE("", copy.src_layers.first_array_layer < src->desc.array_layers);
    VALIDATE("", (copy.src_layers.first_array_layer +
                  copy.src_layers.num_array_layers) <= src->desc.array_layers);

    VALIDATE("", has_bits(dst->desc.aspects, copy.dst_layers.aspects));
    VALIDATE("", copy.dst_layers.mip_level < dst->desc.mip_levels);
    VALIDATE("", copy.dst_layers.first_array_layer < dst->desc.array_layers);
    VALIDATE("", (copy.dst_layers.first_array_layer +
                  copy.dst_layers.num_array_layers) <= dst->desc.array_layers);
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
    VkExtent3D extent{copy.extent.width, copy.extent.height, copy.extent.depth};

    vk_copies[i] = VkImageCopy{.srcSubresource = src_subresource,
                               .srcOffset      = src_offset,
                               .dstSubresource = dst_subresource,
                               .dstOffset      = dst_offset,
                               .extent         = extent};
  }

  access_image(self, src, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_READ_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
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

  VALIDATE("", num_copies > 0);
  for (u32 i = 0; i < num_copies; i++)
  {
    gfx::BufferImageCopy const &copy = copies[i];
    VALIDATE("", copy.buffer_offset < src->desc.size);
    VALIDATE("", URect3D{{}, dst->desc.extent}.contains(
                     URect3D{copy.image_offset, copy.image_extent}));

    VALIDATE("", has_bits(dst->desc.aspects, copy.image_layers.aspects));
    VALIDATE("", copy.image_layers.mip_level < dst->desc.mip_levels);
    VALIDATE("", copy.image_layers.first_array_layer < dst->desc.array_layers);
    VALIDATE("",
             (copy.image_layers.first_array_layer +
              copy.image_layers.num_array_layers) <= dst->desc.array_layers);
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
    vk_copies[i] =
        VkBufferImageCopy{.bufferOffset      = copy.buffer_offset,
                          .bufferRowLength   = copy.buffer_row_length,
                          .bufferImageHeight = copy.buffer_image_height,
                          .imageSubresource  = image_subresource,
                          .imageOffset = VkOffset3D{(i32) copy.image_offset.x,
                                                    (i32) copy.image_offset.y,
                                                    (i32) copy.image_offset.z},
                          .imageExtent = VkExtent3D{copy.image_extent.width,
                                                    copy.image_extent.height,
                                                    copy.image_extent.depth}};
  }

  access_buffer(self, src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_READ_BIT);
  access_image(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
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

  VALIDATE("", num_blits > 0);
  for (u32 i = 0; i < num_blits; i++)
  {
    gfx::ImageBlit const &blit = blits[i];
    VALIDATE("", blit.src_offsets[0].x <= src->desc.extent.width &&
                     blit.src_offsets[0].y <= src->desc.extent.height &&
                     blit.src_offsets[0].z <= src->desc.extent.depth &&
                     blit.src_offsets[1].x <= src->desc.extent.width &&
                     blit.src_offsets[1].y <= src->desc.extent.height &&
                     blit.src_offsets[1].z <= src->desc.extent.depth);
    VALIDATE("", blit.dst_offsets[0].x <= dst->desc.extent.width &&
                     blit.dst_offsets[0].y <= dst->desc.extent.height &&
                     blit.dst_offsets[0].z <= dst->desc.extent.depth &&
                     blit.dst_offsets[1].x <= dst->desc.extent.width &&
                     blit.dst_offsets[1].y <= dst->desc.extent.height &&
                     blit.dst_offsets[1].z <= dst->desc.extent.depth);

    VALIDATE("", has_bits(src->desc.aspects, blit.src_layers.aspects));
    VALIDATE("", blit.src_layers.mip_level < src->desc.mip_levels);
    VALIDATE("", blit.src_layers.first_array_layer < src->desc.array_layers);
    VALIDATE("", (blit.src_layers.first_array_layer +
                  blit.src_layers.num_array_layers) <= src->desc.array_layers);

    VALIDATE("", has_bits(dst->desc.aspects, blit.dst_layers.aspects));
    VALIDATE("", blit.dst_layers.mip_level < dst->desc.mip_levels);
    VALIDATE("", blit.dst_layers.first_array_layer < dst->desc.array_layers);
    VALIDATE("", (blit.dst_layers.first_array_layer +
                  blit.dst_layers.num_array_layers) <= dst->desc.array_layers);
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

  access_image(self, src, VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_ACCESS_TRANSFER_READ_BIT,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_image(self, dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
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
    gfx::RenderPass render_pass_, URect render_area,
    Span<gfx::Color const>   color_attachments_clear_values,
    gfx::DepthStencil const &depth_stencil_attachment_clear_value)
{
  CommandEncoder *const self        = (CommandEncoder *) self_;
  Framebuffer *const    framebuffer = (Framebuffer *) framebuffer_;
  RenderPass *const     render_pass = (RenderPass *) render_pass_;
  u32 const  num_color_clear_values = (u32) color_attachments_clear_values.size;
  bool const has_depth_stencil_attachment =
      framebuffer->desc.depth_stencil_attachment != nullptr;
  u32 const num_vk_clear_values =
      num_color_clear_values + (has_depth_stencil_attachment ? 1U : 0U);

  VALIDATE("", is_renderpass_compatible(render_pass, framebuffer->desc));
  VALIDATE("", color_attachments_clear_values.size ==
                   framebuffer->desc.color_attachments.size);
  VALIDATE("", URect{{}, framebuffer->desc.extent}.contains(render_area));
  static_assert(sizeof(gfx::Color) == sizeof(VkClearColorValue));

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

  for (u32 i = 0; i < framebuffer->desc.color_attachments.size; i++)
  {
    access_image(
        self, IMAGE_FROM_VIEW(framebuffer->desc.color_attachments[i]),
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        color_attachment_image_access(render_pass->desc.color_attachments[i]),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  }

  if (has_depth_stencil_attachment)
  {
    VkAccessFlags access = depth_stencil_attachment_image_access(
        render_pass->desc.depth_stencil_attachment);
    access_image(
        self,
        IMAGE_FROM_VIEW(self->bound_framebuffer->desc.depth_stencil_attachment),
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        access,
        has_write_access(access) ?
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
  }

  VkRect2D vk_render_area{.offset = VkOffset2D{.x = (i32) render_area.offset.x,
                                               .y = (i32) render_area.offset.y},
                          .extent =
                              VkExtent2D{.width  = render_area.extent.width,
                                         .height = render_area.extent.height}};
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

  VALIDATE("", self->bound_render_pass != nullptr);

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

  VALIDATE("", num_sets <= gfx::MAX_PIPELINE_DESCRIPTOR_SETS);
  VALIDATE("", descriptor_heaps.size == groups.size);
  VALIDATE("", groups.size == sets.size);
  VALIDATE("", num_dynamic_offsets <= num_sets);
  for (u32 iset = 0; iset < num_sets; iset++)
  {
    DescriptorHeap *descriptor_heap = (DescriptorHeap *) descriptor_heaps[iset];
    VALIDATE("", groups[iset] < descriptor_heap->num_pools *
                                    descriptor_heap->num_groups_per_pool);
    VALIDATE("", sets[iset] < descriptor_heap->num_sets_per_group);
  }

  VkDescriptorSet vk_sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS];

  for (u32 iset = 0; iset < num_sets; iset++)
  {
    DescriptorHeap *descriptor_heap = (DescriptorHeap *) descriptor_heaps[iset];
    vk_sets[iset] =
        descriptor_heap->vk_descriptor_sets
            [descriptor_heap->num_sets_per_group * groups[iset] + sets[iset]];
    self->bound_descriptor_set_heaps[iset]  = descriptor_heap;
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

  VALIDATE("", !(self->bound_compute_pipeline == nullptr &&
                 self->bound_graphics_pipeline == nullptr));
  VALIDATE("", push_constants_data.size_bytes() <= gfx::MAX_PUSH_CONSTANT_SIZE);

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

  VALIDATE("", self->bound_compute_pipeline != nullptr);
  VALIDATE("", group_count_x <= gfx::MAX_COMPUTE_GROUP_COUNT_X);
  VALIDATE("", group_count_y <= gfx::MAX_COMPUTE_GROUP_COUNT_Y);
  VALIDATE("", group_count_z <= gfx::MAX_COMPUTE_GROUP_COUNT_Z);

  self->device->vk_table.CmdDispatch(self->vk_command_buffer, group_count_x,
                                     group_count_y, group_count_z);
}

void CommandEncoderInterface::dispatch_indirect(gfx::CommandEncoder self_,
                                                gfx::Buffer buffer_, u64 offset)
{
  CommandEncoder *const self   = (CommandEncoder *) self_;
  Buffer *const         buffer = (Buffer *) buffer_;

  VALIDATE("", self->bound_compute_pipeline != nullptr);
  VALIDATE("", has_bits(buffer->desc.usage, gfx::BufferUsage::IndirectBuffer));
  VALIDATE("", offset < buffer->desc.size);

  self->device->vk_table.CmdDispatchIndirect(self->vk_command_buffer,
                                             buffer->vk_buffer, offset);
}

void CommandEncoderInterface::set_viewport(gfx::CommandEncoder  self_,
                                           gfx::Viewport const &viewport)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  // TODO(lamarrr): generate access for dispatches
  // and bindings and framebuffers
  // index buffer, vertex buffer

  VkViewport vk_viewport{.x        = viewport.area.offset.x,
                         .y        = viewport.area.offset.y,
                         .width    = viewport.area.extent.x,
                         .height   = viewport.area.extent.y,
                         .minDepth = viewport.min_depth,
                         .maxDepth = viewport.max_depth};
  self->device->vk_table.CmdSetViewport(self->vk_command_buffer, 0, 1,
                                        &vk_viewport);
}

void CommandEncoderInterface::set_scissor(gfx::CommandEncoder self_,
                                          URect               scissor)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  VkRect2D vk_scissor{
      .offset = VkOffset2D{(i32) scissor.offset.x, (i32) scissor.offset.y},
      .extent = VkExtent2D{scissor.extent.width, scissor.extent.height}};
  self->device->vk_table.CmdSetScissor(self->vk_command_buffer, 0, 1,
                                       &vk_scissor);
}

void CommandEncoderInterface::set_blend_constants(gfx::CommandEncoder self_,
                                                  Vec4 blend_constants)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  f32 vk_constants[4] = {blend_constants.x, blend_constants.y,
                         blend_constants.z, blend_constants.w};
  self->device->vk_table.CmdSetBlendConstants(self->vk_command_buffer,
                                              vk_constants);
}

void CommandEncoderInterface::set_stencil_compare_mask(
    gfx::CommandEncoder self_, gfx::StencilFaces faces, u32 mask)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);

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

  VALIDATE("", self->bound_graphics_pipeline != nullptr);

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

  VALIDATE("", self->bound_graphics_pipeline != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdSetStencilWriteMask(
      self->vk_command_buffer, (VkStencilFaceFlags) faces, mask);
}

void CommandEncoderInterface::set_vertex_buffers(
    gfx::CommandEncoder self_, Span<gfx::Buffer const> vertex_buffers,
    Span<u64 const> offsets)
{
  CommandEncoder *const self        = (CommandEncoder *) self_;
  u32 const             num_buffers = (u32) vertex_buffers.size;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);
  VALIDATE("", num_buffers > 0);
  VALIDATE("", offsets.size == vertex_buffers.size);

  if (self->status != Status::Success)
  {
    return;
  }

  VkBuffer *vk_buffers = self->allocator.allocate_typed<VkBuffer>(num_buffers);

  if (vk_buffers == nullptr)
  {
    self->status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_buffers; i++)
  {
    vk_buffers[i] = ((Buffer *) vertex_buffers[i])->vk_buffer;
  }

  self->device->vk_table.CmdBindVertexBuffers(
      self->vk_command_buffer, 0, num_buffers, vk_buffers, offsets.data);

  self->allocator.deallocate_typed(vk_buffers, num_buffers);
}

void CommandEncoderInterface::set_index_buffer(gfx::CommandEncoder self_,
                                               gfx::Buffer         index_buffer,
                                               u64                 offset)
{
  CommandEncoder *const self              = (CommandEncoder *) self_;
  Buffer *const         index_buffer_impl = (Buffer *) index_buffer;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);
  VALIDATE("", offset < index_buffer_impl->desc.size);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdBindIndexBuffer(self->vk_command_buffer,
                                            index_buffer_impl->vk_buffer,
                                            offset, VK_INDEX_TYPE_UINT32);
}

void CommandEncoderInterface::draw(gfx::CommandEncoder self_, u32 first_index,
                                   u32 num_indices, i32 vertex_offset,
                                   u32 first_instance, u32 num_instances)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);
  VALIDATE("", self->bound_render_pass != nullptr);
  VALIDATE("", self->bound_framebuffer != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdDrawIndexed(self->vk_command_buffer, num_indices,
                                        num_instances, first_index,
                                        vertex_offset, first_instance);
}

void CommandEncoderInterface::draw_indirect(gfx::CommandEncoder self_,
                                            gfx::Buffer buffer, u64 offset,
                                            u32 draw_count, u32 stride)
{
  CommandEncoder *const self = (CommandEncoder *) self_;

  VALIDATE("", self->bound_graphics_pipeline != nullptr);
  VALIDATE("", self->bound_render_pass != nullptr);
  VALIDATE("", self->bound_framebuffer != nullptr);

  if (self->status != Status::Success)
  {
    return;
  }

  self->device->vk_table.CmdDrawIndexedIndirect(self->vk_command_buffer,
                                                ((Buffer *) buffer)->vk_buffer,
                                                offset, draw_count, stride);
}

}        // namespace vk
}        // namespace ash
