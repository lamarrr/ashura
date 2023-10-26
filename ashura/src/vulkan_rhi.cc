#include "ashura/vulkan_rhi.h"
#include "stx/span.h"
#include "vulkan/vulkan.h"

namespace ash
{

namespace rhi
{

template <typename T>
T *allocate()
{
  return (T *) malloc(sizeof(T));
}

bool load_device_table(VkDevice device, VulkanDeviceTable &table)
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

VulkanDriver::~VulkanDriver()
{
  table->DestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}

struct VulkanBuffer
{
  VkBuffer       vk_buffer;
  VkDeviceMemory vk_memory;
  void          *host_map;
};

struct VulkanImage
{
  VkImage        vk_image;
  VkDeviceMemory vk_memory;
};

gfx::Buffer VulkanDriver::create(gfx::BufferDesc const &desc)
{
  VkBufferCreateInfo create_info{.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext                 = nullptr,
                                 .flags                 = 0,
                                 .size                  = desc.size,
                                 .usage                 = (VkBufferUsageFlags) desc.usage,
                                 .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 1,
                                 .pQueueFamilyIndices   = nullptr};

  VulkanBuffer *buffer = allocate<VulkanBuffer>();
  buffer->host_map     = nullptr;

  table->CreateBuffer(device, &create_info, nullptr, &buffer->vk_buffer);

  // TODO(lamarrr): allocate memory
  return (gfx::Buffer) buffer;
}

gfx::BufferView VulkanDriver::create(gfx::BufferViewDesc const &desc)
{
  return nullptr;
}

gfx::Image VulkanDriver::create(gfx::ImageDesc const &desc)
{
  VkImageCreateInfo create_info{.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                .pNext                 = nullptr,
                                .flags                 = 0,
                                .imageType             = (VkImageType) desc.type,
                                .format                = (VkFormat) desc.format,
                                .extent                = VkExtent3D{.width  = desc.extent.width,
                                                                    .height = desc.extent.height,
                                                                    .depth  = desc.extent.height},
                                .mipLevels             = desc.mips,
                                .arrayLayers           = desc.array_layers,
                                .samples               = VK_SAMPLE_COUNT_1_BIT,
                                .tiling                = VK_IMAGE_TILING_OPTIMAL,
                                .usage                 = (VkImageUsageFlags) desc.usage,
                                .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
                                .queueFamilyIndexCount = 1,
                                .pQueueFamilyIndices   = nullptr,        //TODO??
                                .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};

  VulkanImage *image = allocate<VulkanImage>();

  table->CreateImage(device, &create_info, nullptr, &image->vk_image);

  return (gfx::Image) image;
}

gfx::ImageView VulkanDriver::create(gfx::ImageViewDesc const &desc)
{
  VkImageViewCreateInfo create_info{
      .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = 0,
      .image            = ((VulkanImage *) desc.image)->vk_image,
      .viewType         = (VkImageViewType) desc.view_type,
      .format           = (VkFormat) desc.view_format,
      .components       = VkComponentMapping{.r = (VkComponentSwizzle) desc.mapping.r,
                                             .g = (VkComponentSwizzle) desc.mapping.g,
                                             .b = (VkComponentSwizzle) desc.mapping.b,
                                             .a = (VkComponentSwizzle) desc.mapping.a},
      .subresourceRange = VkImageSubresourceRange{.aspectMask   = (VkImageAspectFlags) desc.aspects,
                                                  .baseMipLevel = desc.first_mip_level,
                                                  .levelCount   = desc.num_mip_levels,
                                                  .baseArrayLayer = 0,
                                                  .layerCount     = 1}};

  VkImageView view;
  table->CreateImageView(device, &create_info, nullptr, &view);

  return (gfx::ImageView) view;
}

gfx::Sampler VulkanDriver::create(gfx::SamplerDesc const &desc)
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
  table->CreateSampler(device, &create_info, nullptr, &sampler);

  return (gfx::Sampler) sampler;
}

gfx::BindGroupLayout VulkanDriver::create_bind_group_layout(gfx::BindGroupLayoutDesc const &desc)
{
  VulkanBindGroupLayout *layout = allocate<VulkanBindGroupLayout>();

  for (u32 i = 0; i < desc.num_bindings; i++)
  {
    VkDescriptorSetLayoutBinding binding{.binding         = 0,
                                         .descriptorType  = (VkDescriptorType) desc.layout[i].type,
                                         .descriptorCount = 1,
                                         .stageFlags = (VkShaderStageFlags) desc.layout[i].stages,
                                         .pImmutableSamplers = nullptr};

    VkDescriptorSetLayoutCreateInfo create_info{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = 1,
        .pBindings    = &binding};

    table->CreateDescriptorSetLayout(device, &create_info, nullptr,
                                     layout->vk_descriptor_set_layouts + i);
  }

  return (gfx::BindGroupLayout)(layout);
}

gfx::BindGroup VulkanDriver::create_bind_group(gfx::BindGroupDesc const &desc)
{
  VulkanBindGroupLayout *layout = (VulkanBindGroupLayout *) desc.layout;

  VkDescriptorSetAllocateInfo allocate_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                            .pNext = nullptr,
                                            .descriptorPool     = nullptr,
                                            .descriptorSetCount = desc.num_bindings,
                                            .pSetLayouts = layout->vk_descriptor_set_layouts};

  VulkanBindGroup *bind_group = allocate<VulkanBindGroup>();

  table->AllocateDescriptorSets(device, nullptr, bind_group->descriptor_sets);

  update_bind_group((gfx::BindGroup) bind_group,
                    stx::Span{desc.bindings}.slice(0, desc.num_bindings));

  return nullptr;
}

void VulkanDriver::update_bind_group(gfx::BindGroup                          bind_group_h,
                                     stx::Span<gfx::DescriptorBinding const> bindings)
{
  VkWriteDescriptorSet   writes[gfx::MAX_BIND_GROUP_ENTRIES];
  VkDescriptorImageInfo  image_infos[gfx::MAX_BIND_GROUP_ENTRIES];
  VkDescriptorBufferInfo buffer_infos[gfx::MAX_BIND_GROUP_ENTRIES];
  VkBufferView           texel_buffer_views[gfx::MAX_BIND_GROUP_ENTRIES];
  u8                     write_sets[gfx::MAX_BIND_GROUP_ENTRIES];
  u8                     num_images        = 0;
  u8                     num_buffers       = 0;
  u8                     num_texel_buffers = 0;

  VulkanBindGroup *bind_group   = (VulkanBindGroup *) bind_group_h;
  u8               num_bindings = (u8) bindings.size();

  for (u8 i = 0; i < num_bindings; i++)
  {
    gfx::DescriptorBinding const &binding = bindings[i];
    switch (binding.type)
    {
      case gfx::DescriptorType::CombinedImageSampler:
      {
        image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_infos[num_images].imageView = (VkImageView) binding.combined_image_sampler.image_view;
        image_infos[num_images].sampler   = (VkSampler) binding.combined_image_sampler.sampler;

        writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         .dstSet           = bind_group->descriptor_sets[i],
                                         .dstBinding       = 0,
                                         .dstArrayElement  = 0,
                                         .descriptorCount  = 1,
                                         .descriptorType   = (VkDescriptorType) binding.type,
                                         .pImageInfo       = image_infos + num_images,
                                         .pBufferInfo      = nullptr,
                                         .pTexelBufferView = nullptr};
        num_images++;
      }
      break;

      case gfx::DescriptorType::InputAttachment:
      {
      }
      break;

      case gfx::DescriptorType::SampledImage:
      {
        image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_infos[num_images].imageView = (VkImageView) binding.combined_image_sampler.image_view;
        image_infos[num_images].sampler   = nullptr;

        writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         .dstSet           = bind_group->descriptor_sets[i],
                                         .dstBinding       = 0,
                                         .dstArrayElement  = 0,
                                         .descriptorCount  = 1,
                                         .descriptorType   = (VkDescriptorType) binding.type,
                                         .pImageInfo       = image_infos + num_images,
                                         .pBufferInfo      = nullptr,
                                         .pTexelBufferView = nullptr};
        num_images++;
      }
      break;

      case gfx::DescriptorType::Sampler:
      {
        image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_infos[num_images].imageView   = nullptr;
        image_infos[num_images].sampler     = (VkSampler) binding.sampler.sampler;

        writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         .dstSet           = bind_group->descriptor_sets[i],
                                         .dstBinding       = 0,
                                         .dstArrayElement  = 0,
                                         .descriptorCount  = 1,
                                         .descriptorType   = (VkDescriptorType) binding.type,
                                         .pImageInfo       = image_infos + num_images,
                                         .pBufferInfo      = nullptr,
                                         .pTexelBufferView = nullptr};
        num_images++;
      }
      break;

      case gfx::DescriptorType::StorageBuffer:
      {
        buffer_infos[num_buffers].buffer =
            ((VulkanBuffer *) binding.storage_buffer.buffer)->vk_buffer;
        buffer_infos[num_buffers].offset = binding.storage_buffer.offset;
        buffer_infos[num_buffers].range  = binding.storage_buffer.size;

        writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         .dstSet           = bind_group->descriptor_sets[i],
                                         .dstBinding       = 0,
                                         .dstArrayElement  = 0,
                                         .descriptorCount  = 1,
                                         .descriptorType   = (VkDescriptorType) binding.type,
                                         .pImageInfo       = nullptr,
                                         .pBufferInfo      = buffer_infos + num_buffers,
                                         .pTexelBufferView = nullptr};
        num_buffers++;
      }
      break;

      case gfx::DescriptorType::StorageImage:
      {
        image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        image_infos[num_images].imageView   = (VkImageView) binding.storage_image.image_view;
        image_infos[num_images].sampler     = nullptr;

        writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         .dstSet           = bind_group->descriptor_sets[i],
                                         .dstBinding       = 0,
                                         .dstArrayElement  = 0,
                                         .descriptorCount  = 1,
                                         .descriptorType   = (VkDescriptorType) binding.type,
                                         .pImageInfo       = image_infos + num_images,
                                         .pBufferInfo      = nullptr,
                                         .pTexelBufferView = nullptr};
        num_images++;
      }
      break;

      case gfx::DescriptorType::StorageTexelBuffer:
      {
        texel_buffer_views[num_texel_buffers] =
            (VkBufferView) binding.storage_texel_buffer.buffer_view;

        writes[i] =
            VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                 .dstSet           = bind_group->descriptor_sets[i],
                                 .dstBinding       = 0,
                                 .dstArrayElement  = 0,
                                 .descriptorCount  = 1,
                                 .descriptorType   = (VkDescriptorType) binding.type,
                                 .pImageInfo       = nullptr,
                                 .pBufferInfo      = nullptr,
                                 .pTexelBufferView = texel_buffer_views + num_texel_buffers};
        num_texel_buffers++;
      }
      break;

      case gfx::DescriptorType::UniformBuffer:
      {
        buffer_infos[num_buffers].buffer =
            ((VulkanBuffer *) binding.uniform_buffer.buffer)->vk_buffer;
        buffer_infos[num_buffers].offset = binding.uniform_buffer.offset;
        buffer_infos[num_buffers].range  = binding.uniform_buffer.size;

        writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                         .dstSet           = bind_group->descriptor_sets[i],
                                         .dstBinding       = 0,
                                         .dstArrayElement  = 0,
                                         .descriptorCount  = 1,
                                         .descriptorType   = (VkDescriptorType) binding.type,
                                         .pImageInfo       = nullptr,
                                         .pBufferInfo      = buffer_infos + num_buffers,
                                         .pTexelBufferView = nullptr};
        num_buffers++;
      }
      break;

      case gfx::DescriptorType::UniformTexelBuffer:
      {
        texel_buffer_views[num_texel_buffers] =
            (VkBufferView) binding.uniform_texel_buffer.buffer_view;

        writes[i] =
            VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                 .dstSet           = bind_group->descriptor_sets[i],
                                 .dstBinding       = 0,
                                 .dstArrayElement  = 0,
                                 .descriptorCount  = 1,
                                 .descriptorType   = (VkDescriptorType) binding.type,
                                 .pImageInfo       = nullptr,
                                 .pBufferInfo      = nullptr,
                                 .pTexelBufferView = texel_buffer_views + num_texel_buffers};
        num_texel_buffers++;
      }
      break;
    }
  }

  table->UpdateDescriptorSets(device, num_bindings, writes, 0, nullptr);
}

void VulkanDriver::release_buffer(gfx::Buffer buffer)
{
  // release memory
  table->DestroyBuffer(device, (VkBuffer) buffer, nullptr);
}

void VulkanDriver::cmd_copy_buffer(gfx::CommandBuffer command_buffer, gfx::Buffer src,
                                   gfx::Buffer dst, stx::Span<gfx::BufferCopy const> copies)
{
  // TODO(lamarrr): we might need to allocate
  for (gfx::BufferCopy const &copy : copies)
  {
    VkBufferCopy buffer_copy{
        .srcOffset = copy.src_offset, .dstOffset = copy.dst_offset, .size = copy.size};
    vkCmdCopyBuffer((VkCommandBuffer) command_buffer, (VkBuffer) src, (VkBuffer) dst, 1,
                    &buffer_copy);
  }
}

}        // namespace rhi
}        // namespace ash
