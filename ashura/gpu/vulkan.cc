/// SPDX-License-Identifier: MIT
#include "ashura/gpu/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "ashura/std/sformat.h"
#include "vulkan/vulkan.h"
#include <cstring>

namespace ash
{
namespace vk
{

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

Layout64 MemoryGroup::layout() const
{
  return alias_offsets.is_empty() ?
           Layout64{.alignment = alignment, .size = 0} :
           Layout64{.alignment = alignment, .size = alias_offsets.last()};
}

u32 DescriptorBinding::sync_size() const
{
  return sync_resources.match(
    [](None) { return (u32) 0; }, [](auto & v) { return size32(v); },
    [](auto & v) { return size32(v); }, [](auto & v) { return size32(v); });
}

void DescriptorSet::remove_bind_loc(BindLocations &      locations,
                                    BindLocation const & loc)
{
  auto pos = find(locations.view(), loc).as_slice_of(locations);
  locations.erase(pos);
}

void DescriptorSet::update_link(u32 ibinding, u32 first_element,
                                Span<gpu::BufferBinding const> buffers)
{
  auto & binding        = bindings[ibinding];
  auto & sync_resources = binding.sync_resources[v1];

  for (auto [i, buffer] : enumerate<u32>(buffers))
  {
    auto    element = first_element + i;
    auto *& current = sync_resources[element];
    auto *  next    = (Buffer *) buffer.buffer;

    if (current == next)
    {
      continue;
    }

    auto loc =
      BindLocation{.set = this, .binding = ibinding, .element = element};

    if (current != nullptr)
    {
      remove_bind_loc(current->bind_locations, loc);
    }

    if (next != nullptr)
    {
      next->bind_locations.push(loc).unwrap();
    }

    current = next;
  }
}

void DescriptorSet::update_link(u32 ibinding, u32 first_element,
                                Span<gpu::BufferView const> buffer_views)
{
  auto & binding        = bindings[ibinding];
  auto & sync_resources = binding.sync_resources[v2];

  for (auto [i, buffer_view] : enumerate<u32>(buffer_views))
  {
    auto    element = first_element + i;
    auto *& current = sync_resources[element];
    auto *  next    = (BufferView *) buffer_view;

    if (current == next)
    {
      continue;
    }

    auto loc =
      BindLocation{.set = this, .binding = ibinding, .element = element};

    if (current != nullptr)
    {
      remove_bind_loc(current->bind_locations, loc);
    }

    if (next != nullptr)
    {
      next->bind_locations.push(loc).unwrap();
    }

    current = next;
  }
}

void DescriptorSet::update_link(u32 ibinding, u32 first_element,
                                Span<gpu::ImageBinding const> images)
{
  auto & binding        = bindings[ibinding];
  auto & sync_resources = binding.sync_resources[v3];

  for (auto [i, image] : enumerate<u32>(images))
  {
    auto    element = first_element + i;
    auto *& current = sync_resources[element];
    auto *  next    = (ImageView *) image.image_view;

    if (current == next)
    {
      continue;
    }

    auto loc =
      BindLocation{.set = this, .binding = ibinding, .element = element};

    if (current != nullptr)
    {
      remove_bind_loc(current->bind_locations, loc);
    }

    if (next != nullptr)
    {
      next->bind_locations.push(loc).unwrap();
    }

    current = next;
  }
}

constexpr VkAccessFlags descriptor_access(gpu::DescriptorType type)
{
  switch (type)
  {
    case gpu::DescriptorType::Sampler:
      return VK_ACCESS_NONE;
    case gpu::DescriptorType::CombinedImageSampler:
      return VK_ACCESS_SHADER_READ_BIT;
    case gpu::DescriptorType::SampledImage:
      return VK_ACCESS_SHADER_READ_BIT;
    case gpu::DescriptorType::StorageImage:
      return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case gpu::DescriptorType::UniformTexelBuffer:
      return VK_ACCESS_SHADER_READ_BIT;
    case gpu::DescriptorType::StorageTexelBuffer:
      return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case gpu::DescriptorType::UniformBuffer:
      return VK_ACCESS_SHADER_READ_BIT;
    case gpu::DescriptorType::ReadStorageBuffer:
      return VK_ACCESS_SHADER_READ_BIT;
    case gpu::DescriptorType::RWStorageBuffer:
      return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case gpu::DescriptorType::DynUniformBuffer:
      return VK_ACCESS_SHADER_READ_BIT;
    case gpu::DescriptorType::DynReadStorageBuffer:
      return VK_ACCESS_SHADER_READ_BIT;
    case gpu::DescriptorType::DynRWStorageBuffer:
      return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    case gpu::DescriptorType::InputAttachment:
      return VK_ACCESS_SHADER_READ_BIT;
    default:
      return VK_ACCESS_NONE;
  }
}

constexpr VkImageLayout descriptor_image_layout(gpu::DescriptorType type)
{
  switch (type)
  {
    case gpu::DescriptorType::Sampler:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::CombinedImageSampler:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case gpu::DescriptorType::SampledImage:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case gpu::DescriptorType::StorageImage:
      return VK_IMAGE_LAYOUT_GENERAL;
    case gpu::DescriptorType::UniformTexelBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::StorageTexelBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::UniformBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::ReadStorageBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::RWStorageBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::DynUniformBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::DynReadStorageBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::DynRWStorageBuffer:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case gpu::DescriptorType::InputAttachment:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    default:
      return VK_IMAGE_LAYOUT_UNDEFINED;
  }
}

constexpr gpu::ImageUsage descriptor_image_usage(gpu::DescriptorType type)
{
  switch (type)
  {
    case gpu::DescriptorType::Sampler:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::CombinedImageSampler:
      return gpu::ImageUsage::Sampled;
    case gpu::DescriptorType::SampledImage:
      return gpu::ImageUsage::Sampled;
    case gpu::DescriptorType::StorageImage:
      return gpu::ImageUsage::Storage;
    case gpu::DescriptorType::UniformTexelBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::StorageTexelBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::UniformBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::ReadStorageBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::RWStorageBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::DynUniformBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::DynReadStorageBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::DynRWStorageBuffer:
      return gpu::ImageUsage::None;
    case gpu::DescriptorType::InputAttachment:
      return gpu::ImageUsage::InputAttachment;
    default:
      return gpu::ImageUsage::None;
  }
}

constexpr gpu::BufferUsage descriptor_buffer_usage(gpu::DescriptorType type)
{
  switch (type)
  {
    case gpu::DescriptorType::Sampler:
      return gpu::BufferUsage::None;
    case gpu::DescriptorType::CombinedImageSampler:
      return gpu::BufferUsage::None;
    case gpu::DescriptorType::SampledImage:
      return gpu::BufferUsage::None;
    case gpu::DescriptorType::StorageImage:
      return gpu::BufferUsage::None;
    case gpu::DescriptorType::UniformTexelBuffer:
      return gpu::BufferUsage::UniformTexelBuffer;
    case gpu::DescriptorType::StorageTexelBuffer:
      return gpu::BufferUsage::StorageTexelBuffer;
    case gpu::DescriptorType::UniformBuffer:
      return gpu::BufferUsage::UniformBuffer;
    case gpu::DescriptorType::ReadStorageBuffer:
      return gpu::BufferUsage::StorageBuffer;
    case gpu::DescriptorType::RWStorageBuffer:
      return gpu::BufferUsage::StorageBuffer;
    case gpu::DescriptorType::DynUniformBuffer:
      return gpu::BufferUsage::UniformBuffer;
    case gpu::DescriptorType::DynReadStorageBuffer:
      return gpu::BufferUsage::StorageBuffer;
    case gpu::DescriptorType::DynRWStorageBuffer:
      return gpu::BufferUsage::StorageBuffer;
    case gpu::DescriptorType::InputAttachment:
      return gpu::BufferUsage::None;
    default:
      return gpu::BufferUsage::None;
  }
}

enum class SyncResourceType : u8
{
  None       = 0,
  Buffer     = 1,
  BufferView = 2,
  ImageView  = 3
};

constexpr SyncResourceType
  descriptor_sync_resource_type(gpu::DescriptorType type)
{
  switch (type)
  {
    case gpu::DescriptorType::Sampler:
      return SyncResourceType::None;
    case gpu::DescriptorType::CombinedImageSampler:
      return SyncResourceType::ImageView;
    case gpu::DescriptorType::SampledImage:
      return SyncResourceType::ImageView;
    case gpu::DescriptorType::StorageImage:
      return SyncResourceType::ImageView;
    case gpu::DescriptorType::UniformTexelBuffer:
      return SyncResourceType::BufferView;
    case gpu::DescriptorType::StorageTexelBuffer:
      return SyncResourceType::BufferView;
    case gpu::DescriptorType::UniformBuffer:
      return SyncResourceType::Buffer;
    case gpu::DescriptorType::ReadStorageBuffer:
      return SyncResourceType::Buffer;
    case gpu::DescriptorType::RWStorageBuffer:
      return SyncResourceType::Buffer;
    case gpu::DescriptorType::DynUniformBuffer:
      return SyncResourceType::Buffer;
    case gpu::DescriptorType::DynReadStorageBuffer:
      return SyncResourceType::Buffer;
    case gpu::DescriptorType::DynRWStorageBuffer:
      return SyncResourceType::Buffer;
    case gpu::DescriptorType::InputAttachment:
      return SyncResourceType::ImageView;
    default:
      return SyncResourceType::None;
  }
}

MemAccess Hazard::latest_acccess() const
{
  switch (type)
  {
    case HazardType::None:
      return MemAccess{.stages = VK_PIPELINE_STAGE_NONE,
                       .access = VK_ACCESS_NONE};
    case HazardType::Reads:
      return MemAccess{.stages = reads.stages, .access = reads.access};
    case HazardType::Write:
      return MemAccess{.stages = write.stages, .access = write.access};
    case HazardType::ReadsAfterWrite:
      return MemAccess{.stages = reads.stages, .access = reads.access};
  }
}

void HazardBarriers::begin_pass()
{
  if (passes_.is_empty())
  {
    passes_.push(Entry{}).unwrap();
  }

  passes_.push(passes_.last()).unwrap();
}

void HazardBarriers::end_pass()
{
}

void HazardBarriers::buffer(VkPipelineStageFlags src, VkPipelineStageFlags dst,
                            VkBufferMemoryBarrier buffer)
{
  // [ ] shouldn't stages be merged?
  stage_.push(Stage{.src = src, .dst = dst}).unwrap();
  buffer_.push(buffer).unwrap();
  passes_.last().buffers++;
}

void HazardBarriers::image(VkPipelineStageFlags src, VkPipelineStageFlags dst,
                           VkImageMemoryBarrier image)
{
  stage_.push(Stage{.src = src, .dst = dst}).unwrap();
  image_.push(image).unwrap();
  passes_.last().images++;
}

void HazardBarriers::mem(VkPipelineStageFlags src, VkPipelineStageFlags dst,
                         VkMemoryBarrier memory)
{
  stage_.push(Stage{.src = src, .dst = dst}).unwrap();
  mem_.push(memory).unwrap();
  passes_.last().memory++;
}

// layout transitions are considered write operations even if only a read
// happens so multiple ones can't happen at the same time
//
// we'll kind of be waiting on a barrier operation which doesn't make sense cos
// the barrier might have already taken care of us even when they both only
// perform reads
//
// if their scopes don't line-up, they won't observe the effects same
void EncoderResourceStates::barrier(Image const &     image,
                                    MemAccess const & old_access,
                                    VkImageLayout     old_layout,
                                    MemAccess const & new_access,
                                    VkImageLayout     new_layout,
                                    HazardBarriers &  barriers)
{
  barriers.image(
    old_access.stages, new_access.stages,
    {
      .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext               = nullptr,
      .srcAccessMask       = old_access.access,
      .dstAccessMask       = new_access.access,
      .oldLayout           = old_layout,
      .newLayout           = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image               = image.vk,
      .subresourceRange    = {.aspectMask     = (VkImageAspectFlags) image.aspects,
                              .baseMipLevel   = 0,
                              .levelCount     = VK_REMAINING_MIP_LEVELS,
                              .baseArrayLayer = 0,
                              .layerCount     = VK_REMAINING_ARRAY_LAYERS}
  });
}

void EncoderResourceStates::barrier(Buffer const &    buffer,
                                    MemAccess const & old_state,
                                    MemAccess const & new_state,
                                    HazardBarriers &  barriers)
{
  barriers.buffer(old_state.stages, new_state.stages,
                  {.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                   .pNext         = nullptr,
                   .srcAccessMask = old_state.access,
                   .dstAccessMask = new_state.access,
                   .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                   .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                   .buffer              = buffer.vk,
                   .offset              = 0,
                   .size                = VK_WHOLE_SIZE});
}

void EncoderResourceStates::barrier(MemAccess const & old_state,
                                    MemAccess const & new_state,
                                    HazardBarriers &  barriers)
{
  barriers.mem(old_state.stages, new_state.stages,
               {.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                .pNext         = nullptr,
                .srcAccessMask = old_state.access,
                .dstAccessMask = new_state.access});
}

void EncoderResourceStates::access(Image const &     image,
                                   MemAccess const & access,
                                   VkImageLayout layout, u32 pass,
                                   HazardBarriers & barriers)
{
  // only one element of the alias can issue a barrier for a pass
  auto alias_id = image.memory.memory_group->alias_ids[image.memory.alias];
  auto has_read = has_read_access(access.access);
  auto element  = image.memory.element;

  auto [has_changed, last_accessed, state, hazard] = memory_[alias_id];

  // it was already accessed, resources are only allowed to be in one state for a pass
  if (last_accessed != U32_MAX && last_accessed == pass)
  {
    last_accessed = pass;
    return;
  }

  last_accessed = pass;

  auto new_state = ImageMemState{.element = element, .layout = layout};

  auto mark = [&](Hazard const & h) {
    state  = new_state;
    hazard = h;
    memory_.dense.v0.set(memory_.to_index(alias_id), true);
  };

  auto discard = [&]() {
    auto old_access = hazard.latest_acccess();
    barrier(old_access, access, barriers);
    barrier(image, old_access, VK_IMAGE_LAYOUT_UNDEFINED, access, layout,
            barriers);

    mark(Hazard{.type = HazardType::Write, .reads = {}, .write = access});
  };

  state.match(
    [&](None) {
      discard();
      return;
    },
    [&](BufferMemState const &) {
      discard();
      return;
    },
    [&](ImageMemState const & h) {
      auto current_layout   = h.layout;
      auto needs_transition = current_layout != layout;
      auto has_write      = has_write_access(access.access) || needs_transition;
      auto previous_reads = hazard.reads;
      auto previous_write = hazard.write;

      auto barrier = [&](MemAccess const & from) {
        this->barrier(image, from, current_layout, access, layout, barriers);
      };

      if (h.element != element)
      {
        discard();
        return;
      }

      switch (hazard.type)
      {
        case HazardType::None:
        {
          // no sync needed, no accessor before this
          if (has_write)
          {
            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            if (needs_transition)
            {
              barrier(MemAccess{.stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                .access = VK_ACCESS_NONE});

              return;
            }

            return;
          }

          if (has_read)
          {
            mark(
              Hazard{.type = HazardType::Reads, .reads = access, .write = {}});

            return;
          }

          return;
        }
        case HazardType::Reads:
        {
          if (has_write)
          {
            // wait till done reading before modifying
            // reset access sequence since all stages following this write need to
            // wait on this write

            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            barrier(previous_reads);

            return;
          }

          if (has_read)
          {
            // combine all subsequent reads, so the next writer knows to wait on all
            // combined reads to complete

            mark(Hazard{
              .type  = HazardType::Reads,
              .reads = {.stages = previous_reads.stages | access.stages,
                        .access = previous_reads.access | access.access},
              .write = {}
            });

            return;
          }

          return;
        }
        case HazardType::Write:
        {
          if (has_write)
          {
            // wait till done writing before modifying
            // remove previous write since this access already waits on another
            // access to complete and the next access will have to wait on this
            // access
            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            barrier(previous_write);

            return;
          }

          if (has_read)
          {
            // wait till all write stages are done

            mark(Hazard{.type  = HazardType::ReadsAfterWrite,
                        .reads = access,
                        .write = previous_write});

            barrier(previous_write);

            return;
          }

          return;
        }
        case HazardType::ReadsAfterWrite:
        {
          if (has_write)
          {
            // wait for all reading stages only
            // stages can be reset and point only to the latest write stage, since
            // they all need to wait for this write anyway.

            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            barrier(previous_reads);

            return;
          }

          if (has_read)
          {
            // wait for all write stages to be done
            // no need to wait on other reads since we are only performing a read.

            //
            // mask all subsequent reads so next writer knows to wait on all reads
            // to complete
            //
            // if stage and access intersects previous barrier, no need to add new
            // one as we'll observe the effect
            auto implictly_merged =
              has_any_bit(previous_reads.stages, access.stages) &&
              has_any_bit(previous_reads.access, access.access);

            if (implictly_merged)
            {
              mark(Hazard{
                .type  = HazardType::ReadsAfterWrite,
                .reads = {.stages = previous_reads.stages | access.stages,
                          .access = previous_reads.access | access.access},
                .write = previous_write
              });
              return;
            }

            mark(Hazard{
              .type  = HazardType::ReadsAfterWrite,
              .reads = {.stages = previous_reads.stages | access.stages,
                        .access = previous_reads.access | access.access},
              .write = previous_write
            });

            barrier(previous_write);

            return;
          }

          return;
        }

        default:
          return;
      }
    });

  return;
}

void EncoderResourceStates::access(ImageView const & image,
                                   MemAccess const & access,
                                   VkImageLayout layout, u32 pass,
                                   HazardBarriers & barriers)
{
  this->access(*image.image, access, layout, pass, barriers);
}

void EncoderResourceStates::access(Buffer const &    buffer,
                                   MemAccess const & access, u32 pass,
                                   HazardBarriers & barriers)
{
  auto alias_id = buffer.memory.memory_group->alias_ids[buffer.memory.alias];

  auto has_write = has_write_access(access.access);
  auto has_read  = has_read_access(access.access);
  auto element   = buffer.memory.element;

  auto [has_changed, last_accessed, state, hazard] = memory_[alias_id];

  // it was already accessed, resources are only allowed to be in one state for a pass
  if (last_accessed != U32_MAX && last_accessed == pass)
  {
    last_accessed = pass;
    return;
  }

  last_accessed = pass;

  auto new_state = BufferMemState{.element = element};

  auto mark = [&](Hazard const & h) {
    state  = new_state;
    hazard = h;
    memory_.dense.v0.set(memory_.to_index(alias_id), true);
  };

  auto discard = [&]() {
    auto old_access = hazard.latest_acccess();
    barrier(old_access, access, barriers);
    barrier(buffer, old_access, access, barriers);

    mark(Hazard{.type = HazardType::Write, .reads = {}, .write = access});
  };

  state.match(
    [&](None) {
      discard();
      return;
    },
    [&](BufferMemState const & h) {
      auto previous_reads = hazard.reads;
      auto previous_write = hazard.write;

      auto barrier = [&](MemAccess const & from) {
        this->barrier(buffer, from, access, barriers);
      };

      if (h.element != element)
      {
        discard();
        return;
      }

      switch (hazard.type)
      {
        case HazardType::None:
        {
          if (has_write)
          {
            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            return;
          }

          if (has_read)
          {
            mark(
              Hazard{.type = HazardType::Reads, .reads = access, .write = {}});

            return;
          }

          return;
        }

        case HazardType::Reads:
        {
          if (has_write)
          {
            // wait till done reading before modifying
            // reset access sequence since all stages following this write need to
            // wait on this write
            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            barrier(previous_reads);

            return;
          }

          if (has_read)
          {
            // combine all subsequent reads, so the next writer knows to wait on all
            // combined reads to complete

            mark(Hazard{
              .type  = HazardType::Reads,
              .reads = {.stages = previous_reads.stages | access.stages,
                        .access = previous_reads.access | access.access},
              .write = {}
            });

            return;
          }

          return;
        }
        case HazardType::Write:
        {
          if (has_write)
          {
            // wait till done writing before modifying
            // remove previous write since this access already waits on another
            // access to complete and the next access will have to wait on this
            // access
            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            barrier(previous_write);

            return;
          }

          if (has_read)
          {
            // wait till all write stages are done
            mark(Hazard{.type  = HazardType::ReadsAfterWrite,
                        .reads = access,
                        .write = previous_write});

            barrier(previous_write);

            return;
          }

          return;
        }
        case HazardType::ReadsAfterWrite:
        {
          if (has_write)
          {
            // wait for all reading stages only
            // stages can be reset and point only to the latest write stage, since
            // they all need to wait for this write anyway.

            mark(
              Hazard{.type = HazardType::Write, .reads = {}, .write = access});

            barrier(previous_reads);

            return;
          }

          if (has_read)
          {
            // wait for all write stages to be done
            // no need to wait on other reads since we are only performing a read
            // mask all subsequent reads so next writer knows to wait on all reads
            // to complete

            // if stage and access intersects previous barrier, no need to add new
            // one

            auto implicitly_merged =
              has_any_bit(previous_reads.stages, access.stages) &&
              has_any_bit(previous_reads.access, access.access);

            if (implicitly_merged)
            {
              mark(Hazard{
                .type  = HazardType::ReadsAfterWrite,
                .reads = {.stages = previous_reads.stages | access.stages,
                          .access = previous_reads.access | access.access},
                .write = previous_write
              });
              return;
            }

            mark(Hazard{
              .type  = HazardType::ReadsAfterWrite,
              .reads = {.stages = previous_reads.stages | access.stages,
                        .access = previous_reads.access | access.access},
              .write = previous_write
            });

            barrier(previous_write);

            return;
          }

          return;
        }

        default:
          return;
      }
    },
    [&](ImageMemState const &) {
      discard();
      return;
    });
}

void EncoderResourceStates::access(DescriptorSet const & set, u32 pass,
                                   VkPipelineStageFlags shader_stages,
                                   HazardBarriers &     barriers)
{
  auto [last_accessed] = descriptor_set_[set.id];

  /// if it is a read-only descriptor set we don't need to synchronize after the first synchronization pass; resources can be in only one state for a pass
  if (!set.is_mutating && pass != 0 && last_accessed != U32_MAX &&
      (last_accessed == (pass - 1)))
  {
    last_accessed = pass;
    return;
  }

  last_accessed = pass;

  for (auto & binding : set.bindings)
  {
    auto access_flags = descriptor_access(binding.type);
    auto acc = MemAccess{.stages = shader_stages, .access = access_flags};

    binding.sync_resources.match(
      [](None) {},
      [&](auto & buffers) {
        for (Buffer * buffer : buffers)
        {
          if (buffer != nullptr)
          {
            access(*buffer, acc, pass, barriers);
          }
        }
      },
      [&](auto & buffer_views) {
        for (BufferView * buffer_view : buffer_views)
        {
          if (buffer_view != nullptr)
          {
            access(*buffer_view->buffer, acc, pass, barriers);
          }
        }
      },
      [&](auto & image_views) {
        for (ImageView * image_view : image_views)
        {
          if (image_view != nullptr)
          {
            auto layout = descriptor_image_layout(binding.type);
            access(*image_view->image, acc, layout, pass, barriers);
          }
        }
      });
  }
}

void EncoderResourceStates::rebuild(DeviceResourceStates const & upstream)
{
  memory_.clear();

  memory_.reserve(upstream.memory_.size()).unwrap();

  for (auto [i, state] : zip(range(upstream.memory_.size()), upstream.memory_))
  {
    auto id = upstream.memory_.to_id(i);
    memory_.id_to_index_.push(i).unwrap();
    memory_.index_to_id_.push(id).unwrap();      // [ ] fix
    memory_.dense.v0.push(false).unwrap();       // was modified
    memory_.dense.v1.push(U32_MAX).unwrap();     // last_accessed
    memory_.dense.v2.push(state.v0).unwrap();    // memory state
    memory_.dense.v3.push(Hazard{.type = HazardType::None})
      .unwrap();    // hazard type
  }

  // [ ] if image id is re-used we need to reset
  // [ ] deletion update & deletion queue management
  // [ ] if resource is created after begin call and added to the command buffer?; what about if a destroyed resource is accesed?; id-reuse?
}

void EncoderResourceStates::commit(DeviceResourceStates & upstream)
{
  for (auto [i, m] : zip(range(memory_.size()), memory_))
  {
    // if resource was modified
    if (m.v0) [[unlikely]]
    {
      upstream.memory_[memory_.to_id(i)].v0 = m.v2;
    }
  }
}

u32 AccessEncoder::begin_pass()
{
  auto index = size32(passes_);

  if (passes_.is_empty())
  {
    passes_.push(Entry{}).unwrap();
  }

  auto last = passes_.last();
  passes_.push(last).unwrap();
  return index;
}

void AccessEncoder::end_pass()
{
}

void AccessEncoder::access(Buffer * buffer, VkPipelineStageFlags stages,
                           VkAccessFlags access)
{
  buffers_.push(buffer, stages, access).unwrap();
  passes_.last().buffers++;
}

void AccessEncoder::access(Image * image, VkPipelineStageFlags stages,
                           VkAccessFlags access, VkImageLayout layout)
{
  images_.push(image, stages, access, layout).unwrap();
  passes_.last().images++;
}

void AccessEncoder::access(ImageView * image, VkPipelineStageFlags stages,
                           VkAccessFlags access, VkImageLayout layout)
{
  this->access(image->image, stages, access, layout);
}

void AccessEncoder::access(DescriptorSet * set, VkShaderStageFlags stages,
                           VkAccessFlags access)
{
  descriptor_sets_.push(set, stages, access).unwrap();
  passes_.last().descriptor_sets++;
}

constexpr bool is_image_view_type_compatible(gpu::ImageType     image_type,
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

constexpr u64 index_type_size(gpu::IndexType type)
{
  switch (type)
  {
    case gpu::IndexType::U16:
      return 2;
    case gpu::IndexType::U32:
      return 4;
    default:
      CHECK_UNREACHABLE();
  }
}

constexpr bool is_valid_buffer_access(u64 size, Slice64 access_range,
                                      u64 offset_alignment, u64 size_alignment)
{
  access_range.span = access_range.span == U64_MAX ?
                        (size - access_range.offset) :
                        access_range.span;

  bool is_valid_offset = access_range.offset < size;
  bool is_valid_span   = access_range.span > 0;
  bool is_valid_end    = access_range.end() <= size;

  return is_valid_offset && is_valid_span && is_valid_end &&
         is_aligned(offset_alignment, access_range.offset) &&
         is_aligned(size_alignment, access_range.span);
}

constexpr bool is_valid_image_access(gpu::ImageAspects aspects,
                                     u32 num_mip_levels, u32 num_array_layers,
                                     gpu::ImageAspects access_aspects,
                                     Slice32           access_mip_levels,
                                     Slice32           access_array_layers)
{
  access_mip_levels.span   = access_mip_levels.span == U32_MAX ?
                               (num_mip_levels - access_mip_levels.offset) :
                               access_mip_levels.span;
  access_array_layers.span = access_array_layers.span == U32_MAX ?
                               (num_mip_levels - access_array_layers.offset) :
                               access_array_layers.span;

  bool is_valid_offset = access_mip_levels.offset < num_mip_levels &&
                         access_array_layers.offset < num_array_layers;
  bool is_valid_span =
    access_mip_levels.span > 0 && access_array_layers.span > 0;
  bool is_valid_end = access_mip_levels.end() <= num_mip_levels &&
                      access_array_layers.end() <= num_array_layers;

  return is_valid_offset && is_valid_span && is_valid_end &&
         has_bits(aspects, access_aspects) &&
         access_aspects != gpu::ImageAspects::None;
}

static VkBool32 VKAPI_ATTR VKAPI_CALL
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT       message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT              message_type,
                 VkDebugUtilsMessengerCallbackDataEXT const * data,
                 [[maybe_unused]] void *                      pUserData)
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

Result<Dyn<gpu::Instance *>, Status> create_instance(AllocatorRef allocator,
                                                     bool enable_validation)
{
  u32  num_exts;
  auto result =
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
    .pUserData       = nullptr};

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

  Result instance =
    dyn<Instance>(inplace, allocator, allocator, vk_table, vk_instance,
                  vk_debug_messenger, validation_enabled);

  if (!instance)
  {
    return Err{Status::OutOfHostMemory};
  }

  vk_instance = nullptr;

  return Ok{cast<gpu::Instance *>(std::move(instance.v()))};
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
  if (vk_instance_ == nullptr)
  {
    return;
  }

  if (validation_enabled_)
  {
    vk_table_.DestroyDebugUtilsMessengerEXT(vk_instance_, vk_debug_messenger_,
                                            nullptr);
  }
  vk_table_.DestroyInstance(vk_instance_, nullptr);
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
                          Span<gpu::DeviceType const> preferred_types)
{
  constexpr u32 MAX_QUEUE_FAMILIES = 16;

  // CHECK(buffering > 0, "");
  // CHECK(buffering <= gpu::MAX_FRAME_BUFFERING, "");

  u32  num_devs;
  auto result =
    vk_table_.EnumeratePhysicalDevices(vk_instance_, &num_devs, nullptr);

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
    result = vk_table_.EnumeratePhysicalDevices(vk_instance_, &num_read_devs,
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
    vk_table_.GetPhysicalDeviceFeatures(vk_dev, &dev.vk_features);
    vk_table_.GetPhysicalDeviceMemoryProperties(vk_dev,
                                                &dev.vk_memory_properties);
    vk_table_.GetPhysicalDeviceProperties(vk_dev, &dev.vk_properties);
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
    vk_table_.GetPhysicalDeviceQueueFamilyProperties(
      dev.vk_phy_dev, &num_queue_families, nullptr);

    CHECK(num_queue_families <= MAX_QUEUE_FAMILIES, "");

    VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

    {
      u32 num_read_queue_families = num_queue_families;
      vk_table_.GetPhysicalDeviceQueueFamilyProperties(
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
      vk_table_.GetPhysicalDeviceQueueFamilyProperties(
        dev.vk_phy_dev, &num_queue_families, nullptr);

      CHECK(num_queue_families <= MAX_QUEUE_FAMILIES, "");

      VkQueueFamilyProperties queue_family_properties[MAX_QUEUE_FAMILIES];

      {
        u32 num_read_queue_families = num_queue_families;
        vk_table_.GetPhysicalDeviceQueueFamilyProperties(
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
  result = vk_table_.EnumerateDeviceExtensionProperties(
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
    result            = vk_table_.EnumerateDeviceExtensionProperties(
      selected_dev.vk_phy_dev, nullptr, &num_read_exts, exts);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }
    CHECK(num_exts == num_read_exts, "");
  }

  u32 num_layers;
  result = vk_table_.EnumerateDeviceLayerProperties(selected_dev.vk_phy_dev,
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
    result = vk_table_.EnumerateDeviceLayerProperties(selected_dev.vk_phy_dev,
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
  if (vk_debug_messenger_ != nullptr && has_validation_layer)
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
    .fragmentStoresAndAtomics       = VK_TRUE,
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

  VkPhysicalDeviceExtendedDynamicStateFeaturesEXT
    extended_dynamic_state_features{
      .sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
      .pNext                = nullptr,
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
  result = vk_table_.CreateDevice(selected_dev.vk_phy_dev, &create_info,
                                  nullptr, &vk_dev);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  DeviceTable        vk_dev_table;
  VmaVulkanFunctions vma_table;
  CHECK(
    load_device_table(vk_dev, vk_table_, vk_dev_table, has_debug_marker_ext),
    "");

  load_vma_table(vk_table_, vk_dev_table, vma_table);

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
    .instance                       = vk_instance_,
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

  new (dev) Device{allocator,
                   this,
                   selected_dev,
                   vk_dev_table,
                   vma_table,
                   vk_dev,
                   selected_queue_family,
                   vk_queue,
                   vma_allocator,
                   Vec<u8>{allocator}};

  vma_allocator = nullptr;
  vk_dev        = nullptr;
  dev           = nullptr;

  return Ok<gpu::Device *>{dev};
}

gpu::Backend Instance::get_backend()
{
  return gpu::Backend::Vulkan;
}

void Instance::uninit(gpu::Device * device_)
{
  auto * dev = (Device *) device_;

  if (dev == nullptr)
  {
    return;
  }

  dev->uninit();
  vmaDestroyAllocator(dev->vma_allocator_);
  dev->vk_table_.DestroyDevice(dev->vk_dev_, nullptr);
  allocator_->ndealloc(1, dev);
}

void Instance::uninit(gpu::Surface surface)
{
  vk_table_.DestroySurfaceKHR(vk_instance_, (Surface) surface, nullptr);
}

void Device::set_resource_name(Str label, void const * resource,
                               VkObjectType               type,
                               VkDebugReportObjectTypeEXT debug_type)
{
  SmallVec<char, 512> label_c_str{allocator_};

  label_c_str.extend(label).unwrap();
  label_c_str.push('\0').unwrap();

  VkDebugUtilsObjectNameInfoEXT name_info{
    .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
    .pNext        = nullptr,
    .objectType   = type,
    .objectHandle = (u64) resource,
    .pObjectName  = label_c_str.data()};

  instance_->vk_table_.SetDebugUtilsObjectNameEXT(vk_dev_, &name_info);
  VkDebugMarkerObjectNameInfoEXT debug_info{
    .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
    .pNext       = nullptr,
    .objectType  = debug_type,
    .object      = (u64) resource,
    .pObjectName = label_c_str.data()};

  vk_table_.DebugMarkerSetObjectNameEXT(vk_dev_, &debug_info);
}

gpu::DeviceProperties Device::get_properties()
{
  VkPhysicalDeviceFeatures const &   vk_features   = phy_dev_.vk_features;
  VkPhysicalDeviceProperties const & vk_properties = phy_dev_.vk_properties;

  bool has_uma = false;
  for (u32 i = 0; i < phy_dev_.vk_memory_properties.memoryTypeCount; i++)
  {
    if (has_bits(phy_dev_.vk_memory_properties.memoryTypes[i].propertyFlags,
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
  instance_->vk_table_.GetPhysicalDeviceFormatProperties(
    phy_dev_.vk_phy_dev, (VkFormat) format, &props);
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

  VkBuffer vk;

  auto result = vk_table_.CreateBuffer(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT);

  Buffer * buffer;
  if (!allocator_->nalloc(1, buffer))
  {
    vk_table_.DestroyBuffer(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (buffer) Buffer{
    .vk             = vk,
    .usage          = info.usage,
    .host_mapped    = info.host_mapped,
    .size           = info.size,
    .memory         = MemoryInfo{.memory_group = nullptr,
                                 .alias        = 0,
                                 .element      = 0,
                                 .type         = gpu::MemoryType::Unique},
    .bind_locations = BindLocations{allocator_}
  };

  if (info.memory_type == gpu::MemoryType::Unique)
  {
    Enum<gpu::Buffer, gpu::Image> const resources[] = {(gpu::Buffer) buffer};
    u32 const                           aliases[]   = {0, 1};

    auto status = create_memory_group(
      gpu::MemoryGroupInfo{.resources = resources, .aliases = aliases});

    if (!status)
    {
      uninit((gpu::Buffer) buffer);
      return Err{status.err()};
    }
  }

  return Ok{(gpu::Buffer) buffer};
}

Result<gpu::BufferView, Status>
  Device::create_buffer_view(gpu::BufferViewInfo const & info)
{
  auto * buffer = (Buffer *) info.buffer;

  CHECK(buffer != nullptr, "");
  CHECK(has_any_bit(buffer->usage, gpu::BufferUsage::UniformTexelBuffer |
                                     gpu::BufferUsage::StorageTexelBuffer),
        "");
  CHECK(info.format != gpu::Format::Undefined, "");
  CHECK(is_valid_buffer_access(buffer->size, info.slice, 1, 1), "");

  auto slice = info.slice(buffer->size);

  VkBufferViewCreateInfo create_info{
    .sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
    .pNext  = nullptr,
    .flags  = 0,
    .buffer = buffer->vk,
    .format = (VkFormat) info.format,
    .offset = slice.offset,
    .range  = slice.span};

  VkBufferView vk;

  auto result = vk_table_.CreateBufferView(vk_dev_, &create_info, nullptr, &vk);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_BUFFER_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT);

  BufferView * view;

  if (!allocator_->nalloc(1, view))
  {
    vk_table_.DestroyBufferView(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) BufferView{.vk             = vk,
                        .buffer         = buffer,
                        .slice          = slice,
                        .bind_locations = BindLocations{allocator_}};

  return Ok{(gpu::BufferView) view};
}

Result<gpu::Image, Status> Device::create_image(gpu::ImageInfo const & info)
{
  CHECK(info.format != gpu::Format::Undefined, "");
  CHECK(info.usage != gpu::ImageUsage::None, "");
  CHECK(info.aspects != gpu::ImageAspects::None, "");
  CHECK(info.sample_count != gpu::SampleCount::None, "");
  CHECK(!info.extent.any_zero(), "");
  CHECK(info.mip_levels > 0, "");
  CHECK(info.mip_levels <= info.extent.mips(), "");
  CHECK(info.array_layers > 0, "");
  CHECK(info.array_layers <= gpu::MAX_IMAGE_ARRAY_LAYERS, "");

  switch (info.type)
  {
    case gpu::ImageType::Type1D:
      CHECK(info.extent.x() <= gpu::MAX_IMAGE_EXTENT_1D, "");
      CHECK(info.extent.y() == 1, "");
      CHECK(info.extent.z() == 1, "");
      break;

    case gpu::ImageType::Type2D:
      CHECK(info.extent.x() <= gpu::MAX_IMAGE_EXTENT_2D, "");
      CHECK(info.extent.y() <= gpu::MAX_IMAGE_EXTENT_2D, "");
      CHECK(info.extent.z() == 1, "");
      break;

    case gpu::ImageType::Type3D:
      CHECK(info.extent.x() <= gpu::MAX_IMAGE_EXTENT_3D, "");
      CHECK(info.extent.y() <= gpu::MAX_IMAGE_EXTENT_3D, "");
      CHECK(info.extent.z() <= gpu::MAX_IMAGE_EXTENT_3D, "");
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
    .extent                = VkExtent3D{.width  = info.extent.x(),
                                        .height = info.extent.y(),
                                        .depth  = info.extent.z()},
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

  VkImage vk;

  auto result = vk_table_.CreateImage(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_IMAGE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);

  Image * image;

  if (!allocator_->nalloc(1, image))
  {
    vk_table_.DestroyImage(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (image) Image{
    .vk                 = vk,
    .type               = info.type,
    .usage              = info.usage,
    .aspects            = info.aspects,
    .sample_count       = info.sample_count,
    .extent             = info.extent,
    .mip_levels         = info.mip_levels,
    .array_layers       = info.array_layers,
    .is_swapchain_image = false,
    .memory             = MemoryInfo{.memory_group = nullptr,
                                     .alias        = 0,
                                     .element      = 0,
                                     .type         = info.memory_type}
  };

  if (info.memory_type == gpu::MemoryType::Unique)
  {
    Enum<gpu::Buffer, gpu::Image> const resources[] = {(gpu::Image) image};
    u32 const                           aliases[]   = {0, 1};

    auto status = create_memory_group(
      gpu::MemoryGroupInfo{.resources = resources, .aliases = aliases});

    if (!status)
    {
      uninit((gpu::Image) image);
      return Err{status.err()};
    }
  }

  return Ok{(gpu::Image) image};
}

Result<gpu::ImageView, Status>
  Device::create_image_view(gpu::ImageViewInfo const & info)
{
  auto * src_image = (Image *) info.image;

  CHECK(info.image != nullptr, "");
  CHECK(info.view_format != gpu::Format::Undefined, "");
  CHECK(is_image_view_type_compatible(src_image->type, info.view_type), "");
  CHECK(is_valid_image_access(src_image->aspects, src_image->mip_levels,
                              src_image->array_layers, info.aspects,
                              info.mip_levels, info.array_layers),
        "");

  auto mip_levels   = info.mip_levels(src_image->mip_levels);
  auto array_layers = info.array_layers(src_image->array_layers);

  VkImageViewCreateInfo create_info{
    .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext      = nullptr,
    .flags      = 0,
    .image      = src_image->vk,
    .viewType   = (VkImageViewType) info.view_type,
    .format     = (VkFormat) info.view_format,
    .components = VkComponentMapping{.r = (VkComponentSwizzle) info.mapping.r,
                                     .g = (VkComponentSwizzle) info.mapping.g,
                                     .b = (VkComponentSwizzle) info.mapping.b,
                                     .a = (VkComponentSwizzle) info.mapping.a},
    .subresourceRange =
      VkImageSubresourceRange{.aspectMask   = (VkImageAspectFlags) info.aspects,
                                     .baseMipLevel = mip_levels.offset,
                                     .levelCount   = mip_levels.span,
                                     .baseArrayLayer = array_layers.offset,
                                     .layerCount     = array_layers.span}
  };

  VkImageView vk;
  auto result = vk_table_.CreateImageView(vk_dev_, &create_info, nullptr, &vk);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_IMAGE_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT);

  ImageView * view;
  if (!allocator_->nalloc(1, view))
  {
    vk_table_.DestroyImageView(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) ImageView{.vk             = vk,
                       .image          = src_image,
                       .format         = info.view_format,
                       .mip_levels     = mip_levels,
                       .array_layers   = array_layers,
                       .bind_locations = BindLocations{allocator_}};

  return Ok{(gpu::ImageView) view};
}

Result<gpu::MemoryGroup, Status>
  Device::create_memory_group(gpu::MemoryGroupInfo const & info)
{
  CHECK(info.aliases.size() > 1, "");
  auto num_aliases = size32(info.aliases) - 1;

  auto min_alias_offset_alignment =
    max(phy_dev_.vk_properties.limits.bufferImageGranularity,
        phy_dev_.vk_properties.limits.nonCoherentAtomSize,
        phy_dev_.vk_properties.limits.minMemoryMapAlignment,
        phy_dev_.vk_properties.limits.minStorageBufferOffsetAlignment,
        phy_dev_.vk_properties.limits.minTexelBufferOffsetAlignment,
        phy_dev_.vk_properties.limits.minUniformBufferOffsetAlignment);

  for (auto i : range(num_aliases))
  {
    auto alias_begin = info.aliases[i];
    auto alias_end   = info.aliases[i + 1];

    for (auto & resource :
         info.resources.slice(Slice::range(alias_begin, alias_end)))
    {
      resource.match(
        [&](gpu::Buffer p) {
          auto buffer = (Buffer *) p;
          CHECK(buffer->memory.memory_group == nullptr, "");
        },
        [&](gpu::Image p) {
          auto image = (Image *) p;
          CHECK(image->memory.memory_group == nullptr, "");
        });
    }
  }

  Layout64 group_layout{};
  u32      group_memory_type_bits = 0;
  bool     host_mapped            = false;

  SmallVec<u64> alias_offsets{allocator_};

  alias_offsets.push((u64) 0).unwrap();

  for (auto i : range(num_aliases))
  {
    Layout64 alias_layout{};
    u32      alias_memory_type_bits = 0;

    auto alias_begin       = info.aliases[i];
    auto alias_end         = info.aliases[i + 1];
    auto alias_host_mapped = false;

    for (auto & resource :
         info.resources.slice(Slice::range(alias_begin, alias_end)))
    {
      VkMemoryRequirements req{};
      resource.match(
        [&](gpu::Buffer p) {
          auto buffer = ptr(p);
          vk_table_.GetBufferMemoryRequirements(vk_dev_, buffer->vk, &req);
          alias_host_mapped = alias_host_mapped || buffer->host_mapped;
        },
        [&](gpu::Image p) {
          auto image = ptr(p);
          vk_table_.GetImageMemoryRequirements(vk_dev_, image->vk, &req);
        });

      alias_layout = alias_layout.unioned(
        Layout64{.alignment = req.alignment, .size = req.size});

      alias_memory_type_bits |= req.memoryTypeBits;
    }

    alias_layout.alignment =
      max(alias_layout.alignment, min_alias_offset_alignment);
    group_layout = group_layout.append(alias_layout.aligned());
    group_memory_type_bits |= alias_memory_type_bits;
    alias_offsets.push(group_layout.size).unwrap();
    host_mapped = host_mapped || alias_host_mapped;
  }

  group_layout = group_layout.aligned();

  VmaAllocationCreateFlags flags =
    host_mapped ? (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                   VMA_ALLOCATION_CREATE_MAPPED_BIT) :
                  0;

  VmaAllocationCreateInfo alloc_create_info{
    .flags          = flags,
    .usage          = VMA_MEMORY_USAGE_AUTO,
    .requiredFlags  = {},
    .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    .memoryTypeBits = group_memory_type_bits,
    .pool           = nullptr,
    .pUserData      = nullptr,
    .priority       = 0};

  VmaAllocationInfo    vma_allocation_info;
  VmaAllocation        vma_allocation;
  VkMemoryRequirements group_requirements{.size      = group_layout.size,
                                          .alignment = group_layout.alignment,
                                          .memoryTypeBits =
                                            group_memory_type_bits};

  auto result =
    vmaAllocateMemory(vma_allocator_, &group_requirements, &alloc_create_info,
                      &vma_allocation, &vma_allocation_info);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  CHECK(!(host_mapped && (vma_allocation_info.pMappedData == nullptr)), "");

  SmallVec<AliasId> alias_ids{allocator_};

  for (auto i : range(num_aliases))
  {
    auto first_alias  = info.aliases[i];
    auto alias_end    = info.aliases[i + 1];
    auto alias_offset = alias_offsets[i];

    for (auto [ialias, resource] :
         enumerate(info.resources.slice(Slice::range(first_alias, alias_end))))
    {
      VkResult result = VK_SUCCESS;
      resource.match(
        [&](gpu::Buffer p) {
          auto buffer = ptr(p);
          result      = vmaBindBufferMemory2(vma_allocator_, vma_allocation,
                                             alias_offset, buffer->vk, nullptr);
          buffer->memory.alias   = i;
          buffer->memory.element = ialias;
        },
        [&](gpu::Image p) {
          auto image = ptr(p);
          result     = vmaBindImageMemory2(vma_allocator_, vma_allocation,
                                           alias_offset, image->vk, nullptr);
          image->memory.alias   = i;
          image->memory.element = ialias;
        });

      CHECK(result == VK_SUCCESS, "");
    }

    alias_ids.push(allocate_alias_id()).unwrap();
  }

  MemoryGroup * group;

  CHECK(allocator_->nalloc(1, group), "");

  // [ ] add clear_memory_group and rebind_group

  new (group) MemoryGroup{.vma_allocation = vma_allocation,
                          .alignment      = 0,
                          .map            = vma_allocation_info.pMappedData,
                          .alias_offsets  = std::move(alias_offsets),
                          .alias_ids      = std::move(alias_ids)};

  return Ok{(gpu::MemoryGroup) group};
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
  auto      result =
    vk_table_.CreateSampler(vk_dev_, &create_info, nullptr, &vk_sampler);
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
  auto           result =
    vk_table_.CreateShaderModule(vk_dev_, &create_info, nullptr, &vk_shader);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_shader, VK_OBJECT_TYPE_SHADER_MODULE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT);

  return Ok{(gpu::Shader) vk_shader};
}

constexpr VkDescriptorType to_vk(gpu::DescriptorType type)
{
  switch (type)
  {
    case gpu::DescriptorType::Sampler:
      return VK_DESCRIPTOR_TYPE_SAMPLER;
    case gpu::DescriptorType::CombinedImageSampler:
      return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case gpu::DescriptorType::SampledImage:
      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case gpu::DescriptorType::StorageImage:
      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case gpu::DescriptorType::UniformTexelBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case gpu::DescriptorType::StorageTexelBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case gpu::DescriptorType::UniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case gpu::DescriptorType::ReadStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case gpu::DescriptorType::RWStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case gpu::DescriptorType::DynUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case gpu::DescriptorType::DynReadStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case gpu::DescriptorType::DynRWStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case gpu::DescriptorType::InputAttachment:
      return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    default:
      return VK_DESCRIPTOR_TYPE_MAX_ENUM;
  }
}

bool is_mutating_binding(gpu::DescriptorBindingInfo const & binding)
{
  auto access_flags = descriptor_access(binding.type);
  return has_write_access(access_flags);
}

bool is_mutating_set(Span<gpu::DescriptorBindingInfo const> bindings)
{
  return any_is(bindings, is_mutating_binding);
}

Result<gpu::DescriptorSetLayout, Status> Device::create_descriptor_set_layout(
  gpu::DescriptorSetLayoutInfo const & info)
{
  u32                                   num_descriptors     = 0;
  u32                                   num_variable_length = 0;
  Array<u32, gpu::NUM_DESCRIPTOR_TYPES> sizing              = {};

  for (gpu::DescriptorBindingInfo const & info : info.bindings)
  {
    num_descriptors += info.count;
    sizing[(u32) info.type] += info.count;
    num_variable_length += info.is_variable_length ? 1 : 0;
  }

  auto num_dynamic_read_storage_buffers =
    sizing[(u32) gpu::DescriptorType::DynReadStorageBuffer];
  auto num_dynamic_read_write_storage_buffers =
    sizing[(u32) gpu::DescriptorType::DynReadStorageBuffer];
  auto num_dynamic_uniform_buffers =
    sizing[(u32) gpu::DescriptorType::DynUniformBuffer];

  CHECK(info.bindings.size() > 0, "");
  CHECK(info.bindings.size() <= gpu::MAX_DESCRIPTOR_SET_BINDINGS, "");
  CHECK((num_dynamic_read_storage_buffers +
         num_dynamic_read_write_storage_buffers) <=
          gpu::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS,
        "");
  CHECK(num_dynamic_uniform_buffers <=
          gpu::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS,
        "");
  CHECK(num_descriptors <= gpu::MAX_DESCRIPTOR_SET_DESCRIPTORS, "");
  CHECK(num_variable_length <= 1, "");
  CHECK(
    !(num_variable_length > 0 && (num_dynamic_read_storage_buffers > 0 ||
                                  num_dynamic_read_write_storage_buffers > 0 ||
                                  num_dynamic_uniform_buffers > 0)),
    "Variable-length descriptor sets must not have dynamic offsets");

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
    auto stage_flags =
      (VkShaderStageFlags) (binding.type ==
                                gpu::DescriptorType::InputAttachment ?
                              VK_SHADER_STAGE_FRAGMENT_BIT :
                              VK_SHADER_STAGE_ALL);
    vk_bindings
      .push(VkDescriptorSetLayoutBinding{.binding         = i,
                                         .descriptorType  = to_vk(binding.type),
                                         .descriptorCount = binding.count,
                                         .stageFlags      = stage_flags,
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

  VkDescriptorSetLayout vk;
  auto                  result =
    vk_table_.CreateDescriptorSetLayout(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_{[&] {
    if (vk != nullptr)
    {
      vk_table_.DestroyDescriptorSetLayout(vk_dev_, vk, nullptr);
    }
  }};

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);

  DescriptorSetLayout * layout;
  if (!allocator_->nalloc(1, layout))
  {
    return Err{Status::OutOfHostMemory};
  }

  InplaceVec<gpu::DescriptorBindingInfo, gpu::MAX_DESCRIPTOR_SET_BINDINGS>
    bindings;
  bindings.extend(info.bindings).unwrap();

  auto is_mutating = is_mutating_set(info.bindings);

  new (layout) DescriptorSetLayout{.vk                  = vk,
                                   .bindings            = bindings,
                                   .num_variable_length = num_variable_length,
                                   .is_mutating         = is_mutating};

  vk = nullptr;

  return Ok{(gpu::DescriptorSetLayout) layout};
}

Result<gpu::DescriptorSet, Status>
  Device::create_descriptor_set(gpu::DescriptorSetInfo const & info)
{
  auto * layout = (DescriptorSetLayout *) info.layout;
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

  constexpr u32 NUM_VK_DESCRIPTOR_TYPES = 11;

  InplaceVec<u32, gpu::MAX_DESCRIPTOR_SET_BINDINGS> bindings_sizes;

  Array<u32, NUM_VK_DESCRIPTOR_TYPES> vk_type_count;

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

      vk_type_count[(u32) to_vk(binding.type)] += count;
      bindings_sizes.push(count).unwrap();
    }
  }

  InplaceVec<VkDescriptorPoolSize, gpu::NUM_DESCRIPTOR_TYPES> pool_sizes;

  for (auto [i, count] : enumerate<u32>(vk_type_count))
  {
    if (vk_type_count[i] == 0)
    {
      continue;
    }

    pool_sizes
      .push(VkDescriptorPoolSize{.type            = (VkDescriptorType) i,
                                 .descriptorCount = vk_type_count[i]})
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
  auto             result =
    vk_table_.CreateDescriptorPool(vk_dev_, &create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_pool_{
    [&] { vk_table_.DestroyDescriptorPool(vk_dev_, vk_pool, nullptr); }};

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
    .pSetLayouts        = &layout->vk};

  VkDescriptorSet vk;
  result = vk_table_.AllocateDescriptorSets(vk_dev_, &alloc_info, &vk);

  // must not have these errors
  CHECK(result != VK_ERROR_OUT_OF_POOL_MEMORY &&
          result != VK_ERROR_FRAGMENTED_POOL,
        "");

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_DESCRIPTOR_SET,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT);

  set_resource_name(info.label, vk_pool, VK_OBJECT_TYPE_DESCRIPTOR_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT);

  InplaceVec<DescriptorBinding, gpu::MAX_DESCRIPTOR_SET_BINDINGS> bindings;

  for (auto [size, info] : zip(bindings_sizes, layout->bindings))
  {
    bindings
      .push(DescriptorBinding{
        .sync_resources = none, .type = info.type, .count = size})
      .unwrap();
  }

  for (auto [binding, size] : zip(bindings, bindings_sizes))
  {
    auto sync_type = descriptor_sync_resource_type(binding.type);

    switch (sync_type)
    {
      case SyncResourceType::None:
        break;
      case SyncResourceType::Buffer:
      {
        binding.sync_resources =
          SmallVec<Buffer *, 4>::make(size, allocator_).unwrap();
      }
      break;
      case SyncResourceType::BufferView:
      {
        binding.sync_resources =
          SmallVec<BufferView *, 4>::make(size, allocator_).unwrap();
      }
      break;
      case SyncResourceType::ImageView:
      {
        binding.sync_resources =
          SmallVec<ImageView *, 4>::make(size, allocator_).unwrap();
      }
      break;
    }
  }

  DescriptorSet * set;

  if (!allocator_->nalloc(1, set))
  {
    return Err{(Status) result};
  }

  auto id = allocate_descriptor_set_id();

  new (set) DescriptorSet{.vk          = vk,
                          .vk_pool     = vk_pool,
                          .id          = id,
                          .is_mutating = layout->is_mutating,
                          .bindings    = std::move(bindings)};

  vk_pool = nullptr;
  vk      = nullptr;

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
  auto            result =
    vk_table_.CreatePipelineCache(vk_dev_, &create_info, nullptr, &vk_cache);
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
    vk_descriptor_set_layouts.push(((DescriptorSetLayout *) layout)->vk)
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
  auto result = vk_table_.CreatePipelineLayout(vk_dev_, &layout_create_info,
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

  VkPipeline vk;
  result = vk_table_.CreateComputePipelines(
    vk_dev_, info.cache == nullptr ? nullptr : (PipelineCache) info.cache, 1,
    &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    vk_table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
  set_resource_name(info.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);

  ComputePipeline * pipeline;
  if (!allocator_->nalloc(1, pipeline))
  {
    vk_table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    vk_table_.DestroyPipeline(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
    ComputePipeline{.vk                  = vk,
                    .vk_layout           = vk_layout,
                    .push_constants_size = info.push_constants_size,
                    .num_sets            = size32(info.descriptor_set_layouts)};

  return Ok{(gpu::ComputePipeline) pipeline};
}

Result<gpu::GraphicsPipeline, Status>
  Device::create_graphics_pipeline(gpu::GraphicsPipelineInfo const & info)
{
  CHECK(!(info.rasterization_state.polygon_mode != gpu::PolygonMode::Fill &&
          !phy_dev_.vk_features.fillModeNonSolid),
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
    vk_descriptor_set_layouts.push(((DescriptorSetLayout *) layout)->vk)
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

  auto result = vk_table_.CreatePipelineLayout(vk_dev_, &layout_create_info,
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
    .blendConstants  = {info.color_blend_state.blend_constant.x(),
                        info.color_blend_state.blend_constant.y(),
                        info.color_blend_state.blend_constant.z(),
                        info.color_blend_state.blend_constant.w()}
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

  VkPipeline vk;
  result = vk_table_.CreateGraphicsPipelines(
    vk_dev_, info.cache == nullptr ? nullptr : (PipelineCache) info.cache, 1,
    &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    vk_table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT);
  set_resource_name(info.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT);

  GraphicsPipeline * pipeline;
  if (!allocator_->nalloc(1, pipeline))
  {
    vk_table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    vk_table_.DestroyPipeline(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
    GraphicsPipeline{.vk                  = vk,
                     .vk_layout           = vk_layout,
                     .push_constants_size = info.push_constants_size,
                     .num_sets            = size32(info.descriptor_set_layouts),
                     .depth_fmt           = info.depth_format,
                     .stencil_fmt         = info.stencil_format,
                     .sample_count = info.rasterization_state.sample_count};

  pipeline->color_fmts.extend(info.color_formats).unwrap();

  return Ok{(gpu::GraphicsPipeline) pipeline};
}

Result<gpu::Swapchain, Status>
  Device::create_swapchain(gpu::SwapchainInfo const & info)
{
  CHECK(info.preferred_extent.x() > 0, "");
  CHECK(info.preferred_extent.y() > 0, "");
  CHECK(info.preferred_buffering <= gpu::MAX_SWAPCHAIN_IMAGES, "");

  auto vk_surface    = (VkSurfaceKHR) info.surface;
  auto old_swapchain = (Swapchain *) info.old;

  defer old_swapchain_{[&] {
    if (old_swapchain != nullptr)
    {
      uninit((gpu::Swapchain) old_swapchain);
    }
  }};

  VkSurfaceCapabilitiesKHR surface_capabilities;
  auto result = instance_->vk_table_.GetPhysicalDeviceSurfaceCapabilitiesKHR(
    phy_dev_.vk_phy_dev, vk_surface, &surface_capabilities);

  if (result != VK_SUCCESS)
  {
    old_swapchain = nullptr;
    return Err{(Status) result};
  }

  CHECK(has_bits(surface_capabilities.supportedUsageFlags,
                 (VkImageUsageFlags) info.usage),
        "");
  CHECK(has_bits(surface_capabilities.supportedCompositeAlpha,
                 (VkImageUsageFlags) info.composite_alpha),
        "");

  if (surface_capabilities.currentExtent.width == 0 ||
      surface_capabilities.currentExtent.height == 0)
  {
    uninit(info.old);

    Swapchain * swapchain;
    if (!allocator_->nalloc(1, swapchain))
    {
      return Err{Status::OutOfHostMemory};
    }

    new (swapchain) Swapchain{
      .vk              = nullptr,
      .vk_surface      = vk_surface,
      .images          = {},
      .current_image   = none,
      .is_out_of_date  = false,
      .is_optimal      = true,
      .is_zero_sized   = true,
      .format          = info.format,
      .usage           = info.usage,
      .present_mode    = info.present_mode,
      .extent          = {0, 0},
      .composite_alpha = info.composite_alpha
    };

    return Ok{(gpu::Swapchain) swapchain};
  }

  VkExtent2D vk_extent;

  if (surface_capabilities.currentExtent.width == 0xFFFF'FFFFU &&
      surface_capabilities.currentExtent.height == 0xFFFF'FFFFU)
  {
    vk_extent.width  = clamp(info.preferred_extent.x(),
                             surface_capabilities.minImageExtent.width,
                             surface_capabilities.maxImageExtent.width);
    vk_extent.height = clamp(info.preferred_extent.y(),
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
    min_image_count =
      clamp(info.preferred_buffering, surface_capabilities.minImageCount,
            surface_capabilities.maxImageCount);
  }
  else
  {
    min_image_count = max(min_image_count, surface_capabilities.minImageCount);
  }

  VkSwapchainCreateInfoKHR create_info{
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext                 = nullptr,
    .flags                 = 0,
    .surface               = vk_surface,
    .minImageCount         = min_image_count,
    .imageFormat           = (VkFormat) info.format.format,
    .imageColorSpace       = (VkColorSpaceKHR) info.format.color_space,
    .imageExtent           = vk_extent,
    .imageArrayLayers      = 1,
    .imageUsage            = (VkImageUsageFlags) info.usage,
    .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 1,
    .pQueueFamilyIndices   = nullptr,
    .preTransform          = surface_capabilities.currentTransform,
    .compositeAlpha        = (VkCompositeAlphaFlagBitsKHR) info.composite_alpha,
    .presentMode           = (VkPresentModeKHR) info.present_mode,
    .clipped               = VK_TRUE,
    .oldSwapchain = old_swapchain == nullptr ? nullptr : old_swapchain->vk};

  VkSwapchainKHR vk;

  result = vk_table_.CreateSwapchainKHR(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    old_swapchain = nullptr;
    return Err{(Status) result};
  }

  defer vk_{[&] {
    if (vk != nullptr)
    {
      vk_table_.DestroySwapchainKHR(vk_dev_, vk, nullptr);
    }
  }};

  u32 num_images;
  result = vk_table_.GetSwapchainImagesKHR(vk_dev_, vk, &num_images, nullptr);

  if (result != VK_SUCCESS)
  {
    old_swapchain = nullptr;
    return Err{(Status) result};
  }

  InplaceVec<Image *, gpu::MAX_SWAPCHAIN_IMAGES> images;
  images.resize(num_images).unwrap();
  InplaceVec<VkImage, gpu::MAX_SWAPCHAIN_IMAGES> vk_images;
  vk_images.resize(num_images).unwrap();

  result =
    vk_table_.GetSwapchainImagesKHR(vk_dev_, vk, &num_images, vk_images.data());

  if (result != VK_SUCCESS)
  {
    old_swapchain = nullptr;
    return Err{(Status) result};
  }

  for (auto [vk, image] : zip(vk_images, images))
  {
    CHECK(allocator_->nalloc(1, image), "");

    new (image) Image{
      .type               = gpu::ImageType::Type2D,
      .aspects            = gpu::ImageAspects::Color,
      .mip_levels         = 1,
      .array_layers       = 1,
      .is_swapchain_image = true,
      .memory             = MemoryInfo{.memory_group = nullptr,
                                       .alias        = 0,
                                       .element      = 0,
                                       .type         = gpu::MemoryType::Unique}
    };
  }

  auto swapchain_label =
    ssformat<256>(allocator_, "{}:Swapchain"_str, info.label).unwrap();

  set_resource_name(swapchain_label, vk, VK_OBJECT_TYPE_SWAPCHAIN_KHR,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT);
  for (auto [i, image] : enumerate(images))
  {
    auto label =
      ssformat<256>(allocator_, "{}:Swapchain.Image:{}"_str, info.label, i)
        .unwrap();
    set_resource_name(label, image->vk, VK_OBJECT_TYPE_IMAGE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT);
  }

  Swapchain * swapchain;
  if (!allocator_->nalloc(1, swapchain))
  {
    old_swapchain = nullptr;
    return Err{Status::OutOfHostMemory};
  }

  new (swapchain) Swapchain{
    .vk              = vk,
    .vk_surface      = vk_surface,
    .images          = std::move(images),
    .current_image   = none,
    .is_out_of_date  = false,
    .is_optimal      = true,
    .is_zero_sized   = false,
    .format          = info.format,
    .usage           = info.usage,
    .present_mode    = info.present_mode,
    .extent          = {vk_extent.width, vk_extent.height},
    .composite_alpha = info.composite_alpha
  };

  return Ok{(gpu::Swapchain) swapchain};
}

Result<gpu::TimestampQuery, Status>
  Device::create_timestamp_query(gpu::TimestampQueryInfo const & info)
{
  CHECK(info.count > 0, "");

  VkQueryPoolCreateInfo create_info{.sType =
                                      VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                                    .pNext      = nullptr,
                                    .flags      = 0,
                                    .queryType  = VK_QUERY_TYPE_TIMESTAMP,
                                    .queryCount = info.count,
                                    .pipelineStatistics = 0};
  VkQueryPool           vk_pool;
  auto                  result =
    vk_table_.CreateQueryPool(vk_dev_, &create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_pool, VK_OBJECT_TYPE_QUERY_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT);

  return Ok{(gpu::TimestampQuery) vk_pool};
}

Result<gpu::StatisticsQuery, Status>
  Device::create_statistics_query(gpu::StatisticsQueryInfo const & info)
{
  CHECK(info.count > 0, "");

  if (phy_dev_.vk_features.pipelineStatisticsQuery != VK_TRUE)
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
    .queryCount         = info.count,
    .pipelineStatistics = QUERY_STATS};

  VkQueryPool vk_pool;
  auto        result =
    vk_table_.CreateQueryPool(vk_dev_, &create_info, nullptr, &vk_pool);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_pool, VK_OBJECT_TYPE_QUERY_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT);

  return Ok{(gpu::StatisticsQuery) vk_pool};
}

Result<gpu::CommandEncoderPtr, Status>
  Device::create_command_encoder(gpu::CommandEncoderInfo const &)
{
  CommandEncoder * enc;

  if (!allocator_->nalloc(1, enc))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (enc) CommandEncoder{this, allocator_};

  return Ok{(gpu::CommandEncoderPtr) enc};
}

Result<gpu::CommandBufferPtr, Status>
  Device::create_command_buffer(gpu::CommandBufferInfo const & info)
{
  VkCommandPoolCreateInfo pool_create_info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = nullptr,
    .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = queue_family_};

  VkCommandPool vk_pool;
  auto          result =
    vk_table_.CreateCommandPool(vk_dev_, &pool_create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer pool_{[&] {
    if (vk_pool != nullptr)
    {
      vk_table_.DestroyCommandPool(vk_dev_, vk_pool, nullptr);
    }
  }};

  auto pool_label =
    ssformat<512>(allocator_, "{}:CommandPool"_str, info.label).unwrap();
  set_resource_name(pool_label, vk_pool, VK_OBJECT_TYPE_COMMAND_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT);

  VkCommandBufferAllocateInfo allocate_info{
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext              = nullptr,
    .commandPool        = vk_pool,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1};

  VkCommandBuffer vk;
  result = vk_table_.AllocateCommandBuffers(vk_dev_, &allocate_info, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer buff_{[&] {
    if (vk != nullptr)
    {
      vk_table_.FreeCommandBuffers(vk_dev_, vk_pool, 1, &vk);
    }
  }};

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_COMMAND_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT);

  CommandBuffer * buff;

  if (!allocator_->nalloc(1, buff))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (buff) CommandBuffer{this, vk_pool, vk, allocator_};

  vk_pool = nullptr;
  vk      = nullptr;

  return Ok{(gpu::CommandBufferPtr) buff};
}

Result<gpu::QueueScope, Status>
  Device::create_queue_scope(gpu::QueueScopeInfo const & info)
{
  SmallVec<VkSemaphore, gpu::MAX_FRAME_BUFFERING> acquire_semaphores{
    allocator_};
  SmallVec<VkSemaphore, gpu::MAX_FRAME_BUFFERING> submit_semaphores{allocator_};
  SmallVec<VkFence, gpu::MAX_FRAME_BUFFERING>     submit_fences{allocator_};

  defer _{[&] {
    for (auto sem : acquire_semaphores)
    {
      vk_table_.DestroySemaphore(vk_dev_, sem, nullptr);
    }

    for (auto sem : submit_semaphores)
    {
      vk_table_.DestroySemaphore(vk_dev_, sem, nullptr);
    }

    for (auto fence : submit_fences)
    {
      vk_table_.DestroyFence(vk_dev_, fence, nullptr);
    }
  }};

  VkSemaphoreCreateInfo sem_info{.sType =
                                   VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0};

  VkFenceCreateInfo fence_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                               .pNext = nullptr,
                               .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  for (auto i : range(info.buffering))
  {
    auto acq_sem_label =
      ssformat<256>(allocator_, "{}:QueueScope.AcquireSemaphore:{}"_str,
                    info.label, i)
        .unwrap();
    auto sbm_sem_label =
      ssformat<256>(allocator_, "{}:QueueScope.SubmitSemaphore:{}"_str,
                    info.label, i)
        .unwrap();
    auto sbm_fnc_label =
      ssformat<256>(allocator_, "{}:QueueScope.SubmitFence:{}"_str, info.label,
                    i)
        .unwrap();

    VkSemaphore acquire_sem;
    auto        result =
      vk_table_.CreateSemaphore(vk_dev_, &sem_info, nullptr, &acquire_sem);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    set_resource_name(acq_sem_label, acquire_sem, VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);

    acquire_semaphores.push(acquire_sem).unwrap();

    VkSemaphore submit_sem;

    result =
      vk_table_.CreateSemaphore(vk_dev_, &sem_info, nullptr, &submit_sem);

    set_resource_name(sbm_sem_label, submit_sem, VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT);

    submit_semaphores.push(submit_sem).unwrap();

    VkFence submit_fence;
    result =
      vk_table_.CreateFence(vk_dev_, &fence_info, nullptr, &submit_fence);

    set_resource_name(sbm_fnc_label, submit_fence, VK_OBJECT_TYPE_FENCE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT);

    submit_fences.push(submit_fence).unwrap();
  }

  QueueScope * scope;

  if (!allocator_->nalloc(1, scope))
  {
    return Err{Status::OutOfDeviceMemory};
  }

  new (scope)
    QueueScope{info.buffering, std::move(acquire_semaphores),
               std::move(submit_semaphores), std::move(submit_fences)};

  return Ok{(gpu::QueueScope) scope};
}

void Device::uninit()
{
}

void Device::uninit(gpu::Buffer buffer_)
{
  auto * buffer = (Buffer *) buffer_;

  if (buffer == nullptr)
  {
    return;
  }

  for (auto loc : buffer->bind_locations)
  {
    loc.set->bindings[loc.binding].sync_resources[v1][loc.element] =
      (Buffer *) nullptr;
  }

  if (buffer->memory.memory_group != nullptr)
  {
    switch (buffer->memory.type)
    {
      case gpu::MemoryType::Unique:
      {
        uninit((gpu::MemoryGroup) buffer->memory.memory_group);
      }
      break;
      case gpu::MemoryType::Group:
      {
      }
      break;
    }
  }

  vk_table_.DestroyBuffer(vk_dev_, buffer->vk, nullptr);
  buffer->~Buffer();
  allocator_->ndealloc(1, buffer);
}

void Device::uninit(gpu::BufferView buffer_view_)
{
  auto * buffer_view = (BufferView *) buffer_view_;

  if (buffer_view == nullptr)
  {
    return;
  }

  for (auto loc : buffer_view->bind_locations)
  {
    loc.set->bindings[loc.binding].sync_resources[v2][loc.element] =
      (BufferView *) nullptr;
  }

  vk_table_.DestroyBufferView(vk_dev_, buffer_view->vk, nullptr);
  buffer_view->~BufferView();
  allocator_->ndealloc(1, buffer_view);
}

void Device::uninit(gpu::Image image_)
{
  auto * image = (Image *) image_;

  if (image == nullptr)
  {
    return;
  }

  if (image->is_swapchain_image)
  {
    image->~Image();
    allocator_->ndealloc(1, image);
    return;
  }

  if (image->memory.memory_group != nullptr)
  {
    switch (image->memory.type)
    {
      case gpu::MemoryType::Unique:
      {
        uninit((gpu::MemoryGroup) image->memory.memory_group);
      }
      break;
      case gpu::MemoryType::Group:
      {
      }
      break;
    }
  }

  vk_table_.DestroyImage(vk_dev_, image->vk, nullptr);
  image->~Image();
  allocator_->ndealloc(1, image);
}

void Device::uninit(gpu::ImageView image_view_)
{
  auto * image_view = (ImageView *) image_view_;

  if (image_view == nullptr)
  {
    return;
  }

  for (auto loc : image_view->bind_locations)
  {
    loc.set->bindings[loc.binding].sync_resources[v3][loc.element] =
      (ImageView *) nullptr;
  }

  vk_table_.DestroyImageView(vk_dev_, image_view->vk, nullptr);
  image_view->~ImageView();
  allocator_->ndealloc(1, image_view);
}

void Device::uninit(gpu::MemoryGroup group_)
{
  auto * group = (MemoryGroup *) group_;

  if (group == nullptr)
  {
    return;
  }

  vmaFreeMemory(vma_allocator_, group->vma_allocation);

  for (auto alias_id : group->alias_ids)
  {
    release_alias_id(alias_id);
  }

  group->~MemoryGroup();
  allocator_->ndealloc(1, group);
}

void Device::uninit(gpu::Sampler sampler_)
{
  vk_table_.DestroySampler(vk_dev_, (Sampler) sampler_, nullptr);
}

void Device::uninit(gpu::Shader shader_)
{
  vk_table_.DestroyShaderModule(vk_dev_, (Shader) shader_, nullptr);
}

void Device::uninit(gpu::DescriptorSetLayout layout_)
{
  auto * layout = (DescriptorSetLayout *) layout_;

  if (layout == nullptr)
  {
    return;
  }

  vk_table_.DestroyDescriptorSetLayout(vk_dev_, layout->vk, nullptr);
  layout->~DescriptorSetLayout();
  allocator_->ndealloc(1, layout);
}

void Device::uninit(gpu::DescriptorSet set_)
{
  auto * set = (DescriptorSet *) set_;

  if (set == nullptr)
  {
    return;
  }

  for (auto [ibinding, binding] : enumerate<u32>(set->bindings))
  {
    binding.sync_resources.match(
      [&](None) {},
      [&](auto & buffers) {
        for (auto [i, buffer] : enumerate<u32>(buffers))
        {
          if (buffer != nullptr)
          {
            DescriptorSet::remove_bind_loc(
              buffer->bind_locations,
              BindLocation{.set = set, .binding = ibinding, .element = i});
          }
        }
      },
      [&](auto & buffer_views) {
        for (auto [i, buffer_view] : enumerate<u32>(buffer_views))
        {
          if (buffer_view != nullptr)
          {
            DescriptorSet::remove_bind_loc(
              buffer_view->bind_locations,
              BindLocation{.set = set, .binding = ibinding, .element = i});
          }
        }
      },
      [&](auto & image_views) {
        for (auto [i, image_view] : enumerate<u32>(image_views))
        {
          if (image_view != nullptr)
          {
            DescriptorSet::remove_bind_loc(
              image_view->bind_locations,
              BindLocation{.set = set, .binding = ibinding, .element = i});
          }
        }
      });
  }

  release_descriptor_set_id(set->id);
  vk_table_.FreeDescriptorSets(vk_dev_, set->vk_pool, 1, &set->vk);
  vk_table_.DestroyDescriptorPool(vk_dev_, set->vk_pool, nullptr);
  set->~DescriptorSet();
  allocator_->ndealloc(1, set);
}

void Device::uninit(gpu::PipelineCache cache_)
{
  vk_table_.DestroyPipelineCache(vk_dev_, (PipelineCache) cache_, nullptr);
}

void Device::uninit(gpu::ComputePipeline pipeline_)
{
  auto * pipeline = (ComputePipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  vk_table_.DestroyPipeline(vk_dev_, pipeline->vk, nullptr);
  vk_table_.DestroyPipelineLayout(vk_dev_, pipeline->vk_layout, nullptr);
  pipeline->~ComputePipeline();
  allocator_->ndealloc(1, pipeline);
}

void Device::uninit(gpu::GraphicsPipeline pipeline_)
{
  auto * pipeline = (GraphicsPipeline *) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  vk_table_.DestroyPipeline(vk_dev_, pipeline->vk, nullptr);
  vk_table_.DestroyPipelineLayout(vk_dev_, pipeline->vk_layout, nullptr);
  pipeline->~GraphicsPipeline();
  allocator_->ndealloc(1, pipeline);
}

void Device::uninit(gpu::Swapchain swapchain_)
{
  auto * swapchain = (Swapchain *) swapchain_;

  if (swapchain == nullptr)
  {
    return;
  }

  vk_table_.DestroySwapchainKHR(vk_dev_, swapchain->vk, nullptr);
  swapchain->~Swapchain();
  allocator_->ndealloc(1, swapchain);
}

void Device::uninit(gpu::TimestampQuery query_)
{
  auto vk_pool = (VkQueryPool) query_;

  vk_table_.DestroyQueryPool(vk_dev_, vk_pool, nullptr);
}

void Device::uninit(gpu::StatisticsQuery query_)
{
  auto vk_pool = (VkQueryPool) query_;

  vk_table_.DestroyQueryPool(vk_dev_, vk_pool, nullptr);
}

void Device::uninit(gpu::CommandEncoder * enc_)
{
  auto enc = (CommandEncoder *) enc_;

  if (enc == nullptr)
  {
    return;
  }

  enc->~CommandEncoder();
  allocator_->ndealloc(1, enc);
}

void Device::uninit(gpu::CommandBuffer * buff_)
{
  auto buff = (CommandBuffer *) buff_;
  if (buff == nullptr)
  {
    return;
  }

  vk_table_.FreeCommandBuffers(vk_dev_, buff->vk_pool_, 1, &buff->vk_);
  vk_table_.DestroyCommandPool(vk_dev_, buff->vk_pool_, nullptr);

  buff->~CommandBuffer();
  allocator_->ndealloc(1, buff);
}

void Device::uninit(gpu::QueueScope scope_)
{
  auto scope = (QueueScope *) scope_;

  if (scope == nullptr)
  {
    return;
  }

  for (auto sem : scope->acquire_semaphores_)
  {
    vk_table_.DestroySemaphore(vk_dev_, sem, nullptr);
  }

  for (auto sem : scope->submit_semaphores_)
  {
    vk_table_.DestroySemaphore(vk_dev_, sem, nullptr);
  }

  for (auto fence : scope->submit_fences_)
  {
    vk_table_.DestroyFence(vk_dev_, fence, nullptr);
  }

  scope->~QueueScope();
  allocator_->ndealloc(1, scope);
}

Result<Span<u8>, Status> Device::get_memory_map(gpu::Buffer buffer_)
{
  auto * buffer = (Buffer *) buffer_;

  CHECK(buffer->host_mapped, "");
  CHECK(buffer->memory.memory_group != nullptr, "");

  auto & mem       = buffer->memory;
  auto * group     = mem.memory_group;
  auto * alias_map = ((u8 *) group->map) + group->alias_offsets[mem.alias];

  return Ok{
    Span<u8>{alias_map, buffer->size}
  };
}

Result<Void, Status> Device::invalidate_mapped_memory(gpu::Buffer buffer_,
                                                      Slice64     range)
{
  auto * buffer = (Buffer *) buffer_;

  CHECK(buffer->host_mapped, "");
  CHECK(buffer->memory.memory_group != nullptr, "");

  range = range(buffer->size);

  auto * group        = buffer->memory.memory_group;
  auto   alias_offset = group->alias_offsets[buffer->memory.alias];

  auto result =
    vmaInvalidateAllocation(vma_allocator_, group->vma_allocation,
                            alias_offset + range.offset, range.span);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<Void, Status> Device::flush_mapped_memory(gpu::Buffer buffer_,
                                                 Slice64     range)
{
  auto * buffer = (Buffer *) buffer_;

  CHECK(buffer->host_mapped, "");
  CHECK(buffer->memory.memory_group != nullptr, "");

  range = range(buffer->size);

  auto * group        = buffer->memory.memory_group;
  auto   alias_offset = group->alias_offsets[buffer->memory.alias];

  auto result = vmaFlushAllocation(vma_allocator_, group->vma_allocation,
                                   alias_offset + range.offset, range.span);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<usize, Status> Device::get_pipeline_cache_size(gpu::PipelineCache cache)
{
  usize size;

  auto result = vk_table_.GetPipelineCacheData(vk_dev_, (PipelineCache) cache,
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

  auto result = vk_table_.GetPipelineCacheData(vk_dev_, (PipelineCache) cache,
                                               &size, nullptr);

  if (result == VK_SUCCESS)
  {
    return Ok{};
  }

  if (result != VK_INCOMPLETE)
  {
    return Err{(Status) result};
  }

  auto offset = out.size();

  if (!out.extend_uninit(size))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = vk_table_.GetPipelineCacheData(vk_dev_, (PipelineCache) cache, &size,
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
  auto num_srcs = size32(srcs);

  CHECK(num_srcs > 0, "");

  auto result = vk_table_.MergePipelineCaches(
    vk_dev_, (PipelineCache) dst, num_srcs, (PipelineCache *) srcs.data());

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

  auto * set = (DescriptorSet *) update.set;

  CHECK(update.binding < set->bindings.size(), "");
  auto & binding = set->bindings[update.binding];
  CHECK(update.first_element < binding.count, "");

  auto sync_type = descriptor_sync_resource_type(binding.type);

  switch (sync_type)
  {
    case SyncResourceType::None:
      break;
    case SyncResourceType::Buffer:
    {
      for (gpu::BufferBinding const & b : update.buffers)
      {
        auto * buffer = (Buffer *) b.buffer;
        if (buffer == nullptr)
        {
          continue;
        }
        CHECK(has_bits(buffer->usage, descriptor_buffer_usage(binding.type)),
              "");
        CHECK(is_valid_buffer_access(buffer->size, b.range,
                                     gpu::BUFFER_OFFSET_ALIGNMENT, 1),
              "");
      }
    }
    break;
    case SyncResourceType::BufferView:
    {
      for (gpu::BufferView v : update.texel_buffers)
      {
        auto * view = (BufferView *) v;
        if (view == nullptr)
        {
          continue;
        }
        CHECK(
          has_bits(view->buffer->usage, descriptor_buffer_usage(binding.type)),
          "");
      }
    }
    break;
    case SyncResourceType::ImageView:
    {
      for (gpu::ImageBinding const & b : update.images)
      {
        auto * view = (ImageView *) b.image_view;
        if (view == nullptr)
        {
          continue;
        }
        auto * image = (Image *) view->image;
        CHECK(has_bits(image->usage, descriptor_image_usage(binding.type)), "");
        CHECK(image->sample_count == gpu::SampleCount::C1, "");
      }
    }
    break;

    default:
      CHECK_UNREACHABLE();
  }

  Span<VkDescriptorImageInfo>  image_infos;
  Span<VkDescriptorBufferInfo> buffer_infos;
  Span<VkBufferView>           texel_buffer_views;

  scratch_
    .resize(max(sizeof(VkDescriptorBufferInfo), sizeof(VkBufferView),
                sizeof(VkDescriptorImageInfo)) *
            binding.count)
    .unwrap();

  switch (binding.type)
  {
    case gpu::DescriptorType::UniformBuffer:
    case gpu::DescriptorType::DynUniformBuffer:
    case gpu::DescriptorType::ReadStorageBuffer:
    case gpu::DescriptorType::DynReadStorageBuffer:
    case gpu::DescriptorType::RWStorageBuffer:
    case gpu::DescriptorType::DynRWStorageBuffer:
    {
      buffer_infos =
        Span{(VkDescriptorBufferInfo *) scratch_.data(), update.buffers.size()};
      for (auto [vk, b] : zip(buffer_infos, update.buffers))
      {
        auto * buffer = (Buffer *) b.buffer;
        auto   range  = b.range(buffer->size);
        vk = VkDescriptorBufferInfo{.buffer = (buffer == nullptr) ? nullptr :
                                                                    buffer->vk,
                                    .offset = range.offset,
                                    .range  = range.span};
      }
    }
    break;

    case gpu::DescriptorType::Sampler:
    case gpu::DescriptorType::SampledImage:
    case gpu::DescriptorType::CombinedImageSampler:
    case gpu::DescriptorType::StorageImage:
    case gpu::DescriptorType::InputAttachment:
    {
      image_infos =
        Span{(VkDescriptorImageInfo *) scratch_.data(), update.images.size()};
      for (auto [vk, b] : zip(image_infos, update.images))
      {
        auto * view    = (ImageView *) b.image_view;
        auto * sampler = (Sampler) b.sampler;
        vk             = VkDescriptorImageInfo{
                      .sampler     = sampler,
                      .imageView   = (view == nullptr) ? nullptr : view->vk,
                      .imageLayout = descriptor_image_layout(binding.type)};
      }
    }
    break;

    case gpu::DescriptorType::StorageTexelBuffer:
    case gpu::DescriptorType::UniformTexelBuffer:
    {
      texel_buffer_views =
        Span{(VkBufferView *) scratch_.data(), update.texel_buffers.size()};
      for (auto [vk, b] : zip(texel_buffer_views, update.texel_buffers))
      {
        auto * view = (BufferView *) b;
        vk          = (view == nullptr) ? nullptr : view->vk;
      }
    }
    break;

    default:
      CHECK_UNREACHABLE();
  }

  VkWriteDescriptorSet vk_write{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet          = set->vk,
                                .dstBinding      = update.binding,
                                .dstArrayElement = update.first_element,
                                .descriptorCount =
                                  max(size32(image_infos), size32(buffer_infos),
                                      size32(texel_buffer_views)),
                                .descriptorType   = to_vk(binding.type),
                                .pImageInfo       = image_infos.data(),
                                .pBufferInfo      = buffer_infos.data(),
                                .pTexelBufferView = texel_buffer_views.data()};

  vk_table_.UpdateDescriptorSets(vk_dev_, 1, &vk_write, 0, nullptr);

  scratch_.shrink().unwrap();
  scratch_.clear();

  switch (sync_type)
  {
    case SyncResourceType::Buffer:
    {
      set->update_link(update.binding, update.first_element, update.buffers);
      break;
    }
    break;
    case SyncResourceType::BufferView:
    {
      set->update_link(update.binding, update.first_element,
                       update.texel_buffers);
      break;
    }
    break;
    case SyncResourceType::ImageView:
    {
      set->update_link(update.binding, update.first_element, update.images);
      break;
    }
    break;
    default:
      CHECK_UNREACHABLE();
  }
}

gpu::QueueScopeState Device::get_queue_scope_state(gpu::QueueScope scope_)
{
  CHECK(scope_ != nullptr, "");
  auto scope = (QueueScope *) scope_;

  return gpu::QueueScopeState{.tail_frame    = scope->tail_frame_,
                              .current_frame = scope->current_frame_,
                              .ring_index    = scope->ring_index_,
                              .buffering     = scope->buffering_};
}

Result<Void, Status> Device::wait_idle()
{
  auto result = vk_table_.DeviceWaitIdle(vk_dev_);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<Void, Status> Device::wait_queue_idle()
{
  auto result = vk_table_.QueueWaitIdle(vk_queue_);
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
  auto surface = (VkSurfaceKHR) surface_;

  u32  num_supported;
  auto result = instance_->vk_table_.GetPhysicalDeviceSurfaceFormatsKHR(
    phy_dev_.vk_phy_dev, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkSurfaceFormatKHR * vk_formats;
  if (!allocator_->nalloc(num_supported, vk_formats))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_formats_{[&] { allocator_->ndealloc(num_supported, vk_formats); }};

  {
    u32 num_read = num_supported;
    result       = instance_->vk_table_.GetPhysicalDeviceSurfaceFormatsKHR(
      phy_dev_.vk_phy_dev, surface, &num_supported, vk_formats);

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

  u32  num_supported;
  auto result = instance_->vk_table_.GetPhysicalDeviceSurfacePresentModesKHR(
    phy_dev_.vk_phy_dev, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  VkPresentModeKHR * vk_present_modes;
  if (!allocator_->nalloc(num_supported, vk_present_modes))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer vk_present_modes_{
    [&] { allocator_->ndealloc(num_supported, vk_present_modes); }};

  {
    u32 num_read = num_supported;
    result       = instance_->vk_table_.GetPhysicalDeviceSurfacePresentModesKHR(
      phy_dev_.vk_phy_dev, surface, &num_supported, vk_present_modes);

    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
      return Err{(Status) result};
    }

    CHECK(num_read == num_supported && result != VK_INCOMPLETE, "");
  }

  auto offset = modes.size();

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
  auto                     surface = (VkSurfaceKHR) surface_;
  VkSurfaceCapabilitiesKHR capabilities;
  auto result = instance_->vk_table_.GetPhysicalDeviceSurfaceCapabilitiesKHR(
    phy_dev_.vk_phy_dev, surface, &capabilities);

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
  auto * swapchain = (Swapchain *) swapchain_;

  gpu::SwapchainState state{
    .extent        = swapchain->extent,
    .format        = swapchain->format,
    .images        = swapchain->images.view().reinterpret<gpu::Image>(),
    .current_image = swapchain->current_image};

  return Ok{state};
}

Result<Void, Status>
  Device::invalidate_swapchain(gpu::Swapchain             swapchain_,
                               gpu::SwapchainInfo const & info)
{
  // [ ] fix
  CHECK(info.preferred_extent.x() > 0, "");
  CHECK(info.preferred_extent.y() > 0, "");
  auto * swapchain      = (Swapchain *) swapchain_;
  swapchain->is_optimal = false;
  // swapchain->info        = info;
  return Ok{Void{}};
}

Result<Void, Status>
  Device::get_timestamp_query_result(gpu::TimestampQuery query_, Slice32 range,
                                     Vec<u64> & timestamps)
{
  if (range.is_empty())
  {
    return Ok{};
  }

  auto vk_pool = (VkQueryPool) query_;

  auto offset = timestamps.size();
  timestamps.extend_uninit(range.span).unwrap();

  auto result = vk_table_.GetQueryPoolResults(
    vk_dev_, vk_pool, range.offset, range.span, sizeof(u64) * range.span,
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
  if (phy_dev_.vk_features.pipelineStatisticsQuery != VK_TRUE)
  {
    return Err{Status::FeatureNotPresent};
  }

  if (range.is_empty())
  {
    return Ok{};
  }

  auto vk_pool = (VkQueryPool) query_;

  auto offset = statistics.size();
  statistics.extend_uninit(range.span).unwrap();

  auto result = vk_table_.GetQueryPoolResults(
    vk_dev_, vk_pool, range.offset, range.span,
    sizeof(gpu::PipelineStatistics) * range.span, statistics.data() + offset,
    sizeof(gpu::PipelineStatistics), VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    statistics.resize(offset).unwrap();
    return Err{(Status) result};
  }

  return Ok{};
}

Result<gpu::Swapchain, Status>
  Device::submit(Span<gpu::CommandBufferPtr const> buffers_,
                 gpu::QueueScope scope_, gpu::Swapchain swapchain_)
{
  char              reserved_[512];
  FallbackAllocator scratch{Arena::from(reserved_), allocator_};

  // [ ] implement
  CHECK(!buffers_.is_empty(), "");
  for (auto buffer_ : buffers_)
  {
    CHECK(buffer_ != nullptr, "");
    auto * buffer = (CommandBuffer *) buffer_;
    CHECK(buffer->state_ == CommandBufferState::Recorded, "");
  }
  CHECK(scope_ != nullptr, "");

  auto scope     = (QueueScope *) scope_;
  auto swapchain = (Swapchain *) swapchain_;

  auto submit_fence      = scope->submit_fences_[scope->ring_index_];
  auto submit_semaphore  = scope->submit_semaphores_[scope->ring_index_];
  auto acquire_semaphore = scope->acquire_semaphores_[scope->ring_index_];

  // wait to re-use sync primitives
  auto result =
    vk_table_.WaitForFences(vk_dev_, 1, &submit_fence, VK_TRUE, U64_MAX);

  CHECK(result == VK_SUCCESS, "");

  result = vk_table_.ResetFences(vk_dev_, 1, &submit_fence);

  CHECK(result == VK_SUCCESS, "");

  bool has_acquire = false;

  if (swapchain != nullptr)
  {
    if (swapchain->is_out_of_date || !swapchain->is_optimal ||
        swapchain->vk == nullptr)
    {
      // await all pending submitted operations on the device possibly using
      // the swapchain, to avoid destroying whilst in use
      result = vk_table_.QueueWaitIdle(vk_queue_);
      CHECK(result == VK_SUCCESS, "");

      // [ ] fix params
      swapchain =
        (Swapchain *) create_swapchain(
          gpu::SwapchainInfo{.surface = (gpu::Surface) swapchain->vk_surface,
                             .format  = swapchain->format,
                             .usage   = swapchain->usage,
                             .preferred_buffering = 0,
                             .present_mode        = swapchain->present_mode,
                             .preferred_extent    = {},
                             .composite_alpha     = swapchain->composite_alpha,
                             .old                 = (gpu::Swapchain) swapchain})
          .unwrap();
    }

    if (!swapchain->is_zero_sized)
    {
      u32 next_image;
      result = vk_table_.AcquireNextImageKHR(
        vk_dev_, swapchain->vk, U64_MAX,
        scope->acquire_semaphores_[scope->ring_index_], nullptr, &next_image);

      if (result == VK_SUBOPTIMAL_KHR)
      {
        swapchain->is_optimal = false;
      }
      else
      {
        CHECK(result == VK_SUCCESS, "");
      }

      has_acquire              = true;
      swapchain->current_image = next_image;
    }
  }

  auto is_presenting = swapchain != nullptr && !swapchain->is_out_of_date &&
                       !swapchain->is_zero_sized;

  // [ ] swapchain images neeed to be transitioned; add a present command to the commandbuffer
  //

  // if (was_acquired)
  // {
  //   // enc.access_image(swapchain->image_impls[swapchain->current_image],
  //   //                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_NONE,
  //   //                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  // }

  Vec<VkCommandBuffer> vk_buffers{scratch};

  for (auto buff_ : buffers_)
  {
    auto buff = (CommandBuffer *) buff_;
    vk_buffers.push(buff->vk_).unwrap();
  }

  VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  auto submit_info = VkSubmitInfo{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = nullptr,
    .waitSemaphoreCount   = has_acquire ? 1U : 0U,
    .pWaitSemaphores      = has_acquire ? &acquire_semaphore : nullptr,
    .pWaitDstStageMask    = has_acquire ? &wait_stages : nullptr,
    .commandBufferCount   = size32(vk_buffers),
    .pCommandBuffers      = vk_buffers.data(),
    .signalSemaphoreCount = is_presenting ? 1U : 0U,
    .pSignalSemaphores    = is_presenting ? &submit_semaphore : nullptr};

  result = vk_table_.QueueSubmit(vk_queue_, 1, &submit_info, submit_fence);

  CHECK(result == VK_SUCCESS, "");

  // ctx.swapchain = swapchain;
  // enc.state = CommandEncoderState::End;

  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands could have been executed.
  scope->current_frame_++;
  scope->tail_frame_ = (scope->current_frame_ < scope->buffering_) ?
                         0 :
                         (scope->current_frame_ - scope->buffering_);
  scope->ring_index_ = (scope->ring_index_ + 1) % scope->buffering_;

  if (is_presenting)
  {
    auto image = swapchain->current_image.unwrap();
    auto present_info =
      VkPresentInfoKHR{.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                       .pNext              = nullptr,
                       .waitSemaphoreCount = 1,
                       .pWaitSemaphores    = &submit_semaphore,
                       .swapchainCount     = 1,
                       .pSwapchains        = &swapchain->vk,
                       .pImageIndices      = &image,
                       .pResults           = nullptr};
    result = vk_table_.QueuePresentKHR(vk_queue_, &present_info);

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

  for (auto buffer_ : buffers_)
  {
    auto * buffer  = (CommandBuffer *) buffer_;
    buffer->state_ = CommandBufferState::Submitted;
  }
}

void CommandEncoder::begin()
{
  CHECK(state_ == CommandBufferState::Reset, "");
  state_ = CommandBufferState::Recording;
}

Status CommandEncoder::end()
{
  CHECK(state_ == CommandBufferState::Recording, "");
  state_ = CommandBufferState::Recorded;
  return status_;
}

void CommandEncoder::reset()
{
  CHECK(state_ == CommandBufferState::Reset ||
          state_ == CommandBufferState::Recorded ||
          state_ == CommandBufferState::Submitted,
        "");
  // [ ] implement shrink for pool
  pool_.reclaim();
  status_    = Status::Success;
  state_     = CommandBufferState::Reset;
  pass_      = Pass::None;
  ctx_       = {};
  first_cmd_ = nullptr;
  last_cmd_  = nullptr;
  passes_.shrink().unwrap();
  passes_.clear();
}

#define PRELUDE()                                           \
  CHECK(this->state_ == CommandBufferState::Recording, ""); \
  if (this->status_ != Status::Success)                     \
  {                                                         \
    return;                                                 \
  }

#define CMD(...)                             \
  auto * cmd = push(__VA_ARGS__);            \
  if (cmd == nullptr)                        \
  {                                          \
    this->status_ = Status::OutOfHostMemory; \
    return;                                  \
  }

void CommandEncoder::reset_timestamp_query(gpu::TimestampQuery query,
                                           Slice32             range)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(cmd::ResetTimestampQuery{.query = (VkQueryPool) query, .range = range});
}

void CommandEncoder::reset_statistics_query(gpu::StatisticsQuery query,
                                            Slice32              range)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(cmd::ResetStatisticsQuery{.query = (VkQueryPool) query, .range = range});
}

void CommandEncoder::write_timestamp(gpu::TimestampQuery query,
                                     gpu::PipelineStages stage, u32 index)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(cmd::WriteTimestamp{.query  = (VkQueryPool) query,
                          .stages = (VkPipelineStageFlagBits) stage,
                          .index  = index});
}

void CommandEncoder::begin_statistics(gpu::StatisticsQuery query, u32 index)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(cmd::BeginStatistics{.query = (VkQueryPool) query, .index = index});
}

void CommandEncoder::end_statistics(gpu::StatisticsQuery query, u32 index)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(cmd::EndStatistics{.query = (VkQueryPool) query, .index = index});
}

void CommandEncoder::begin_debug_marker(Str region_name_, f32x4 color)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  Vec<char> region_name{pool_};
  if (!region_name.extend_uninit(region_name_.size() + 1))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  mem::copy(region_name_, region_name.data());

  region_name.last() = '\0';

  VkDebugMarkerMarkerInfoEXT info{
    .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
    .pNext       = nullptr,
    .pMarkerName = region_name.data(),
    .color       = {color.x(), color.y(), color.z(), color.w()}
  };

  CMD(cmd::BeginDebugMarker{.info = info});

  region_name.leak();
}

void CommandEncoder::end_debug_marker()
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(cmd::EndDebugMarker{});
}

void CommandEncoder::fill_buffer(gpu::Buffer dst_, Slice64 range, u32 data)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  auto * dst = ptr(dst_);

  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->size, range, 4, 4), "");

  CMD(cmd::FillBuffer{.dst = dst->vk, .range = range, .data = data});

  access_.begin_pass();
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT);
  access_.end_pass();
}

void CommandEncoder::copy_buffer(gpu::Buffer src_, gpu::Buffer dst_,
                                 Span<gpu::BufferCopy const> copies_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!copies_.is_empty(), "");

  auto * src = (Buffer *) src_;
  auto * dst = (Buffer *) dst_;

  CHECK(has_bits(src->usage, gpu::BufferUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");

  for (auto & copy : copies_)
  {
    CHECK(is_valid_buffer_access(src->size, copy.src_range, 1, 1), "");
    CHECK(is_valid_buffer_access(
            dst->size, Slice64{copy.dst_offset, copy.src_range.span}, 1, 1),
          "");
  }

  Vec<VkBufferCopy> copies{pool_};

  if (!copies.resize_uninit(copies_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, copy] : zip(copies, copies_))
  {
    vk = VkBufferCopy{.srcOffset = copy.src_range.offset,
                      .dstOffset = copy.dst_offset,
                      .size      = copy.src_range.span};
  }

  CMD(cmd::CopyBuffer{.src = src->vk, .dst = dst->vk, .copies = copies});

  copies.leak();

  access_.begin_pass();
  access_.access(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT);
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT);
  access_.end_pass();
}

void CommandEncoder::update_buffer(Span<u8 const> src_, u64 dst_offset,
                                   gpu::Buffer dst_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(src_.size_bytes() <= gpu::MAX_UPDATE_BUFFER_SIZE, "");

  auto * dst       = (Buffer *) dst_;
  auto   copy_size = size64(src_);

  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->size, Slice64{dst_offset, copy_size}, 4, 4),
        "");

  Vec<u8> src{pool_};

  if (!src.extend(src_))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  CMD(cmd::UpdateBuffer{.src = src, .dst_offset = dst_offset, .dst = dst->vk});

  src.leak();

  access_.begin_pass();
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT);
  access_.end_pass();
}

void CommandEncoder::clear_color_image(
  gpu::Image dst_, gpu::Color value,
  Span<gpu::ImageSubresourceRange const> ranges_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!ranges_.is_empty(), "");

  auto * dst = (Image *) dst_;

  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");

  for (auto & range : ranges_)
  {
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, range.aspects,
                                range.mip_levels, range.array_layers),
          "");
  }

  Vec<VkImageSubresourceRange> ranges{pool_};

  if (!ranges.extend_uninit(ranges_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, range] : zip(ranges, ranges_))
  {
    vk =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.mip_levels.offset,
                              .levelCount     = range.mip_levels.span,
                              .baseArrayLayer = range.array_layers.offset,
                              .layerCount     = range.array_layers.span};
  }

  CMD(cmd::ClearColorImage{
    .dst        = dst->vk,
    .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .value  = {.uint32{value.u32[0], value.u32[1], value.u32[2], value.u32[3]}},
    .ranges = ranges});

  ranges.leak();

  access_.begin_pass();
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  access_.end_pass();
}

void CommandEncoder::clear_depth_stencil_image(
  gpu::Image dst_, gpu::DepthStencil value,
  Span<gpu::ImageSubresourceRange const> ranges_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!ranges_.is_empty(), "");

  auto * dst = (Image *) dst_;

  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");

  for (auto & range : ranges_)
  {
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, range.aspects,
                                range.mip_levels, range.array_layers),
          "");
  }

  Vec<VkImageSubresourceRange> ranges{pool_};

  if (!ranges.extend_uninit(ranges_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, range] : zip(ranges, ranges_))
  {
    vk =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.mip_levels.offset,
                              .levelCount     = range.mip_levels.span,
                              .baseArrayLayer = range.array_layers.offset,
                              .layerCount     = range.array_layers.span};
  }

  VkClearDepthStencilValue vk_depth_stencil{.depth   = value.depth,
                                            .stencil = value.stencil};

  CMD(cmd::ClearDepthStencilImage{.dst = dst->vk,
                                  .dst_layout =
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  .value  = vk_depth_stencil,
                                  .ranges = ranges});

  ranges.leak();

  access_.begin_pass();
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  access_.end_pass();
}

void CommandEncoder::copy_image(gpu::Image src_, gpu::Image dst_,
                                Span<gpu::ImageCopy const> copies_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!copies_.is_empty(), "");

  auto * src = (Image *) src_;
  auto * dst = (Image *) dst_;

  CHECK(has_bits(src->usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");

  for (auto & copy : copies_)
  {
    CHECK(is_valid_image_access(src->aspects, src->mip_levels,
                                src->array_layers, copy.src_layers.aspects,
                                Slice32{copy.src_layers.mip_level, 1},
                                copy.src_layers.array_layers),
          "");
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, copy.dst_layers.aspects,
                                Slice32{copy.dst_layers.mip_level, 1},
                                copy.dst_layers.array_layers),
          "");

    auto src_extent = src->extent.mip(copy.src_layers.mip_level);
    auto dst_extent = dst->extent.mip(copy.dst_layers.mip_level);
    CHECK(copy.src_area.extent.x() > 0, "");
    CHECK(copy.src_area.extent.y() > 0, "");
    CHECK(copy.src_area.extent.z() > 0, "");
    CHECK(copy.src_area.offset.x() <= src_extent.x(), "");
    CHECK(copy.src_area.offset.y() <= src_extent.y(), "");
    CHECK(copy.src_area.offset.z() <= src_extent.z(), "");
    CHECK(copy.src_area.end().x() <= src_extent.x(), "");
    CHECK(copy.src_area.end().y() <= src_extent.y(), "");
    CHECK(copy.src_area.end().z() <= src_extent.z(), "");
    CHECK(copy.dst_offset.x() <= dst_extent.x(), "");
    CHECK(copy.dst_offset.y() <= dst_extent.y(), "");
    CHECK(copy.dst_offset.z() <= dst_extent.z(), "");
    CHECK((copy.dst_offset.x() + copy.src_area.extent.x()) <= dst_extent.x(),
          "");
    CHECK((copy.dst_offset.y() + copy.src_area.extent.y()) <= dst_extent.y(),
          "");
    CHECK((copy.dst_offset.z() + copy.src_area.extent.z()) <= dst_extent.z(),
          "");
  }

  Vec<VkImageCopy> copies{pool_};

  if (!copies.extend_uninit(copies_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, copy] : zip(copies, copies_))
  {
    VkImageSubresourceLayers src_subresource{
      .aspectMask     = (VkImageAspectFlags) copy.src_layers.aspects,
      .mipLevel       = copy.src_layers.mip_level,
      .baseArrayLayer = copy.src_layers.array_layers.offset,
      .layerCount     = copy.src_layers.array_layers.span};
    VkOffset3D               src_offset{(i32) copy.src_area.offset.x(),
                          (i32) copy.src_area.offset.y(),
                          (i32) copy.src_area.offset.z()};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) copy.dst_layers.aspects,
      .mipLevel       = copy.dst_layers.mip_level,
      .baseArrayLayer = copy.dst_layers.array_layers.offset,
      .layerCount     = copy.dst_layers.array_layers.span};
    VkOffset3D dst_offset{(i32) copy.dst_offset.x(), (i32) copy.dst_offset.y(),
                          (i32) copy.dst_offset.z()};
    VkExtent3D extent{copy.src_area.extent.x(), copy.src_area.extent.y(),
                      copy.src_area.extent.z()};

    vk = VkImageCopy{.srcSubresource = src_subresource,
                     .srcOffset      = src_offset,
                     .dstSubresource = dst_subresource,
                     .dstOffset      = dst_offset,
                     .extent         = extent};
  }

  CMD(cmd::CopyImage{.src        = src->vk,
                     .src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     .dst        = dst->vk,
                     .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     .copies     = copies});

  copies.leak();

  access_.begin_pass();
  access_.access(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  access_.end_pass();
}

void CommandEncoder::copy_buffer_to_image(
  gpu::Buffer src_, gpu::Image dst_, Span<gpu::BufferImageCopy const> copies_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!copies_.is_empty() > 0, "");

  auto * src = (Buffer *) src_;
  auto * dst = (Image *) dst_;

  CHECK(has_bits(src->usage, gpu::BufferUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");

  for (auto & copy : copies_)
  {
    CHECK(is_valid_buffer_access(
            src->size, Slice64{copy.buffer_offset, gpu::WHOLE_SIZE}, 1, 1),
          "");

    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, copy.image_layers.aspects,
                                Slice32{copy.image_layers.mip_level, 1},
                                copy.image_layers.array_layers),
          "");

    CHECK(copy.image_area.extent.x() > 0, "");
    CHECK(copy.image_area.extent.y() > 0, "");
    CHECK(copy.image_area.extent.z() > 0, "");
    auto dst_extent = dst->extent.mip(copy.image_layers.mip_level);
    CHECK(copy.image_area.extent.x() <= dst_extent.x(), "");
    CHECK(copy.image_area.extent.y() <= dst_extent.y(), "");
    CHECK(copy.image_area.extent.z() <= dst_extent.z(), "");
    CHECK(copy.image_area.end().x() <= dst_extent.x(), "");
    CHECK(copy.image_area.end().y() <= dst_extent.y(), "");
    CHECK(copy.image_area.end().z() <= dst_extent.z(), "");
  }

  Vec<VkBufferImageCopy> copies{pool_};

  if (!copies.extend_uninit(copies_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, copy] : zip(copies, copies_))
  {
    VkImageSubresourceLayers image_subresource{
      .aspectMask     = (VkImageAspectFlags) copy.image_layers.aspects,
      .mipLevel       = copy.image_layers.mip_level,
      .baseArrayLayer = copy.image_layers.array_layers.offset,
      .layerCount     = copy.image_layers.array_layers.span};
    vk = VkBufferImageCopy{
      .bufferOffset      = copy.buffer_offset,
      .bufferRowLength   = copy.buffer_row_length,
      .bufferImageHeight = copy.buffer_image_height,
      .imageSubresource  = image_subresource,
      .imageOffset       = VkOffset3D{(i32) copy.image_area.offset.x(),
                                      (i32) copy.image_area.offset.y(),
                                      (i32) copy.image_area.offset.z()},
      .imageExtent =
        VkExtent3D{copy.image_area.extent.x(),       copy.image_area.extent.y(),
                                      copy.image_area.extent.z()      }
    };
  }

  CMD(cmd::CopyBufferToImage{.src        = src->vk,
                             .dst        = dst->vk,
                             .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             .copies     = copies});

  copies.leak();

  access_.begin_pass();
  access_.access(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT);
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  access_.end_pass();
}

void CommandEncoder::blit_image(gpu::Image src_, gpu::Image dst_,
                                Span<gpu::ImageBlit const> blits_,
                                gpu::Filter                filter)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!blits_.is_empty(), "");

  auto * src = (Image *) src_;
  auto * dst = (Image *) dst_;

  CHECK(has_bits(src->usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");

  for (auto & blit : blits_)
  {
    CHECK(is_valid_image_access(src->aspects, src->mip_levels,
                                src->array_layers, blit.src_layers.aspects,
                                Slice32{blit.src_layers.mip_level, 1},
                                blit.src_layers.array_layers),
          "");

    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, blit.dst_layers.aspects,
                                Slice32{blit.dst_layers.mip_level, 1},
                                blit.dst_layers.array_layers),
          "");

    auto src_extent = src->extent.mip(blit.src_layers.mip_level);
    auto dst_extent = dst->extent.mip(blit.dst_layers.mip_level);
    CHECK(blit.src_area.offset.x() <= src_extent.x(), "");
    CHECK(blit.src_area.offset.y() <= src_extent.y(), "");
    CHECK(blit.src_area.offset.z() <= src_extent.z(), "");
    CHECK(blit.src_area.end().x() <= src_extent.x(), "");
    CHECK(blit.src_area.end().y() <= src_extent.y(), "");
    CHECK(blit.src_area.end().z() <= src_extent.z(), "");
    CHECK(blit.dst_area.offset.x() <= dst_extent.x(), "");
    CHECK(blit.dst_area.offset.y() <= dst_extent.y(), "");
    CHECK(blit.dst_area.offset.z() <= dst_extent.z(), "");
    CHECK(blit.dst_area.end().x() <= dst_extent.x(), "");
    CHECK(blit.dst_area.end().y() <= dst_extent.y(), "");
    CHECK(blit.dst_area.end().z() <= dst_extent.z(), "");
  }

  Vec<VkImageBlit> blits{pool_};

  if (!blits.resize_uninit(blits_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, blit] : zip(blits, blits_))
  {
    VkImageSubresourceLayers src_subresource{
      .aspectMask     = (VkImageAspectFlags) blit.src_layers.aspects,
      .mipLevel       = blit.src_layers.mip_level,
      .baseArrayLayer = blit.src_layers.array_layers.offset,
      .layerCount     = blit.src_layers.array_layers.span};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) blit.dst_layers.aspects,
      .mipLevel       = blit.dst_layers.mip_level,
      .baseArrayLayer = blit.dst_layers.array_layers.offset,
      .layerCount     = blit.dst_layers.array_layers.span};
    vk = VkImageBlit{
      .srcSubresource = src_subresource,
      .srcOffsets     = {VkOffset3D{(i32) blit.src_area.offset.x(),
                                (i32) blit.src_area.offset.y(),
                                (i32) blit.src_area.offset.z()},
                         VkOffset3D{(i32) blit.src_area.end().x(),
                                (i32) blit.src_area.end().y(),
                                (i32) blit.src_area.end().z()}},
      .dstSubresource = dst_subresource,
      .dstOffsets     = {VkOffset3D{(i32) blit.dst_area.offset.x(),
                                (i32) blit.dst_area.offset.y(),
                                (i32) blit.dst_area.offset.z()},
                         VkOffset3D{(i32) blit.dst_area.end().x(),
                                (i32) blit.dst_area.end().y(),
                                (i32) blit.dst_area.end().z()}}
    };
  }

  CMD(cmd::BlitImage{.src        = src->vk,
                     .src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     .dst        = dst->vk,
                     .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     .blits      = blits,
                     .filter     = (VkFilter) filter});

  blits.leak();

  access_.begin_pass();
  access_.access(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  access_.end_pass();
}

void CommandEncoder::resolve_image(gpu::Image src_, gpu::Image dst_,
                                   Span<gpu::ImageResolve const> resolves_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!resolves_.is_empty(), "");

  auto * src = (Image *) src_;
  auto * dst = (Image *) dst_;

  CHECK(has_bits(src->usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");
  CHECK(has_bits(dst->sample_count, gpu::SampleCount::C1), "");

  for (auto & resolve : resolves_)
  {
    CHECK(is_valid_image_access(src->aspects, src->mip_levels,
                                src->array_layers, resolve.src_layers.aspects,
                                Slice32{resolve.src_layers.mip_level, 1},
                                resolve.src_layers.array_layers),
          "");
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, resolve.dst_layers.aspects,
                                Slice32{resolve.dst_layers.mip_level, 1},
                                resolve.dst_layers.array_layers),
          "");

    auto src_extent = src->extent.mip(resolve.src_layers.mip_level);
    auto dst_extent = dst->extent.mip(resolve.dst_layers.mip_level);
    CHECK(resolve.src_area.extent.x() > 0, "");
    CHECK(resolve.src_area.extent.y() > 0, "");
    CHECK(resolve.src_area.extent.z() > 0, "");
    CHECK(resolve.src_area.offset.x() <= src_extent.x(), "");
    CHECK(resolve.src_area.offset.y() <= src_extent.y(), "");
    CHECK(resolve.src_area.offset.z() <= src_extent.z(), "");
    CHECK(resolve.src_area.end().x() <= src_extent.x(), "");
    CHECK(resolve.src_area.end().y() <= src_extent.y(), "");
    CHECK(resolve.src_area.end().z() <= src_extent.z(), "");
    CHECK(resolve.dst_offset.x() <= dst_extent.x(), "");
    CHECK(resolve.dst_offset.y() <= dst_extent.y(), "");
    CHECK(resolve.dst_offset.z() <= dst_extent.z(), "");
    CHECK((resolve.dst_offset.x() + resolve.src_area.extent.x()) <=
            dst_extent.x(),
          "");
    CHECK((resolve.dst_offset.y() + resolve.src_area.extent.y()) <=
            dst_extent.y(),
          "");
    CHECK((resolve.dst_offset.z() + resolve.src_area.extent.z()) <=
            dst_extent.z(),
          "");
  }

  Vec<VkImageResolve> resolves{pool_};

  if (!resolves.resize_uninit(resolves_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, resolve] : zip(resolves, resolves_))
  {
    VkImageSubresourceLayers src_subresource{
      .aspectMask     = (VkImageAspectFlags) resolve.src_layers.aspects,
      .mipLevel       = resolve.src_layers.mip_level,
      .baseArrayLayer = resolve.src_layers.array_layers.offset,
      .layerCount     = resolve.src_layers.array_layers.span};
    VkOffset3D               src_offset{(i32) resolve.src_area.offset.x(),
                          (i32) resolve.src_area.offset.y(),
                          (i32) resolve.src_area.offset.z()};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) resolve.dst_layers.aspects,
      .mipLevel       = resolve.dst_layers.mip_level,
      .baseArrayLayer = resolve.dst_layers.array_layers.offset,
      .layerCount     = resolve.dst_layers.array_layers.span};
    VkOffset3D dst_offset{(i32) resolve.dst_offset.x(),
                          (i32) resolve.dst_offset.y(),
                          (i32) resolve.dst_offset.z()};
    VkExtent3D extent{resolve.src_area.extent.x(), resolve.src_area.extent.y(),
                      resolve.src_area.extent.z()};

    vk = VkImageResolve{.srcSubresource = src_subresource,
                        .srcOffset      = src_offset,
                        .dstSubresource = dst_subresource,
                        .dstOffset      = dst_offset,
                        .extent         = extent};
  }

  CMD(cmd::ResolveImage{.src        = src->vk,
                        .src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        .dst        = dst->vk,
                        .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        .resolves   = resolves});

  resolves.leak();

  access_.begin_pass();
  access_.access(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  access_.access(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  access_.end_pass();
}

void CommandEncoder::begin_compute_pass()
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  pass_ = Pass::Compute;

  access_.begin_pass();
}

void CommandEncoder::end_compute_pass()
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");

  pass_ = Pass::None;
  access_.end_pass();
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
    auto image = ((ImageView *) info.view)->image;
    CHECK(has_bits(image->aspects, aspects), "");
    CHECK(has_bits(image->usage, usage), "");
    CHECK(!(info.resolve_mode != gpu::ResolveModes::None &&
            image->sample_count == gpu::SampleCount::C1),
          "");
  }
  if (info.resolve != nullptr)
  {
    auto image = ((ImageView *) info.resolve)->image;
    CHECK(has_bits(image->aspects, aspects), "");
    CHECK(has_bits(image->usage, usage), "");
    CHECK(image->sample_count == gpu::SampleCount::C1, "");
  }
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
  depth_stencil_attachment_access(gpu::RenderingAttachment const & attachment)
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

void CommandEncoder::begin_rendering(gpu::RenderingInfo const & info)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(info.color_attachments.size() <= gpu::MAX_PIPELINE_COLOR_ATTACHMENTS,
        "");
  CHECK(info.render_area.extent.x() > 0, "");
  CHECK(info.render_area.extent.y() > 0, "");
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

  Vec<VkRenderingAttachmentInfo> color_attachments{pool_};
  Vec<VkRenderingAttachmentInfo> depth_attachment{pool_};
  Vec<VkRenderingAttachmentInfo> stencil_attachment{pool_};

  constexpr VkImageLayout COLOR_LAYOUT =
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  constexpr VkPipelineStageFlags RESOLVE_STAGE =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  constexpr VkAccessFlags RESOLVE_COLOR_SRC_ACCESS =
    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

  constexpr VkAccessFlags RESOLVE_COLOR_DST_ACCESS =
    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  constexpr VkAccessFlags RESOLVE_DEPTH_STENCIL_SRC_ACCESS =
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  constexpr VkAccessFlags RESOLVE_DEPTH_STENCIL_DST_ACCESS =
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  constexpr VkImageLayout RESOLVE_COLOR_LAYOUT =
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  constexpr VkImageLayout RESOLVE_DEPTH_STENCIL_LAYOUT =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  constexpr VkImageLayout READ_WRITE_DEPTH_STENCIL_LAYOUT =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  constexpr VkImageLayout READ_DEPTH_STENCIL_LAYOUT =
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  if (!color_attachments.resize_uninit(info.color_attachments.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, attachment] : zip(color_attachments, info.color_attachments))
  {
    auto view = attachment.view == nullptr ? nullptr : ptr(attachment.view)->vk;
    auto resolve =
      attachment.resolve == nullptr ? nullptr : ptr(attachment.resolve)->vk;

    VkClearValue clear_value{.color{
      .uint32{attachment.clear.color.u32.x(), attachment.clear.color.u32.y(),
              attachment.clear.color.u32.z(), attachment.clear.color.u32.w()}}};

    vk = VkRenderingAttachmentInfoKHR{
      .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
      .pNext              = nullptr,
      .imageView          = view,
      .imageLayout        = COLOR_LAYOUT,
      .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
      .resolveImageView   = resolve,
      .resolveImageLayout = RESOLVE_COLOR_LAYOUT,
      .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
      .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
      .clearValue         = clear_value};
  }

  if (!depth_attachment.resize_uninit(info.depth_attachment.is_some() ? 1 : 0))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, attachment] : zip(depth_attachment, info.depth_attachment))
  {
    auto view = attachment.view == nullptr ? nullptr : ptr(attachment.view)->vk;
    auto resolve =
      attachment.resolve == nullptr ? nullptr : ptr(attachment.resolve)->vk;

    VkAccessFlags access = depth_stencil_attachment_access(attachment) |
                           RESOLVE_DEPTH_STENCIL_SRC_ACCESS;
    auto layout = has_write_access(access) ? READ_WRITE_DEPTH_STENCIL_LAYOUT :
                                             READ_DEPTH_STENCIL_LAYOUT;

    VkClearValue clear_value{
      .depthStencil{.depth   = attachment.clear.depth_stencil.depth,
                    .stencil = attachment.clear.depth_stencil.stencil}
    };

    vk = VkRenderingAttachmentInfoKHR{
      .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
      .pNext              = nullptr,
      .imageView          = view,
      .imageLayout        = layout,
      .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
      .resolveImageView   = resolve,
      .resolveImageLayout = RESOLVE_DEPTH_STENCIL_LAYOUT,
      .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
      .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
      .clearValue         = clear_value};
  }

  if (!stencil_attachment.resize_uninit(info.stencil_attachment.is_some() ? 1 :
                                                                            0))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, attachment] : zip(stencil_attachment, info.stencil_attachment))
  {
    auto view = attachment.view == nullptr ? nullptr : ptr(attachment.view)->vk;
    auto resolve =
      attachment.resolve == nullptr ? nullptr : ptr(attachment.resolve)->vk;

    VkAccessFlags access = depth_stencil_attachment_access(attachment) |
                           RESOLVE_DEPTH_STENCIL_SRC_ACCESS;

    auto layout = has_write_access(access) ? READ_WRITE_DEPTH_STENCIL_LAYOUT :
                                             READ_DEPTH_STENCIL_LAYOUT;

    VkClearValue clear_value{
      .depthStencil{.depth   = attachment.clear.depth_stencil.depth,
                    .stencil = attachment.clear.depth_stencil.stencil}
    };

    vk = VkRenderingAttachmentInfoKHR{
      .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
      .pNext              = nullptr,
      .imageView          = view,
      .imageLayout        = layout,
      .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
      .resolveImageView   = resolve,
      .resolveImageLayout = RESOLVE_DEPTH_STENCIL_LAYOUT,
      .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
      .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
      .clearValue         = clear_value};
  }

  CMD(cmd::BeginRendering{
    .info = VkRenderingInfoKHR{
                               .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                               .pNext = nullptr,
                               .flags = {},
                               .renderArea =
        VkRect2D{.offset = VkOffset2D{.x = (i32) info.render_area.offset.x(),
                                      .y = (i32) info.render_area.offset.y()},
                 .extent = VkExtent2D{.width  = info.render_area.extent.x(),
                                      .height = info.render_area.extent.y()}},
                               .layerCount           = info.num_layers,
                               .viewMask             = 0,
                               .colorAttachmentCount = size32(color_attachments),
                               .pColorAttachments    = color_attachments.data(),
                               .pDepthAttachment     = depth_attachment.data(),
                               .pStencilAttachment   = stencil_attachment.data()}
  });

  color_attachments.leak();
  depth_attachment.leak();
  stencil_attachment.leak();

  pass_ = Pass::Render;
  ctx_  = Ctx{.rendering = cmd};

  access_.begin_pass();

  for (auto & attachment : info.color_attachments)
  {
    VkAccessFlags        access = color_attachment_access(attachment);
    VkPipelineStageFlags stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (attachment.resolve_mode != gpu::ResolveModes::None)
    {
      access |= RESOLVE_COLOR_SRC_ACCESS;
      stages |= RESOLVE_STAGE;
    }

    if (attachment.view != nullptr)
    {
      access_.access(ptr(attachment.view), stages, access, COLOR_LAYOUT);
    }

    if (attachment.resolve != nullptr)
    {
      access_.access(ptr(attachment.resolve), RESOLVE_STAGE,
                     RESOLVE_COLOR_DST_ACCESS, RESOLVE_COLOR_LAYOUT);
    }
  }

  for (auto & attachment : info.depth_attachment)
  {
    VkAccessFlags access = depth_stencil_attachment_access(attachment) |
                           RESOLVE_DEPTH_STENCIL_SRC_ACCESS;
    VkPipelineStageFlags stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                  RESOLVE_STAGE;
    VkImageLayout layout = has_write_access(access) ?
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    if (attachment.resolve_mode != gpu::ResolveModes::None)
    {
      access |= RESOLVE_COLOR_SRC_ACCESS;
      stages |= RESOLVE_STAGE;
    }

    if (attachment.view != nullptr)
    {
      access_.access(ptr(attachment.view), stages, access, layout);
    }

    if (attachment.resolve != nullptr)
    {
      access_.access(ptr(attachment.resolve), RESOLVE_STAGE,
                     RESOLVE_DEPTH_STENCIL_DST_ACCESS |
                       RESOLVE_DEPTH_STENCIL_SRC_ACCESS,
                     RESOLVE_DEPTH_STENCIL_LAYOUT);
    }
  }

  for (auto & attachment : info.stencil_attachment)
  {
    VkAccessFlags access = depth_stencil_attachment_access(attachment) |
                           RESOLVE_DEPTH_STENCIL_SRC_ACCESS;
    VkImageLayout        layout = has_write_access(access) ?
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkPipelineStageFlags stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                  RESOLVE_STAGE;

    if (attachment.resolve_mode != gpu::ResolveModes::None)
    {
      access |= RESOLVE_COLOR_SRC_ACCESS;
      stages |= RESOLVE_STAGE;
    }

    if (attachment.view != nullptr)
    {
      access_.access(ptr(attachment.view), stages, access, layout);
    }

    if (attachment.resolve != nullptr)
    {
      access_.access(ptr(attachment.resolve), RESOLVE_STAGE,
                     RESOLVE_DEPTH_STENCIL_DST_ACCESS,
                     RESOLVE_DEPTH_STENCIL_LAYOUT);
    }
  }
}

void CommandEncoder::end_rendering()
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");

  CMD(cmd::EndRendering{});

  ctx_  = Ctx{};
  pass_ = Pass::None;

  access_.end_pass();
}

void CommandEncoder::bind_compute_pipeline(gpu::ComputePipeline pipeline)
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");

  CMD(cmd::BindPipeline{.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .pipeline   = ptr(pipeline)->vk});

  ctx_.compute_pipeline = cmd;
}

void  validate_pipeline_compatible(
  gpu::GraphicsPipeline                pipeline_,
  Span<gpu::RenderingAttachment const> color_attachments,
  Option<gpu::RenderingAttachment>     depth_attachment,
  Option<gpu::RenderingAttachment>     stencil_attachment)
{
  GraphicsPipeline * pipeline = (GraphicsPipeline *) pipeline_;

  CHECK(pipeline->color_fmts.size() == color_attachments.size(), "");
  CHECK(!(pipeline->depth_fmt.is_none() && depth_attachment.is_some()), "");
  CHECK(!(pipeline->stencil_fmt.is_none() && depth_attachment.is_some()), "");

  for (auto [pipeline_fmt, attachment] :
       zip(pipeline->color_fmts, color_attachments))
  {
    if (pipeline_fmt != gpu::Format::Undefined)
    {
      CHECK(attachment.view != nullptr, "");
      CHECK(pipeline_fmt == ptr(attachment.view)->format, "");
      CHECK(pipeline->sample_count == ptr(attachment.view)->image->sample_count,
            "");
    }
  }

  depth_attachment.match([&](auto & attachment) {
    CHECK(attachment.view != nullptr, "");
    CHECK(pipeline->depth_fmt == ptr(attachment.view)->format, "");
  });

  stencil_attachment.match([&](auto & attachment) {
    CHECK(attachment.view != nullptr, "");
    CHECK(pipeline->stencil_fmt == ptr(attachment.view)->format, "");
  });
}

void CommandEncoder::bind_graphics_pipeline(gpu::GraphicsPipeline pipeline)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.rendering != nullptr, "");
  // [ ] fill
  validate_pipeline_compatible(pipeline, {}, {}, {});

  CMD(cmd::BindPipeline{.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        .pipeline   = ptr(pipeline)->vk});

  ctx_.graphics_pipeline = cmd;
}

void CommandEncoder::bind_descriptor_sets(
  Span<gpu::DescriptorSet const> descriptor_sets_,
  Span<u32 const>                dynamic_offsets_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render || pass_ == Pass::Compute, "");
  CHECK(descriptor_sets_.size() <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS, "");
  CHECK(dynamic_offsets_.size() <= (gpu::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS +
                                    gpu::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS),
        "");

  for (auto offset : dynamic_offsets_)
  {
    CHECK(is_aligned<u64>(gpu::BUFFER_OFFSET_ALIGNMENT, offset), "");
  }

  switch (pass_)
  {
    case Pass::Render:
    {
      CHECK(ctx_.graphics_pipeline != nullptr, "");
      // auto pipeline = ptr(ctx_.graphics_pipeline->pipeline);
      // CHECK(pipeline->num_sets == descriptor_sets_.size(), "");
      // [ ] check that descriptor layout maches
    }
    break;
    case Pass::Compute:
    {
      CHECK(ctx_.compute_pipeline != nullptr, "");
      // auto pipeline = ptr(ctx_.compute_pipeline->pipeline);
      // CHECK(pipeline->num_sets == descriptor_sets_.size(), "");
    }
    break;
    default:
      break;
  }

  Vec<VkDescriptorSet> descriptor_sets{pool_};

  if (!descriptor_sets.resize_uninit(descriptor_sets_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, set_] : zip(descriptor_sets, descriptor_sets_))
  {
    auto * set = (DescriptorSet *) set_;
    vk         = set->vk;
  }

  // compute_ctx.sets.clear();
  // for (auto set : descriptor_sets)
  // {
  //   compute_ctx.sets.push((DescriptorSet *) set).unwrap();
  // }

  Vec<u32> dynamic_offsets{pool_};

  if (!dynamic_offsets.extend(dynamic_offsets_))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  CMD(cmd::BindDescriptorSets{.bind_point = {},    // [ ] impl
                              .layout     = {},    // [ ] inpl
                              .sets       = descriptor_sets,

                              .dynamic_offsets = dynamic_offsets});

  descriptor_sets.leak();
  dynamic_offsets.leak();

  ctx_.descriptor_sets = cmd;
}

void CommandEncoder::push_constants(Span<u8 const> constants_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render || pass_ == Pass::Compute, "");
  CHECK(constants_.size_bytes() <= gpu::MAX_PUSH_CONSTANTS_SIZE, "");
  CHECK(is_aligned<u32>(4U, constants_.size()), "");

  switch (pass_)
  {
    case Pass::Render:
    {
      CHECK(ctx_.graphics_pipeline != nullptr, "");
      // auto pipeline = ptr(ctx_.graphics_pipeline->pipeline);
      // CHECK(constants_.size() == pipeline->push_constants_size, "");
    }
    break;
    case Pass::Compute:
    {
      CHECK(ctx_.compute_pipeline != nullptr, "");
      // auto pipeline = ptr(ctx_.compute_pipeline->pipeline);
      // CHECK(constants_.size() == pipeline->push_constants_size, "");
    }
    break;
    default:
      break;
  }

  Vec<u8> constants{pool_};

  if (!constants.extend(constants_))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  constants.leak();

  CMD(cmd::PushConstants{.layout   = {},    // [ ] impl
                         .constant = constants});
}

void CommandEncoder::dispatch(u32x3 group_count)
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");

  CHECK(ctx_.compute_pipeline != nullptr, "");
  CHECK(group_count.x() <=
          dev_->phy_dev_.vk_properties.limits.maxComputeWorkGroupCount[0],
        "");
  CHECK(group_count.y() <=
          dev_->phy_dev_.vk_properties.limits.maxComputeWorkGroupCount[1],
        "");
  CHECK(group_count.z() <=
          dev_->phy_dev_.vk_properties.limits.maxComputeWorkGroupCount[2],
        "");

  CMD(cmd::Dispatch{.group_count = group_count});

  for (auto set : ctx_.descriptor_sets->sets)
  {
    // access_.access(set, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
  }
}

void CommandEncoder::dispatch_indirect(gpu::Buffer buffer_, u64 offset)
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");
  CHECK(buffer_ != nullptr, "");
  CHECK(ctx_.compute_pipeline != nullptr, "");

  auto * buffer = (Buffer *) buffer_;

  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(is_valid_buffer_access(buffer->size,
                               Slice64{offset, sizeof(gpu::DispatchCommand)}, 4,
                               alignof(gpu::DispatchCommand)),
        "");

  // [ ] descriptor set bounds checks

  CMD(cmd::DispatchIndirect{.buffer = buffer->vk, .offset = offset});

  for (auto set : ctx_.descriptor_sets->sets)
  {
    access_.access(set, DescriptorSetState::ComputeShaderView);
  }
}

void CommandEncoder::set_graphics_state(gpu::GraphicsState const & state)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline != nullptr, "");
  CHECK(state.viewport.min_depth >= 0.0F, "");
  CHECK(state.viewport.min_depth <= 1.0F, "");
  CHECK(state.viewport.max_depth >= 0.0F, "");
  CHECK(state.viewport.max_depth <= 1.0F, "");

  CMD(cmd::SetGraphicsState{.state = state});

  ctx_.graphics_state = cmd;
}

void CommandEncoder::bind_vertex_buffers(
  Span<gpu::Buffer const> vertex_buffers_, Span<u64 const> offsets_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(!vertex_buffers_.is_empty(), "");
  CHECK(vertex_buffers_.size() <= gpu::MAX_VERTEX_ATTRIBUTES, "");
  CHECK(offsets_.size() == vertex_buffers_.size(), "");

  for (auto [buff_, off] : zip(vertex_buffers_, offsets_))
  {
    auto * buff = (Buffer *) buff_;
    CHECK(is_valid_buffer_access(buff->size, Slice64{off, 1}, 1, 1), "");
    CHECK(has_bits(buff->usage, gpu::BufferUsage::VertexBuffer), "");
  }

  Vec<VkBuffer> vertex_buffers{pool_};

  if (!vertex_buffers.resize_uninit(vertex_buffers_.size()))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  for (auto [vk, buff_] : zip(vertex_buffers, vertex_buffers_))
  {
    auto * buff = (Buffer *) buff_;
    vk          = buff->vk;
  }

  Vec<u64> offsets{pool_};

  if (!offsets.extend(offsets_))
  {
    status_ = Status::OutOfHostMemory;
    return;
  }

  CMD(cmd::BindVertexBuffers{.buffers = vertex_buffers, .offsets = offsets});

  vertex_buffers.leak();
  offsets.leak();

  //  access_buffer(*c.buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
  // VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, ctx.pass_timestamp);

  ctx_.vertex_buffer = cmd;
}

void CommandEncoder::bind_index_buffer(gpu::Buffer index_buffer_, u64 offset,
                                       gpu::IndexType index_type)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(index_buffer_ != nullptr, "");

  auto * index_buffer = (Buffer *) index_buffer_;
  auto   index_size   = index_type_size(index_type);

  CHECK(has_bits(index_buffer->usage, gpu::BufferUsage::IndexBuffer), "");
  CHECK(is_valid_buffer_access(index_buffer->size, Slice64{offset, index_size},
                               index_size, index_size),
        "");

  CMD(cmd::BindIndexBuffer{.buffer     = index_buffer->vk,
                           .offset     = offset,
                           .index_type = (VkIndexType) index_type});

  //  access_buffer(*c.buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
  // VK_ACCESS_INDEX_READ_BIT, ctx.pass_timestamp);
  ctx_.index_buffer = cmd;
}

void CommandEncoder::draw(Slice32 vertices, Slice32 instances)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline != nullptr, "");
  CHECK(ctx_.graphics_state != nullptr, "");

  CMD(cmd::Draw{.vertices = vertices, .instances = instances});

  // [ ] move to bind-time
  // [ ] idx buffer, vtx buffer, manage aliasing
  for (auto set : ctx_.descriptor_sets->sets)
  {
    access_.access(set, DescriptorSetState::GraphicsShaderView);
  }
}

void CommandEncoder::draw_indexed(Slice32 indices, Slice32 instances,
                                  i32 vertex_offset)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline != nullptr, "");
  CHECK(ctx_.graphics_state != nullptr, "");

  /* [ ] if bound
  u64 const index_size = index_type_size(ctx.index_type);
  CHECK((ctx.index_buffer_offset + first_index * index_size) <
          ctx.index_buffer->size,
        "");
  CHECK((ctx.index_buffer_offset + (first_index + num_indices) * index_size) <=
          ctx.index_buffer->size,
        "");
  */

  CMD(cmd::DrawIndexed{.indices       = indices,
                       .instances     = instances,
                       .vertex_offset = vertex_offset});

  for (auto set : ctx_.descriptor_sets->sets)
  {
    access_.access(set, DescriptorSetState::GraphicsShaderView);
  }
}

void CommandEncoder::draw_indirect(gpu::Buffer buffer_, u64 offset,
                                   u32 draw_count, u32 stride)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline != nullptr, "");
  CHECK(ctx_.graphics_state != nullptr, "");

  auto * buffer = (Buffer *) buffer_;

  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(stride >= sizeof(gpu::DrawCommand), "");
  CHECK(is_valid_buffer_access(
          buffer->size, Slice64{offset, (u64) draw_count * (u64) stride}, 4, 4),
        "");

  CMD(cmd::DrawIndirect{.buffer     = buffer->vk,
                        .offset     = offset,
                        .draw_count = draw_count,
                        .stride     = stride});

  // access_buffer(*c.buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
  // VK_ACCESS_INDIRECT_COMMAND_READ_BIT, ctx.pass_timestamp);

  for (auto set : ctx_.descriptor_sets->sets)
  {
    access_.access(set, DescriptorSetState::GraphicsShaderView);
  }
}

void CommandEncoder::draw_indexed_indirect(gpu::Buffer buffer_, u64 offset,
                                           u32 draw_count, u32 stride)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline != nullptr, "");
  CHECK(ctx_.graphics_state != nullptr, "");

  auto * buffer = (Buffer *) buffer_;

  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(is_valid_buffer_access(
          buffer->size, Slice64{offset, (u64) draw_count * (u64) stride}, 4, 4),
        "");
  CHECK(stride >= sizeof(gpu::DrawIndexedCommand), "");

  CMD(cmd::DrawIndexedIndirect{.buffer     = buffer->vk,
                               .offset     = offset,
                               .draw_count = draw_count,
                               .stride     = stride});

  for (auto set : ctx_.descriptor_sets->sets)
  {
    access_.access(set, DescriptorSetState::GraphicsShaderView);
  }

  // [ ] access idx, vtx buffer
  //  access_buffer(*c.buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
  // VK_ACCESS_INDIRECT_COMMAND_READ_BIT, ctx.pass_timestamp);
}

void CommandBuffer::begin()
{
  CHECK(state_ == CommandBufferState::Reset, "");

  {
    ReadGuard guard{dev_->resource_states_.lock_};
    resource_states_.rebuild(dev_->resource_states_);
  }

  VkCommandBufferBeginInfo info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext            = nullptr,
    .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = nullptr};

  auto result = dev_->vk_table_.BeginCommandBuffer(vk_, &info);

  CHECK(result == VK_SUCCESS, "");

  state_ = CommandBufferState::Recording;
}

Status CommandBuffer::end()
{
  CHECK(state_ == CommandBufferState::Recording, "");

  dev_->vk_table_.EndCommandBuffer(vk_);

  state_ = CommandBufferState::Recorded;
  return status_;
}

void CommandBuffer::reset()
{
  CHECK(state_ == CommandBufferState::Reset ||
          state_ == CommandBufferState::Submitted,
        "");

  dev_->vk_table_.ResetCommandBuffer(vk_, 0);

  state_ = CommandBufferState::Reset;
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::ResetTimestampQuery const & cmd)
{
  t.CmdResetQueryPool(vk, cmd.query, cmd.range.offset, cmd.range.span);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::ResetStatisticsQuery const & cmd)
{
  t.CmdResetQueryPool(vk, cmd.query, cmd.range.offset, cmd.range.span);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::WriteTimestamp const & cmd)
{
  t.CmdWriteTimestamp(vk, cmd.stages, cmd.query, cmd.index);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BeginStatistics const & cmd)
{
  t.CmdBeginQuery(vk, cmd.query, cmd.index, 0);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::EndStatistics const & cmd)
{
  t.CmdEndQuery(vk, cmd.query, cmd.index);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BeginDebugMarker const & cmd)
{
  t.CmdDebugMarkerBeginEXT(vk, &cmd.info);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::EndDebugMarker const &)
{
  t.CmdDebugMarkerEndEXT(vk);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::FillBuffer const & cmd)
{
  t.CmdFillBuffer(vk, cmd.dst, cmd.range.offset, cmd.range.span, cmd.data);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::CopyBuffer const & cmd)
{
  t.CmdCopyBuffer(vk, cmd.src, cmd.dst, size32(cmd.copies), cmd.copies.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::UpdateBuffer const & cmd)
{
  t.CmdUpdateBuffer(vk, cmd.dst, cmd.dst_offset, size64(cmd.src),
                    cmd.src.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::ClearColorImage const & cmd)
{
  t.CmdClearColorImage(vk, cmd.dst, cmd.dst_layout, &cmd.value,
                       size32(cmd.ranges), cmd.ranges.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::ClearDepthStencilImage const & cmd)
{
  t.CmdClearDepthStencilImage(vk, cmd.dst, cmd.dst_layout, &cmd.value,
                              size32(cmd.ranges), cmd.ranges.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::CopyImage const & cmd)
{
  t.CmdCopyImage(vk, cmd.src, cmd.src_layout, cmd.dst, cmd.dst_layout,
                 size32(cmd.copies), cmd.copies.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::CopyBufferToImage const & cmd)
{
  t.CmdCopyBufferToImage(vk, cmd.src, cmd.dst, cmd.dst_layout,
                         size32(cmd.copies), cmd.copies.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BlitImage const & cmd)
{
  t.CmdBlitImage(vk, cmd.src, cmd.src_layout, cmd.dst, cmd.dst_layout,
                 size32(cmd.blits), cmd.blits.data(), cmd.filter);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::ResolveImage const & cmd)
{
  t.CmdResolveImage(vk, cmd.src, cmd.src_layout, cmd.dst, cmd.dst_layout,
                    size32(cmd.resolves), cmd.resolves.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BeginRendering const & cmd)
{
  t.CmdBeginRenderingKHR(vk, &cmd.info);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::EndRendering const &)
{
  t.CmdEndRenderingKHR(vk);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BindPipeline const & cmd)
{
  t.CmdBindPipeline(vk, cmd.bind_point, cmd.pipeline);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BindDescriptorSets const & cmd)
{
  t.CmdBindDescriptorSets(vk, cmd.bind_point, cmd.layout, 0, size32(cmd.sets),
                          cmd.sets.data(), size32(cmd.dynamic_offsets),
                          cmd.dynamic_offsets.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::PushConstants const & cmd)
{
  t.CmdPushConstants(vk, cmd.layout, VK_SHADER_STAGE_ALL, 0,
                     size32(cmd.constant), cmd.constant.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::Dispatch const & cmd)
{
  t.CmdDispatch(vk, cmd.group_count.x(), cmd.group_count.y(),
                cmd.group_count.z());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::DispatchIndirect const & cmd)
{
  t.CmdDispatchIndirect(vk, cmd.buffer, cmd.offset);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::SetGraphicsState const & cmd)
{
  auto & s = cmd.state;

  VkRect2D vk_scissor{
    .offset =
      VkOffset2D{(i32) s.scissor.offset.x(), (i32) s.scissor.offset.y()},
    .extent = VkExtent2D{s.scissor.extent.x(),       s.scissor.extent.y()      }
  };
  t.CmdSetScissor(vk, 0, 1, &vk_scissor);

  VkViewport vk_viewport{.x        = s.viewport.offset.x(),
                         .y        = s.viewport.offset.y(),
                         .width    = s.viewport.extent.x(),
                         .height   = s.viewport.extent.y(),
                         .minDepth = s.viewport.min_depth,
                         .maxDepth = s.viewport.max_depth};
  t.CmdSetViewport(vk, 0, 1, &vk_viewport);

  f32 vk_constant[4] = {s.blend_constant.x(), s.blend_constant.y(),
                        s.blend_constant.z(), s.blend_constant.w()};
  t.CmdSetBlendConstants(vk, vk_constant);

  t.CmdSetStencilTestEnableEXT(vk, s.stencil_test_enable);

  t.CmdSetStencilReference(vk, VK_STENCIL_FACE_FRONT_BIT,
                           s.front_face_stencil.reference);
  t.CmdSetStencilCompareMask(vk, VK_STENCIL_FACE_FRONT_BIT,
                             s.front_face_stencil.compare_mask);
  t.CmdSetStencilWriteMask(vk, VK_STENCIL_FACE_FRONT_BIT,
                           s.front_face_stencil.write_mask);
  t.CmdSetStencilOpEXT(vk, VK_STENCIL_FACE_FRONT_BIT,
                       (VkStencilOp) s.front_face_stencil.fail_op,
                       (VkStencilOp) s.front_face_stencil.pass_op,
                       (VkStencilOp) s.front_face_stencil.depth_fail_op,
                       (VkCompareOp) s.front_face_stencil.compare_op);

  t.CmdSetStencilReference(vk, VK_STENCIL_FACE_BACK_BIT,
                           s.back_face_stencil.reference);
  t.CmdSetStencilCompareMask(vk, VK_STENCIL_FACE_BACK_BIT,
                             s.back_face_stencil.compare_mask);
  t.CmdSetStencilWriteMask(vk, VK_STENCIL_FACE_BACK_BIT,
                           s.back_face_stencil.write_mask);
  t.CmdSetStencilOpEXT(vk, VK_STENCIL_FACE_BACK_BIT,
                       (VkStencilOp) s.back_face_stencil.fail_op,
                       (VkStencilOp) s.back_face_stencil.pass_op,
                       (VkStencilOp) s.back_face_stencil.depth_fail_op,
                       (VkCompareOp) s.back_face_stencil.compare_op);
  t.CmdSetCullModeEXT(vk, (VkCullModeFlags) s.cull_mode);
  t.CmdSetFrontFaceEXT(vk, (VkFrontFace) s.front_face);
  t.CmdSetDepthTestEnableEXT(vk, s.depth_test_enable);
  t.CmdSetDepthCompareOpEXT(vk, (VkCompareOp) s.depth_compare_op);
  t.CmdSetDepthWriteEnableEXT(vk, s.depth_write_enable);
  t.CmdSetDepthBoundsTestEnableEXT(vk, s.depth_bounds_test_enable);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BindVertexBuffers const & cmd)
{
  t.CmdBindVertexBuffers(vk, 0, size32(cmd.buffers), cmd.buffers.data(),
                         cmd.offsets.data());
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::BindIndexBuffer const & cmd)
{
  t.CmdBindIndexBuffer(vk, cmd.buffer, cmd.offset, cmd.index_type);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::Draw const & cmd)
{
  t.CmdDraw(vk, cmd.vertices.span, cmd.instances.span, cmd.vertices.offset,
            cmd.instances.offset);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::DrawIndexed const & cmd)
{
  t.CmdDrawIndexed(vk, cmd.indices.span, cmd.instances.span, cmd.indices.offset,
                   cmd.vertex_offset, cmd.instances.offset);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::DrawIndirect const & cmd)
{
  t.CmdDrawIndirect(vk, cmd.buffer, cmd.offset, cmd.draw_count, cmd.stride);
}

ASH_FORCE_INLINE inline void command(DeviceTable const & t, VkCommandBuffer vk,
                                     cmd::DrawIndexedIndirect const & cmd)
{
  t.CmdDrawIndexedIndirect(vk, cmd.buffer, cmd.offset, cmd.draw_count,
                           cmd.stride);
}

inline cmd::Command * commands(DeviceTable const & t, VkCommandBuffer vk,
                               cmd::Command * cmd_, usize n)
{
#define CMD_CASE(type)               \
  case cmd::Type::type:              \
  {                                  \
    auto * cmd = (cmd::type *) cmd_; \
    command(t, vk, *cmd);            \
  }                                  \
  break;

  while (cmd_ != nullptr && n != 0)
  {
    switch (cmd_->type)
    {
      CMD_CASE(ResetTimestampQuery);
      CMD_CASE(ResetStatisticsQuery);
      CMD_CASE(WriteTimestamp);
      CMD_CASE(BeginStatistics);
      CMD_CASE(EndStatistics);
      CMD_CASE(BeginDebugMarker);
      CMD_CASE(EndDebugMarker);
      CMD_CASE(FillBuffer);
      CMD_CASE(CopyBuffer);
      CMD_CASE(UpdateBuffer);
      CMD_CASE(ClearColorImage);
      CMD_CASE(ClearDepthStencilImage);
      CMD_CASE(CopyImage);
      CMD_CASE(CopyBufferToImage);
      CMD_CASE(BlitImage);
      CMD_CASE(ResolveImage);
      CMD_CASE(BeginRendering);
      CMD_CASE(EndRendering);
      CMD_CASE(BindPipeline);
      CMD_CASE(BindDescriptorSets);
      CMD_CASE(PushConstants);
      CMD_CASE(Dispatch);
      CMD_CASE(DispatchIndirect);
      CMD_CASE(SetGraphicsState);
      CMD_CASE(BindVertexBuffers);
      CMD_CASE(BindIndexBuffer);
      CMD_CASE(Draw);
      CMD_CASE(DrawIndexed);
      CMD_CASE(DrawIndirect);
      CMD_CASE(DrawIndexedIndirect);
    }

    cmd_ = cmd_->next;
    n--;
  }

  return cmd_;
}

void CommandBuffer::record(gpu::CommandEncoder & encoder_)
{
  CHECK(state_ == CommandBufferState::Recording, "");

  // [ ] encode into command buffer and transition states
  // [ ] we need to issue hard memory barrier after encoding is done, semaphores won't ensure visibility
}

void CommandBuffer::commit_resource_states()
{
  // [ ] at point of submission, write back resource states
  {
    WriteGuard guard{dev_->resource_states_.lock_};
    resource_states_.commit(dev_->resource_states_);
  }
}

}    // namespace vk
}    // namespace ash
