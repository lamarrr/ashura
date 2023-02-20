#pragma once

#include <utility>

#include "ashura/asset_bundle.h"
#include "ashura/font.h"
#include "ashura/vulkan.h"
#include "stx/rc.h"
#include "stx/span.h"
#include "stx/vec.h"

namespace ash {
namespace vk {

struct UploadContext {
  VkCommandPool cmd_pool = VK_NULL_HANDLE;
  VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
  VkFence fence = VK_NULL_HANDLE;
  stx::Option<stx::Rc<CommandQueue*>> queue;

  void init(stx::Rc<CommandQueue*> aqueue) {
    queue = stx::Some(std::move(aqueue));
    VkDevice dev = queue.value()->device->dev;

    VkCommandPoolCreateInfo cmd_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue.value()->info.family.index};

    ASH_VK_CHECK(
        vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    ASH_VK_CHECK(
        vkAllocateCommandBuffers(dev, &cmd_buffer_allocate_info, &cmd_buffer));

    VkFenceCreateInfo fence_create_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0};

    ASH_VK_CHECK(vkCreateFence(dev, &fence_create_info, nullptr, &fence));
  }

  void destroy() {
    VkDevice dev = queue.value()->device->dev;

    ASH_VK_CHECK(vkDeviceWaitIdle(dev));

    vkFreeCommandBuffers(dev, cmd_pool, 1, &cmd_buffer);

    vkDestroyCommandPool(dev, cmd_pool, nullptr);

    vkDestroyFence(dev, fence, nullptr);
  }

  stx::Rc<ImageResource*> upload_image(stx::Span<u8 const> data, extent extent,
                                       u32 nchannels) {
    CommandQueue& cqueue = *queue.value().handle;
    VkDevice dev = cqueue.device->dev;
    VkPhysicalDeviceMemoryProperties const& memory_properties =
        cqueue.device->phy_dev->memory_properties;

    ASH_CHECK(data.size_bytes() == extent.area() * nchannels);
    ASH_CHECK(nchannels == 4, "only 4-channel images presently supported");
    ASH_CHECK(extent.is_visible());
    ASH_CHECK(nchannels != 0);

    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    if (nchannels == 4) {
    } else if (nchannels == 3) {
      format = VK_FORMAT_R8G8B8_SRGB;
    } else if (nchannels == 1) {
      format = VK_FORMAT_R8_SRGB;
    } else {
      ASH_PANIC("image channels must either be 1, 3, or 4");
    }

    VkImageCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent =
            VkExtent3D{
                .width = extent.width, .height = extent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    VkImage image;

    ASH_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

    VkMemoryRequirements memory_requirements;

    vkGetImageMemoryRequirements(dev, image, &memory_requirements);

    u32 memory_type_index =
        find_suitable_memory_type(memory_properties, memory_requirements,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .unwrap();

    VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index};

    VkDeviceMemory memory;

    ASH_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

    ASH_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

    VkImageViewCreateInfo view_create_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                         .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                         .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                         .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange =
            VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1}};

    VkImageView view;

    ASH_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

    Buffer staging_buffer =
        create_host_buffer(dev, memory_properties, data.size_bytes(),
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    staging_buffer.write(data.data());

    VkCommandBufferBeginInfo cmd_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr};

    ASH_VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info));

    VkImageMemoryBarrier pre_upload_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_NONE_KHR,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1}};

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, nullptr, 1,
                         &pre_upload_barrier);

    VkBufferImageCopy copy{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .mipLevel = 0,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1},
        .imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
        .imageExtent = VkExtent3D{
            .width = extent.width, .height = extent.height, .depth = 1}};

    vkCmdCopyBufferToImage(cmd_buffer, staging_buffer.buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    VkImageMemoryBarrier post_upload_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1}};

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &post_upload_barrier);

    ASH_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext = nullptr,
                             .waitSemaphoreCount = 0,
                             .pWaitSemaphores = nullptr,
                             .pWaitDstStageMask = nullptr,
                             .commandBufferCount = 1,
                             .pCommandBuffers = &cmd_buffer,
                             .signalSemaphoreCount = 0,
                             .pSignalSemaphores = nullptr};

    ASH_VK_CHECK(vkResetFences(dev, 1, &fence));

    ASH_VK_CHECK(vkQueueSubmit(cqueue.info.queue, 1, &submit_info, fence));

    ASH_VK_CHECK(vkWaitForFences(dev, 1, &fence, VK_TRUE, COMMAND_TIMEOUT));

    ASH_VK_CHECK(vkResetCommandBuffer(cmd_buffer, 0));

    staging_buffer.destroy();

    return stx::rc::make_inplace<ImageResource>(stx::os_allocator, image, view,
                                                memory, queue.value().share())
        .unwrap();
  }

  std::pair<stx::Rc<ImageResource*>, gfx::FontAtlas> cache_font(
      stx::Rc<Font*> font, u32 font_height) {
    VkImageFormatProperties image_format_properties;

    ASH_VK_CHECK(vkGetPhysicalDeviceImageFormatProperties(
        queue.value()->device->phy_dev->phy_device, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT,
        0, &image_format_properties));

    auto [atlas, image_buffer] =
        gfx::render_atlas(*font.handle, font_height,
                          extent{image_format_properties.maxExtent.width,
                                 image_format_properties.maxExtent.height});

    return std::make_pair(
        upload_image(image_buffer.span(), image_buffer.extent, 4),
        std::move(atlas));
  }
};

struct RecordingContext {
  VkCommandPool cmd_pool = VK_NULL_HANDLE;
  stx::Vec<VkCommandBuffer> cmd_buffers{stx::os_allocator};
  VkShaderModule vertex_shader = VK_NULL_HANDLE;
  VkShaderModule fragment_shader = VK_NULL_HANDLE;
  Pipeline pipeline;

  // one descriptor pool per frame in flight
  stx::Vec<VkDescriptorPool> descriptor_pools{stx::os_allocator};
  stx::Vec<DescriptorPoolInfo> descriptor_pool_infos{stx::os_allocator};

  // specifications describing binding types/layouts for the descriptor sets
  // used. we will have multiple of each
  stx::Vec<DescriptorSetSpec> descriptor_set_specs{stx::os_allocator};

  // the created layouts for each of the descriptor sets
  stx::Vec<VkDescriptorSetLayout> descriptor_set_layouts{stx::os_allocator};

  // the allocated descriptor sets, the first vec is for each frame in flight
  // and the second vec contains the descriptor sets repeated for each of the
  // draw calls. i.e. num_draw_calls x num_descriptor_sets_per_frame
  stx::Vec<stx::Vec<VkDescriptorSet>> descriptor_sets{stx::os_allocator};
  stx::Vec<VkVertexInputAttributeDescription> vertex_input_attr{
      stx::os_allocator};
  u32 vertex_input_size = 0;
  u32 push_constant_size = 0;
  usize max_nframes_in_flight = 0;
  stx::Option<stx::Rc<CommandQueue*>> queue;

  void init(
      stx::Rc<CommandQueue*> aqueue, stx::Span<u32 const> vertex_shader_code,
      stx::Span<u32 const> fragment_shader_code,
      stx::Span<VkVertexInputAttributeDescription const> avertex_input_attr,
      u32 avertex_input_size, u32 apush_constant_size,
      usize max_nframes_in_flight,
      stx::Span<DescriptorSetSpec> adescriptor_sets_specs,
      stx::Span<VkDescriptorPoolSize const> adescriptor_pool_sizes,
      u32 max_descriptor_sets) {
    queue = stx::Some(std::move(aqueue));

    CommandQueue& cqueue = *queue.value().handle;
    VkDevice dev = cqueue.device->device;

    auto create_shader = [dev](stx::Span<u32 const> code) {
      VkShaderModuleCreateInfo create_info{
          .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .codeSize = code.size_bytes(),
          .pCode = code.data()};

      VkShaderModule shader;

      ASH_VK_CHECK(vkCreateShaderModule(dev, &create_info, nullptr, &shader));

      return shader;
    };

    vertex_shader = create_shader(vertex_shader_code);

    fragment_shader = create_shader(fragment_shader_code);

    VkCommandPoolCreateInfo cmd_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = cqueue.info.family.index};

    ASH_VK_CHECK(
        vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    vertex_input_attr.extend(avertex_input_attr).unwrap();
    vertex_input_size = avertex_input_size;
    push_constant_size = apush_constant_size;

    descriptor_set_specs.extend_move(adescriptor_sets_specs).unwrap();

    for (DescriptorSetSpec const& spec : descriptor_set_specs) {
      stx::Vec<VkDescriptorSetLayoutBinding> bindings{stx::os_allocator};

      u32 ibinding = 0;

      for (VkDescriptorType type : spec.bindings) {
        VkDescriptorSetLayoutBinding binding{
            .binding = ibinding,
            .descriptorType = type,
            .descriptorCount = 1,
            .stageFlags =
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

        bindings.push_inplace(binding).unwrap();
        ibinding++;
      }

      VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
          .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .bindingCount = AS_U32(bindings.size()),
          .pBindings = bindings.data()};

      VkDescriptorSetLayout descriptor_set_layout;

      ASH_VK_CHECK(
          vkCreateDescriptorSetLayout(dev, &descriptor_set_layout_create_info,
                                      nullptr, &descriptor_set_layout));

      descriptor_set_layouts.push_inplace(descriptor_set_layout).unwrap();
    }

    cmd_buffers.resize(max_nframes_in_flight).unwrap();

    VkCommandBufferAllocateInfo cmd_buffers_allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = max_nframes_in_flight};

    ASH_VK_CHECK(vkAllocateCommandBuffers(dev, &cmd_buffers_allocate_info,
                                          cmd_buffers.data()));

    for (usize i = 0; i < max_nframes_in_flight; i++) {
      stx::Vec<VkDescriptorPoolSize> pool_sizes{stx::os_allocator};
      pool_sizes.extend(adescriptor_pool_sizes).unwrap();

      VkDescriptorPoolCreateInfo descriptor_pool_create_info{
          .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
          .pNext = nullptr,
          .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
          .maxSets = max_descriptor_sets,
          .poolSizeCount = AS_U32(adescriptor_pool_sizes.size()),
          .pPoolSizes = adescriptor_pool_sizes.data()};

      VkDescriptorPool descriptor_pool;

      ASH_VK_CHECK(vkCreateDescriptorPool(dev, &descriptor_pool_create_info,
                                          nullptr, &descriptor_pool));

      descriptor_pools.push_inplace(descriptor_pool).unwrap();

      descriptor_pool_infos
          .push(DescriptorPoolInfo{
              .sizes = std::move(pool_sizes),
              .max_sets = descriptor_pool_create_info.maxSets})
          .unwrap();
    }

    for (usize i = 0; i < max_nframes_in_flight; i++) {
      descriptor_sets.push(stx::Vec<VkDescriptorSet>{stx::os_allocator})
          .unwrap();
    }
  }

  void rebuild(VkRenderPass target_render_pass,
               VkSampleCountFlagBits msaa_sample_count) {
    VkDevice dev = queue.value()->device->device;
    if (pipeline.layout != nullptr) {
      vkDestroyPipelineLayout(dev, pipeline.layout, nullptr);
    }

    if (pipeline.pipeline != nullptr) {
      vkDestroyPipeline(dev, pipeline.pipeline, nullptr);
    }

    pipeline.build(queue.value()->device->device, vertex_shader,
                   fragment_shader, target_render_pass, msaa_sample_count,
                   descriptor_set_layouts, vertex_input_attr, vertex_input_size,
                   push_constant_size);
  }

  void destroy() {
    VkDevice dev = queue.value()->device->device;

    ASH_VK_CHECK(vkDeviceWaitIdle(dev));

    vkDestroyShaderModule(dev, vertex_shader, nullptr);

    vkDestroyShaderModule(dev, fragment_shader, nullptr);

    vkFreeCommandBuffers(dev, cmd_pool, max_nframes_in_flight,
                         cmd_buffers.data());

    vkDestroyCommandPool(dev, cmd_pool, nullptr);

    for (VkDescriptorSetLayout layout : descriptor_set_layouts) {
      vkDestroyDescriptorSetLayout(dev, layout, nullptr);
    }

    u32 frame_index = 0;
    for (stx::Vec<VkDescriptorSet> const& set : descriptor_sets) {
      vkFreeDescriptorSets(dev, descriptor_pools[frame_index],
                           AS_U32(set.size()), set.data());
      frame_index++;
    }

    for (VkDescriptorPool descriptor_pool : descriptor_pools) {
      vkDestroyDescriptorPool(dev, descriptor_pool, nullptr);
    }

    pipeline.destroy(dev);
  }
};

inline gfx::CachedFont cache_font(UploadContext& context,
                                  AssetBundle<stx::Rc<ImageResource*>>& bundle,
                                  stx::Rc<Font*> font, u32 font_height) {
  auto [image, atlas] = context.cache_font(font.share(), font_height);
  gfx::image texture = bundle.add(std::move(image));
  atlas.image = texture;
  return gfx::CachedFont{.font = std::move(font), .atlas = std::move(atlas)};
}

}  // namespace vk
}  // namespace ash
