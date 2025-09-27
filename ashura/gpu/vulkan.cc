/// SPDX-License-Identifier: MIT
#include "ashura/gpu/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "ashura/std/sformat.h"

// clang-format off

#include "vulkan/vulkan_beta.h"
#include "vulkan/vk_enum_string_helper.h"

// clang-format on

namespace ash
{
namespace vk
{

#define SCRATCH_STACK_RESERVE_SIZE 1_KB
#define SCRATCH_ALLOCATOR(upstream)                              \
  u8                scratch_memory_[SCRATCH_STACK_RESERVE_SIZE]; \
  FallbackAllocator scratch_                                     \
  {                                                              \
    scratch_memory_, upstream                                    \
  }

constexpr auto DEBUG_LAYER_EXTENSION_NAME = "VK_LAYER_KHRONOS_validation"_str;

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
  LOAD_VK(GetPhysicalDeviceFeatures2KHR);
  LOAD_VK(GetPhysicalDeviceFormatProperties2KHR);
  LOAD_VK(GetPhysicalDeviceImageFormatProperties2KHR);
  LOAD_VK(GetPhysicalDeviceMemoryProperties);
  LOAD_VK(GetPhysicalDeviceMemoryProperties2KHR);
  LOAD_VK(GetPhysicalDeviceProperties2KHR);
  LOAD_VK(GetPhysicalDeviceQueueFamilyProperties2KHR);
  LOAD_VK(GetPhysicalDeviceSparseImageFormatProperties2KHR);

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
  SET_VMA_INST(GetPhysicalDeviceMemoryProperties2KHR);
#undef SET_VMA_INST

#define SET_VMA_DEV(function) vma_table.vk##function = vk_table.function
  SET_VMA_DEV(AllocateMemory);
  SET_VMA_DEV(FreeMemory);
  SET_VMA_DEV(MapMemory);
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

u32 DescriptorBinding::sync_size() const
{
  return sync_resources.match(
    [](None) { return (u32) 0; }, [](auto & v) { return size32(v); },
    [](auto & v) { return size32(v); }, [](auto & v) { return size32(v); });
}

void IDescriptorSet::remove_bind_loc(BindLocations &      locations,
                                     BindLocation const & loc)
{
  auto pos = find(locations.view(), loc).as_slice_of(locations);
  locations.erase(pos);
}

void IDescriptorSet::update_link(u32 ibinding, u32 first_element,
                                 Span<gpu::BufferBinding const> buffers)
{
  auto & binding        = bindings[ibinding];
  auto & sync_resources = binding.sync_resources[v1];

  for (auto [i, buffer] : enumerate<u32>(buffers))
  {
    auto    element = first_element + i;
    auto *& current = sync_resources[element];
    auto *  next    = (Buffer) buffer.buffer;

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

void IDescriptorSet::update_link(u32 ibinding, u32 first_element,
                                 Span<gpu::BufferView const> buffer_views)
{
  auto & binding        = bindings[ibinding];
  auto & sync_resources = binding.sync_resources[v2];

  for (auto [i, buffer_view] : enumerate<u32>(buffer_views))
  {
    auto    element = first_element + i;
    auto *& current = sync_resources[element];
    auto *  next    = (BufferView) buffer_view;

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

void IDescriptorSet::update_link(u32 ibinding, u32 first_element,
                                 Span<gpu::ImageBinding const> images)
{
  auto & binding        = bindings[ibinding];
  auto & sync_resources = binding.sync_resources[v3];

  for (auto [i, image] : enumerate<u32>(images))
  {
    auto    element = first_element + i;
    auto *& current = sync_resources[element];
    auto *  next    = (ImageView) image.image_view;

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

void HazardBarriers::clear()
{
  buffers_.clear();
  mem_buffers_.clear();
  images_.clear();
  mem_images_.clear();
}

void HazardBarriers::buffer(VkPipelineStageFlags src, VkPipelineStageFlags dst,
                            VkBufferMemoryBarrier const & buffer)
{
  buffers_.push(src, dst, buffer).unwrap();
}

void HazardBarriers::buffer(VkPipelineStageFlags src, VkPipelineStageFlags dst,
                            VkMemoryBarrier const &       mem,
                            VkBufferMemoryBarrier const & buffer)
{
  mem_buffers_.push(src, dst, mem, buffer).unwrap();
}

void HazardBarriers::image(VkPipelineStageFlags src, VkPipelineStageFlags dst,
                           VkImageMemoryBarrier const & image)
{
  images_.push(src, dst, image).unwrap();
}

void HazardBarriers::image(VkPipelineStageFlags src, VkPipelineStageFlags dst,
                           VkMemoryBarrier const &      mem,
                           VkImageMemoryBarrier const & image)
{
  mem_images_.push(src, dst, mem, image).unwrap();
}

// layout transitions are considered write operations even if only a read
// happens so multiple ones can't happen at the same time
//
// we'll kind of be waiting on a barrier operation which doesn't make sense cos
// the barrier might have already taken care of us even when they both only
// perform reads
//
// if their scopes don't line-up, they won't observe the effects same
void HazardBarriers::barrier(IImage const & image, MemAccess old_access,
                             VkImageLayout old_layout, MemAccess new_access,
                             VkImageLayout new_layout)
{
  this->image(
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

void HazardBarriers::discard_barrier(IImage const & image, MemAccess old_access,
                                     MemAccess     new_access,
                                     VkImageLayout new_layout)
{
  this->image(
    old_access.stages, new_access.stages,
    VkMemoryBarrier{
      .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
      .pNext         = nullptr,
      .srcAccessMask = old_access.access,
      .dstAccessMask = new_access.access
  },
    VkImageMemoryBarrier{
      .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext               = nullptr,
      .srcAccessMask       = VK_ACCESS_NONE,
      .dstAccessMask       = new_access.access,
      .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
      .newLayout           = new_layout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image               = image.vk,
      .subresourceRange    = {.aspectMask     = (VkImageAspectFlags) image.aspects,
                              .baseMipLevel   = 0,
                              .levelCount     = VK_REMAINING_MIP_LEVELS,
                              .baseArrayLayer = 0,
                              .layerCount     = VK_REMAINING_ARRAY_LAYERS}});
}

void HazardBarriers::barrier(IBuffer const & buffer, MemAccess old_access,
                             MemAccess new_access)
{
  this->buffer(old_access.stages, new_access.stages,
               {.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .pNext               = nullptr,
                .srcAccessMask       = old_access.access,
                .dstAccessMask       = new_access.access,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer              = buffer.vk,
                .offset              = 0,
                .size                = VK_WHOLE_SIZE});
}

void HazardBarriers::discard_barrier(IBuffer const & buffer,
                                     MemAccess old_access, MemAccess new_access)
{
  this->buffer(old_access.stages, new_access.stages,
               VkMemoryBarrier{.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                               .pNext = nullptr,
                               .srcAccessMask = old_access.access,
                               .dstAccessMask = new_access.access},
               {.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .pNext               = nullptr,
                .srcAccessMask       = VK_ACCESS_NONE,
                .dstAccessMask       = new_access.access,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer              = buffer.vk,
                .offset              = 0,
                .size                = VK_WHOLE_SIZE});
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

void EncoderResourceStates::access(IImage const &    image,
                                   MemAccess const & access,
                                   VkImageLayout layout, u32 pass,
                                   HazardBarriers & barriers)
{
  // only one element of the alias can issue a barrier for a pass
  auto alias    = image.memory.alias->id;
  auto has_read = has_read_access(access.access);
  auto element  = image.memory.element;

  auto [hazard, _, last_accessed] = alias_[alias];

  // it was already accessed, resources are only allowed to be in one state for a pass
  if (last_accessed != U32_MAX && last_accessed == pass)
  {
    last_accessed = pass;
    return;
  }

  last_accessed = pass;

  auto mark = [&](HazardType type, MemAccess previous) {
    hazard = Hazard{
      .type     = type,
      .latest   = access,
      .previous = previous,
      .state    = ImageMemState{.element = element, .layout = layout}
    };
    alias_.dense.v1.set_bit(alias_.to_index(alias));
  };

  auto mark_combined = [&](HazardType type, MemAccess previous,
                           MemAccess latest) {
    hazard = Hazard{
      .type     = type,
      .latest   = latest,
      .previous = previous,
      .state    = ImageMemState{.element = element, .layout = layout}
    };
    alias_.dense.v1.set_bit(alias_.to_index(alias));
  };

  auto discard = [&]() {
    barriers.discard_barrier(image, hazard.latest, access, layout);
    mark(HazardType::Write, {});
  };

  hazard.state.match(
    [&](None) {
      discard();
      return;
    },
    [&](BufferMemState const &) {
      // hard memory barrier for aliasing
      discard();
      return;
    },
    [&](ImageMemState const & h) {
      auto current_layout   = h.layout;
      auto needs_transition = current_layout != layout;
      auto has_write = has_write_access(access.access) || needs_transition;

      auto barrier = [&](MemAccess const & from) {
        barriers.barrier(image, from, current_layout, access, layout);
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
            mark(HazardType::Write, {});

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
            mark(HazardType::Reads, {});

            return;
          }

          return;
        }
        case HazardType::Reads:
        {
          auto previous_reads = hazard.latest;

          if (has_write)
          {
            // wait till done reading before modifying
            // reset access sequence since all stages following this write need to
            // wait on this write

            mark(HazardType::Write, {});

            barrier(previous_reads);

            return;
          }

          if (has_read)
          {
            // combine all subsequent reads, so the next writer knows to wait on all
            // combined reads to complete

            mark_combined(
              HazardType::Reads, {},
              MemAccess{.stages = previous_reads.stages | access.stages,
                        .access = previous_reads.access | access.access});

            return;
          }

          return;
        }
        case HazardType::Write:
        {
          auto previous_write = hazard.latest;

          if (has_write)
          {
            // wait till done writing before modifying
            // remove previous write since this access already waits on another
            // access to complete and the next access will have to wait on this
            // access
            mark(HazardType::Write, {});

            barrier(previous_write);

            return;
          }

          if (has_read)
          {
            // wait till all write stages are done
            mark(HazardType::ReadsAfterWrite, previous_write);

            barrier(previous_write);

            return;
          }

          return;
        }
        case HazardType::ReadsAfterWrite:
        {
          auto previous_reads = hazard.latest;
          auto previous_write = hazard.previous;

          if (has_write)
          {
            // wait for all reading stages only
            // stages can be reset and point only to the latest write stage, since
            // they all need to wait for this write anyway.
            mark(HazardType::Write, {});

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
              mark_combined(HazardType::ReadsAfterWrite, previous_write,
                            {.stages = previous_reads.stages | access.stages,
                             .access = previous_reads.access | access.access});
              return;
            }

            mark_combined(HazardType::ReadsAfterWrite, previous_write,
                          {.stages = previous_reads.stages | access.stages,
                           .access = previous_reads.access | access.access});

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

void EncoderResourceStates::access(IImageView const & image,
                                   MemAccess const &  access,
                                   VkImageLayout layout, u32 pass,
                                   HazardBarriers & barriers)
{
  this->access(*image.image, access, layout, pass, barriers);
}

void EncoderResourceStates::access(IBuffer const &   buffer,
                                   MemAccess const & access, u32 pass,
                                   HazardBarriers & barriers)
{
  auto alias     = buffer.memory.alias->id;
  auto has_write = has_write_access(access.access);
  auto has_read  = has_read_access(access.access);
  auto element   = buffer.memory.element;

  auto [hazard, _, last_accessed] = alias_[alias];

  // it was already accessed, resources are only allowed to be in one state for a pass
  if (last_accessed != U32_MAX && last_accessed == pass)
  {
    last_accessed = pass;
    return;
  }

  last_accessed = pass;

  auto mark = [&](HazardType type, MemAccess previous) {
    hazard = Hazard{.type     = type,
                    .latest   = access,
                    .previous = previous,
                    .state    = BufferMemState{.element = element}};
    alias_.dense.v1.set_bit(alias_.to_index(alias));
  };

  auto mark_combined = [&](HazardType type, MemAccess previous,
                           MemAccess latest) {
    hazard = Hazard{.type     = type,
                    .latest   = latest,
                    .previous = previous,
                    .state    = ImageMemState{.element = element}};
    alias_.dense.v1.set_bit(alias_.to_index(alias));
  };

  auto discard = [&]() {
    barriers.discard_barrier(buffer, hazard.latest, access);

    mark(HazardType::Write, {});
  };

  hazard.state.match(
    [&](None) {
      discard();
      return;
    },
    [&](BufferMemState const & h) {
      auto barrier = [&](MemAccess const & from) {
        barriers.barrier(buffer, from, access);
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
            mark(HazardType::Write, {});

            return;
          }

          if (has_read)
          {
            mark(HazardType::Reads, {});

            return;
          }

          return;
        }

        case HazardType::Reads:
        {
          auto previous_reads = hazard.latest;

          if (has_write)
          {
            // wait till done reading before modifying
            // reset access sequence since all stages following this write need to
            // wait on this write
            mark(HazardType::Write, {});

            barrier(previous_reads);

            return;
          }

          if (has_read)
          {
            // combine all subsequent reads, so the next writer knows to wait on all
            // combined reads to complete

            mark_combined(HazardType::Reads, {},
                          {.stages = previous_reads.stages | access.stages,
                           .access = previous_reads.access | access.access});

            return;
          }

          return;
        }
        case HazardType::Write:
        {
          auto previous_write = hazard.latest;

          if (has_write)
          {
            // wait till done writing before modifying
            // remove previous write since this access already waits on another
            // access to complete and the next access will have to wait on this
            // access
            mark(HazardType::Write, {});

            barrier(previous_write);

            return;
          }

          if (has_read)
          {
            // wait till all write stages are done
            mark(HazardType::ReadsAfterWrite, previous_write);

            barrier(previous_write);

            return;
          }

          return;
        }
        case HazardType::ReadsAfterWrite:
        {
          auto previous_reads = hazard.latest;
          auto previous_write = hazard.previous;

          if (has_write)
          {
            // wait for all reading stages only
            // stages can be reset and point only to the latest write stage, since
            // they all need to wait for this write anyway.

            mark(HazardType::Write, {});

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
              mark_combined(HazardType::ReadsAfterWrite, previous_write,
                            {.stages = previous_reads.stages | access.stages,
                             .access = previous_reads.access | access.access});
              return;
            }

            mark_combined(HazardType::ReadsAfterWrite, previous_write,
                          {.stages = previous_reads.stages | access.stages,
                           .access = previous_reads.access | access.access});

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

void EncoderResourceStates::access(IDescriptorSet const & set, u32 pass,
                                   VkPipelineStageFlags shader_stages,
                                   HazardBarriers &     barriers)
{
  auto [last_accessed] = descriptor_sets_[set.id];

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
        for (Buffer buffer : buffers)
        {
          if (buffer != nullptr)
          {
            access(*buffer, acc, pass, barriers);
          }
        }
      },
      [&](auto & buffer_views) {
        for (BufferView buffer_view : buffer_views)
        {
          if (buffer_view != nullptr)
          {
            access(*buffer_view->buffer, acc, pass, barriers);
          }
        }
      },
      [&](auto & image_views) {
        for (ImageView image_view : image_views)
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
  alias_.clear();

  alias_.id_to_index_.extend(upstream.alias_.id_to_index_).unwrap();
  alias_.index_to_id_.extend(upstream.alias_.index_to_id_).unwrap();

  // memory hazard
  alias_.dense.v0.extend(upstream.alias_.dense.v0).unwrap();
  // was modified
  alias_.dense.v1.resize(upstream.alias_.dense.v0.size()).unwrap();
  // last_accessed
  alias_.dense.v2.resize_uninit(upstream.alias_.dense.v0.size()).unwrap();

  fill(alias_.dense.v2.view(), U32_MAX);

  descriptor_sets_.clear();

  descriptor_sets_.id_to_index_.extend(upstream.descriptor_sets_.id_to_index_)
    .unwrap();
  descriptor_sets_.index_to_id_.extend(upstream.descriptor_sets_.index_to_id_)
    .unwrap();
}

void EncoderResourceStates::commit(DeviceResourceStates & upstream)
{
  for (auto [i, m] : zip(range(alias_.size()), alias_))
  {
    // if resource was modified
    if (m.v1) [[unlikely]]
    {
      upstream.alias_[alias_.to_id(i)].v0 = m.v0;
    }
  }
}

u32 CommandTracker::begin_pass()
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

void CommandTracker::command(cmd::Cmd * cmd)
{
  if (last_cmd_ == nullptr)
  {
    first_cmd_ = cmd;
    last_cmd_  = cmd;
    return;
  }

  last_cmd_->next = cmd;
  last_cmd_       = cmd;

  passes_.last().commands++;
}

void CommandTracker::end_pass()
{
}

void CommandTracker::track(Buffer buffer, VkPipelineStageFlags stages,
                           VkAccessFlags access)
{
  buffers_.push(buffer, stages, access).unwrap();
  passes_.last().buffers++;
}

void CommandTracker::track(Image image, VkPipelineStageFlags stages,
                           VkAccessFlags access, VkImageLayout layout)
{
  images_.push(image, stages, access, layout).unwrap();
  passes_.last().images++;
}

void CommandTracker::track(DescriptorSet set, VkShaderStageFlags stages)
{
  descriptor_sets_.push(set, stages).unwrap();
  passes_.last().descriptor_sets++;
}

void CommandTracker::reset()
{
  buffers_.shrink().unwrap();
  buffers_.clear();
  images_.shrink().unwrap();
  images_.clear();
  descriptor_sets_.shrink().unwrap();
  descriptor_sets_.clear();
  passes_.shrink().unwrap();
  passes_.clear();
  first_cmd_ = nullptr;
  last_cmd_  = nullptr;
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
  logger->log(
    level, "[Type: {}, Id: {}, Name: {} ] {}"_str, span(message_type_s),
    data->messageIdNumber, cstr(data->pMessageIdName),
    data->pMessage == nullptr ? "(empty message)"_str : cstr(data->pMessage));

  if (data->objectCount > 0)
  {
    logger->log(level, "Objects Involved:"_str);
  }

  for (auto obj : Span{data->pObjects, data->objectCount})
  {
    logger->log(
      level, "[Type: {}] {}"_str, cstr(string_VkObjectType(obj.objectType)),
      obj.pObjectName == nullptr ? "(unnamed)"_str : cstr(obj.pObjectName));
  }

  if (data->queueLabelCount > 0)
  {
    logger->log(level, "Command Queues Involved:"_str);
  }

  for (auto queue : Span{data->pQueueLabels, data->queueLabelCount})
  {
    logger->log(level, "{}"_str,
                queue.pLabelName == nullptr ? "(unnamed)"_str :
                                              cstr(queue.pLabelName));
  }

  if (data->cmdBufLabelCount > 0)
  {
    logger->log(level, "Command Buffers Involved:"_str);
  }

  for (auto cmdbuf : Span{data->pCmdBufLabels, data->cmdBufLabelCount})
  {
    logger->log(level, "{}"_str,
                cmdbuf.pLabelName == nullptr ? "(unnamed)"_str :
                                               cstr(cmdbuf.pLabelName));
  }

  return VK_FALSE;
}

Result<Dyn<gpu::Instance>, Status> create_instance(Allocator allocator,
                                                   bool      enable_validation)
{
  SCRATCH_ALLOCATOR(allocator);

  u32  num_exts;
  auto result =
    vkEnumerateInstanceExtensionProperties(nullptr, &num_exts, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  Vec<VkExtensionProperties> extensions{scratch_};

  if (!extensions.resize_uninit(num_exts))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = vkEnumerateInstanceExtensionProperties(nullptr, &num_exts,
                                                  extensions.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  CHECK(extensions.size() == num_exts, "");

  u32 num_layers;
  result = vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  Vec<VkLayerProperties> layers{scratch_};

  if (!layers.resize_uninit(num_layers))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = vkEnumerateInstanceLayerProperties(&num_layers, layers.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  CHECK(layers.size() == num_layers, "");

  trace("Available Extensions:"_str);

  for (auto const & ext : extensions)
  {
    trace("{}\t\t(spec version {}.{}.{} variant {})"_str,
          cstr(ext.extensionName), VK_API_VERSION_MAJOR(ext.specVersion),
          VK_API_VERSION_MINOR(ext.specVersion),
          VK_API_VERSION_PATCH(ext.specVersion),
          VK_API_VERSION_VARIANT(ext.specVersion));
  }

  trace("Available Layers:"_str);

  for (auto const & layer : layers)
  {
    trace("{}\t\t(spec version {}.{}.{} variant {}, implementation version: "
          "{}.{}.{} variant {})"_str,
          cstr(layer.layerName), VK_API_VERSION_MAJOR(layer.specVersion),
          VK_API_VERSION_MINOR(layer.specVersion),
          VK_API_VERSION_PATCH(layer.specVersion),
          VK_API_VERSION_VARIANT(layer.specVersion),
          VK_API_VERSION_MAJOR(layer.implementationVersion),
          VK_API_VERSION_MINOR(layer.implementationVersion),
          VK_API_VERSION_PATCH(layer.implementationVersion),
          VK_API_VERSION_VARIANT(layer.implementationVersion));
  }

  Vec<Str> load_extensions{scratch_};

  Vec<Str> required_extensions{scratch_};

  required_extensions
    .extend(
      span({cstr(VK_KHR_SURFACE_EXTENSION_NAME),
            cstr(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)}))
    .unwrap();

  Vec<Str> optional_extensions{scratch_};

  optional_extensions
    .extend(span({cstr(VK_EXT_DEBUG_UTILS_EXTENSION_NAME),
                  cstr(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME),
                  "VK_KHR_android_surface"_str, "VK_MVK_ios_surface"_str,
                  "VK_MVK_macos_surface"_str, "VK_EXT_metal_surface"_str,
                  "VK_NN_vi_surface"_str, "VK_KHR_wayland_surface"_str,
                  "VK_KHR_win32_surface"_str, "VK_KHR_xcb_surface"_str,
                  "VK_KHR_xlib_surface"_str}))
    .unwrap();

  if (enable_validation)
  {
    optional_extensions.push(cstr(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)).unwrap();
  }

  for (auto ext : required_extensions)
  {
    CHECK(
      !find(extensions.view(), ext,
            [](auto a, auto b) { return mem::eq(cstr(a.extensionName), b); })
         .is_empty(),
      "Required Vulkan "
      "Instance Extension: {}  is not supported on instance",
      ext);
    load_extensions.push(ext).unwrap();
  }

  for (auto ext : optional_extensions)
  {
    if (find(extensions.view(), ext, [](auto a, auto b) {
          return mem::eq(cstr(a.extensionName), b);
        }).is_empty())
    {
      trace("Optional Instance Extension: {} is not supported"_str, ext);
    }
    else
    {
      optional_extensions.push(ext).unwrap();
    }
  }

  Vec<Str> optional_layers{scratch_};

  if (enable_validation)
  {
    optional_layers.push(DEBUG_LAYER_EXTENSION_NAME).unwrap();
  }

  Vec<Str> load_layers{scratch_};

  for (auto layer : optional_layers)
  {
    if (find(layers.view(), layer, [](auto a, auto b) {
          return mem::eq(cstr(a.layerName), b);
        }).is_empty())
    {
      trace("Optional Instance Layer: {} is not supported"_str, layer);
    }
    else
    {
      load_layers.push(layer).unwrap();
    }
  }

  auto validation_enabled =
    enable_validation &&
    !find(load_extensions.view(), cstr(VK_EXT_DEBUG_UTILS_EXTENSION_NAME),
          str_eq)
       .is_empty() &&
    !find(load_layers.view(), DEBUG_LAYER_EXTENSION_NAME, str_eq).is_empty();

  // setup before vkInstance to allow debug reporter report
  // messages through the pointer to it

  VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pNext = nullptr,
                             .pApplicationName   = CLIENT_NAME,
                             .applicationVersion = CLIENT_VERSION,
                             .pEngineName        = ENGINE_NAME,
                             .engineVersion      = ENGINE_VERSION,
                             .apiVersion         = ENGINE_VULKAN_VERSION};

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

  auto has_portability_ext =
    !find(load_extensions.view(),
          cstr(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME), str_eq)
       .is_empty();

  Vec<char const *> load_extensions_c{scratch_};

  for (auto l : load_extensions)
  {
    load_extensions_c.push(l.data()).unwrap();
  }

  Vec<char const *> load_layers_c{scratch_};

  for (auto l : load_layers)
  {
    load_layers_c.push(l.data()).unwrap();
  }

  // .pNext helps to debug issues with vkDestroyInstance and vkCreateInstance
  // i.e. (before and after the debug messenger is installed)
  VkInstanceCreateInfo create_info{
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext                   = enable_validation ? &debug_create_info : nullptr,
    .flags                   = has_portability_ext ?
                                 VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR :
                                 ((VkInstanceCreateFlags) 0),
    .pApplicationInfo        = &app_info,
    .enabledLayerCount       = size32(load_layers_c),
    .ppEnabledLayerNames     = load_layers_c.data(),
    .enabledExtensionCount   = size32(load_extensions_c),
    .ppEnabledExtensionNames = load_extensions_c.data()};

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
    dyn<IInstance>(inplace, allocator, allocator, vk_table, vk_instance,
                   vk_debug_messenger, validation_enabled);

  if (!instance)
  {
    return Err{Status::OutOfHostMemory};
  }

  vk_instance = nullptr;

  return Ok{cast<gpu::Instance>(std::move(instance.v()))};
}
}    // namespace vk

namespace gpu
{

Result<Dyn<gpu::Instance>, Status>
  create_vulkan_instance(Allocator allocator, bool enable_validation)
{
  return vk::create_instance(allocator, enable_validation);
}

}    // namespace gpu

namespace vk
{

IInstance::~IInstance()
{
  if (vk_ == nullptr)
  {
    return;
  }

  if (validation_enabled_)
  {
    table_.DestroyDebugUtilsMessengerEXT(vk_, vk_debug_messenger_, nullptr);
  }
  table_.DestroyInstance(vk_, nullptr);
}

void check_device_features(VkPhysicalDeviceFeatures const & feat)
{
  CHECK(feat.imageCubeArray == VK_TRUE, "");
  CHECK(feat.independentBlend == VK_TRUE, "");
  CHECK(feat.dualSrcBlend == VK_TRUE, "");
  CHECK(feat.depthClamp == VK_TRUE, "");
  CHECK(feat.depthBiasClamp == VK_TRUE, "");
  CHECK(feat.fillModeNonSolid == VK_TRUE, "");
  CHECK(feat.samplerAnisotropy == VK_TRUE, "");
  CHECK(feat.pipelineStatisticsQuery == VK_TRUE, "");
  CHECK(feat.fragmentStoresAndAtomics == VK_TRUE, "");
  CHECK(feat.shaderUniformBufferArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.shaderSampledImageArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.shaderStorageBufferArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.shaderStorageImageArrayDynamicIndexing == VK_TRUE, "");
  CHECK(feat.multiDrawIndirect == VK_TRUE, "");
  CHECK(feat.drawIndirectFirstInstance == VK_TRUE, "");
}

Result<gpu::Device, Status>
  IInstance::create_device(Allocator                   allocator,
                           Span<gpu::DeviceType const> preferred_types)
{
  SCRATCH_ALLOCATOR(allocator_);

  u32  num_devs;
  auto result = table_.EnumeratePhysicalDevices(vk_, &num_devs, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  if (num_devs == 0)
  {
    return Err{Status::DeviceLost};
  }

  Vec<VkPhysicalDevice> vk_phy_devs{scratch_};

  if (!vk_phy_devs.resize_uninit(num_devs))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = table_.EnumeratePhysicalDevices(vk_, &num_devs, vk_phy_devs.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  CHECK(num_devs == vk_phy_devs.size(), "");

  Vec<IPhysicalDevice> physical_devs{scratch_};
  if (!physical_devs.resize_uninit(num_devs))
  {
    return Err{Status::OutOfHostMemory};
  }

  for (auto [dev, vk_dev] : zip(physical_devs, vk_phy_devs))
  {
    VkPhysicalDeviceFeatures2KHR features{
      .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
      .pNext    = nullptr,
      .features = {}};
    table_.GetPhysicalDeviceFeatures2KHR(vk_dev, &features);
    table_.GetPhysicalDeviceMemoryProperties(vk_dev, &dev.vk_memory_properties);

    VkPhysicalDeviceDescriptorIndexingPropertiesEXT descriptor_properties{
      .sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT,
      .pNext                                                = nullptr,
      .maxUpdateAfterBindDescriptorsInAllPools              = {},
      .shaderUniformBufferArrayNonUniformIndexingNative     = {},
      .shaderSampledImageArrayNonUniformIndexingNative      = {},
      .shaderStorageBufferArrayNonUniformIndexingNative     = {},
      .shaderStorageImageArrayNonUniformIndexingNative      = {},
      .shaderInputAttachmentArrayNonUniformIndexingNative   = {},
      .robustBufferAccessUpdateAfterBind                    = {},
      .quadDivergentImplicitLod                             = {},
      .maxPerStageDescriptorUpdateAfterBindSamplers         = {},
      .maxPerStageDescriptorUpdateAfterBindUniformBuffers   = {},
      .maxPerStageDescriptorUpdateAfterBindStorageBuffers   = {},
      .maxPerStageDescriptorUpdateAfterBindSampledImages    = {},
      .maxPerStageDescriptorUpdateAfterBindStorageImages    = {},
      .maxPerStageDescriptorUpdateAfterBindInputAttachments = {},
      .maxPerStageUpdateAfterBindResources                  = {},
      .maxDescriptorSetUpdateAfterBindSamplers              = {},
      .maxDescriptorSetUpdateAfterBindUniformBuffers        = {},
      .maxDescriptorSetUpdateAfterBindUniformBuffersDynamic = {},
      .maxDescriptorSetUpdateAfterBindStorageBuffers        = {},
      .maxDescriptorSetUpdateAfterBindStorageBuffersDynamic = {},
      .maxDescriptorSetUpdateAfterBindSampledImages         = {},
      .maxDescriptorSetUpdateAfterBindStorageImages         = {},
      .maxDescriptorSetUpdateAfterBindInputAttachments      = {}};

    VkPhysicalDeviceProperties2KHR properties{
      .sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR,
      .pNext      = &descriptor_properties,
      .properties = {}};

    table_.GetPhysicalDeviceProperties2KHR(vk_dev, &properties);

    dev.vk                       = vk_dev;
    dev.vk_features              = features.features;
    dev.vk_properties            = properties.properties;
    dev.vk_descriptor_properties = descriptor_properties;
  }

  trace("Available Devices:"_str);
  for (auto [i, dev] : enumerate(physical_devs))
  {
    auto const & properties = dev.vk_properties;
    trace("[Device: {}] {} {} Vulkan API version {}.{}.{} variant {}, Driver "
          "Version: {}, Vendor ID: {}, Device ID: {}"_str,
          i, cstr(string_VkPhysicalDeviceType(properties.deviceType)),
          properties.deviceName, VK_API_VERSION_MAJOR(properties.apiVersion),
          VK_API_VERSION_MINOR(properties.apiVersion),
          VK_API_VERSION_PATCH(properties.apiVersion),
          VK_API_VERSION_VARIANT(properties.apiVersion),
          properties.driverVersion, properties.vendorID, properties.deviceID);

    u32 num_queue_families;
    table_.GetPhysicalDeviceQueueFamilyProperties2KHR(
      dev.vk, &num_queue_families, nullptr);

    Vec<VkQueueFamilyProperties2KHR> queue_family_properties{scratch_};

    for (auto _ : range(num_queue_families))
    {
      queue_family_properties
        .push(VkQueueFamilyProperties2KHR{
          .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR,
          .pNext = nullptr,
          .queueFamilyProperties = {}})
        .unwrap();
    }

    table_.GetPhysicalDeviceQueueFamilyProperties2KHR(
      dev.vk, &num_queue_families, queue_family_properties.data());
    CHECK(queue_family_properties.size() == num_queue_families, "");

    for (auto [i, prop] : enumerate<u32>(queue_family_properties))
    {
      trace("\t\tQueue Family: {}, Count: {}, Flags: {}"_str, i,
            prop.queueFamilyProperties.queueCount,
            string_VkQueueFlags(prop.queueFamilyProperties.queueFlags));
    }
  }

  u32 selected_dev_idx      = U32_MAX;
  u32 selected_queue_family = U32_MAX;

  for (auto preferred_type : preferred_types)
  {
    for (auto [idev, dev] : enumerate<u32>(physical_devs))
    {
      u32 num_queue_families;
      table_.GetPhysicalDeviceQueueFamilyProperties2KHR(
        dev.vk, &num_queue_families, nullptr);

      Vec<VkQueueFamilyProperties2KHR> queue_family_properties{scratch_};

      for (auto _ : range(num_queue_families))
      {
        queue_family_properties
          .push(VkQueueFamilyProperties2KHR{
            .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2_KHR,
            .pNext = nullptr,
            .queueFamilyProperties = {}})
          .unwrap();
      }

      table_.GetPhysicalDeviceQueueFamilyProperties2KHR(
        dev.vk, &num_queue_families, queue_family_properties.data());
      CHECK(queue_family_properties.size() == num_queue_families, "");

      if (((VkPhysicalDeviceType) preferred_type) ==
          dev.vk_properties.deviceType)
      {
        for (auto [iqueue_family, queue_family_prop] :
             enumerate<u32>(queue_family_properties))
        {
          if (has_bits(queue_family_prop.queueFamilyProperties.queueFlags,
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

      if (selected_dev_idx != U32_MAX)
      {
        break;
      }
    }

    if (selected_dev_idx != U32_MAX)
    {
      break;
    }
  }

  if (selected_dev_idx == U32_MAX)
  {
    trace("No Suitable Device Found"_str);
    return Err{Status::DeviceLost};
  }

  IPhysicalDevice selected_dev = physical_devs[selected_dev_idx];

  check_device_features(selected_dev.vk_features);

  trace("Selected Device {}"_str, selected_dev_idx);

  u32 num_extensions;
  result = table_.EnumerateDeviceExtensionProperties(selected_dev.vk, nullptr,
                                                     &num_extensions, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  Vec<VkExtensionProperties> extensions{scratch_};

  if (!extensions.resize_uninit(num_extensions))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = table_.EnumerateDeviceExtensionProperties(
    selected_dev.vk, nullptr, &num_extensions, extensions.data());
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  CHECK(num_extensions == extensions.size(), "");

  u32 num_layers;
  result = table_.EnumerateDeviceLayerProperties(selected_dev.vk, &num_layers,
                                                 nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  Vec<VkLayerProperties> layers{scratch_};

  if (!layers.resize_uninit(num_layers))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = table_.EnumerateDeviceLayerProperties(selected_dev.vk, &num_layers,
                                                 layers.data());
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  CHECK(layers.size() == num_layers, "");

  trace("Available Extensions:"_str);

  for (auto & ext : extensions)
  {
    trace("\t\t{} (spec version: {}.{}.{} variant {})"_str,
          cstr(ext.extensionName), VK_API_VERSION_MAJOR(ext.specVersion),
          VK_API_VERSION_MINOR(ext.specVersion),
          VK_API_VERSION_PATCH(ext.specVersion),
          VK_API_VERSION_VARIANT(ext.specVersion));
  }

  trace("Available Layers:");

  for (auto & layer : layers)
  {
    trace("\t\t{} (spec version: {}.{}.{} variant {}, implementation version: "
          "{}.{}.{} variant {})"_str,
          cstr(layer.layerName), VK_API_VERSION_MAJOR(layer.specVersion),
          VK_API_VERSION_MINOR(layer.specVersion),
          VK_API_VERSION_PATCH(layer.specVersion),
          VK_API_VERSION_VARIANT(layer.specVersion),
          VK_API_VERSION_MAJOR(layer.implementationVersion),
          VK_API_VERSION_MINOR(layer.implementationVersion),
          VK_API_VERSION_PATCH(layer.implementationVersion),
          VK_API_VERSION_VARIANT(layer.implementationVersion));
  }

  Vec<Str> required_extensions{scratch_};

  required_extensions
    .extend(span<Str>({cstr(VK_KHR_SWAPCHAIN_EXTENSION_NAME),
                       cstr(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME),
                       cstr(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME),
                       cstr(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME),
                       cstr(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME),
                       cstr(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)}))
    .unwrap();

  Vec<Str> optional_extensions{scratch_};

  optional_extensions
    .extend(span<Str>({cstr(VK_EXT_DEBUG_MARKER_EXTENSION_NAME),
                       cstr(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)}))
    .unwrap();

  Vec<Str> load_extensions{scratch_};

  for (auto ext : required_extensions)
  {
    CHECK(
      !find(extensions.view(), ext,
            [](auto a, auto b) { return mem::eq(cstr(a.extensionName), b); })
         .is_empty(),
      "Required Vulkan "
      "Device Extension: {}  is not supported on instance",
      ext);
    load_extensions.push(ext).unwrap();
  }

  for (auto ext : optional_extensions)
  {
    if (find(extensions.view(), ext, [](auto a, auto b) {
          return mem::eq(cstr(a.extensionName), b);
        }).is_empty())
    {
      trace("Optional Device Extension: {} is not supported"_str, ext);
    }
    else
    {
      load_extensions.push(ext).unwrap();
    }
  }

  auto has_debug_marker_ext =
    !find(load_extensions.view(), cstr(VK_EXT_DEBUG_MARKER_EXTENSION_NAME),
          str_eq)
       .is_empty();

  Vec<Str> optional_layers{scratch_};

  if (vk_debug_messenger_ != nullptr)
  {
    optional_layers.push(DEBUG_LAYER_EXTENSION_NAME).unwrap();
  }

  Vec<Str> load_layers{scratch_};

  for (auto layer : optional_layers)
  {
    if (find(layers.view(), layer, [](auto a, auto b) {
          return mem::eq(cstr(a.layerName), b);
        }).is_empty())
    {
      trace("Optional Device Layer: {} is not supported"_str, layer);
    }
    else
    {
      load_layers.push(layer).unwrap();
    }
  }

  Vec<char const *> load_extensions_c{scratch_};

  for (auto l : load_extensions)
  {
    load_extensions_c.push(l.data()).unwrap();
  }

  Vec<char const *> load_layers_c{scratch_};

  for (auto l : load_layers)
  {
    load_layers_c.push(l.data()).unwrap();
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
    .robustBufferAccess        = VK_FALSE,
    .fullDrawIndexUint32       = VK_FALSE,
    .imageCubeArray            = VK_TRUE,
    .independentBlend          = VK_TRUE,
    .geometryShader            = selected_dev.vk_features.geometryShader,
    .tessellationShader        = selected_dev.vk_features.tessellationShader,
    .sampleRateShading         = VK_FALSE,
    .dualSrcBlend              = VK_TRUE,
    .logicOp                   = VK_FALSE,
    .multiDrawIndirect         = VK_TRUE,
    .drawIndirectFirstInstance = VK_TRUE,
    .depthClamp                = VK_TRUE,
    .depthBiasClamp            = VK_TRUE,
    .fillModeNonSolid          = selected_dev.vk_features.fillModeNonSolid,
    .depthBounds               = selected_dev.vk_features.depthBounds,
    .wideLines                 = selected_dev.vk_features.wideLines,
    .largePoints               = selected_dev.vk_features.largePoints,
    .alphaToOne                = selected_dev.vk_features.alphaToOne,
    .multiViewport             = selected_dev.vk_features.multiViewport,
    .samplerAnisotropy         = VK_TRUE,
    .textureCompressionETC2 = selected_dev.vk_features.textureCompressionETC2,
    .textureCompressionASTC_LDR =
      selected_dev.vk_features.textureCompressionASTC_LDR,
    .textureCompressionBC    = selected_dev.vk_features.textureCompressionBC,
    .occlusionQueryPrecise   = VK_FALSE,
    .pipelineStatisticsQuery = VK_TRUE,
    .vertexPipelineStoresAndAtomics          = VK_FALSE,
    .fragmentStoresAndAtomics                = VK_TRUE,
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

  VkDeviceCreateInfo create_info{
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext                   = &descriptor_indexing_features,
    .flags                   = 0,
    .queueCreateInfoCount    = 1,
    .pQueueCreateInfos       = &queue_create_info,
    .enabledLayerCount       = size32(load_layers_c),
    .ppEnabledLayerNames     = load_layers_c.data(),
    .enabledExtensionCount   = size32(load_extensions_c),
    .ppEnabledExtensionNames = load_extensions_c.data(),
    .pEnabledFeatures        = &features};

  VkDevice vk_dev;
  result = table_.CreateDevice(selected_dev.vk, &create_info, nullptr, &vk_dev);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  DeviceTable        vk_dev_table;
  VmaVulkanFunctions vma_table;
  CHECK(load_device_table(vk_dev, table_, vk_dev_table, has_debug_marker_ext),
        "");

  load_vma_table(table_, vk_dev_table, vma_table);

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
    .physicalDevice = selected_dev.vk,
    .device         = vk_dev,
    .preferredLargeHeapBlockSize    = 0,
    .pAllocationCallbacks           = nullptr,
    .pDeviceMemoryCallbacks         = nullptr,
    .pHeapSizeLimit                 = nullptr,
    .pVulkanFunctions               = &vma_table,
    .instance                       = vk_,
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

  IDevice * dev;

  if (!allocator->nalloc(1, dev))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (dev) IDevice{allocator,    *this,  selected_dev,          vk_dev_table,
                    vma_table,    vk_dev, selected_queue_family, vk_queue,
                    vma_allocator};

  vma_allocator = nullptr;
  vk_dev        = nullptr;
  dev           = nullptr;

  auto queue_label = "CommandQueue 0"_str;
  dev->set_resource_name(queue_label, dev->vk_queue_, VK_OBJECT_TYPE_QUEUE,
                         VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, scratch_);

  return Ok<gpu::Device>{dev};
}

gpu::Backend IInstance::get_backend()
{
  return gpu::Backend::Vulkan;
}

void IInstance::uninit(gpu::Device device_)
{
  auto * dev = (Device) device_;

  if (dev == nullptr)
  {
    return;
  }

  dev->uninit();
  allocator_->ndealloc(1, dev);
}

void IInstance::uninit(gpu::Surface surface)
{
  table_.DestroySurfaceKHR(vk_, (Surface) surface, nullptr);
}

void IDevice::set_resource_name(Str label, void const * resource,
                                VkObjectType               type,
                                VkDebugReportObjectTypeEXT debug_type,
                                Allocator                  scratch)
{
  Vec<char> label_c_str{scratch};

  label_c_str.extend(label).unwrap();
  label_c_str.push('\0').unwrap();

  VkDebugUtilsObjectNameInfoEXT name_info{
    .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
    .pNext        = nullptr,
    .objectType   = type,
    .objectHandle = (u64) resource,
    .pObjectName  = label_c_str.data()};

  instance_->table_.SetDebugUtilsObjectNameEXT(vk_dev_, &name_info);
  VkDebugMarkerObjectNameInfoEXT debug_info{
    .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
    .pNext       = nullptr,
    .objectType  = debug_type,
    .object      = (u64) resource,
    .pObjectName = label_c_str.data()};

  table_.DebugMarkerSetObjectNameEXT(vk_dev_, &debug_info);
}

gpu::DeviceProperties IDevice::get_properties()
{
  VkPhysicalDeviceProperties const & vk_properties = phy_.vk_properties;

  bool has_uma = false;
  for (u32 i = 0; i < phy_.vk_memory_properties.memoryTypeCount; i++)
  {
    if (has_bits(phy_.vk_memory_properties.memoryTypes[i].propertyFlags,
                 (VkMemoryPropertyFlags) (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)))
    {
      has_uma = true;
      break;
    }
  }

  auto & vklimit = phy_.vk_properties.limits;

  gpu::DeviceLimits limits{
    .image_extent_1d           = vklimit.maxImageDimension1D,
    .image_extent_2d           = vklimit.maxImageDimension2D,
    .image_extent_3d           = vklimit.maxImageDimension3D,
    .image_extent_cube         = vklimit.maxImageDimensionCube,
    .image_array_layers        = vklimit.maxImageArrayLayers,
    .uniform_buffer_range      = vklimit.maxUniformBufferRange,
    .storage_buffer_range      = vklimit.maxStorageBufferRange,
    .push_constants_size       = vklimit.maxPushConstantsSize,
    .bound_descriptor_sets     = vklimit.maxBoundDescriptorSets,
    .per_stage_samplers        = vklimit.maxPerStageDescriptorSamplers,
    .per_stage_uniform_buffers = vklimit.maxPerStageDescriptorUniformBuffers,
    .per_stage_storage_buffers = vklimit.maxPerStageDescriptorStorageBuffers,
    .per_stage_sampled_images  = vklimit.maxPerStageDescriptorSampledImages,
    .per_stage_storage_images  = vklimit.maxPerStageDescriptorStorageImages,
    .per_stage_input_attachments =
      vklimit.maxPerStageDescriptorInputAttachments,
    .per_stage_resources            = vklimit.maxPerStageResources,
    .descriptor_set_samplers        = vklimit.maxDescriptorSetSamplers,
    .descriptor_set_uniform_buffers = vklimit.maxDescriptorSetUniformBuffers,
    .descriptor_set_uniform_buffers_dynamic =
      vklimit.maxDescriptorSetUniformBuffersDynamic,
    .descriptor_set_storage_buffers = vklimit.maxDescriptorSetStorageBuffers,
    .descriptor_set_storage_buffers_dynamic =
      vklimit.maxDescriptorSetStorageBuffersDynamic,
    .descriptor_set_sampled_images = vklimit.maxDescriptorSetSampledImages,
    .descriptor_set_storage_images = vklimit.maxDescriptorSetStorageImages,
    .descriptor_set_input_attachments =
      vklimit.maxDescriptorSetInputAttachments,
    .compute_work_group_invocations  = vklimit.maxComputeWorkGroupInvocations,
    .compute_shared_memory_size      = vklimit.maxComputeSharedMemorySize,
    .compute_work_groups             = {vklimit.maxComputeWorkGroupCount[0],
                                        vklimit.maxComputeWorkGroupCount[1],
                                        vklimit.maxComputeWorkGroupCount[2]},
    .compute_work_group_size         = {vklimit.maxComputeWorkGroupSize[0],
                                        vklimit.maxComputeWorkGroupSize[1],
                                        vklimit.maxComputeWorkGroupSize[2]},
    .draw_indirect                   = vklimit.maxDrawIndirectCount,
    .sampler_lod                     = vklimit.maxSamplerLodBias,
    .sampler_anisotropy              = vklimit.maxSamplerAnisotropy,
    .viewports                       = vklimit.maxViewports,
    .viewport_extent                 = {vklimit.maxViewportDimensions[0],
                                        vklimit.maxViewportDimensions[1]},
    .uniform_buffer_offset_alignment = vklimit.minUniformBufferOffsetAlignment,
    .texel_buffer_offset_alignment   = vklimit.minTexelBufferOffsetAlignment,
    .storage_buffer_offset_alignment = vklimit.minStorageBufferOffsetAlignment,
    .framebuffer_extent              = {vklimit.maxFramebufferWidth,
                                        vklimit.maxFramebufferHeight},
    .framebuffer_layers              = vklimit.maxFramebufferLayers,
    .framebuffer_color_samples       = vklimit.framebufferColorSampleCounts,
    .framebuffer_depth_samples       = vklimit.framebufferDepthSampleCounts,
    .framebuffer_stencil_samples     = vklimit.framebufferStencilSampleCounts,
    .sampled_image_color_samples     = vklimit.sampledImageColorSampleCounts,
    .sampled_image_depth_samples     = vklimit.sampledImageDepthSampleCounts,
    .sampled_image_stencil_samples   = vklimit.sampledImageStencilSampleCounts,
    .storage_image_sample_samples    = vklimit.storageImageSampleCounts
  };

  auto & vkfeat = phy_.vk_features;

  gpu::DeviceFeatures features{
    .geometry_shader          = vkfeat.geometryShader == VK_TRUE,
    .tessellation_shader      = vkfeat.tessellationShader == VK_TRUE,
    .depth_bounds             = vkfeat.depthBounds == VK_TRUE,
    .wide_lines               = vkfeat.wideLines == VK_TRUE,
    .large_points             = vkfeat.largePoints == VK_TRUE,
    .alpha_to_one             = vkfeat.alphaToOne == VK_TRUE,
    .multi_viewport           = vkfeat.multiViewport == VK_TRUE,
    .texture_compression_etc2 = vkfeat.textureCompressionETC2 == VK_TRUE,
    .texture_compression_astc_ldr =
      vkfeat.textureCompressionASTC_LDR == VK_TRUE,
    .texture_compression_bc = vkfeat.textureCompressionBC == VK_TRUE,
    .shader_float64         = vkfeat.shaderFloat64 == VK_TRUE,
    .shader_int64           = vkfeat.shaderInt64 == VK_TRUE,
    .shader_int16           = vkfeat.shaderInt16 == VK_TRUE,
  };

  gpu::DeviceProperties properties{
    .api_version        = vk_properties.apiVersion,
    .driver_version     = vk_properties.driverVersion,
    .vendor_id          = vk_properties.vendorID,
    .device_id          = vk_properties.deviceID,
    .device_name        = cstr(vk_properties.deviceName),
    .type               = (gpu::DeviceType) vk_properties.deviceType,
    .has_unified_memory = has_uma,
    .timestamp_period   = vk_properties.limits.timestampPeriod,
    .features           = features,
    .limits             = limits};

  return properties;
}

Result<gpu::FormatProperties, Status>
  IDevice::get_format_properties(gpu::Format format)
{
  VkFormatProperties2KHR props{.sType =
                                 VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2_KHR,
                               .pNext            = nullptr,
                               .formatProperties = {}};
  instance_->table_.GetPhysicalDeviceFormatProperties2KHR(
    phy_.vk, (VkFormat) format, &props);
  return Ok(gpu::FormatProperties{
    .linear_tiling_features =
      (gpu::FormatFeatures) props.formatProperties.linearTilingFeatures,
    .optimal_tiling_features =
      (gpu::FormatFeatures) props.formatProperties.optimalTilingFeatures,
    .buffer_features =
      (gpu::FormatFeatures) props.formatProperties.bufferFeatures});
}

Result<gpu::Buffer, Status> IDevice::create_buffer(gpu::BufferInfo const & info)
{
  CHECK(info.size != 0, "");
  CHECK(info.usage != gpu::BufferUsage::None, "");
  SCRATCH_ALLOCATOR(allocator_);

  VkBufferCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size  = info.size,
                                 .usage = (VkBufferUsageFlags) info.usage,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 1,
                                 .pQueueFamilyIndices   = nullptr};

  VkBuffer vk;

  auto result = table_.CreateBuffer(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, scratch_);

  IBuffer * buffer;

  if (!allocator_->nalloc(1, buffer))
  {
    table_.DestroyBuffer(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (buffer) IBuffer{
    .vk             = vk,
    .usage          = info.usage,
    .host_mapped    = info.host_mapped,
    .size           = info.size,
    .memory         = MemoryInfo{.alias   = nullptr,
                                 .element = 0,
                                 .type    = gpu::MemoryType::Unique},
    .bind_locations = BindLocations{allocator_}
  };

  if (info.memory_type == gpu::MemoryType::Unique)
  {
    Enum<gpu::Buffer, gpu::Image> const resources[] = {(gpu::Buffer) buffer};

    auto status = create_alias(gpu::AliasInfo{.resources = resources});

    if (!status)
    {
      uninit((gpu::Buffer) buffer);
      return Err{status.err()};
    }
  }

  return Ok{(gpu::Buffer) buffer};
}

Result<gpu::BufferView, Status>
  IDevice::create_buffer_view(gpu::BufferViewInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  auto * buffer = (Buffer) info.buffer;

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

  auto result = table_.CreateBufferView(vk_dev_, &create_info, nullptr, &vk);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_BUFFER_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, scratch_);

  IBufferView * view;

  if (!allocator_->nalloc(1, view))
  {
    table_.DestroyBufferView(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) IBufferView{.vk             = vk,
                         .buffer         = buffer,
                         .slice          = slice,
                         .bind_locations = BindLocations{allocator_}};

  return Ok{(gpu::BufferView) view};
}

Result<gpu::Image, Status> IDevice::create_image(gpu::ImageInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  CHECK(info.format != gpu::Format::Undefined, "");
  CHECK(info.usage != gpu::ImageUsage::None, "");
  CHECK(info.aspects != gpu::ImageAspects::None, "");
  CHECK(info.sample_count != gpu::SampleCount::None, "");
  CHECK(!info.extent.any_zero(), "");
  CHECK(info.mip_levels > 0, "");
  CHECK(info.mip_levels <= info.extent.mips(), "");
  CHECK(info.array_layers > 0, "");
  CHECK(info.array_layers <= phy_.vk_properties.limits.maxImageArrayLayers, "");

  switch (info.type)
  {
    case gpu::ImageType::Type1D:
      CHECK(info.extent.x() <= phy_.vk_properties.limits.maxImageDimension1D,
            "");
      CHECK(info.extent.y() == 1, "");
      CHECK(info.extent.z() == 1, "");
      break;

    case gpu::ImageType::Type2D:
      CHECK(info.extent.x() <= phy_.vk_properties.limits.maxImageDimension2D,
            "");
      CHECK(info.extent.y() <= phy_.vk_properties.limits.maxImageDimension2D,
            "");
      CHECK(info.extent.z() == 1, "");
      break;

    case gpu::ImageType::Type3D:
      CHECK(info.extent.x() <= phy_.vk_properties.limits.maxImageDimension3D,
            "");
      CHECK(info.extent.y() <= phy_.vk_properties.limits.maxImageDimension3D,
            "");
      CHECK(info.extent.z() <= phy_.vk_properties.limits.maxImageDimension3D,
            "");
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

  auto result = table_.CreateImage(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_IMAGE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, scratch_);

  IImage * image;

  if (!allocator_->nalloc(1, image))
  {
    table_.DestroyImage(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (image) IImage{
    .vk                 = vk,
    .type               = info.type,
    .usage              = info.usage,
    .aspects            = info.aspects,
    .sample_count       = info.sample_count,
    .extent             = info.extent,
    .mip_levels         = info.mip_levels,
    .array_layers       = info.array_layers,
    .is_swapchain_image = false,
    .memory =
      MemoryInfo{.alias = nullptr, .element = 0, .type = info.memory_type}
  };

  if (info.memory_type == gpu::MemoryType::Unique)
  {
    Enum<gpu::Buffer, gpu::Image> const resources[] = {(gpu::Image) image};

    auto status = create_alias(gpu::AliasInfo{.resources = resources});

    if (!status)
    {
      uninit((gpu::Image) image);
      return Err{status.err()};
    }
  }

  return Ok{(gpu::Image) image};
}

Result<gpu::ImageView, Status>
  IDevice::create_image_view(gpu::ImageViewInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  auto * src_image = (Image) info.image;

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
  auto result = table_.CreateImageView(vk_dev_, &create_info, nullptr, &vk);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_IMAGE_VIEW,
                    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, scratch_);

  IImageView * view;

  if (!allocator_->nalloc(1, view))
  {
    table_.DestroyImageView(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (view) IImageView{.vk             = vk,
                        .image          = src_image,
                        .format         = info.view_format,
                        .mip_levels     = mip_levels,
                        .array_layers   = array_layers,
                        .bind_locations = BindLocations{allocator_}};

  return Ok{(gpu::ImageView) view};
}

Result<gpu::Alias, Status> IDevice::create_alias(gpu::AliasInfo const & info)
{
  CHECK(!info.resources.is_empty(), "");

  for (auto & resource : info.resources)
  {
    resource.match(
      [&](gpu::Buffer p) {
        auto buffer = (Buffer) p;
        CHECK(buffer->memory.alias == nullptr, "");
      },
      [&](gpu::Image p) {
        auto image = (Image) p;
        CHECK(image->memory.alias == nullptr, "");
      });
  }

  Layout64 layout{};
  u32      memory_type_bits = 0;
  auto     host_mapped      = false;

  for (auto & resource : info.resources)
  {
    VkMemoryRequirements req{};
    resource.match(
      [&](gpu::Buffer p) {
        auto buffer = ptr(p);
        table_.GetBufferMemoryRequirements(vk_dev_, buffer->vk, &req);
        host_mapped = host_mapped || buffer->host_mapped;
      },
      [&](gpu::Image p) {
        auto image = ptr(p);
        table_.GetImageMemoryRequirements(vk_dev_, image->vk, &req);
      });

    layout =
      layout.unioned(Layout64{.alignment = req.alignment, .size = req.size});

    memory_type_bits |= req.memoryTypeBits;
  }

  layout = layout.aligned();

  VmaAllocationCreateFlags flags =
    host_mapped ? (VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                   VMA_ALLOCATION_CREATE_MAPPED_BIT) :
                  0;

  VmaAllocationCreateInfo alloc_create_info{
    .flags          = flags,
    .usage          = VMA_MEMORY_USAGE_AUTO,
    .requiredFlags  = {},
    .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    .memoryTypeBits = memory_type_bits,
    .pool           = nullptr,
    .pUserData      = nullptr,
    .priority       = 0};

  VmaAllocationInfo    vma_allocation_info;
  VmaAllocation        vma_allocation;
  VkMemoryRequirements requirements{.size           = layout.size,
                                    .alignment      = layout.alignment,
                                    .memoryTypeBits = memory_type_bits};

  auto result =
    vmaAllocateMemory(vma_allocator_, &requirements, &alloc_create_info,
                      &vma_allocation, &vma_allocation_info);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  CHECK(!(host_mapped && (vma_allocation_info.pMappedData == nullptr)), "");

  auto id = allocate_alias_id();

  IAlias * alias;

  CHECK(allocator_->nalloc(1, alias), "");

  new (alias) IAlias{.id             = id,
                     .vma_allocation = vma_allocation,
                     .layout         = layout,
                     .map            = vma_allocation_info.pMappedData};

  for (auto [i, resource] : enumerate(info.resources))
  {
    VkResult result = VK_SUCCESS;
    resource.match(
      [&](gpu::Buffer p) {
        auto buffer = ptr(p);
        result      = vmaBindBufferMemory2(vma_allocator_, vma_allocation, 0,
                                           buffer->vk, nullptr);
        buffer->memory.alias   = alias;
        buffer->memory.element = i;
      },
      [&](gpu::Image p) {
        auto image = ptr(p);
        result     = vmaBindImageMemory2(vma_allocator_, vma_allocation, 0,
                                         image->vk, nullptr);
        image->memory.alias   = alias;
        image->memory.element = i;
      });

    CHECK(result == VK_SUCCESS, "");
  }

  return Ok{(gpu::Alias) alias};
}

Result<gpu::Alias, Status>
  IDevice::create_shim_alias(gpu::AliasInfo const & info)
{
  CHECK(!info.resources.is_empty(), "");

  for (auto & resource : info.resources)
  {
    resource.match([&](gpu::Buffer) { CHECK(false, ""); },
                   [&](gpu::Image p) {
                     auto image = (Image) p;
                     CHECK(image->memory.alias == nullptr, "");
                   });
  }

  auto id = allocate_alias_id();

  IAlias * alias;

  CHECK(allocator_->nalloc(1, alias), "");

  new (alias)
    IAlias{.id = id, .vma_allocation = nullptr, .layout = {}, .map = nullptr};

  for (auto [i, resource] : enumerate(info.resources))
  {
    VkResult result = VK_SUCCESS;
    resource.match([&](gpu::Buffer) { CHECK(false, ""); },
                   [&](gpu::Image p) {
                     auto image            = ptr(p);
                     image->memory.alias   = alias;
                     image->memory.element = i;
                   });

    CHECK(result == VK_SUCCESS, "");
  }

  return Ok{(gpu::Alias) alias};
}

Result<gpu::Sampler, Status>
  IDevice::create_sampler(gpu::SamplerInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  CHECK(
    !(info.anisotropy_enable &&
      (info.max_anisotropy > phy_.vk_properties.limits.maxSamplerAnisotropy)),
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
    table_.CreateSampler(vk_dev_, &create_info, nullptr, &vk_sampler);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_sampler, VK_OBJECT_TYPE_SAMPLER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, scratch_);

  return Ok{(gpu::Sampler) vk_sampler};
}

Result<gpu::Shader, Status> IDevice::create_shader(gpu::ShaderInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  CHECK(info.spirv_code.size_bytes() > 0, "");

  VkShaderModuleCreateInfo create_info{
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = nullptr,
    .flags    = 0,
    .codeSize = info.spirv_code.size_bytes(),
    .pCode    = info.spirv_code.data()};

  VkShaderModule vk_shader;
  auto           result =
    table_.CreateShaderModule(vk_dev_, &create_info, nullptr, &vk_shader);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_shader, VK_OBJECT_TYPE_SHADER_MODULE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, scratch_);

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

Result<gpu::DescriptorSetLayout, Status> IDevice::create_descriptor_set_layout(
  gpu::DescriptorSetLayoutInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

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
  CHECK((num_dynamic_read_storage_buffers +
         num_dynamic_read_write_storage_buffers) <=
          phy_.vk_descriptor_properties
            .maxDescriptorSetUpdateAfterBindStorageBuffersDynamic,
        "");
  CHECK(num_dynamic_uniform_buffers <=
          phy_.vk_descriptor_properties
            .maxDescriptorSetUpdateAfterBindUniformBuffersDynamic,
        "");
  CHECK(num_descriptors <=
          phy_.vk_descriptor_properties.maxPerStageUpdateAfterBindResources,
        "");
  CHECK(num_variable_length <= 1, "");
  CHECK(
    !(num_variable_length > 0 && (num_dynamic_read_storage_buffers > 0 ||
                                  num_dynamic_read_write_storage_buffers > 0 ||
                                  num_dynamic_uniform_buffers > 0)),
    "Variable-length descriptor sets must not have dynamic offsets");

  for (auto [i, binding] : enumerate<u32>(info.bindings))
  {
    CHECK(binding.count > 0, "");
    CHECK(binding.count <=
            phy_.vk_descriptor_properties.maxPerStageUpdateAfterBindResources,
          "");
    CHECK(!(binding.is_variable_length && (i != (info.bindings.size() - 1))),
          "");
  }

  Vec<VkDescriptorSetLayoutBinding, 0> vk_bindings{scratch_};
  Vec<VkDescriptorBindingFlagsEXT, 0>  vk_binding_flags;

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
    table_.CreateDescriptorSetLayout(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_{[&] {
    if (vk != nullptr)
    {
      table_.DestroyDescriptorSetLayout(vk_dev_, vk, nullptr);
    }
  }};

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
                    scratch_);

  IDescriptorSetLayout * layout;

  if (!allocator_->nalloc(1, layout))
  {
    return Err{Status::OutOfHostMemory};
  }

  SmallVec<gpu::DescriptorBindingInfo, 1, 0> bindings{allocator_};
  bindings.extend(info.bindings).unwrap();

  auto is_mutating = is_mutating_set(info.bindings);

  new (layout) IDescriptorSetLayout{.vk                  = vk,
                                    .bindings            = std::move(bindings),
                                    .num_variable_length = num_variable_length,
                                    .is_mutating         = is_mutating};

  vk = nullptr;

  return Ok{(gpu::DescriptorSetLayout) layout};
}

Result<gpu::DescriptorSet, Status>
  IDevice::create_descriptor_set(gpu::DescriptorSetInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  auto * layout = (DescriptorSetLayout) info.layout;
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

  Vec<u32, 0> bindings_sizes{scratch_};

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
    table_.CreateDescriptorPool(vk_dev_, &create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer vk_pool_{
    [&] { table_.DestroyDescriptorPool(vk_dev_, vk_pool, nullptr); }};

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
  result = table_.AllocateDescriptorSets(vk_dev_, &alloc_info, &vk);

  // must not have these errors
  CHECK(result != VK_ERROR_OUT_OF_POOL_MEMORY &&
          result != VK_ERROR_FRAGMENTED_POOL,
        "");

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_DESCRIPTOR_SET,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, scratch_);

  set_resource_name(info.label, vk_pool, VK_OBJECT_TYPE_DESCRIPTOR_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, scratch_);

  SmallVec<DescriptorBinding, 1, 0> bindings{allocator_};

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
          SmallVec<Buffer, 4, 0>::make(size, allocator_).unwrap();
      }
      break;
      case SyncResourceType::BufferView:
      {
        binding.sync_resources =
          SmallVec<BufferView, 4, 0>::make(size, allocator_).unwrap();
      }
      break;
      case SyncResourceType::ImageView:
      {
        binding.sync_resources =
          SmallVec<ImageView, 4, 0>::make(size, allocator_).unwrap();
      }
      break;
    }
  }

  IDescriptorSet * set;

  if (!allocator_->nalloc(1, set))
  {
    return Err{(Status) result};
  }

  auto id = allocate_descriptor_set_id();

  new (set) IDescriptorSet{.vk          = vk,
                           .vk_pool     = vk_pool,
                           .id          = id,
                           .is_mutating = layout->is_mutating,
                           .bindings    = std::move(bindings)};

  vk_pool = nullptr;
  vk      = nullptr;

  return Ok{(gpu::DescriptorSet) set};
}

Result<gpu::PipelineCache, Status>
  IDevice::create_pipeline_cache(gpu::PipelineCacheInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  VkPipelineCacheCreateInfo create_info{
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
    .pNext           = nullptr,
    .flags           = 0,
    .initialDataSize = info.initial_data.size_bytes(),
    .pInitialData    = info.initial_data.data()};

  VkPipelineCache vk_cache;
  auto            result =
    table_.CreatePipelineCache(vk_dev_, &create_info, nullptr, &vk_cache);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_cache, VK_OBJECT_TYPE_PIPELINE_CACHE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, scratch_);

  return Ok{(gpu::PipelineCache) vk_cache};
}

Result<gpu::ComputePipeline, Status>
  IDevice::create_compute_pipeline(gpu::ComputePipelineInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  CHECK(info.descriptor_set_layouts.size() <=
          phy_.vk_properties.limits.maxBoundDescriptorSets,
        "");
  CHECK(info.push_constants_size <=
          phy_.vk_properties.limits.maxPushConstantsSize,
        "");
  CHECK(is_aligned(4U, info.push_constants_size), "");
  CHECK(info.compute_shader.entry_point.size() > 0 &&
          info.compute_shader.entry_point.size() < 256,
        "");
  CHECK(info.compute_shader.shader != nullptr, "");

  Vec<VkDescriptorSetLayout, 0> vk_descriptor_set_layouts{scratch_};

  for (auto layout : info.descriptor_set_layouts)
  {
    vk_descriptor_set_layouts.push(((DescriptorSetLayout) layout)->vk).unwrap();
  }

  VkSpecializationInfo vk_specialization{
    .mapEntryCount = size32(info.compute_shader.specialization_constants),
    .pMapEntries   = (VkSpecializationMapEntry const *)
                     info.compute_shader.specialization_constants.data(),
    .dataSize = info.compute_shader.specialization_constants_data.size_bytes(),
    .pData    = info.compute_shader.specialization_constants_data.data()};

  Vec<char> entry_point{scratch_};
  entry_point.extend(info.compute_shader.entry_point).unwrap();
  entry_point.push('\0').unwrap();

  VkPipelineShaderStageCreateInfo vk_stage{
    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext               = nullptr,
    .flags               = 0,
    .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
    .module              = (Shader) info.compute_shader.shader,
    .pName               = entry_point.data(),
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
  auto result = table_.CreatePipelineLayout(vk_dev_, &layout_create_info,
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
  result = table_.CreateComputePipelines(
    vk_dev_, info.cache == nullptr ? nullptr : (PipelineCache) info.cache, 1,
    &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, scratch_);
  set_resource_name(info.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, scratch_);

  IComputePipeline * pipeline;

  if (!allocator_->nalloc(1, pipeline))
  {
    table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    table_.DestroyPipeline(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
    IComputePipeline{.vk                  = vk,
                     .vk_layout           = vk_layout,
                     .push_constants_size = info.push_constants_size,
                     .num_sets = size32(info.descriptor_set_layouts)};

  return Ok{(gpu::ComputePipeline) pipeline};
}

Result<gpu::GraphicsPipeline, Status>
  IDevice::create_graphics_pipeline(gpu::GraphicsPipelineInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  CHECK(!(info.rasterization_state.polygon_mode != gpu::PolygonMode::Fill &&
          !phy_.vk_features.fillModeNonSolid),
        "");
  CHECK(info.descriptor_set_layouts.size() <=
          phy_.vk_properties.limits.maxBoundDescriptorSets,
        "");
  CHECK(info.push_constants_size <=
          phy_.vk_properties.limits.maxPushConstantsSize,
        "");
  CHECK(is_aligned(4U, info.push_constants_size), "");
  CHECK(!info.vertex_shader.entry_point.is_empty(), "");
  CHECK(!info.fragment_shader.entry_point.is_empty(), "");
  CHECK(info.vertex_attributes.size() <=
          phy_.vk_properties.limits.maxVertexInputAttributes,
        "");
  CHECK(info.color_blend_state.attachments.size() <=
          phy_.vk_properties.limits.maxColorAttachments,
        "");

  Vec<VkDescriptorSetLayout, 0> vk_descriptor_set_layouts{scratch_};

  for (auto layout : info.descriptor_set_layouts)
  {
    vk_descriptor_set_layouts.push(((DescriptorSetLayout) layout)->vk).unwrap();
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

  Vec<char> vs_entry_point{scratch_};
  vs_entry_point.extend(info.vertex_shader.entry_point).unwrap();
  vs_entry_point.push('\0').unwrap();

  Vec<char> fs_entry_point{scratch_};
  fs_entry_point.extend(info.fragment_shader.entry_point).unwrap();
  fs_entry_point.push('\0').unwrap();

  VkPipelineShaderStageCreateInfo vk_stages[2] = {
    {.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
     .pNext               = nullptr,
     .flags               = 0,
     .stage               = VK_SHADER_STAGE_VERTEX_BIT,
     .module              = (Shader) info.vertex_shader.shader,
     .pName               = vs_entry_point.data(),
     .pSpecializationInfo = &vk_vs_specialization},
    {.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
     .pNext               = nullptr,
     .flags               = 0,
     .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
     .module              = (Shader) info.fragment_shader.shader,
     .pName               = fs_entry_point.data(),
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

  auto result = table_.CreatePipelineLayout(vk_dev_, &layout_create_info,
                                            nullptr, &vk_layout);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  Vec<VkVertexInputBindingDescription, 0> input_bindings{scratch_};

  for (auto binding : info.vertex_input_bindings)
  {
    input_bindings
      .push(VkVertexInputBindingDescription{
        .binding   = binding.binding,
        .stride    = binding.stride,
        .inputRate = (VkVertexInputRate) binding.input_rate})
      .unwrap();
  }

  Vec<VkVertexInputAttributeDescription, 0> attributes{scratch_};

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

  Vec<VkPipelineColorBlendAttachmentState, 0> attachment_states{scratch_};

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

  Vec<VkFormat, 0> color_formats{scratch_};

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
  result = table_.CreateGraphicsPipelines(
    vk_dev_, info.cache == nullptr ? nullptr : (PipelineCache) info.cache, 1,
    &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_PIPELINE,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, scratch_);
  set_resource_name(info.label, vk_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, scratch_);

  IGraphicsPipeline * pipeline;

  if (!allocator_->nalloc(1, pipeline))
  {
    table_.DestroyPipelineLayout(vk_dev_, vk_layout, nullptr);
    table_.DestroyPipeline(vk_dev_, vk, nullptr);
    return Err{Status::OutOfHostMemory};
  }

  new (pipeline)
    IGraphicsPipeline{.vk                  = vk,
                      .vk_layout           = vk_layout,
                      .push_constants_size = info.push_constants_size,
                      .num_sets = size32(info.descriptor_set_layouts),
                      .color_fmts{allocator_},
                      .depth_fmt    = info.depth_format,
                      .stencil_fmt  = info.stencil_format,
                      .sample_count = info.rasterization_state.sample_count,
                      .num_vertex_attributes = size32(info.vertex_attributes)};

  pipeline->color_fmts.extend(info.color_formats).unwrap();

  return Ok{(gpu::GraphicsPipeline) pipeline};
}

Result<Void, Status> IDevice::recreate_swapchain(Swapchain swapchain)
{
  SCRATCH_ALLOCATOR(allocator_);

  auto info = std::move(swapchain->preference);
  CHECK(info.preferred_extent.x() > 0, "");
  CHECK(info.preferred_extent.y() > 0, "");

  auto vk_surface       = (VkSurfaceKHR) info.surface;
  auto old_vk_swapchain = swapchain->vk;

  defer old_vk_swapchain_{[&] {
    if (old_vk_swapchain != nullptr)
    {
      table_.DestroySwapchainKHR(vk_dev_, old_vk_swapchain, nullptr);
    }
  }};

  VkSurfaceCapabilitiesKHR surface_capabilities;
  auto result = instance_->table_.GetPhysicalDeviceSurfaceCapabilitiesKHR(
    phy_.vk, vk_surface, &surface_capabilities);

  if (result != VK_SUCCESS)
  {
    old_vk_swapchain = nullptr;
    return Err{(Status) result};
  }

  CHECK(info.preferred_buffering <= surface_capabilities.maxImageCount, "");
  CHECK(has_bits(surface_capabilities.supportedUsageFlags,
                 (VkImageUsageFlags) info.usage),
        "");
  CHECK(has_bits(surface_capabilities.supportedCompositeAlpha,
                 (VkImageUsageFlags) info.composite_alpha),
        "");

  if (surface_capabilities.currentExtent.width == 0 ||
      surface_capabilities.currentExtent.height == 0)
  {
    swapchain->is_deferred = true;
    return Ok{};
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
    .oldSwapchain          = old_vk_swapchain};

  VkSwapchainKHR vk;

  result = table_.CreateSwapchainKHR(vk_dev_, &create_info, nullptr, &vk);

  if (result != VK_SUCCESS)
  {
    old_vk_swapchain = nullptr;
    return Err{(Status) result};
  }

  defer vk_{[&] {
    if (vk != nullptr)
    {
      table_.DestroySwapchainKHR(vk_dev_, vk, nullptr);
    }
  }};

  u32 num_images;
  result = table_.GetSwapchainImagesKHR(vk_dev_, vk, &num_images, nullptr);

  CHECK(result == VK_SUCCESS, "");

  Vec<VkImage, 0> vk_images{scratch_};
  vk_images.resize_uninit(num_images).unwrap();

  result =
    table_.GetSwapchainImagesKHR(vk_dev_, vk, &num_images, vk_images.data());

  CHECK(num_images == vk_images.size(), "");
  CHECK(result == VK_SUCCESS, "");

  VkSemaphoreCreateInfo sem_info{.sType =
                                   VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0};

  SmallVec<Image, 8, 0> images{allocator_};
  images.resize(num_images).unwrap();
  SmallVec<VkSemaphore, 8, 0> acquire_semaphores{allocator_};
  acquire_semaphores.resize(num_images).unwrap();

  for (auto [i, vk, image, acquire_semaphore] :
       zip(range(vk_images.size()), vk_images, images, acquire_semaphores))
  {
    CHECK(allocator_->nalloc(1, image), "");

    new (image) IImage{
      .vk                 = vk,
      .type               = gpu::ImageType::Type2D,
      .usage              = info.usage,
      .aspects            = gpu::ImageAspects::Color,
      .sample_count       = gpu::SampleCount::C1,
      .extent             = {vk_extent.width,  vk_extent.height, 1                              },
      .mip_levels         = 1,
      .array_layers       = 1,
      .is_swapchain_image = true,
      .memory             = MemoryInfo{
                             .alias = nullptr, .element = 0,     .type = gpu::MemoryType::Unique}
    };

    auto result =
      table_.CreateSemaphore(vk_dev_, &sem_info, nullptr, &acquire_semaphore);

    CHECK(result == VK_SUCCESS, "");
  }

  auto swapchain_label =
    sformat(scratch_, "{} / Swapchain"_str, info.label).unwrap();

  set_resource_name(swapchain_label, vk, VK_OBJECT_TYPE_SWAPCHAIN_KHR,
                    VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, scratch_);
  for (auto [i, image, acquire_semaphore] :
       zip(range(images.size()), images, acquire_semaphores))
  {
    auto label =
      sformat(scratch_, "{} / SwapchainImage {}"_str, info.label, i).unwrap();
    set_resource_name(label, image->vk, VK_OBJECT_TYPE_IMAGE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, scratch_);

    auto acq_sem_label =
      sformat(scratch_, "{} / AcquireSemaphore {}"_str, info.label, i).unwrap();

    set_resource_name(acq_sem_label, acquire_semaphore,
                      VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, scratch_);
  }

  for (auto image : images)
  {
    Enum<gpu::Buffer, gpu::Image> resources[] = {(gpu::Image) image};

    create_shim_alias(gpu::AliasInfo{.label = {}, .resources = resources})
      .unwrap();
  }

  release(*swapchain);
  swapchain->~ISwapchain();

  new (swapchain) ISwapchain{
    .vk                 = vk,
    .vk_surface         = vk_surface,
    .images             = std::move(images),
    .acquire_semaphores = std::move(acquire_semaphores),
    .current_image      = none,
    .current_semaphore  = none,
    .is_deferred        = false,
    .is_out_of_date     = false,
    .is_optimal         = true,
    .format             = info.format,
    .usage              = info.usage,
    .present_mode       = info.present_mode,
    .extent             = {vk_extent.width, vk_extent.height},
    .composite_alpha    = info.composite_alpha,
    .preference         = std::move(info)
  };

  old_vk_swapchain = nullptr;
  vk               = nullptr;

  return Ok{};
}

Result<gpu::Swapchain, Status>
  IDevice::create_swapchain(gpu::SwapchainInfo const & info)
{
  Vec<char> label{allocator_};
  if (!label.extend(info.label))
  {
    return Err{Status::OutOfHostMemory};
  }

  SwapchainPreference pref{.label               = std::move(label),
                           .surface             = info.surface,
                           .format              = info.format,
                           .usage               = info.usage,
                           .preferred_buffering = info.preferred_buffering,
                           .present_mode        = info.present_mode,
                           .preferred_extent    = info.preferred_extent,
                           .composite_alpha     = info.composite_alpha};

  ISwapchain * shim;

  if (!allocator_->nalloc(1, shim))
  {
    return Err{Status::OutOfHostMemory};
  }

  defer shim_{[&] {
    if (shim != nullptr)
    {
      uninit((gpu::Swapchain) shim);
    }
  }};

  new (shim) ISwapchain{
    .vk                 = nullptr,
    .vk_surface         = nullptr,
    .images             = {},
    .acquire_semaphores = {},
    .current_image      = none,
    .current_semaphore  = none,
    .is_deferred        = false,
    .is_out_of_date     = false,
    .is_optimal         = true,
    .format             = info.format,
    .usage              = info.usage,
    .present_mode       = info.present_mode,
    .extent             = {0, 0},
    .composite_alpha    = info.composite_alpha,
    .preference         = std::move(pref)
  };

  auto result = recreate_swapchain(shim);

  if (!result)
  {
    return Err{result.err()};
  }

  gpu::Swapchain swapchain = (gpu::Swapchain) shim;
  shim                     = nullptr;

  return Ok{swapchain};
}

Result<gpu::TimestampQuery, Status>
  IDevice::create_timestamp_query(gpu::TimestampQueryInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

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
    table_.CreateQueryPool(vk_dev_, &create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_pool, VK_OBJECT_TYPE_QUERY_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, scratch_);

  return Ok{(gpu::TimestampQuery) vk_pool};
}

Result<gpu::StatisticsQuery, Status>
  IDevice::create_statistics_query(gpu::StatisticsQueryInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  CHECK(info.count > 0, "");

  if (phy_.vk_features.pipelineStatisticsQuery != VK_TRUE)
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
    table_.CreateQueryPool(vk_dev_, &create_info, nullptr, &vk_pool);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  set_resource_name(info.label, vk_pool, VK_OBJECT_TYPE_QUERY_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, scratch_);

  return Ok{(gpu::StatisticsQuery) vk_pool};
}

Result<gpu::CommandEncoder, Status>
  IDevice::create_command_encoder(gpu::CommandEncoderInfo const &)
{
  ICommandEncoder * enc;

  if (!allocator_->nalloc(1, enc))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (enc) ICommandEncoder{*this, allocator_};

  return Ok{(gpu::CommandEncoder) enc};
}

Result<gpu::CommandBuffer, Status>
  IDevice::create_command_buffer(gpu::CommandBufferInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  VkCommandPoolCreateInfo pool_create_info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = nullptr,
    .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = queue_family_};

  VkCommandPool vk_pool;
  auto          result =
    table_.CreateCommandPool(vk_dev_, &pool_create_info, nullptr, &vk_pool);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer pool_{[&] {
    if (vk_pool != nullptr)
    {
      table_.DestroyCommandPool(vk_dev_, vk_pool, nullptr);
    }
  }};

  auto pool_label =
    sformat(scratch_, "{} / CommandPool"_str, info.label).unwrap();
  set_resource_name(pool_label, vk_pool, VK_OBJECT_TYPE_COMMAND_POOL,
                    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, scratch_);

  VkCommandBufferAllocateInfo allocate_info{
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext              = nullptr,
    .commandPool        = vk_pool,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1};

  VkCommandBuffer vk;
  result = table_.AllocateCommandBuffers(vk_dev_, &allocate_info, &vk);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  defer buff_{[&] {
    if (vk != nullptr)
    {
      table_.FreeCommandBuffers(vk_dev_, vk_pool, 1, &vk);
    }
  }};

  set_resource_name(info.label, vk, VK_OBJECT_TYPE_COMMAND_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, scratch_);

  ICommandBuffer * buff;

  if (!allocator_->nalloc(1, buff))
  {
    return Err{Status::OutOfHostMemory};
  }

  new (buff) ICommandBuffer{*this, vk_pool, vk, allocator_};

  vk_pool = nullptr;
  vk      = nullptr;

  return Ok{(gpu::CommandBuffer) buff};
}

Result<gpu::QueueScope, Status>
  IDevice::create_queue_scope(gpu::QueueScopeInfo const & info)
{
  SCRATCH_ALLOCATOR(allocator_);

  SmallVec<VkSemaphore, 4, 0> submit_semaphores{allocator_};
  SmallVec<VkFence, 4, 0>     submit_fences{allocator_};

  defer _{[&] {
    for (auto sem : submit_semaphores)
    {
      table_.DestroySemaphore(vk_dev_, sem, nullptr);
    }

    for (auto fence : submit_fences)
    {
      table_.DestroyFence(vk_dev_, fence, nullptr);
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
    auto sbm_sem_label =
      sformat(scratch_, "{} / SubmitSemaphore {}"_str, info.label, i).unwrap();
    auto sbm_fnc_label =
      sformat(scratch_, "{} / SubmitFence {}"_str, info.label, i).unwrap();

    VkSemaphore acquire_sem;
    auto        result =
      table_.CreateSemaphore(vk_dev_, &sem_info, nullptr, &acquire_sem);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    VkSemaphore submit_sem;

    result = table_.CreateSemaphore(vk_dev_, &sem_info, nullptr, &submit_sem);

    set_resource_name(sbm_sem_label, submit_sem, VK_OBJECT_TYPE_SEMAPHORE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, scratch_);

    submit_semaphores.push(submit_sem).unwrap();

    VkFence submit_fence;
    result = table_.CreateFence(vk_dev_, &fence_info, nullptr, &submit_fence);

    set_resource_name(sbm_fnc_label, submit_fence, VK_OBJECT_TYPE_FENCE,
                      VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, scratch_);

    submit_fences.push(submit_fence).unwrap();
  }

  IQueueScope * scope;

  if (!allocator_->nalloc(1, scope))
  {
    return Err{Status::OutOfDeviceMemory};
  }

  new (scope) IQueueScope{info.buffering, std::move(submit_semaphores),
                          std::move(submit_fences)};

  return Ok{(gpu::QueueScope) scope};
}

AliasId IDevice::allocate_alias_id()
{
  WriteGuard guard{resource_states_.lock_};
  auto       id = resource_states_.alias_.push(Hazard{}).unwrap();
  return static_cast<AliasId>(id);
}

void IDevice::release_alias_id(AliasId id)
{
  WriteGuard guard{resource_states_.lock_};
  resource_states_.alias_.erase(id);
  return;
}

DescriptorSetId IDevice::allocate_descriptor_set_id()
{
  WriteGuard guard{resource_states_.lock_};
  auto       id = resource_states_.descriptor_sets_.push().unwrap();
  return static_cast<DescriptorSetId>(id);
}

void IDevice::release_descriptor_set_id(DescriptorSetId id)
{
  WriteGuard guard{resource_states_.lock_};
  resource_states_.descriptor_sets_.erase(id);
  return;
}

void IDevice::uninit()
{
  vmaDestroyAllocator(vma_allocator_);
  table_.DestroyDevice(vk_dev_, nullptr);
}

void IDevice::uninit(gpu::Buffer buffer_)
{
  auto * buffer = (Buffer) buffer_;

  if (buffer == nullptr)
  {
    return;
  }

  for (auto loc : buffer->bind_locations)
  {
    loc.set->bindings[loc.binding].sync_resources[v1][loc.element] =
      (Buffer) nullptr;
  }

  if (buffer->memory.alias != nullptr)
  {
    switch (buffer->memory.type)
    {
      case gpu::MemoryType::Unique:
      {
        uninit((gpu::Alias) buffer->memory.alias);
      }
      break;
      case gpu::MemoryType::Aliased:
      {
      }
      break;
    }
  }

  table_.DestroyBuffer(vk_dev_, buffer->vk, nullptr);
  buffer->~IBuffer();
  allocator_->ndealloc(1, buffer);
}

void IDevice::uninit(gpu::BufferView buffer_view_)
{
  auto * buffer_view = (BufferView) buffer_view_;

  if (buffer_view == nullptr)
  {
    return;
  }

  for (auto & loc : buffer_view->bind_locations)
  {
    loc.set->bindings[loc.binding].sync_resources[v2][loc.element] =
      (BufferView) nullptr;
  }

  table_.DestroyBufferView(vk_dev_, buffer_view->vk, nullptr);
  buffer_view->~IBufferView();
  allocator_->ndealloc(1, buffer_view);
}

void IDevice::uninit(gpu::Image image_)
{
  auto * image = (Image) image_;

  if (image == nullptr)
  {
    return;
  }

  if (image->is_swapchain_image)
  {
    image->~IImage();
    allocator_->ndealloc(1, image);
    return;
  }

  if (image->memory.alias != nullptr)
  {
    switch (image->memory.type)
    {
      case gpu::MemoryType::Unique:
      {
        uninit((gpu::Alias) image->memory.alias);
      }
      break;
      case gpu::MemoryType::Aliased:
      {
      }
      break;
    }
  }

  table_.DestroyImage(vk_dev_, image->vk, nullptr);
  image->~IImage();
  allocator_->ndealloc(1, image);
}

void IDevice::uninit(gpu::ImageView image_view_)
{
  auto * image_view = (ImageView) image_view_;

  if (image_view == nullptr)
  {
    return;
  }

  for (auto loc : image_view->bind_locations)
  {
    loc.set->bindings[loc.binding].sync_resources[v3][loc.element] =
      (ImageView) nullptr;
  }

  table_.DestroyImageView(vk_dev_, image_view->vk, nullptr);
  image_view->~IImageView();
  allocator_->ndealloc(1, image_view);
}

void IDevice::uninit(gpu::Alias alias_)
{
  auto * alias = (Alias) alias_;

  if (alias == nullptr)
  {
    return;
  }

  if (alias->vma_allocation != nullptr)
  {
    vmaFreeMemory(vma_allocator_, alias->vma_allocation);
  }

  release_alias_id(alias->id);

  alias->~IAlias();
  allocator_->ndealloc(1, alias);
}

void IDevice::uninit(gpu::Sampler sampler_)
{
  table_.DestroySampler(vk_dev_, (Sampler) sampler_, nullptr);
}

void IDevice::uninit(gpu::Shader shader_)
{
  table_.DestroyShaderModule(vk_dev_, (Shader) shader_, nullptr);
}

void IDevice::uninit(gpu::DescriptorSetLayout layout_)
{
  auto * layout = (DescriptorSetLayout) layout_;

  if (layout == nullptr)
  {
    return;
  }

  table_.DestroyDescriptorSetLayout(vk_dev_, layout->vk, nullptr);
  layout->~IDescriptorSetLayout();
  allocator_->ndealloc(1, layout);
}

void IDevice::uninit(gpu::DescriptorSet set_)
{
  auto * set = (DescriptorSet) set_;

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
            IDescriptorSet::remove_bind_loc(
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
            IDescriptorSet::remove_bind_loc(
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
            IDescriptorSet::remove_bind_loc(
              image_view->bind_locations,
              BindLocation{.set = set, .binding = ibinding, .element = i});
          }
        }
      });
  }

  release_descriptor_set_id(set->id);
  table_.FreeDescriptorSets(vk_dev_, set->vk_pool, 1, &set->vk);
  table_.DestroyDescriptorPool(vk_dev_, set->vk_pool, nullptr);
  set->~IDescriptorSet();
  allocator_->ndealloc(1, set);
}

void IDevice::uninit(gpu::PipelineCache cache_)
{
  table_.DestroyPipelineCache(vk_dev_, (PipelineCache) cache_, nullptr);
}

void IDevice::uninit(gpu::ComputePipeline pipeline_)
{
  auto * pipeline = (ComputePipeline) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  table_.DestroyPipeline(vk_dev_, pipeline->vk, nullptr);
  table_.DestroyPipelineLayout(vk_dev_, pipeline->vk_layout, nullptr);
  pipeline->~IComputePipeline();
  allocator_->ndealloc(1, pipeline);
}

void IDevice::uninit(gpu::GraphicsPipeline pipeline_)
{
  auto * pipeline = (GraphicsPipeline) pipeline_;

  if (pipeline == nullptr)
  {
    return;
  }

  table_.DestroyPipeline(vk_dev_, pipeline->vk, nullptr);
  table_.DestroyPipelineLayout(vk_dev_, pipeline->vk_layout, nullptr);
  pipeline->~IGraphicsPipeline();
  allocator_->ndealloc(1, pipeline);
}

void IDevice::release(ISwapchain & swapchain)
{
  for (auto image : swapchain.images)
  {
    uninit((gpu::Image) image);
  }

  for (auto sem : swapchain.acquire_semaphores)
  {
    table_.DestroySemaphore(vk_dev_, sem, nullptr);
  }

  table_.DestroySwapchainKHR(vk_dev_, swapchain.vk, nullptr);
}

void IDevice::uninit(gpu::Swapchain swapchain_)
{
  auto * swapchain = (Swapchain) swapchain_;

  if (swapchain == nullptr)
  {
    return;
  }

  release(*swapchain);

  swapchain->~ISwapchain();
  allocator_->ndealloc(1, swapchain);
}

void IDevice::uninit(gpu::TimestampQuery query_)
{
  auto vk_pool = (VkQueryPool) query_;

  table_.DestroyQueryPool(vk_dev_, vk_pool, nullptr);
}

void IDevice::uninit(gpu::StatisticsQuery query_)
{
  auto vk_pool = (VkQueryPool) query_;

  table_.DestroyQueryPool(vk_dev_, vk_pool, nullptr);
}

void IDevice::uninit(gpu::CommandEncoder enc_)
{
  auto enc = (CommandEncoder) enc_;

  if (enc == nullptr)
  {
    return;
  }

  enc->~ICommandEncoder();
  allocator_->ndealloc(1, enc);
}

void IDevice::uninit(gpu::CommandBuffer buff_)
{
  auto buff = (CommandBuffer) buff_;
  if (buff == nullptr)
  {
    return;
  }

  table_.FreeCommandBuffers(vk_dev_, buff->vk_pool_, 1, &buff->vk_);
  table_.DestroyCommandPool(vk_dev_, buff->vk_pool_, nullptr);

  buff->~ICommandBuffer();
  allocator_->ndealloc(1, buff);
}

void IDevice::uninit(gpu::QueueScope scope_)
{
  auto scope = (QueueScope) scope_;

  if (scope == nullptr)
  {
    return;
  }

  for (auto sem : scope->submit_semaphores_)
  {
    table_.DestroySemaphore(vk_dev_, sem, nullptr);
  }

  for (auto fence : scope->submit_fences_)
  {
    table_.DestroyFence(vk_dev_, fence, nullptr);
  }

  scope->~IQueueScope();
  allocator_->ndealloc(1, scope);
}

Result<Span<u8>, Status> IDevice::get_memory_map(gpu::Buffer buffer_)
{
  auto * buffer = (Buffer) buffer_;

  CHECK(buffer->host_mapped, "");
  CHECK(buffer->memory.alias != nullptr, "");

  auto & mem       = buffer->memory;
  auto * alias_map = (u8 *) mem.alias->map;

  return Ok{
    Span<u8>{alias_map, buffer->size}
  };
}

Result<Void, Status> IDevice::invalidate_mapped_memory(gpu::Buffer buffer_,
                                                       Slice64     range)
{
  auto * buffer = (Buffer) buffer_;

  CHECK(buffer->host_mapped, "");
  CHECK(buffer->memory.alias != nullptr, "");

  range = range(buffer->size);

  auto & mem = buffer->memory;

  auto result = vmaInvalidateAllocation(
    vma_allocator_, mem.alias->vma_allocation, range.offset, range.span);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<Void, Status> IDevice::flush_mapped_memory(gpu::Buffer buffer_,
                                                  Slice64     range)
{
  auto * buffer = (Buffer) buffer_;

  CHECK(buffer->host_mapped, "");
  CHECK(buffer->memory.alias != nullptr, "");

  range = range(buffer->size);

  auto & mem = buffer->memory;

  auto result = vmaFlushAllocation(vma_allocator_, mem.alias->vma_allocation,
                                   range.offset, range.span);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

Result<usize, Status> IDevice::get_pipeline_cache_size(gpu::PipelineCache cache)
{
  usize size;

  auto result =
    table_.GetPipelineCacheData(vk_dev_, (PipelineCache) cache, &size, nullptr);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{(usize) size};
}

Result<Void, Status> IDevice::get_pipeline_cache_data(gpu::PipelineCache cache,
                                                      Vec<u8> &          out)
{
  usize size = 0;

  auto result =
    table_.GetPipelineCacheData(vk_dev_, (PipelineCache) cache, &size, nullptr);

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

  result = table_.GetPipelineCacheData(vk_dev_, (PipelineCache) cache, &size,
                                       out.data() + offset);

  if (result != VK_SUCCESS)
  {
    out.resize_uninit(offset).unwrap();
    return Err{(Status) result};
  }

  return Ok{};
}

Result<Void, Status>
  IDevice::merge_pipeline_cache(gpu::PipelineCache             dst,
                                Span<gpu::PipelineCache const> srcs)
{
  auto num_srcs = size32(srcs);

  CHECK(num_srcs > 0, "");

  auto result = table_.MergePipelineCaches(
    vk_dev_, (PipelineCache) dst, num_srcs, (PipelineCache *) srcs.data());

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }
  return Ok{Void{}};
}

void IDevice::update_descriptor_set(gpu::DescriptorSetUpdate const & update)
{
  SCRATCH_ALLOCATOR(allocator_);

  if (update.buffers.is_empty() && update.texel_buffers.is_empty() &&
      update.images.is_empty())
  {
    return;
  }

  auto * set = (DescriptorSet) update.set;

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
        auto * buffer = (Buffer) b.buffer;
        if (buffer == nullptr)
        {
          continue;
        }
        CHECK(has_bits(buffer->usage, descriptor_buffer_usage(binding.type)),
              "");
        CHECK(is_valid_buffer_access(
                buffer->size, b.range,
                max(phy_.vk_properties.limits.minStorageBufferOffsetAlignment,
                    phy_.vk_properties.limits.minUniformBufferOffsetAlignment),
                1),
              "");
      }
    }
    break;
    case SyncResourceType::BufferView:
    {
      for (gpu::BufferView v : update.texel_buffers)
      {
        auto * view = (BufferView) v;
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
        auto * view = (ImageView) b.image_view;
        if (view == nullptr)
        {
          continue;
        }
        auto * image = (Image) view->image;
        CHECK(has_bits(image->usage, descriptor_image_usage(binding.type)), "");
        CHECK(image->sample_count == gpu::SampleCount::C1, "");
      }
    }
    break;

    default:
      CHECK_UNREACHABLE();
  }

  Vec<VkDescriptorBufferInfo> buffer_infos{scratch_};
  Vec<VkDescriptorImageInfo>  image_infos{scratch_};
  Vec<VkBufferView>           texel_infos{scratch_};

  switch (binding.type)
  {
    case gpu::DescriptorType::UniformBuffer:
    case gpu::DescriptorType::DynUniformBuffer:
    case gpu::DescriptorType::ReadStorageBuffer:
    case gpu::DescriptorType::DynReadStorageBuffer:
    case gpu::DescriptorType::RWStorageBuffer:
    case gpu::DescriptorType::DynRWStorageBuffer:
    {
      buffer_infos.resize_uninit(update.buffers.size()).unwrap();
      for (auto [vk, b] : zip(buffer_infos, update.buffers))
      {
        auto * buffer = (Buffer) b.buffer;
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
      image_infos.resize_uninit(update.images.size()).unwrap();
      for (auto [vk, b] : zip(image_infos, update.images))
      {
        auto * view    = (ImageView) b.image_view;
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
      texel_infos.resize_uninit(update.texel_buffers.size()).unwrap();
      for (auto [vk, b] : zip(texel_infos, update.texel_buffers))
      {
        auto * view = (BufferView) b;
        vk          = (view == nullptr) ? nullptr : view->vk;
      }
    }
    break;

    default:
      CHECK_UNREACHABLE();
  }

  VkWriteDescriptorSet vk_write{
    .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .pNext           = nullptr,
    .dstSet          = set->vk,
    .dstBinding      = update.binding,
    .dstArrayElement = update.first_element,
    .descriptorCount =
      max(size32(image_infos), size32(buffer_infos), size32(texel_infos)),
    .descriptorType   = to_vk(binding.type),
    .pImageInfo       = image_infos.data(),
    .pBufferInfo      = buffer_infos.data(),
    .pTexelBufferView = texel_infos.data()};

  table_.UpdateDescriptorSets(vk_dev_, 1, &vk_write, 0, nullptr);

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

gpu::QueueScopeState IDevice::get_queue_scope_state(gpu::QueueScope scope_)
{
  CHECK(scope_ != nullptr, "");
  auto scope = (QueueScope) scope_;

  return gpu::QueueScopeState{.ring_index = scope->ring_index_,
                              .buffering  = scope->buffering_};
}

Result<Void, Status> IDevice::await_idle()
{
  auto result = table_.DeviceWaitIdle(vk_dev_);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<Void, Status> IDevice::await_queue_idle()
{
  auto result = table_.QueueWaitIdle(vk_queue_);
  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{Void{}};
}

Result<Void, Status>
  IDevice::get_surface_formats(gpu::Surface              surface_,
                               Vec<gpu::SurfaceFormat> & formats)
{
  SCRATCH_ALLOCATOR(allocator_);

  auto surface = (VkSurfaceKHR) surface_;

  u32  num_supported;
  auto result = instance_->table_.GetPhysicalDeviceSurfaceFormatsKHR(
    phy_.vk, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  Vec<VkSurfaceFormatKHR> vk_formats{scratch_};

  if (!vk_formats.resize_uninit(num_supported))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = instance_->table_.GetPhysicalDeviceSurfaceFormatsKHR(
    phy_.vk, surface, &num_supported, vk_formats.data());

  if (result != VK_SUCCESS && result != VK_INCOMPLETE)
  {
    return Err{(Status) result};
  }

  CHECK(vk_formats.size() == num_supported && result != VK_INCOMPLETE, "");

  usize const offset = formats.size();

  if (!formats.extend_uninit(num_supported))
  {
    return Err{Status::OutOfHostMemory};
  }

  for (auto [fmt, vk] : zip(formats.view().slice(offset), vk_formats))
  {
    fmt = gpu::SurfaceFormat{.format      = (gpu::Format) vk.format,
                             .color_space = (gpu::ColorSpace) vk.colorSpace};
  }

  return Ok{};
}

Result<Void, Status>
  IDevice::get_surface_present_modes(gpu::Surface            surface_,
                                     Vec<gpu::PresentMode> & modes)
{
  SCRATCH_ALLOCATOR(allocator_);

  auto surface = (VkSurfaceKHR) surface_;

  u32  num_supported;
  auto result = instance_->table_.GetPhysicalDeviceSurfacePresentModesKHR(
    phy_.vk, surface, &num_supported, nullptr);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  Vec<VkPresentModeKHR> vk_present_modes{scratch_};

  if (!vk_present_modes.resize_uninit(num_supported))
  {
    return Err{Status::OutOfHostMemory};
  }

  result = instance_->table_.GetPhysicalDeviceSurfacePresentModesKHR(
    phy_.vk, surface, &num_supported, vk_present_modes.data());

  if (result != VK_SUCCESS && result != VK_INCOMPLETE)
  {
    return Err{(Status) result};
  }

  CHECK(vk_present_modes.size() == num_supported && result != VK_INCOMPLETE,
        "");

  auto offset = modes.size();

  if (!modes.extend_uninit(num_supported))
  {
    return Err{Status::OutOfHostMemory};
  }

  for (auto [mode, vk] : zip(modes.view().slice(offset), vk_present_modes))
  {
    mode = (gpu::PresentMode) vk;
  }

  return Ok{};
}

Result<gpu::SurfaceCapabilities, Status>
  IDevice::get_surface_capabilities(gpu::Surface surface_)
{
  auto                     surface = (VkSurfaceKHR) surface_;
  VkSurfaceCapabilitiesKHR capabilities;
  auto result = instance_->table_.GetPhysicalDeviceSurfaceCapabilitiesKHR(
    phy_.vk, surface, &capabilities);

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
  IDevice::get_swapchain_state(gpu::Swapchain swapchain_)
{
  auto * swapchain = (Swapchain) swapchain_;

  gpu::SwapchainState state{
    .extent          = swapchain->extent,
    .format          = swapchain->format,
    .present_mode    = swapchain->present_mode,
    .composite_alpha = swapchain->composite_alpha,
    .images          = swapchain->images.view().reinterpret<gpu::Image>(),
    .current_image   = swapchain->current_image};

  return Ok{state};
}

Result<Void, Status>
  IDevice::get_timestamp_query_result(gpu::TimestampQuery query_, u32 first,
                                      Span<u64> timestamps)
{
  if (timestamps.is_empty())
  {
    return Ok{};
  }

  auto vk_pool = (VkQueryPool) query_;

  auto n = size32(timestamps);

  auto result = table_.GetQueryPoolResults(vk_dev_, vk_pool, first, n,
                                           sizeof(u64) * n, timestamps.data(),
                                           sizeof(u64), VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{};
}

Result<Void, Status>
  IDevice::get_statistics_query_result(gpu::StatisticsQuery query_, u32 first,
                                       Span<gpu::PipelineStatistics> statistics)
{
  if (statistics.is_empty())
  {
    return Ok{};
  }

  auto vk_pool = (VkQueryPool) query_;

  auto n = size32(statistics);

  auto result = table_.GetQueryPoolResults(
    vk_dev_, vk_pool, first, n, sizeof(gpu::PipelineStatistics) * n,
    statistics.data(), sizeof(gpu::PipelineStatistics), VK_QUERY_RESULT_64_BIT);

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{};
}

Result<Void, Status> IDevice::acquire_next(gpu::Swapchain swapchain_)
{
  CHECK(swapchain_ != nullptr, "");
  auto swapchain = (Swapchain) swapchain_;

  VkResult result = VK_SUCCESS;

  if (swapchain->is_out_of_date || !swapchain->is_optimal ||
      swapchain->is_deferred)
  {
    // await all pending submitted operations on the device possibly using
    // the swapchain, to avoid destroying whilst in use
    result = table_.QueueWaitIdle(vk_queue_);
    if (result != VK_SUCCESS)
    {
      return Err{(Status) result};
    }

    recreate_swapchain(swapchain).unwrap();
  }

  if (!swapchain->is_deferred)
  {
    u32 next_image;
    result = table_.AcquireNextImageKHR(
      vk_dev_, swapchain->vk, U64_MAX,
      swapchain->acquire_semaphores[swapchain->ring_index], nullptr,
      &next_image);

    if (result == VK_SUBOPTIMAL_KHR)
    {
      swapchain->is_optimal = false;
    }
    else
    {
      return Err{(Status) result};
    }

    swapchain->current_image     = next_image;
    swapchain->current_semaphore = swapchain->ring_index;
    swapchain->ring_index =
      (swapchain->ring_index + 1) % size32(swapchain->images);
  }

  return Ok{};
}

Result<u64, Status> IDevice::submit(gpu::CommandBuffer buffer_,
                                    gpu::QueueScope    scope_)
{
  u8                reserved_[512];
  FallbackAllocator scratch{reserved_, allocator_};

  auto * buffer = (CommandBuffer) buffer_;
  CHECK(buffer != nullptr, "");
  CHECK(buffer->state_ == CommandBufferState::Recorded, "");
  CHECK(buffer->status_ == Status::Success, "");
  CHECK(scope_ != nullptr, "");

  auto scope = (QueueScope) scope_;

  auto submit_fence     = scope->submit_fences_[scope->ring_index_];
  auto submit_semaphore = scope->submit_semaphores_[scope->ring_index_];

  // wait to re-use sync primitives
  auto result =
    table_.WaitForFences(vk_dev_, 1, &submit_fence, VK_TRUE, U64_MAX);

  CHECK(result == VK_SUCCESS, "");

  result = table_.ResetFences(vk_dev_, 1, &submit_fence);

  CHECK(result == VK_SUCCESS, "");

  auto swapchain     = buffer->swapchain_;
  auto is_presenting = swapchain.is_some() && !swapchain->is_out_of_date &&
                       !swapchain->is_deferred;
  auto acquire_semaphore =
    is_presenting ?
      swapchain->acquire_semaphores[swapchain->current_semaphore.unwrap()] :
      nullptr;

  VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  auto submit_info = VkSubmitInfo{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = nullptr,
    .waitSemaphoreCount   = is_presenting ? 1U : 0U,
    .pWaitSemaphores      = is_presenting ? &acquire_semaphore : nullptr,
    .pWaitDstStageMask    = is_presenting ? &wait_stages : nullptr,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &buffer->vk_,
    .signalSemaphoreCount = is_presenting ? 1U : 0U,
    .pSignalSemaphores    = is_presenting ? &submit_semaphore : nullptr};

  result = table_.QueueSubmit(vk_queue_, 1, &submit_info, submit_fence);

  CHECK(result == VK_SUCCESS, "");

  buffer->state_ = CommandBufferState::Submitted;

  // commit the updated state of the resources
  buffer->commit_resource_states();

  auto id            = scope->frame_++;
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
    result = table_.QueuePresentKHR(vk_queue_, &present_info);

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

  return Ok{id};
}

Result<Void, Status> IDevice::await_queue_scope_idle(gpu::QueueScope scope_,
                                                     nanoseconds     timeout)
{
  CHECK(scope_ != nullptr, "");

  auto scope = (QueueScope) scope_;

  auto submit_fence = scope->submit_fences_[scope->ring_index_];

  auto result = table_.WaitForFences(vk_dev_, 1, &submit_fence, VK_TRUE,
                                     (u64) max(timeout.count(), 0LL));

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{};
}

Result<Void, Status> IDevice::await_queue_scope_frame(gpu::QueueScope scope_,
                                                      u64             frame,
                                                      nanoseconds     timeout)
{
  CHECK(scope_ != nullptr, "");
  auto scope = (QueueScope) scope_;

  CHECK(frame <= scope->frame_, "");

  if (sat_sub(scope->frame_, scope->buffering_) >= frame)
  {
    return Ok{};
  }

  auto dist         = scope->frame_ - frame;
  auto ring_index   = ring_sub(scope->ring_index_, dist, scope->buffering_);
  auto submit_fence = scope->submit_fences_[ring_index];

  auto result = table_.WaitForFences(vk_dev_, 1, &submit_fence, VK_TRUE,
                                     (u64) max(timeout.count(), 0LL));

  if (result != VK_SUCCESS)
  {
    return Err{(Status) result};
  }

  return Ok{};
}

void PassContext::clear()
{
  graphics_pipeline = none;
  compute_pipeline  = none;
  color_attachments.clear();
  depth_attachment   = none;
  stencil_attachment = none;
  descriptor_sets.clear();
  vertex_buffers.clear();
  index_buffer        = none;
  index_type          = gpu::IndexType::U16;
  index_buffer_offset = 0;
  has_graphics_state  = false;
}

void ICommandEncoder::begin()
{
  CHECK(state_ == CommandBufferState::Reset, "");
  state_ = CommandBufferState::Recording;
  tracker_.begin_pass();
}

Result<Void, Status> ICommandEncoder::end()
{
  CHECK(state_ == CommandBufferState::Recording, "");
  CHECK(pass_ == Pass::None, "");
  state_ = CommandBufferState::Recorded;

  if (status_ != Status::Success)
  {
    return Err{status_};
  }

  return Ok{};
}

void ICommandEncoder::reset()
{
  CHECK(state_ == CommandBufferState::Reset ||
          state_ == CommandBufferState::Recorded ||
          state_ == CommandBufferState::Submitted,
        "");
  arena_.shrink();
  arena_.reclaim();
  status_ = Status::Success;
  state_  = CommandBufferState::Reset;
  pass_   = Pass::None;
  tracker_.reset();
  ctx_.clear();
  swapchain_ = none;
}

#define PRELUDE()                                           \
  CHECK(this->state_ == CommandBufferState::Recording, ""); \
  if (this->status_ != Status::Success)                     \
  {                                                         \
    return;                                                 \
  }

#define CMD(...)                             \
  auto * cmd = push(vk::cmd::__VA_ARGS__);   \
  if (cmd == nullptr)                        \
  {                                          \
    this->status_ = Status::OutOfHostMemory; \
    return;                                  \
  }

#define MEMTRY(...)                          \
  if (!(__VA_ARGS__))                        \
  {                                          \
    this->status_ = Status::OutOfHostMemory; \
    return;                                  \
  }

void ICommandEncoder::reset_timestamp_query(gpu::TimestampQuery query,
                                            Slice32             range)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(ResetTimestampQuery{.query = (VkQueryPool) query, .range = range});
}

void ICommandEncoder::reset_statistics_query(gpu::StatisticsQuery query,
                                             Slice32              range)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(ResetStatisticsQuery{.query = (VkQueryPool) query, .range = range});
}

void ICommandEncoder::write_timestamp(gpu::TimestampQuery query,
                                      gpu::PipelineStages stage, u32 index)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(WriteTimestamp{.query  = (VkQueryPool) query,
                     .stages = (VkPipelineStageFlagBits) stage,
                     .index  = index});
}

void ICommandEncoder::begin_statistics(gpu::StatisticsQuery query, u32 index)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(BeginStatistics{.query = (VkQueryPool) query, .index = index});
}

void ICommandEncoder::end_statistics(gpu::StatisticsQuery query, u32 index)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  CMD(EndStatistics{.query = (VkQueryPool) query, .index = index});
}

void ICommandEncoder::begin_debug_marker(Str region_name_, f32x4 color)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  VkDebugMarkerMarkerInfoEXT info{
    .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
    .pNext       = nullptr,
    .pMarkerName = nullptr,
    .color       = {color.x(), color.y(), color.z(), color.w()}
  };

  CMD(BeginDebugMarker{.info = info});

  Vec<char, 0> region_name{arena_};
  MEMTRY(region_name.extend_uninit(region_name_.size() + 1));

  mem::copy(region_name_, region_name.data());

  region_name.last()    = '\0';
  cmd->info.pMarkerName = region_name.leak().data();
}

void ICommandEncoder::end_debug_marker()
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  // [ ] how are commands matched to the passes?
  // [ ] begin pass is being called after cmd insertion
  CMD(EndDebugMarker{});
}

void ICommandEncoder::fill_buffer(gpu::Buffer dst_, Slice64 range, u32 data)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  auto * dst = ptr(dst_);

  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->size, range, 4, 4), "");

  CMD(FillBuffer{.dst = dst->vk, .range = range, .data = data});

  tracker_.begin_pass();
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT);
  tracker_.end_pass();
}

void ICommandEncoder::copy_buffer(gpu::Buffer src_, gpu::Buffer dst_,
                                  Span<gpu::BufferCopy const> copies_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!copies_.is_empty(), "");

  auto * src = (Buffer) src_;
  auto * dst = (Buffer) dst_;

  CHECK(has_bits(src->usage, gpu::BufferUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");

  for (auto & copy : copies_)
  {
    CHECK(is_valid_buffer_access(src->size, copy.src_range, 1, 1), "");
    CHECK(is_valid_buffer_access(
            dst->size, Slice64{copy.dst_offset, copy.src_range.span}, 1, 1),
          "");
  }

  CMD(CopyBuffer{.src = src->vk, .dst = dst->vk, .copies{}});

  Vec<VkBufferCopy, 0> copies{arena_};

  MEMTRY(copies.resize_uninit(copies_.size()));

  for (auto [vk, copy] : zip(copies, copies_))
  {
    vk = VkBufferCopy{.srcOffset = copy.src_range.offset,
                      .dstOffset = copy.dst_offset,
                      .size      = copy.src_range.span};
  }

  cmd->copies = copies.leak();

  tracker_.begin_pass();
  tracker_.track(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT);
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT);
  tracker_.end_pass();
}

void ICommandEncoder::update_buffer(Span<u8 const> src_, u64 dst_offset,
                                    gpu::Buffer dst_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(src_.size_bytes() <= 64_KB, "");

  auto * dst       = (Buffer) dst_;
  auto   copy_size = size64(src_);

  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->size, Slice64{dst_offset, copy_size}, 4, 4),
        "");

  CMD(UpdateBuffer{.src = {}, .dst_offset = dst_offset, .dst = dst->vk});

  Vec<u8, 0> src{arena_};

  MEMTRY(src.extend(src_));

  cmd->src = src.leak();

  tracker_.begin_pass();
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT);
  tracker_.end_pass();
}

void ICommandEncoder::clear_color_image(
  gpu::Image dst_, gpu::Color value,
  Span<gpu::ImageSubresourceRange const> ranges_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!ranges_.is_empty(), "");

  auto * dst = (Image) dst_;

  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");

  for (auto & range : ranges_)
  {
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, range.aspects,
                                range.mip_levels, range.array_layers),
          "");
  }

  CMD(ClearColorImage{
    .dst        = dst->vk,
    .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .value  = {.uint32{value.u32[0], value.u32[1], value.u32[2], value.u32[3]}},
    .ranges = {}});

  Vec<VkImageSubresourceRange, 0> ranges{arena_};

  MEMTRY(ranges.extend_uninit(ranges_.size()));

  for (auto [vk, range] : zip(ranges, ranges_))
  {
    vk =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.mip_levels.offset,
                              .levelCount     = range.mip_levels.span,
                              .baseArrayLayer = range.array_layers.offset,
                              .layerCount     = range.array_layers.span};
  }

  cmd->ranges = ranges.leak();

  tracker_.begin_pass();
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  tracker_.end_pass();
}

void ICommandEncoder::clear_depth_stencil_image(
  gpu::Image dst_, gpu::DepthStencil value,
  Span<gpu::ImageSubresourceRange const> ranges_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!ranges_.is_empty(), "");

  auto * dst = (Image) dst_;

  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");

  for (auto & range : ranges_)
  {
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, range.aspects,
                                range.mip_levels, range.array_layers),
          "");
  }

  VkClearDepthStencilValue vk_depth_stencil{.depth   = value.depth,
                                            .stencil = value.stencil};

  CMD(ClearDepthStencilImage{.dst        = dst->vk,
                             .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             .value      = vk_depth_stencil,
                             .ranges     = {}});

  Vec<VkImageSubresourceRange, 0> ranges{arena_};

  MEMTRY(ranges.extend_uninit(ranges_.size()));

  for (auto [vk, range] : zip(ranges, ranges_))
  {
    vk =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.mip_levels.offset,
                              .levelCount     = range.mip_levels.span,
                              .baseArrayLayer = range.array_layers.offset,
                              .layerCount     = range.array_layers.span};
  }

  cmd->ranges = ranges.leak();

  tracker_.begin_pass();
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  tracker_.end_pass();
}

void ICommandEncoder::copy_image(gpu::Image src_, gpu::Image dst_,
                                 Span<gpu::ImageCopy const> copies_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!copies_.is_empty(), "");

  auto * src = (Image) src_;
  auto * dst = (Image) dst_;

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

  CMD(CopyImage{.src        = src->vk,
                .src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .dst        = dst->vk,
                .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .copies     = {}});

  Vec<VkImageCopy, 0> copies{arena_};

  MEMTRY(copies.extend_uninit(copies_.size()));

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

  cmd->copies = copies.leak();

  tracker_.begin_pass();
  tracker_.track(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  tracker_.end_pass();
}

void ICommandEncoder::copy_buffer_to_image(
  gpu::Buffer src_, gpu::Image dst_, Span<gpu::BufferImageCopy const> copies_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!copies_.is_empty() > 0, "");

  auto * src = (Buffer) src_;
  auto * dst = (Image) dst_;

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

  CMD(CopyBufferToImage{.src        = src->vk,
                        .dst        = dst->vk,
                        .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        .copies     = {}});

  Vec<VkBufferImageCopy, 0> copies{arena_};

  MEMTRY(copies.extend_uninit(copies_.size()));

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

  cmd->copies = copies.leak();

  tracker_.begin_pass();
  tracker_.track(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT);
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  tracker_.end_pass();
}

void ICommandEncoder::blit_image(gpu::Image src_, gpu::Image dst_,
                                 Span<gpu::ImageBlit const> blits_,
                                 gpu::Filter                filter)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!blits_.is_empty(), "");

  auto * src = (Image) src_;
  auto * dst = (Image) dst_;

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

  CMD(BlitImage{.src        = src->vk,
                .src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .dst        = dst->vk,
                .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .blits      = {},
                .filter     = (VkFilter) filter});

  Vec<VkImageBlit, 0> blits{arena_};

  MEMTRY(blits.resize_uninit(blits_.size()));

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

  cmd->blits = blits.leak();

  tracker_.begin_pass();
  tracker_.track(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  tracker_.end_pass();
}

void ICommandEncoder::resolve_image(gpu::Image src_, gpu::Image dst_,
                                    Span<gpu::ImageResolve const> resolves_)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(!resolves_.is_empty(), "");

  auto * src = (Image) src_;
  auto * dst = (Image) dst_;

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

  CMD(ResolveImage{.src        = src->vk,
                   .src_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   .dst        = dst->vk,
                   .dst_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   .resolves   = {}});

  Vec<VkImageResolve, 0> resolves{arena_};

  MEMTRY(resolves.resize_uninit(resolves_.size()));

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

  cmd->resolves = resolves.leak();

  tracker_.begin_pass();
  tracker_.track(src, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  tracker_.track(dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  tracker_.end_pass();
}

void ICommandEncoder::begin_compute_pass()
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");

  pass_ = Pass::Compute;

  tracker_.begin_pass();
}

void ICommandEncoder::end_compute_pass()
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");

  pass_ = Pass::None;
  tracker_.end_pass();
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
    auto image = ((ImageView) info.view)->image;
    CHECK(has_bits(image->aspects, aspects), "");
    CHECK(has_bits(image->usage, usage), "");
    CHECK(!(info.resolve_mode != gpu::ResolveModes::None &&
            image->sample_count == gpu::SampleCount::C1),
          "");
  }
  if (info.resolve != nullptr)
  {
    auto image = ((ImageView) info.resolve)->image;
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

void ICommandEncoder::begin_rendering(gpu::RenderingInfo const & info)
{
  PRELUDE();
  CHECK(pass_ == Pass::None, "");
  CHECK(info.color_attachments.size() <=
          dev_->phy_.vk_properties.limits.maxColorAttachments,
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

  CMD(BeginRendering{
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
                               .colorAttachmentCount = 0,
                               .pColorAttachments    = nullptr,
                               .pDepthAttachment     = nullptr,
                               .pStencilAttachment   = nullptr}
  });

  Vec<VkRenderingAttachmentInfo, 0> color_attachments{arena_};
  Vec<VkRenderingAttachmentInfo, 0> depth_attachment{arena_};
  Vec<VkRenderingAttachmentInfo, 0> stencil_attachment{arena_};

  MEMTRY(color_attachments.resize_uninit(info.color_attachments.size()));

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

  MEMTRY(
    depth_attachment.resize_uninit(info.depth_attachment.is_some() ? 1 : 0));

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

  MEMTRY(stencil_attachment.resize_uninit(
    info.stencil_attachment.is_some() ? 1 : 0));

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

  cmd->info.colorAttachmentCount = color_attachments.size();
  cmd->info.pColorAttachments    = color_attachments.leak().data();
  cmd->info.pDepthAttachment     = depth_attachment.leak().data();
  cmd->info.pStencilAttachment   = stencil_attachment.leak().data();

  pass_ = Pass::Render;
  ctx_.clear();

  for (auto & attachment : info.color_attachments)
  {
    ctx_.color_attachments.push(attachment).unwrap();
  }

  ctx_.depth_attachment   = info.depth_attachment;
  ctx_.stencil_attachment = info.stencil_attachment;

  tracker_.begin_pass();

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
      tracker_.track(ptr(attachment.view), stages, access, COLOR_LAYOUT);
    }

    if (attachment.resolve != nullptr)
    {
      tracker_.track(ptr(attachment.resolve), RESOLVE_STAGE,
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
      tracker_.track(ptr(attachment.view), stages, access, layout);
    }

    if (attachment.resolve != nullptr)
    {
      tracker_.track(ptr(attachment.resolve), RESOLVE_STAGE,
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
      tracker_.track(ptr(attachment.view), stages, access, layout);
    }

    if (attachment.resolve != nullptr)
    {
      tracker_.track(ptr(attachment.resolve), RESOLVE_STAGE,
                     RESOLVE_DEPTH_STENCIL_DST_ACCESS,
                     RESOLVE_DEPTH_STENCIL_LAYOUT);
    }
  }
}

void ICommandEncoder::end_rendering()
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");

  CMD(EndRendering{});

  ctx_.clear();
  pass_ = Pass::None;

  tracker_.end_pass();
}

void ICommandEncoder::bind_compute_pipeline(gpu::ComputePipeline pipeline_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");
  auto pipeline = ptr(pipeline_);

  CMD(BindPipeline{.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                   .pipeline   = pipeline->vk});

  ctx_.compute_pipeline = *pipeline;
}

void validate_pipeline_compatible(
  GraphicsPipeline                     pipeline,
  Span<gpu::RenderingAttachment const> color_attachments,
  Option<gpu::RenderingAttachment>     depth_attachment,
  Option<gpu::RenderingAttachment>     stencil_attachment)
{
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

void ICommandEncoder::bind_graphics_pipeline(gpu::GraphicsPipeline pipeline_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  auto pipeline = ptr(pipeline_);
  validate_pipeline_compatible(pipeline, ctx_.color_attachments,
                               ctx_.depth_attachment, ctx_.stencil_attachment);

  CMD(BindPipeline{.bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                   .pipeline   = pipeline->vk});

  ctx_.graphics_pipeline = *pipeline;
}

void ICommandEncoder::bind_descriptor_sets(
  Span<gpu::DescriptorSet const> descriptor_sets_,
  Span<u32 const>                dynamic_offsets_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render || pass_ == Pass::Compute, "");
  CHECK(descriptor_sets_.size() <=
          dev_->phy_.vk_properties.limits.maxBoundDescriptorSets,
        "");
  CHECK(dynamic_offsets_.size() <=
          sat_add(dev_->phy_.vk_descriptor_properties
                    .maxDescriptorSetUpdateAfterBindUniformBuffersDynamic,
                  dev_->phy_.vk_descriptor_properties
                    .maxDescriptorSetUpdateAfterBindStorageBuffersDynamic),
        "");

  for (auto offset : dynamic_offsets_)
  {
    CHECK(
      is_aligned<u64>(
        max(dev_->phy_.vk_properties.limits.minUniformBufferOffsetAlignment,
            dev_->phy_.vk_properties.limits.minStorageBufferOffsetAlignment),
        offset),
      "");
  }

  switch (pass_)
  {
    case Pass::Render:
    {
      CHECK(ctx_.graphics_pipeline.is_some(), "");
      CHECK(ctx_.graphics_pipeline->num_sets == descriptor_sets_.size(), "");
    }
    break;
    case Pass::Compute:
    {
      CHECK(ctx_.compute_pipeline.is_some(), "");
      CHECK(ctx_.compute_pipeline->num_sets == descriptor_sets_.size(), "");
    }
    break;
    default:
      break;
  }

  VkPipelineBindPoint  bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
  VkPipelineLayout     layout     = nullptr;
  VkPipelineStageFlags stages     = VK_SHADER_STAGE_ALL;

  switch (pass_)
  {
    case Pass::Render:
    {
      bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
      layout     = ctx_.graphics_pipeline->vk_layout;
      stages     = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    break;
    case Pass::Compute:
    {
      bind_point = VK_PIPELINE_BIND_POINT_COMPUTE;
      layout     = ctx_.compute_pipeline->vk_layout;
      stages     = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    break;
    default:
      break;
  }

  CMD(BindDescriptorSets{.bind_point      = bind_point,
                         .layout          = layout,
                         .sets            = {},
                         .dynamic_offsets = {}});

  Vec<VkDescriptorSet, 0> descriptor_sets{arena_};

  MEMTRY(descriptor_sets.resize_uninit(descriptor_sets_.size()));

  for (auto [vk, set_] : zip(descriptor_sets, descriptor_sets_))
  {
    auto * set = (DescriptorSet) set_;
    vk         = set->vk;
  }

  Vec<u32, 0> dynamic_offsets{arena_};

  MEMTRY(dynamic_offsets.extend(dynamic_offsets_));

  cmd->sets            = descriptor_sets.leak();
  cmd->dynamic_offsets = dynamic_offsets.leak();

  ctx_.descriptor_sets.clear();

  for (auto set_ : descriptor_sets_)
  {
    ctx_.descriptor_sets.push(ptr(set_)).unwrap();
  }

  for (auto set : descriptor_sets_)
  {
    tracker_.track(ptr(set), stages);
  }
}

void ICommandEncoder::push_constants(Span<u8 const> constants_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render || pass_ == Pass::Compute, "");
  CHECK(constants_.size_bytes() <=
          dev_->phy_.vk_properties.limits.maxPushConstantsSize,
        "");
  CHECK(is_aligned<u32>(4U, constants_.size()), "");

  switch (pass_)
  {
    case Pass::Render:
    {
      CHECK(ctx_.graphics_pipeline.is_some(), "");
      CHECK(constants_.size() == ctx_.graphics_pipeline->push_constants_size,
            "");
    }
    break;
    case Pass::Compute:
    {
      CHECK(ctx_.compute_pipeline.is_some(), "");
      CHECK(constants_.size() == ctx_.compute_pipeline->push_constants_size,
            "");
    }
    break;
    default:
      break;
  }

  VkPipelineLayout layout = nullptr;

  switch (pass_)
  {
    case Pass::Render:
    {
      layout = ctx_.graphics_pipeline->vk_layout;
    }
    break;
    case Pass::Compute:
    {
      layout = ctx_.compute_pipeline->vk_layout;
    }
    break;
    default:
      break;
  }

  CMD(PushConstants{.layout = layout, .constants = {}});

  Vec<u8> constants{arena_};

  MEMTRY(constants.extend(constants_));

  cmd->constants = constants.leak();
}

void ICommandEncoder::dispatch(u32x3 group_count)
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");

  CHECK(ctx_.compute_pipeline.is_some(), "");
  CHECK(group_count.x() <=
          dev_->phy_.vk_properties.limits.maxComputeWorkGroupCount[0],
        "");
  CHECK(group_count.y() <=
          dev_->phy_.vk_properties.limits.maxComputeWorkGroupCount[1],
        "");
  CHECK(group_count.z() <=
          dev_->phy_.vk_properties.limits.maxComputeWorkGroupCount[2],
        "");

  CMD(Dispatch{.group_count = group_count});
}

void ICommandEncoder::dispatch_indirect(gpu::Buffer buffer_, u64 offset)
{
  PRELUDE();
  CHECK(pass_ == Pass::Compute, "");
  CHECK(buffer_ != nullptr, "");
  CHECK(ctx_.compute_pipeline.is_some(), "");

  auto * buffer = (Buffer) buffer_;

  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(is_valid_buffer_access(buffer->size,
                               Slice64{offset, sizeof(gpu::DispatchCommand)}, 4,
                               alignof(gpu::DispatchCommand)),
        "");

  CMD(DispatchIndirect{.buffer = buffer->vk, .offset = offset});

  tracker_.track(buffer, VK_SHADER_STAGE_COMPUTE_BIT,
                 VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
}

void ICommandEncoder::set_graphics_state(gpu::GraphicsState const & state)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline.is_some(), "");
  CHECK(state.viewport.min_depth >= 0.0F, "");
  CHECK(state.viewport.min_depth <= 1.0F, "");
  CHECK(state.viewport.max_depth >= 0.0F, "");
  CHECK(state.viewport.max_depth <= 1.0F, "");

  CMD(SetGraphicsState{.state = state});

  ctx_.has_graphics_state = true;
}

void ICommandEncoder::bind_vertex_buffers(
  Span<gpu::Buffer const> vertex_buffers_, Span<u64 const> offsets_)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(!vertex_buffers_.is_empty(), "");
  CHECK(vertex_buffers_.size() <=
          dev_->phy_.vk_properties.limits.maxVertexInputBindings,
        "");
  CHECK(ctx_.graphics_pipeline.is_some(), "");
  CHECK(vertex_buffers_.size() == ctx_.graphics_pipeline->num_vertex_attributes,
        "");
  CHECK(offsets_.size() == vertex_buffers_.size(), "");

  for (auto [buff_, off] : zip(vertex_buffers_, offsets_))
  {
    auto * buff = (Buffer) buff_;
    CHECK(buff_ != nullptr, "");
    CHECK(is_valid_buffer_access(buff->size, Slice64{off, 1}, 1, 1), "");
    CHECK(has_bits(buff->usage, gpu::BufferUsage::VertexBuffer), "");
  }

  CMD(BindVertexBuffers{.buffers = {}, .offsets = {}});

  Vec<VkBuffer, 0> vertex_buffers{arena_};

  MEMTRY(vertex_buffers.resize_uninit(vertex_buffers_.size()));

  for (auto [vk, buff_] : zip(vertex_buffers, vertex_buffers_))
  {
    auto * buff = (Buffer) buff_;
    vk          = buff->vk;
  }

  Vec<u64, 0> offsets{arena_};

  MEMTRY(offsets.extend(offsets_));

  cmd->buffers = vertex_buffers.leak();
  cmd->offsets = offsets.leak();

  for (auto buff_ : vertex_buffers_)
  {
    tracker_.track(ptr(buff_), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                   VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
  }

  ctx_.vertex_buffers.clear();

  for (auto buff_ : vertex_buffers_)
  {
    ctx_.vertex_buffers.push(ptr(buff_)).unwrap();
  }
}

void ICommandEncoder::bind_index_buffer(gpu::Buffer index_buffer_, u64 offset,
                                        gpu::IndexType index_type)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(index_buffer_ != nullptr, "");
  CHECK(ctx_.graphics_pipeline.is_some(), "");

  auto * index_buffer = (Buffer) index_buffer_;
  auto   index_size   = index_type_size(index_type);

  CHECK(has_bits(index_buffer->usage, gpu::BufferUsage::IndexBuffer), "");
  CHECK(is_valid_buffer_access(index_buffer->size, Slice64{offset, index_size},
                               index_size, index_size),
        "");

  CMD(BindIndexBuffer{.buffer     = index_buffer->vk,
                      .offset     = offset,
                      .index_type = (VkIndexType) index_type});

  tracker_.track(index_buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                 VK_ACCESS_INDEX_READ_BIT);

  ctx_.index_buffer        = *index_buffer;
  ctx_.index_type          = index_type;
  ctx_.index_buffer_offset = offset;
}

void ICommandEncoder::draw(Slice32 vertices, Slice32 instances)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline.is_some(), "");
  CHECK(ctx_.has_graphics_state, "");
  CHECK(ctx_.graphics_pipeline->num_vertex_attributes ==
          ctx_.vertex_buffers.size(),
        "");
  CHECK(ctx_.graphics_pipeline->num_sets == ctx_.descriptor_sets.size(), "");

  CMD(Draw{.vertices = vertices, .instances = instances});
}

void ICommandEncoder::draw_indexed(Slice32 indices, Slice32 instances,
                                   i32 vertex_offset)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline.is_some(), "");
  CHECK(ctx_.has_graphics_state, "");
  CHECK(ctx_.graphics_pipeline->num_vertex_attributes ==
          ctx_.vertex_buffers.size(),
        "");
  CHECK(ctx_.graphics_pipeline->num_sets == ctx_.descriptor_sets.size(), "");
  CHECK(ctx_.index_buffer.is_some(), "");

  auto index_size = index_type_size(ctx_.index_type);
  CHECK(is_valid_buffer_access(
          ctx_.index_buffer->size,
          Slice64{ctx_.index_buffer_offset + indices.offset * index_size,
                  indices.span * index_size},
          index_size, index_size),
        "");

  CMD(DrawIndexed{.indices       = indices,
                  .instances     = instances,
                  .vertex_offset = vertex_offset});
}

void ICommandEncoder::draw_indirect(gpu::Buffer buffer_, u64 offset,
                                    u32 draw_count, u32 stride)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline.is_some(), "");
  CHECK(ctx_.has_graphics_state, "");
  CHECK(ctx_.graphics_pipeline->num_vertex_attributes ==
          ctx_.vertex_buffers.size(),
        "");
  CHECK(ctx_.graphics_pipeline->num_sets == ctx_.descriptor_sets.size(), "");

  auto * buffer = (Buffer) buffer_;

  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(stride >= sizeof(gpu::DrawCommand), "");
  CHECK(is_valid_buffer_access(
          buffer->size, Slice64{offset, (u64) draw_count * (u64) stride}, 4, 4),
        "");

  CMD(DrawIndirect{.buffer     = buffer->vk,
                   .offset     = offset,
                   .draw_count = draw_count,
                   .stride     = stride});

  tracker_.track(buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                 VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
}

void ICommandEncoder::draw_indexed_indirect(gpu::Buffer buffer_, u64 offset,
                                            u32 draw_count, u32 stride)
{
  PRELUDE();
  CHECK(pass_ == Pass::Render, "");
  CHECK(ctx_.graphics_pipeline.is_some(), "");
  CHECK(ctx_.has_graphics_state, "");
  CHECK(ctx_.graphics_pipeline->num_vertex_attributes ==
          ctx_.vertex_buffers.size(),
        "");
  CHECK(ctx_.graphics_pipeline->num_sets == ctx_.descriptor_sets.size(), "");
  CHECK(ctx_.index_buffer.is_some(), "");

  auto * buffer = (Buffer) buffer_;

  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(is_valid_buffer_access(
          buffer->size, Slice64{offset, (u64) draw_count * (u64) stride}, 4, 4),
        "");
  CHECK(stride >= sizeof(gpu::DrawIndexedCommand), "");

  CMD(DrawIndexedIndirect{.buffer     = buffer->vk,
                          .offset     = offset,
                          .draw_count = draw_count,
                          .stride     = stride});

  tracker_.track(buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                 VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
}

void ICommandEncoder::present(gpu::Swapchain swapchain_)
{
  CHECK(state_ == CommandBufferState::Recording, "");

  CHECK(swapchain_ != nullptr, "");
  CHECK(this->swapchain_.is_none(),
        "Can only present one swapchain on a Command Encoder/Command Buffer");

  auto swapchain = (Swapchain) swapchain_;
  CHECK(!swapchain->is_out_of_date,
        "Attempted to present to out-of-date swapchain");
  this->swapchain_ = *swapchain;

  if (!swapchain->is_deferred)
  {
    tracker_.track(swapchain->images[swapchain->current_image.unwrap()],
                   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_NONE,
                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }
}

void ICommandBuffer::begin()
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

  auto result = dev_->table_.BeginCommandBuffer(vk_, &info);

  CHECK(result == VK_SUCCESS, "");

  state_ = CommandBufferState::Recording;
}

Result<Void, Status> ICommandBuffer::end()
{
  CHECK(state_ == CommandBufferState::Recording, "");

  dev_->table_.EndCommandBuffer(vk_);

  state_ = CommandBufferState::Recorded;
  if (status_ != Status::Success)
  {
    return Err{status_};
  }

  return Ok{};
}

void ICommandBuffer::reset()
{
  CHECK(state_ == CommandBufferState::Reset ||
          state_ == CommandBufferState::Submitted,
        "");

  dev_->table_.ResetCommandBuffer(vk_, 0);

  state_ = CommandBufferState::Reset;
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::ResetTimestampQuery const & cmd)
{
  t.CmdResetQueryPool(vk, cmd.query, cmd.range.offset, cmd.range.span);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::ResetStatisticsQuery const & cmd)
{
  t.CmdResetQueryPool(vk, cmd.query, cmd.range.offset, cmd.range.span);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::WriteTimestamp const & cmd)
{
  t.CmdWriteTimestamp(vk, cmd.stages, cmd.query, cmd.index);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BeginStatistics const & cmd)
{
  t.CmdBeginQuery(vk, cmd.query, cmd.index, 0);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::EndStatistics const & cmd)
{
  t.CmdEndQuery(vk, cmd.query, cmd.index);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BeginDebugMarker const & cmd)
{
  t.CmdDebugMarkerBeginEXT(vk, &cmd.info);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::EndDebugMarker const &)
{
  t.CmdDebugMarkerEndEXT(vk);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::FillBuffer const & cmd)
{
  t.CmdFillBuffer(vk, cmd.dst, cmd.range.offset, cmd.range.span, cmd.data);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::CopyBuffer const & cmd)
{
  t.CmdCopyBuffer(vk, cmd.src, cmd.dst, size32(cmd.copies), cmd.copies.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::UpdateBuffer const & cmd)
{
  t.CmdUpdateBuffer(vk, cmd.dst, cmd.dst_offset, size64(cmd.src),
                    cmd.src.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::ClearColorImage const & cmd)
{
  t.CmdClearColorImage(vk, cmd.dst, cmd.dst_layout, &cmd.value,
                       size32(cmd.ranges), cmd.ranges.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::ClearDepthStencilImage const & cmd)
{
  t.CmdClearDepthStencilImage(vk, cmd.dst, cmd.dst_layout, &cmd.value,
                              size32(cmd.ranges), cmd.ranges.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::CopyImage const & cmd)
{
  t.CmdCopyImage(vk, cmd.src, cmd.src_layout, cmd.dst, cmd.dst_layout,
                 size32(cmd.copies), cmd.copies.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::CopyBufferToImage const & cmd)
{
  t.CmdCopyBufferToImage(vk, cmd.src, cmd.dst, cmd.dst_layout,
                         size32(cmd.copies), cmd.copies.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BlitImage const & cmd)
{
  t.CmdBlitImage(vk, cmd.src, cmd.src_layout, cmd.dst, cmd.dst_layout,
                 size32(cmd.blits), cmd.blits.data(), cmd.filter);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::ResolveImage const & cmd)
{
  t.CmdResolveImage(vk, cmd.src, cmd.src_layout, cmd.dst, cmd.dst_layout,
                    size32(cmd.resolves), cmd.resolves.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BeginRendering const & cmd)
{
  t.CmdBeginRenderingKHR(vk, &cmd.info);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::EndRendering const &)
{
  t.CmdEndRenderingKHR(vk);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BindPipeline const & cmd)
{
  t.CmdBindPipeline(vk, cmd.bind_point, cmd.pipeline);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BindDescriptorSets const & cmd)
{
  t.CmdBindDescriptorSets(vk, cmd.bind_point, cmd.layout, 0, size32(cmd.sets),
                          cmd.sets.data(), size32(cmd.dynamic_offsets),
                          cmd.dynamic_offsets.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::PushConstants const & cmd)
{
  t.CmdPushConstants(vk, cmd.layout, VK_SHADER_STAGE_ALL, 0,
                     size32(cmd.constants), cmd.constants.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::Dispatch const & cmd)
{
  t.CmdDispatch(vk, cmd.group_count.x(), cmd.group_count.y(),
                cmd.group_count.z());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::DispatchIndirect const & cmd)
{
  t.CmdDispatchIndirect(vk, cmd.buffer, cmd.offset);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
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

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BindVertexBuffers const & cmd)
{
  t.CmdBindVertexBuffers(vk, 0, size32(cmd.buffers), cmd.buffers.data(),
                         cmd.offsets.data());
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::BindIndexBuffer const & cmd)
{
  t.CmdBindIndexBuffer(vk, cmd.buffer, cmd.offset, cmd.index_type);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::Draw const & cmd)
{
  t.CmdDraw(vk, cmd.vertices.span, cmd.instances.span, cmd.vertices.offset,
            cmd.instances.offset);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::DrawIndexed const & cmd)
{
  t.CmdDrawIndexed(vk, cmd.indices.span, cmd.instances.span, cmd.indices.offset,
                   cmd.vertex_offset, cmd.instances.offset);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::DrawIndirect const & cmd)
{
  t.CmdDrawIndirect(vk, cmd.buffer, cmd.offset, cmd.draw_count, cmd.stride);
}

ASH_FORCE_INLINE void enc(DeviceTable const & t, VkCommandBuffer vk,
                          cmd::DrawIndexedIndirect const & cmd)
{
  t.CmdDrawIndexedIndirect(vk, cmd.buffer, cmd.offset, cmd.draw_count,
                           cmd.stride);
}

inline cmd::Cmd * encode_n(DeviceTable const & t, VkCommandBuffer vk,
                           cmd::Cmd * cmd_, usize n)
{
#define CMD_CASE(type)               \
  case cmd::Type::type:              \
  {                                  \
    auto * cmd = (cmd::type *) cmd_; \
    enc(t, vk, *cmd);                \
  }                                  \
  break;

  while (n != 0)
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

void issue_barriers(DeviceTable const & t, VkCommandBuffer cmd,
                    HazardBarriers const & barriers)
{
  for (auto [src_stage, dst_stage, buffer_barrier] : barriers.buffers_)
  {
    t.CmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 1,
                         &buffer_barrier, 0, nullptr);
  }

  for (auto [src_stage, dst_stage, memory_barrier, buffer_barrier] :
       barriers.mem_buffers_)
  {
    t.CmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 1, &memory_barrier, 1,
                         &buffer_barrier, 0, nullptr);
  }

  for (auto [src_stage, dst_stage, image_barrier] : barriers.images_)
  {
    t.CmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr,
                         1, &image_barrier);
  }

  for (auto [src_stage, dst_stage, memory_barrier, image_barrier] :
       barriers.mem_images_)
  {
    t.CmdPipelineBarrier(cmd, src_stage, dst_stage, 0, 1, &memory_barrier, 0,
                         nullptr, 1, &image_barrier);
  }
}

void ICommandBuffer::record(gpu::CommandEncoder encoder_)
{
  CHECK(state_ == CommandBufferState::Recording, "");
  auto * encoder = (CommandEncoder) encoder_;
  CHECK(encoder != nullptr, "");

  if (encoder->tracker_.passes_.is_empty())
  {
    return;
  }

  HazardBarriers barriers{arena_};
  auto *         next_command = encoder->tracker_.first_cmd_;

  for (auto [ipass, begin] : enumerate(encoder->tracker_.passes_.view().slice(
         0, encoder->tracker_.passes_.size() - 1)))
  {
    auto const & end = encoder->tracker_.passes_[ipass + 1];

    auto commands = Slice32::range(begin.commands, end.commands);
    auto buffers  = Slice32::range(begin.buffers, end.buffers);
    auto images   = Slice32::range(begin.images, end.images);
    auto descriptor_sets =
      Slice32::range(begin.descriptor_sets, end.descriptor_sets);

    for (auto [buffer, stages, access] :
         encoder->tracker_.buffers_.view().slice(buffers))
    {
      resource_states_.access(*buffer,
                              MemAccess{.stages = stages, .access = access},
                              ipass, barriers);
    }

    for (auto [image, stages, access, layout] :
         encoder->tracker_.images_.view().slice(images))
    {
      resource_states_.access(*image,
                              MemAccess{.stages = stages, .access = access},
                              layout, ipass, barriers);
    }

    for (auto [descriptor_set, stages] :
         encoder->tracker_.descriptor_sets_.view().slice(descriptor_sets))
    {
      resource_states_.access(*descriptor_set, stages, ipass, barriers);
    }

    issue_barriers(dev_->table_, vk_, barriers);
    barriers.clear();

    next_command = encode_n(dev_->table_, vk_, next_command, commands.span);
  }
}

void ICommandBuffer::commit_resource_states()
{
  {
    WriteGuard guard{dev_->resource_states_.lock_};
    resource_states_.commit(dev_->resource_states_);
  }
}

}    // namespace vk
}    // namespace ash
