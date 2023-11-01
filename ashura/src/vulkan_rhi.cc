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

VkBuffer VulkanDriver::create(gfx::BufferDesc const &desc)
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
  return (VkBuffer) buffer;
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
                                .mipLevels             = desc.mip_levels,
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

// gfx::BindGroupLayout VulkanDriver::create_bind_group_layout(gfx::BindGroupLayoutDesc const &desc)
// {
//   VulkanBindGroupLayout *layout = allocate<VulkanBindGroupLayout>();

//   for (u32 i = 0; i < desc.num_bindings; i++)
//   {
//     VkDescriptorSetLayoutBinding binding{.binding         = 0,
//                                          .descriptorType  = (VkDescriptorType) desc.layout[i].type,
//                                          .descriptorCount = 1,
//                                          .stageFlags = (VkShaderStageFlags) desc.layout[i].stages,
//                                          .pImmutableSamplers = nullptr};

//     VkDescriptorSetLayoutCreateInfo create_info{
//         .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//         .pNext        = nullptr,
//         .flags        = 0,
//         .bindingCount = 1,
//         .pBindings    = &binding};

//     table->CreateDescriptorSetLayout(device, &create_info, nullptr,
//                                      layout->vk_descriptor_set_layouts + i);
//   }

//   return (gfx::BindGroupLayout)(layout);
// }

// gfx::BindGroup VulkanDriver::create_bind_group(gfx::BindGroupDesc const &desc)
// {
//   VulkanBindGroupLayout *layout = (VulkanBindGroupLayout *) desc.layout;

//   VkDescriptorSetAllocateInfo allocate_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
//                                             .pNext = nullptr,
//                                             .descriptorPool     = nullptr,
//                                             .descriptorSetCount = desc.num_bindings,
//                                             .pSetLayouts = layout->vk_descriptor_set_layouts};

//   VulkanBindGroup *bind_group = allocate<VulkanBindGroup>();

//   table->AllocateDescriptorSets(device, nullptr, bind_group->descriptor_sets);

//   update_bind_group((gfx::BindGroup) bind_group,
//                     stx::Span{desc.bindings}.slice(0, desc.num_bindings));

//   return nullptr;
// }

// void VulkanDriver::update_bind_group(gfx::BindGroup                          bind_group_h,
//                                      stx::Span<gfx::DescriptorBinding const> bindings)
// {
//   VkWriteDescriptorSet   writes[gfx::MAX_BIND_GROUP_ENTRIES];
//   VkDescriptorImageInfo  image_infos[gfx::MAX_BIND_GROUP_ENTRIES];
//   VkDescriptorBufferInfo buffer_infos[gfx::MAX_BIND_GROUP_ENTRIES];
//   VkBufferView           texel_buffer_views[gfx::MAX_BIND_GROUP_ENTRIES];
//   u8                     write_sets[gfx::MAX_BIND_GROUP_ENTRIES];
//   u8                     num_images        = 0;
//   u8                     num_buffers       = 0;
//   u8                     num_texel_buffers = 0;

//   VulkanBindGroup *bind_group   = (VulkanBindGroup *) bind_group_h;
//   u8               num_bindings = (u8) bindings.size();

//   for (u8 i = 0; i < num_bindings; i++)
//   {
//     gfx::DescriptorBinding const &binding = bindings[i];
//     switch (binding.type)
//     {
//       case gfx::DescriptorType::CombinedImageSampler:
//       {
//         image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         image_infos[num_images].imageView = (VkImageView) binding.combined_image_sampler.image_view;
//         image_infos[num_images].sampler   = (VkSampler) binding.combined_image_sampler.sampler;

//         writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                          .dstSet           = bind_group->descriptor_sets[i],
//                                          .dstBinding       = 0,
//                                          .dstArrayElement  = 0,
//                                          .descriptorCount  = 1,
//                                          .descriptorType   = (VkDescriptorType) binding.type,
//                                          .pImageInfo       = image_infos + num_images,
//                                          .pBufferInfo      = nullptr,
//                                          .pTexelBufferView = nullptr};
//         num_images++;
//       }
//       break;

//       case gfx::DescriptorType::InputAttachment:
//       {
//       }
//       break;

//       case gfx::DescriptorType::SampledImage:
//       {
//         image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         image_infos[num_images].imageView = (VkImageView) binding.combined_image_sampler.image_view;
//         image_infos[num_images].sampler   = nullptr;

//         writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                          .dstSet           = bind_group->descriptor_sets[i],
//                                          .dstBinding       = 0,
//                                          .dstArrayElement  = 0,
//                                          .descriptorCount  = 1,
//                                          .descriptorType   = (VkDescriptorType) binding.type,
//                                          .pImageInfo       = image_infos + num_images,
//                                          .pBufferInfo      = nullptr,
//                                          .pTexelBufferView = nullptr};
//         num_images++;
//       }
//       break;

//       case gfx::DescriptorType::Sampler:
//       {
//         image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         image_infos[num_images].imageView   = nullptr;
//         image_infos[num_images].sampler     = (VkSampler) binding.sampler.sampler;

//         writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                          .dstSet           = bind_group->descriptor_sets[i],
//                                          .dstBinding       = 0,
//                                          .dstArrayElement  = 0,
//                                          .descriptorCount  = 1,
//                                          .descriptorType   = (VkDescriptorType) binding.type,
//                                          .pImageInfo       = image_infos + num_images,
//                                          .pBufferInfo      = nullptr,
//                                          .pTexelBufferView = nullptr};
//         num_images++;
//       }
//       break;

//       case gfx::DescriptorType::StorageBuffer:
//       {
//         buffer_infos[num_buffers].buffer =
//             ((VulkanBuffer *) binding.storage_buffer.buffer)->vk_buffer;
//         buffer_infos[num_buffers].offset = binding.storage_buffer.offset;
//         buffer_infos[num_buffers].range  = binding.storage_buffer.size;

//         writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                          .dstSet           = bind_group->descriptor_sets[i],
//                                          .dstBinding       = 0,
//                                          .dstArrayElement  = 0,
//                                          .descriptorCount  = 1,
//                                          .descriptorType   = (VkDescriptorType) binding.type,
//                                          .pImageInfo       = nullptr,
//                                          .pBufferInfo      = buffer_infos + num_buffers,
//                                          .pTexelBufferView = nullptr};
//         num_buffers++;
//       }
//       break;

//       case gfx::DescriptorType::StorageImage:
//       {
//         image_infos[num_images].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
//         image_infos[num_images].imageView   = (VkImageView) binding.storage_image.image_view;
//         image_infos[num_images].sampler     = nullptr;

//         writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                          .dstSet           = bind_group->descriptor_sets[i],
//                                          .dstBinding       = 0,
//                                          .dstArrayElement  = 0,
//                                          .descriptorCount  = 1,
//                                          .descriptorType   = (VkDescriptorType) binding.type,
//                                          .pImageInfo       = image_infos + num_images,
//                                          .pBufferInfo      = nullptr,
//                                          .pTexelBufferView = nullptr};
//         num_images++;
//       }
//       break;

//       case gfx::DescriptorType::StorageTexelBuffer:
//       {
//         texel_buffer_views[num_texel_buffers] =
//             (VkBufferView) binding.storage_texel_buffer.buffer_view;

//         writes[i] =
//             VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                  .dstSet           = bind_group->descriptor_sets[i],
//                                  .dstBinding       = 0,
//                                  .dstArrayElement  = 0,
//                                  .descriptorCount  = 1,
//                                  .descriptorType   = (VkDescriptorType) binding.type,
//                                  .pImageInfo       = nullptr,
//                                  .pBufferInfo      = nullptr,
//                                  .pTexelBufferView = texel_buffer_views + num_texel_buffers};
//         num_texel_buffers++;
//       }
//       break;

//       case gfx::DescriptorType::UniformBuffer:
//       {
//         buffer_infos[num_buffers].buffer =
//             ((VulkanBuffer *) binding.uniform_buffer.buffer)->vk_buffer;
//         buffer_infos[num_buffers].offset = binding.uniform_buffer.offset;
//         buffer_infos[num_buffers].range  = binding.uniform_buffer.size;

//         writes[i] = VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                          .dstSet           = bind_group->descriptor_sets[i],
//                                          .dstBinding       = 0,
//                                          .dstArrayElement  = 0,
//                                          .descriptorCount  = 1,
//                                          .descriptorType   = (VkDescriptorType) binding.type,
//                                          .pImageInfo       = nullptr,
//                                          .pBufferInfo      = buffer_infos + num_buffers,
//                                          .pTexelBufferView = nullptr};
//         num_buffers++;
//       }
//       break;

//       case gfx::DescriptorType::UniformTexelBuffer:
//       {
//         texel_buffer_views[num_texel_buffers] =
//             (VkBufferView) binding.uniform_texel_buffer.buffer_view;

//         writes[i] =
//             VkWriteDescriptorSet{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//                                  .dstSet           = bind_group->descriptor_sets[i],
//                                  .dstBinding       = 0,
//                                  .dstArrayElement  = 0,
//                                  .descriptorCount  = 1,
//                                  .descriptorType   = (VkDescriptorType) binding.type,
//                                  .pImageInfo       = nullptr,
//                                  .pBufferInfo      = nullptr,
//                                  .pTexelBufferView = texel_buffer_views + num_texel_buffers};
//         num_texel_buffers++;
//       }
//       break;
//     }
//   }

//   table->UpdateDescriptorSets(device, num_bindings, writes, 0, nullptr);
// }

// warnings: can't be used as depth stencil and color attachment
// load op clear op, read write matches imageusagescope
// ssbo matches scope
// push constant size match check
// NOTE: renderpass attachments MUST not be accessed in shaders within that renderpass
// NOTE: update_buffer and fill_buffer MUST be multiple of 4 for dst offset and dst size
struct BufferScope
{
  VkPipelineStageFlagBits stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlagBits        access = VK_ACCESS_NONE;
};

struct ImageScope
{
  VkPipelineStageFlagBits stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlagBits        access = VK_ACCESS_NONE;
  VkImageLayout           layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct BarrierStages
{
  VkPipelineStageFlagBits src = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlagBits dst = VK_PIPELINE_STAGE_NONE;
};

template <usize Capacity>
inline void acquire_buffer(VkBuffer buffer, gfx::BufferUsage usage, BufferScope const &scope,
                           stx::Array<VkBufferMemoryBarrier, Capacity> &barriers,
                           stx::Array<BarrierStages, Capacity>         &stages)
{
  if (has_any_bit(usage, gfx::BufferUsage::TransferSrc | gfx::BufferUsage::TransferDst))
  {
    VkAccessFlagBits src_access = VK_ACCESS_NONE;
    if (has_bit(usage, gfx::BufferUsage::TransferSrc))
    {
      src_access |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (has_bit(usage, gfx::BufferUsage::TransferDst))
    {
      src_access |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = src_access,
                                    .dstAccessMask       = scope.access,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = VK_PIPELINE_STAGE_TRANSFER_BIT, .dst = scope.stages});
  }

  if (has_bits(usage, gfx::BufferUsage::IndirectCommand))
  {
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                    .dstAccessMask       = scope.access,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, .dst = scope.stages});
  }

  if (has_any_bit(usage, gfx::BufferUsage::ComputeShaderUniform |
                             gfx::BufferUsage::ComputeShaderUniformTexel |
                             gfx::BufferUsage::ComputeShaderStorage |
                             gfx::BufferUsage::ComputeShaderStorageTexel))
  {
    VkAccessFlagBits src_access = VK_ACCESS_SHADER_READ_BIT;

    if (has_any_bit(gfx::BufferUsage::ComputeShaderStorage |
                    gfx::BufferUsage::ComputeShaderStorageTexel))
    {
      src_access |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = src_access,
                                    .dstAccessMask       = scope.access,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, .dst = scope.stages});
  }

  if (has_bits(usage, gfx::BufferUsage::IndexBuffer))
  {
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = VK_ACCESS_INDEX_READ_BIT,
                                    .dstAccessMask       = scope.access,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, .dst = scope.stages});
  }

  if (has_bits(usage, gfx::BufferUsage::VertexBuffer))
  {
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                                    .dstAccessMask       = scope.access,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, .dst = scope.stages});
  }

  if (has_any_bit(usage,
                  gfx::BufferUsage::VertexShaderUniform | gfx::BufferUsage::FragmentShaderUniform))
  {
    VkPipelineStageFlagBits src_stages = VK_PIPELINE_STAGE_NONE;
    if (has_bits(usage, gfx::BufferUsage::VertexShaderUniform))
    {
      src_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (has_bits(usage, gfx::BufferUsage::FragmentShaderUniform))
    {
      src_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = VK_ACCESS_SHADER_READ_BIT,
                                    .dstAccessMask       = scope.access,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = src_stages, .dst = scope.stages});
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the
// resource
template <usize Capacity>
inline void release_buffer(VkBuffer buffer, gfx::BufferUsage usage, BufferScope const &scope,
                           stx::Array<VkBufferMemoryBarrier, Capacity> &barriers,
                           stx::Array<BarrierStages, Capacity>         &stages)
{
  if (has_bits(usage, gfx::BufferUsage::TransferSrc))
  {
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = scope.access,
                                    .dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = scope.stages, .dst = VK_PIPELINE_STAGE_TRANSFER_BIT});
  }

  if (has_bits(usage, gfx::BufferUsage::IndirectCommand))
  {
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = scope.access,
                                    .dstAccessMask       = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = scope.stages, .dst = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT});
  }

  if (has_any_bit(usage, gfx::BufferUsage::ComputeShaderUniform |
                             gfx::BufferUsage::ComputeShaderUniformTexel))
  {
    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = scope.access,
                                    .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = scope.stages, .dst = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT});
  }

  if (has_any_bit(usage, gfx::BufferUsage::IndexBuffer | gfx::BufferUsage::VertexBuffer))
  {
    VkAccessFlagBits dst_access = VK_ACCESS_NONE;
    if (has_bits(usage, gfx::BufferUsage::IndexBuffer))
    {
      dst_access |= VK_ACCESS_INDEX_READ_BIT;
    }

    if (has_bits(usage, gfx::BufferUsage::VertexBuffer))
    {
      dst_access |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }

    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = scope.access,
                                    .dstAccessMask       = dst_access,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = scope.stages, .dst = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT});
  }

  if (has_any_bit(usage,
                  gfx::BufferUsage::VertexShaderUniform | gfx::BufferUsage::FragmentShaderUniform))
  {
    VkPipelineStageFlagBits dst_stages = VK_PIPELINE_STAGE_NONE;

    if (has_bits(usage, gfx::BufferUsage::VertexShaderUniform))
    {
      dst_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (has_bits(usage, gfx::BufferUsage::FragmentShaderUniform))
    {
      dst_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    barriers
        .push(VkBufferMemoryBarrier{.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                                    .pNext               = nullptr,
                                    .srcAccessMask       = scope.access,
                                    .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
                                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                    .buffer              = buffer,
                                    .offset              = 0,
                                    .size                = VK_WHOLE_SIZE})
        .unwrap();
    stages.push(BarrierStages{.src = scope.stages, .dst = dst_stages});
  }
}

constexpr BufferScope transfer_buffer_scope(gfx::BufferUsage usage)
{
  VkPipelineStageFlagBits stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkAccessFlagBits        access = VK_ACCESS_NONE;

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
// only called on dst
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
//
// and if image was never transitioned we should be good and use undefined src layout?
//
// pre -> op -> post
// if image was already on the queue scope takes care of it
// layout+scope -> transfer src|dst optimal
// all ops that have side-effects
// transfer transfer src|dst optimal -> scope + layout???? not needed?
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
// TODO(lamarrr): undefined layout handling
//
// we need: initial layout, operations providing the initial layout and releasing it to other scopes
// for pickup
//
// - easiest and best would be to have an Undefined + Init function that specifies and places
// barriers for the scopes
//
// - they must all have an initial transfer-dst operation? -
// - acquire_init -> transfer -> release
//
// - acquire_init -> render as color attachment -> release
//
// we need a base layout
// just use fill??? even if not specified with initial data
// Scope Layout -> Other Scopes layout
// can't use fill cos we might not know the initial layout
//
// our barriers assume they are coming from one usagescope and transition to same or another
// usagescope initialization is not representable
//
//
//
// release only upon init
//
// Undefined, None Access, TopOfPipe -> scopes, NoAccessMask will not affect accessmasks
// - multiple setup will mean????
//
// release to transfer will be as transfer dst
// release to color attachment will be as write-only non-flushed
//
//
// multiple none-accesses will cause multiple transitions because there will be no dependency chains
//
// the previous acquire barriers only work on
//
//
// initial layout top of pipe, none access, transition layout to initial layout
//
//
// add is_first?
//
//
template <usize Capacity>
inline void acquire_image(VkImage image, gfx::ImageUsage usage, ImageScope const &scope,
                          VkImageAspectFlags                          aspects,
                          stx::Array<VkImageMemoryBarrier, Capacity> &barriers,
                          stx::Array<BarrierStages, Capacity>        &stages)
{
  if (has_any_bit(usage, gfx::ImageUsage::TransferSrc | gfx::ImageUsage::TransferDst))
  {
    VkAccessFlagBits src_access = VK_ACCESS_NONE;
    VkImageLayout    old_layout = VK_IMAGE_LAYOUT_UNDEFINED;

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

    barriers
        .push(VkImageMemoryBarrier{
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
                                                           .layerCount = VK_REMAINING_ARRAY_LAYERS}})
        .unwrap();

    stages.push(BarrierStages{.src = VK_PIPELINE_STAGE_TRANSFER_BIT, .dst = scope.stages}).unwrap();
  }

  // if scope has compute shader write then it will always be transitioned to general
  if (has_bits(usage,
               gfx::ImageUsage::ComputeShaderStorage | gfx::ImageUsage::ComputeShaderSampled))
  {
    VkImageLayout    old_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAccessFlagBits src_access = VK_ACCESS_SHADER_READ_BIT;

    if (has_bits(usage, gfx::ImageUsage::ComputeShaderStorage))
    {
      old_layout = VK_IMAGE_LAYOUT_GENERAL;
      src_access |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    barriers
        .push(VkImageMemoryBarrier{
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
                                                           .layerCount = VK_REMAINING_ARRAY_LAYERS}})
        .unwrap();

    stages.push(BarrierStages{.src = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, .dst = scope.stages})
        .unwrap();
  }

  if (has_any_bit(usage, gfx::ImageUsage::VertexShaderSampled |
                             gfx::ImageUsage::FragmentShaderSampled |
                             gfx::ImageUsage::InputAttachment))
  {
    VkPipelineStageFlagBits src_stages = VK_PIPELINE_STAGE_NONE;

    if (has_bits(usage, gfx::ImageUsage::VertexShaderSampled))
    {
      src_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (has_any_bit(usage,
                    gfx::ImageUsage::FragmentShaderSampled | gfx::ImageUsage::InputAttachment))
    {
      src_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    barriers
        .push(VkImageMemoryBarrier{
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
                                                           .layerCount = VK_REMAINING_ARRAY_LAYERS}})
        .unwrap();

    stages.push(BarrierStages{.src = src_stages, .dst = scope.stages}).unwrap();
  }

  if (has_any_bit(usage,
                  gfx::ImageUsage::ReadColorAttachment | gfx::ImageUsage::WriteColorAttachment))
  {
    VkAccessFlagBits src_access = VK_ACCESS_NONE;

    if (has_bits(usage, gfx::ImageUsage::ReadColorAttachment))
    {
      src_access |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }

    if (has_bits(usage, gfx::ImageUsage::WriteColorAttachment))
    {
      src_access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    barriers
        .push(VkImageMemoryBarrier{
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
                                                           .layerCount = VK_REMAINING_ARRAY_LAYERS}})
        .unwrap();

    stages
        .push(BarrierStages{.src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            .dst = scope.stages})
        .unwrap();
  }

  if (has_any_bit(usage, gfx::ImageUsage::ReadDepthStencilAttachment |
                             gfx::ImageUsage::WriteDepthStencilAttachment))
  {
    VkImageLayout           old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAccessFlagBits        src_access = VK_ACCESS_NONE;
    VkPipelineStageFlagBits src_stages = VK_PIPELINE_STAGE_NONE;

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

    barriers
        .push(VkImageMemoryBarrier{
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
                                                           .layerCount = VK_REMAINING_ARRAY_LAYERS}})
        .unwrap();

    stages.push(BarrierStages{.src = src_stages, .dst = scope.stages}).unwrap();
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the
// resource
template <usize Capacity>
inline void release_image(VkImage image, gfx::ImageUsage usage, ImageScope const &scope,
                          VkImageAspectFlags                          aspects,
                          stx::Array<VkImageMemoryBarrier, Capacity> &barriers,
                          stx::Array<BarrierStages, Capacity>        &stages)
{
  // only shader-sampled images can run parallel to other command views
  // only transitioned to Shader read only if it is not used as storage at the same stage
  //
  // for all non-shader-read-only-optimal usages, an acquire must be performed
  //
  if (has_bits(usage, gfx::ImageUsage::ComputeShaderSampled) &&
      !has_bits(usage, gfx::ImageUsage::ComputeShaderStorage))
  {
    barriers
        .push(VkImageMemoryBarrier{
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
                                                           .layerCount = VK_REMAINING_ARRAY_LAYERS}})
        .unwrap();

    stages.push(BarrierStages{.src = scope.stages, .dst = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT})
        .unwrap();
  }

  if (has_any_bit(usage, gfx::ImageUsage::VertexShaderSampled |
                             gfx::ImageUsage::FragmentShaderSampled |
                             gfx::ImageUsage::InputAttachment))
  {
    VkPipelineStageFlagBits dst_stages = VK_PIPELINE_STAGE_NONE;

    if (has_bits(usage, gfx::ImageUsage::VertexShaderSampled))
    {
      dst_stages |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (has_any_bit(usage,
                    gfx::ImageUsage::FragmentShaderSampled | gfx::ImageUsage::InputAttachment))
    {
      dst_stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    barriers
        .push(VkImageMemoryBarrier{
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
                                                           .layerCount = VK_REMAINING_ARRAY_LAYERS}})
        .unwrap();

    stages.push(BarrierStages{.src = scope.stages, .dst = dst_stages}).unwrap();
  }
}

// apply to both src and dst since they require layout transitions
constexpr ImageScope transfer_image_scope(gfx::ImageUsage usage)
{
  VkImageLayout           layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkPipelineStageFlagBits stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
  VkAccessFlagBits        access = VK_ACCESS_NONE;

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
  VkAccessFlagBits access = VK_ACCESS_NONE;

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
  VkImageLayout           layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkPipelineStageFlagBits stages = VK_PIPELINE_STAGE_NONE;
  VkAccessFlagBits        access = VK_ACCESS_NONE;

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

}        // namespace rhi
}        // namespace ash
