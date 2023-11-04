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
  Buffer                  buffer;
  VmaAllocationCreateInfo alloc_create_info{.flags = 0,
                                            .usage = VMA_MEMORY_USAGE_UNKNOWN,
                                            .requiredFlags =
                                                (VkMemoryPropertyFlags) desc.properties,
                                            .preferredFlags = 0,
                                            .memoryTypeBits = 0,
                                            .pool           = nullptr,
                                            .pUserData      = nullptr,
                                            .priority       = 0};
  VmaAllocationInfo       alloc_info;
  VK_ERR(vmaCreateBuffer(vk_allocator, &create_info, &alloc_create_info, &buffer.vk_buffer, nullptr,
                         &alloc_info));

  if (has_bits(desc.properties, gfx::MemoryProperties::HostVisible))
  {
    VK_ERR(vk_table.MapMemory(vk_device, alloc_info.deviceMemory, 0, VK_WHOLE_SIZE, 0,
                              &buffer.host_map));
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
        .object      = (u64) buffer.vk_buffer,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  void *handle = vk_resources.buffers.push(buffer);

  return stx::Ok(
      resources.buffers.push(gfx::BufferResource{.refcount = 1, .handle = handle, .desc = desc}));
}

stx::Result<gfx::BufferView, gfx::Status>
    DeviceImpl::create_buffer_view(gfx::BufferViewDesc const &desc)
{
  VkBufferViewCreateInfo create_info{.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                                     .pNext  = nullptr,
                                     .flags  = 0,
                                     .buffer = (VkBuffer) resources.buffers[desc.buffer].handle,
                                     .format = (VkFormat) desc.format,
                                     .offset = desc.offset,
                                     .range  = desc.size};

  VkBufferView view;

  VK_ERR(vk_table.CreateBufferView(vk_device, &create_info, nullptr, &view));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,
        .object      = (u64) view,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  return stx::Ok(resources.buffer_views.push(
      gfx::BufferViewResource{.refcount = 1, .handle = view, .desc = desc}));
}

stx::Result<Image, gfx::Status> create_vk_image(gfx::ImageDesc const &desc, DeviceImpl &device)
{
  VkImageCreateInfo create_info{.sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                .pNext       = nullptr,
                                .flags       = 0,
                                .imageType   = (VkImageType) desc.type,
                                .format      = (VkFormat) desc.format,
                                .extent      = VkExtent3D{.width  = desc.extent.width,
                                                          .height = desc.extent.height,
                                                          .depth  = desc.extent.depth},
                                .mipLevels   = desc.mip_levels,
                                .arrayLayers = desc.array_layers,
                                .samples     = VK_SAMPLE_COUNT_1_BIT,
                                .tiling      = VK_IMAGE_TILING_OPTIMAL,
                                .usage = to_vkUsage(desc.usage | gfx::ImageUsage::TransferDst),
                                .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
                                .queueFamilyIndexCount = 0,
                                .pQueueFamilyIndices   = nullptr,
                                .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};

  Image image;
  VK_ERR(vmaCreateImage(device.vk_allocator, &create_info, nullptr, &image.vk_image,
                        &image.vk_allocation, nullptr));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
        .object      = (u64) image.vk_image,
        .pObjectName = desc.label};
    device.vk_table.DebugMarkerSetObjectNameEXT(device.vk_device, &debug_info);
  }

  return stx::Ok((Image) image);
}

stx::Result<gfx::ImageView, gfx::Status>
    DeviceImpl::create_image_view(gfx::ImageViewDesc const &desc)
{
  VkImageViewCreateInfo create_info{
      .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = 0,
      .image            = vk_resources.images[resources.images[desc.image].handle].vk_image,
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

  VkImageView view;
  VK_ERR(vk_table.CreateImageView(vk_device, &create_info, nullptr, &view));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
        .object      = (u64) view,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  return stx::Ok(resources.image_views.push(
      gfx::ImageViewResource{.refcount = 1, .handle = view, .desc = desc}));
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

  VkSampler sampler;
  VK_ERR(vk_table.CreateSampler(vk_device, &create_info, nullptr, &sampler));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,
        .object      = (u64) sampler,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  return stx::Ok(resources.samplers.push(gfx::SamplerResource{.refcount = 1, .handle = sampler}));
}

stx::Result<gfx::Shader, gfx::Status> DeviceImpl::create_shader(gfx::ShaderDesc const &desc)
{
  VkShaderModuleCreateInfo create_info{.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                       .pNext    = nullptr,
                                       .flags    = 0,
                                       .codeSize = desc.spirv_code.size_bytes(),
                                       .pCode    = desc.spirv_code.data()};

  VkShaderModule shader;
  VK_ERR(vk_table.CreateShaderModule(vk_device, &create_info, nullptr, &shader));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,
        .object      = (u64) shader,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  return stx::Ok(resources.shaders.push(
      gfx::ShaderResource{.refcount = 1, .handle = shader, .label = desc.label}));
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
  VkRenderPass           renderpass;
  VK_ERR(vk_table.CreateRenderPass(vk_device, &create_info, nullptr, &renderpass));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,
        .object      = (u64) renderpass,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  return stx::Ok(resources.render_passes.push(
      gfx::RenderPassResource{.refcount = 1, .handle = renderpass, .desc = desc}));
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

  VkFramebuffer framebuffer;

  VK_ERR(vk_table.CreateFramebuffer(vk_device, &create_info, nullptr, &framebuffer));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,
        .object      = (u64) framebuffer,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  return stx::Ok(resources.framebuffers.push(
      gfx::FramebufferResource{.refcount = 1, .handle = framebuffer, .desc = desc}));
}

stx::Result<gfx::DescriptorSetLayout, gfx::Status>
    DeviceImpl::create_descriptor_set_layout(gfx::DescriptorSetLayoutDesc const &desc)
{
  gfx::DescriptorSetCount                count;
  stx::Vec<VkDescriptorSetLayoutBinding> vk_bindings;
  vk_bindings.unsafe_resize_uninitialized(desc.bindings.size());

  for (usize i = 0; i < desc.bindings.size(); i++)
  {
    gfx::DescriptorBindingDesc const &binding = desc.bindings[i];
    vk_bindings[i] = VkDescriptorSetLayoutBinding{.binding        = binding.binding,
                                                  .descriptorType = (VkDescriptorType) binding.type,
                                                  .descriptorCount    = binding.count,
                                                  .stageFlags         = VK_SHADER_STAGE_ALL,
                                                  .pImmutableSamplers = nullptr};

    switch (binding.type)
    {
      case gfx::DescriptorType::CombinedImageSampler:
        count.num_combined_image_samplers += binding.count;
        break;
      case gfx::DescriptorType::DynamicStorageBuffer:
        count.num_storage_buffers += binding.count;
        break;
      case gfx::DescriptorType::DynamicUniformBuffer:
        count.num_uniform_buffers += binding.count;
        break;
      case gfx::DescriptorType::InputAttachment:
        count.num_input_attachments += binding.count;
        break;
      case gfx::DescriptorType::SampledImage:
        count.num_sampled_images += binding.count;
        break;
      case gfx::DescriptorType::Sampler:
        count.num_samplers += binding.count;
        break;
      case gfx::DescriptorType::StorageBuffer:
        count.num_storage_buffers += binding.count;
        break;
      case gfx::DescriptorType::StorageImage:
        count.num_storage_images += binding.count;
        break;
      case gfx::DescriptorType::StorageTexelBuffer:
        count.num_storage_texel_buffers += binding.count;
        break;
      case gfx::DescriptorType::UniformBuffer:
        count.num_uniform_buffers += binding.count;
        break;
      case gfx::DescriptorType::UniformTexelBuffer:
        count.num_uniform_texel_buffers += binding.count;
        break;
      default:
        break;
    }
  }

  VkDescriptorSetLayoutCreateInfo create_info{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext        = nullptr,
      .flags        = 0,
      .bindingCount = (u32) vk_bindings.size(),
      .pBindings    = vk_bindings.data()};

  VkDescriptorSetLayout layout;
  VK_ERR(vk_table.CreateDescriptorSetLayout(vk_device, &create_info, nullptr, &layout));

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
        .object      = (u64) layout,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  return stx::Ok(resources.descriptor_set_layouts.push(
      gfx::DescriptorSetLayoutResource{.refcount = 1, .handle = layout, .count = count}));
}

stx::Result<gfx::PipelineCache, gfx::Status>
    DeviceImpl::create_pipeline_cache(gfx::PipelineCacheDesc const &desc)
{
  VkPipelineCacheCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
                                        .pNext = nullptr,
                                        .flags = 0,
                                        .initialDataSize = desc.initial_data.size_bytes(),
                                        .pInitialData    = desc.initial_data.data()};

  VkPipelineCache cache;
  VK_ERR(vk_table.CreatePipelineCache(vk_device, &create_info, nullptr, &cache));

  return stx::Ok(
      resources.pipeline_caches.push(gfx::PipelineCacheResource{.refcount = 1, .handle = cache}));
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
      .module = (VkShaderModule) resources.shaders[desc.compute_shader.shader].handle,
      .pName =
          desc.compute_shader.entry_point == nullptr ? "main" : desc.compute_shader.entry_point,
      .pSpecializationInfo = &vk_specialization};

  VkDescriptorSetLayout descriptor_set_layout =
      (VkDescriptorSetLayout) resources.descriptor_set_layouts[desc.descriptor_set_layout].handle;

  VkPushConstantRange push_constant_range{
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT, .offset = 0, .size = desc.push_constant_size};

  VkPipelineLayoutCreateInfo layout_create_info{.sType =
                                                    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                .pNext                  = nullptr,
                                                .flags                  = 0,
                                                .setLayoutCount         = 1,
                                                .pSetLayouts            = &descriptor_set_layout,
                                                .pushConstantRangeCount = 1,
                                                .pPushConstantRanges    = &push_constant_range};

  VkPipelineLayout layout;
  VK_ERR(vk_table.CreatePipelineLayout(vk_device, &layout_create_info, nullptr, &layout));

  VkComputePipelineCreateInfo create_info{.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                                          .pNext  = nullptr,
                                          .flags  = 0,
                                          .stage  = vk_stage,
                                          .layout = layout,
                                          .basePipelineHandle = nullptr,
                                          .basePipelineIndex  = 0};

  VkPipeline      pipeline;
  VkPipelineCache cache = desc.cache == nullptr ?
                              nullptr :
                              (VkPipelineCache) resources.pipeline_caches[desc.cache].handle;
  VkResult        result =
      vk_table.CreateComputePipelines(vk_device, cache, 1, &create_info, nullptr, &pipeline);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyPipelineLayout(vk_device, layout, nullptr);
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
        .object      = (u64) pipeline,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  void *handle =
      vk_resources.compute_pipelines.push(Pipeline{.pipeline = pipeline, .layout = layout});

  return stx::Ok(resources.compute_pipelines.push(
      gfx::ComputePipelineResource{.refcount = 1, .handle = handle}));
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
       .module = (VkShaderModule) resources.shaders[desc.vertex_shader.shader].handle,
       .pName = desc.vertex_shader.entry_point == nullptr ? "main" : desc.vertex_shader.entry_point,
       .pSpecializationInfo = &vk_vs_specialization},
      {.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext  = nullptr,
       .flags  = 0,
       .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = (VkShaderModule) resources.shaders[desc.fragment_shader.shader].handle,
       .pName =
           desc.fragment_shader.entry_point == nullptr ? "main" : desc.fragment_shader.entry_point,
       .pSpecializationInfo = &vk_fs_specialization}};

  VkDescriptorSetLayout descriptor_set_layout =
      (VkDescriptorSetLayout) resources.descriptor_set_layouts[desc.descriptor_set_layout].handle;

  VkPushConstantRange push_constant_range{
      .stageFlags = VK_SHADER_STAGE_ALL, .offset = 0, .size = desc.push_constant_size};

  VkPipelineLayoutCreateInfo layout_create_info{.sType =
                                                    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                                                .pNext                  = nullptr,
                                                .flags                  = 0,
                                                .setLayoutCount         = 1,
                                                .pSetLayouts            = &descriptor_set_layout,
                                                .pushConstantRangeCount = 1,
                                                .pPushConstantRanges    = &push_constant_range};

  VkPipelineLayout layout;
  VK_ERR(vk_table.CreatePipelineLayout(vk_device, &layout_create_info, nullptr, &layout));

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
      .layout              = layout,
      .renderPass          = (VkRenderPass) resources.render_passes[desc.render_pass].handle,
      .subpass             = 0,
      .basePipelineHandle  = nullptr,
      .basePipelineIndex   = 0};

  VkPipeline      pipeline;
  VkPipelineCache cache = desc.cache == nullptr ?
                              nullptr :
                              (VkPipelineCache) resources.pipeline_caches[desc.cache].handle;
  VkResult        result =
      vk_table.CreateGraphicsPipelines(vk_device, cache, 1, &create_info, nullptr, &pipeline);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyPipelineLayout(vk_device, layout, nullptr);
    return stx::Err((gfx::Status) result);
  }

  if (desc.label != nullptr)
  {
    VkDebugMarkerObjectNameInfoEXT debug_info{
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .pNext       = nullptr,
        .objectType  = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,
        .object      = (u64) pipeline,
        .pObjectName = desc.label};
    vk_table.DebugMarkerSetObjectNameEXT(vk_device, &debug_info);
  }

  void *handle =
      vk_resources.graphics_pipelines.push(Pipeline{.pipeline = pipeline, .layout = layout});

  return stx::Ok(resources.graphics_pipelines.push(
      gfx::GraphicsPipelineResource{.refcount = 1, .handle = handle}));
}

stx::Result<gfx::CommandEncoder *, gfx::Status> DeviceImpl::create_command_encoder()
{
}





   void *DeviceImpl::get_buffer_memory_map(gfx::Buffer buffer)
{
}

   void DeviceImpl::invalidate_buffer_memory_map(gfx::Buffer                       buffer,
                                           stx::Span<gfx::MemoryRange const> ranges) {}

void DeviceImpl::flush_buffer_memory_map(gfx::Buffer                       buffer,
                                      stx::Span<gfx::MemoryRange const> ranges) {}

usize DeviceImpl::get_pipeline_cache_size(gfx::PipelineCache cache)
{
}

void DeviceImpl::get_pipeline_cache_data(gfx::PipelineCache cache, stx::Span<u8> out)
{
}

void DeviceImpl::wait_for_fences(stx::Span<gfx::Fence const> fences, bool all, u64 timeout)
{
}

void DeviceImpl::reset_fences(stx::Span<gfx::Fence const> fences)
{
}

gfx::FenceStatus DeviceImpl::get_fence_status(gfx::Fence fence)
{
}

void DeviceImpl::submit(gfx::CommandEncoder *encoder, gfx::Fence signal_fence)
{
}

void DeviceImpl::wait_idle()
{
  vk_table.DeviceWaitIdle(vk_device);
  // todo(lamarrr):check
}



/*



























*/

inline void acquire_buffer(VkBuffer buffer, gfx::BufferUsage usage, BufferScope const &scope,
                           VkCommandBuffer command_buffer, DeviceTable const &table)
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

inline void release_buffer(VkBuffer buffer, gfx::BufferUsage usage, BufferScope const &scope,
                           VkCommandBuffer command_buffer, DeviceTable const &table)
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
inline void acquire_image(VkImage image, gfx::ImageUsage usage, ImageScope const &scope,
                          VkImageAspectFlags aspects, VkCommandBuffer command_buffer,
                          DeviceTable const &table)
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
inline void release_image(VkImage image, gfx::ImageUsage usage, ImageScope const &scope,
                          VkImageAspectFlags aspects, VkCommandBuffer command_buffer,
                          DeviceTable const &table)
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

stx::Result<gfx::Image, gfx::Status>
    CommandEncoderImpl::create_image(gfx::ImageDesc const &old_desc, gfx::Color initial_color)
{
  gfx::ImageDesc desc{old_desc};
  desc.usage |= gfx::ImageUsage::TransferDst;
  TRY_OK(image, create_vk_image(desc, *device));

  gfx::Image out = device->resources.images.push(
      gfx::ImageResource{.refcount           = 1,
                         .handle             = device->vk_resources.images.push(image),
                         .externally_managed = false,
                         .desc               = desc});

  ImageScope scope = transfer_image_scope(desc.usage);

  VkImageSubresourceRange range{.aspectMask     = (VkImageAspectFlags) desc.aspects,
                                .baseMipLevel   = 0,
                                .levelCount     = VK_REMAINING_MIP_LEVELS,
                                .baseArrayLayer = 0,
                                .layerCount     = VK_REMAINING_ARRAY_LAYERS};

  VkImageMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                               .pNext               = nullptr,
                               .srcAccessMask       = VK_ACCESS_NONE,
                               .dstAccessMask       = scope.access,
                               .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
                               .newLayout           = scope.layout,
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .image               = image.vk_image,
                               .subresourceRange    = range};

  device->vk_table.CmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      scope.access, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0,
                                      nullptr, 1, &barrier);

  device->vk_table.CmdClearColorImage(vk_command_buffer, image.vk_image, scope.layout,
                                      (VkClearColorValue *) &initial_color, 1, &range);

  release_image(image.vk_image, desc.usage, scope, (VkImageAspectFlags) desc.aspects,
                vk_command_buffer, device->vk_table);

  return stx::Ok((gfx::Image) out);
}

stx::Result<gfx::Image, gfx::Status>
    CommandEncoderImpl::create_image(gfx::ImageDesc const &old_desc,
                                     gfx::DepthStencil     initial_depth_stencil)
{
  gfx::ImageDesc desc{old_desc};
  desc.usage |= gfx::ImageUsage::TransferDst;
  TRY_OK(image, create_vk_image(desc, *device));

  gfx::Image out = device->resources.images.push(
      gfx::ImageResource{.refcount           = 1,
                         .handle             = device->vk_resources.images.push(image),
                         .externally_managed = false,
                         .desc               = desc});

  ImageScope scope = transfer_image_scope(desc.usage);

  VkImageSubresourceRange range{.aspectMask     = (VkImageAspectFlags) desc.aspects,
                                .baseMipLevel   = 0,
                                .levelCount     = VK_REMAINING_MIP_LEVELS,
                                .baseArrayLayer = 0,
                                .layerCount     = VK_REMAINING_ARRAY_LAYERS};

  VkImageMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                               .pNext               = nullptr,
                               .srcAccessMask       = VK_ACCESS_NONE,
                               .dstAccessMask       = scope.access,
                               .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
                               .newLayout           = scope.layout,
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .image               = image.vk_image,
                               .subresourceRange    = range};

  device->vk_table.CmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      scope.access, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0,
                                      nullptr, 1, &barrier);

  device->vk_table.CmdClearDepthStencilImage(vk_command_buffer, image.vk_image, scope.layout,
                                             (VkClearDepthStencilValue *) &initial_depth_stencil, 1,
                                             &range);

  release_image(image.vk_image, desc.usage, scope, (VkImageAspectFlags) desc.aspects,
                vk_command_buffer, device->vk_table);

  return stx::Ok((gfx::Image) out);
}

stx::Result<gfx::Image, gfx::Status>
    CommandEncoderImpl::create_image(gfx::ImageDesc const &old_desc, gfx::Buffer initial_data,
                                     stx::Span<gfx::BufferImageCopy const> copies)
{
  gfx::ImageDesc desc{old_desc};
  desc.usage |= gfx::ImageUsage::TransferDst;
  TRY_OK(image, create_vk_image(desc, *device));

  gfx::Image out = device->resources.images.push(
      gfx::ImageResource{.refcount           = 1,
                         .handle             = device->vk_resources.images.push(image),
                         .externally_managed = false,
                         .desc               = desc});

  ImageScope scope = transfer_image_scope(desc.usage);

  VkImageSubresourceRange range{.aspectMask     = (VkImageAspectFlags) desc.aspects,
                                .baseMipLevel   = 0,
                                .levelCount     = VK_REMAINING_MIP_LEVELS,
                                .baseArrayLayer = 0,
                                .layerCount     = VK_REMAINING_ARRAY_LAYERS};

  VkImageMemoryBarrier barrier{.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                               .pNext               = nullptr,
                               .srcAccessMask       = VK_ACCESS_NONE,
                               .dstAccessMask       = scope.access,
                               .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
                               .newLayout           = scope.layout,
                               .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                               .image               = image.vk_image,
                               .subresourceRange    = range};

  device->vk_table.CmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      scope.access, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0,
                                      nullptr, 1, &barrier);

  device->vk_table.CmdClearColorImage(vk_command_buffer, image.vk_image, scope.layout,
                                      (VkClearColorValue *) &initial_color, 1, &range);

  release_image(image.vk_image, desc.usage, scope, (VkImageAspectFlags) desc.aspects,
                vk_command_buffer, device->vk_table);

  return stx::Ok((gfx::Image) out);
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
  gfx::BufferResource const &resource  = device->resources.buffers[dst];
  VkBuffer                   vk_buffer = device->vk_resources.buffers[resource.handle].vk_buffer;
  BufferScope                scope     = transfer_buffer_scope(resource.desc.usage);
  acquire_buffer(vk_buffer, resource.desc.usage, scope, vk_command_buffer, device->vk_table);
  device->vk_table.CmdFillBuffer(vk_command_buffer, vk_buffer, offset, size, data);
  release_buffer(vk_buffer, resource.desc.usage, scope, vk_command_buffer, device->vk_table);
}

void CommandEncoderImpl::copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                                     stx::Span<gfx::BufferCopy const> copies)
{
  gfx::BufferResource const &src_rc    = device->resources.buffers[src];
  VkBuffer                   vk_src    = device->vk_resources.buffers[src_rc.handle].vk_buffer;
  gfx::BufferResource const &dst_rc    = device->resources.buffers[dst];
  VkBuffer                   vk_dst    = device->vk_resources.buffers[dst_rc.handle].vk_buffer;
  BufferScope                dst_scope = transfer_buffer_scope(dst_rc.desc.usage);

  acquire_buffer(vk_dst, dst_rc.desc.usage, dst_scope, vk_command_buffer, device->vk_table);

  // todo(lamarrr);
  device->vk_table.CmdCopyBuffer(vk_command_buffer, vk_src, vk_dst, 0, nullptr);

  release_buffer(vk_dst, dst_rc.desc.usage, dst_scope, vk_command_buffer, device->vk_table);
}

void CommandEncoderImpl::update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst)
{
  gfx::BufferResource const &dst_rc    = device->resources.buffers[dst];
  VkBuffer                   vk_dst    = device->vk_resources.buffers[dst_rc.handle].vk_buffer;
  BufferScope                dst_scope = transfer_buffer_scope(dst_rc.desc.usage);

  acquire_buffer(vk_dst, dst_rc.desc.usage, dst_scope, vk_command_buffer, device->vk_table);

  device->vk_table.CmdUpdateBuffer(vk_command_buffer, vk_dst, dst_offset, (u64) src.size(),
                                   src.data());

  release_buffer(vk_dst, dst_rc.desc.usage, dst_scope, vk_command_buffer, device->vk_table);
}

void CommandEncoderImpl::clear_color_image(gfx::Image dst, stx::Span<gfx::Color const> clear_colors,
                                           stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  gfx::ImageResource const &resource = device->resources.images[dst];
  VkImage                   vk_image = device->vk_resources.images[resource.handle].vk_image;
  ImageScope                scope    = transfer_image_scope(resource.desc.usage);
  VkImageSubresourceRange   vk_range[16];

  for (u32 i = 0; i < ranges.size(); i += 16)
  {
    acquire_image(vk_image, resource.desc.usage, scope, (VkImageAspectFlags) resource.desc.aspects,
                  vk_command_buffer, device->vk_table);

    device->vk_table.CmdClearColorImage(vk_command_buffer, vk_image, scope.layout,
                                        (VkClearColorValue *) clear_colors.data(),
                                        (u32) clear_colors.size(), );
  }
}

void CommandBuffer::clear_depth_stencil_image(
    gfx::Image dst, stx::Span<gfx::DepthStencil const> clear_depth_stencils,
    stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  ImageScope scope = transfer_image_scope(graph->images[dst].desc.scope);

  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_clear_depth_stencil_image(rhi, graph->to_rhi(dst), clear_depth_stencils, ranges);

  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::copy_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageCopy const> copies)
{
  ImageScope src_scope = transfer_image_scope(graph->images[src].desc.scope);
  ImageScope dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  acquire_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_copy_image(rhi, src, dst, copies);

  release_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                         stx::Span<gfx::BufferImageCopy const> copies)
{
  BufferScope src_scope = transfer_buffer_scope(graph->buffers[src].desc.scope);
  ImageScope  dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  acquire_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, tmp_image_barriers);

  driver->cmd_copy_buffer_to_image(rhi, src, dst, copies);

  release_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, tmp_image_barriers);
}

void CommandBuffer::blit_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter)
{
  ImageScope src_scope = transfer_image_scope(graph->images[src].desc.scope);
  ImageScope dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  acquire_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_blit_image(rhi, src, dst, blits, filter);

  release_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::begin_render_pass(
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

void CommandBuffer::end_render_pass()
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
