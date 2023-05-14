#pragma once

#include <algorithm>
#include <map>
#include <utility>

#include "ashura/font.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/vulkan.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/span.h"
#include "stx/spinlock.h"
#include "stx/vec.h"

namespace ash
{

namespace vk
{

struct RenderImage
{
  vk::Image               image;
  ImageFormat             format         = ImageFormat::Rgba;
  VkFormat                backend_format = VK_FORMAT_R8G8B8A8_UNORM;
  VkImageLayout           layout         = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageLayout           dst_layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  ash::extent             extent;
  stx::Option<vk::Buffer> staging_buffer;
  bool                    needs_upload = false;
  bool                    needs_delete = false;
  bool                    is_real_time = false;
};

struct RenderResourceManager
{
  VkCommandPool                        cmd_pool   = VK_NULL_HANDLE;
  VkCommandBuffer                      cmd_buffer = VK_NULL_HANDLE;
  VkFence                              fence      = VK_NULL_HANDLE;
  stx::Option<stx::Rc<CommandQueue *>> queue;
  std::map<gfx::image, RenderImage>    images;
  u64                                  next_image_id = 0;

  void init(stx::Rc<CommandQueue *> aqueue)
  {
    queue        = stx::Some(std::move(aqueue));
    VkDevice dev = queue.value()->device->dev;

    VkCommandPoolCreateInfo cmd_pool_create_info{.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                                 .pNext            = nullptr,
                                                 .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                 .queueFamilyIndex = queue.value()->info.family.index};

    ASH_VK_CHECK(vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info{.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                         .pNext              = nullptr,
                                                         .commandPool        = cmd_pool,
                                                         .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                         .commandBufferCount = 1};

    ASH_VK_CHECK(vkAllocateCommandBuffers(dev, &cmd_buffer_allocate_info, &cmd_buffer));

    VkFenceCreateInfo fence_create_info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = nullptr, .flags = 0};

    ASH_VK_CHECK(vkCreateFence(dev, &fence_create_info, nullptr, &fence));
  }

  void destroy()
  {
    VkDevice dev = queue.value()->device->dev;

    ASH_VK_CHECK(vkDeviceWaitIdle(dev));

    for (auto &entry : images)
    {
      entry.second.needs_delete = true;
    }

    flush_deletes();
    images = {};

    vkFreeCommandBuffers(dev, cmd_pool, 1, &cmd_buffer);

    vkDestroyCommandPool(dev, cmd_pool, nullptr);

    vkDestroyFence(dev, fence, nullptr);
  }

  // BGRA input => BGRA output
  // [Alpha, Antialiasing, Gray, RGB, RGBA] inputs => RGBA output
  static void copy_pixels(ImageView view, stx::Span<u8> dst)
  {
    u8 const *in  = view.data.begin();
    u8       *out = dst.begin();

    switch (view.format)
    {
      case ImageFormat::Alpha:
      {
        for (usize i = 0; i < view.extent.area(); i++)
        {
          out[0] = 0x00;
          out[1] = 0x00;
          out[2] = 0x00;
          out[3] = *in;
          out += 4;
          in++;
        }
      }
      break;

      case ImageFormat::Antialiasing:
      {
        for (usize i = 0; i < view.extent.area(); i++)
        {
          out[0] = 0xFF;
          out[1] = 0xFF;
          out[2] = 0xFF;
          out[3] = *in;
          out += 4;
          in++;
        }
      }
      break;

      case ImageFormat::Gray:
      {
        for (usize i = 0; i < view.extent.area(); i++)
        {
          out[0] = *in;
          out[1] = *in;
          out[2] = *in;
          out[3] = 0xFF;
          out += 4;
          in++;
        }
      }
      break;

      case ImageFormat::Rgb:
      {
        for (usize i = 0; i < view.extent.area(); i++)
        {
          out[0] = in[0];
          out[1] = in[1];
          out[2] = in[2];
          out[3] = 0xFF;
          out += 4;
          in += 3;
        }
      }
      break;

      case ImageFormat::Rgba:
      {
        dst.copy(view.data);
      }
      break;

      case ImageFormat::Bgra:
      {
        dst.copy(view.data);
      }
      break;

      default:
      {
        ASH_UNREACHABLE();
      }
      break;
    }
  }

  gfx::image add_image(ImageView image_view, bool is_real_time)
  {
    gfx::image id = next_image_id;
    next_image_id++;

    CommandQueue const                     &queue             = *this->queue.value();
    VkDevice                                dev               = queue.device->dev;
    VkPhysicalDeviceMemoryProperties const &memory_properties = queue.device->phy_dev->memory_properties;

    ASH_CHECK(image_view.extent.is_visible());
    ASH_CHECK(image_view.data.size_bytes() == image_view.extent.area() * nchannel_bytes(image_view.format));

    // use RGBA8888 for everything else
    VkFormat target_format = image_view.format == ImageFormat::Bgra ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo create_info{
        .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = 0,
        .imageType             = VK_IMAGE_TYPE_2D,
        .format                = target_format,
        .extent                = VkExtent3D{.width = image_view.extent.width, .height = image_view.extent.height, .depth = 1},
        .mipLevels             = 1,
        .arrayLayers           = 1,
        .samples               = VK_SAMPLE_COUNT_1_BIT,
        .tiling                = VK_IMAGE_TILING_OPTIMAL,
        .usage                 = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};

    VkImage image;

    ASH_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

    VkMemoryRequirements memory_requirements;

    vkGetImageMemoryRequirements(dev, image, &memory_requirements);

    u32 memory_type_index = find_suitable_memory_type(memory_properties, memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).unwrap();

    VkMemoryAllocateInfo alloc_info{.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                    .pNext           = nullptr,
                                    .allocationSize  = memory_requirements.size,
                                    .memoryTypeIndex = memory_type_index};

    VkDeviceMemory memory;

    ASH_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

    ASH_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

    VkImageViewCreateInfo view_create_info{
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .image            = image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = target_format,
        .components       = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                               .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                               .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                               .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = VkImageSubresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};

    VkImageView view;

    ASH_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

    Buffer staging_buffer = create_host_visible_buffer(dev, memory_properties, image_view.extent.area() * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    auto begin = std::chrono::steady_clock::now();
    copy_pixels(image_view, staging_buffer.span());
    spdlog::info("blitted image #{} in {} ms", id, (std::chrono::steady_clock::now() - begin).count() / 1'000'000.0f);

    images.emplace(id, RenderImage{.image          = Image{.image = image, .view = view, .memory = memory, .dev = dev},
                                   .format         = image_view.format,
                                   .backend_format = target_format,
                                   .layout         = VK_IMAGE_LAYOUT_UNDEFINED,
                                   .dst_layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                   .extent         = image_view.extent,
                                   .staging_buffer = stx::Some(std::move(staging_buffer)),
                                   .needs_upload   = true,
                                   .needs_delete   = false,
                                   .is_real_time   = is_real_time});

    ASH_LOG_INFO(Vulkan_RenderResourceManager, "Created {}{} {}x{} image #{} with format={} and size={} bytes", is_real_time ? "" : "non-", "real-time", image_view.extent.width, image_view.extent.height, id, string_VkFormat(target_format), memory_requirements.size);

    return id;
  }

  void update(gfx::image image, ImageView view)
  {
    auto pos = images.find(image);
    ASH_CHECK(pos != images.end());
    ASH_CHECK(pos->second.format == view.format);
    ASH_CHECK(pos->second.extent == view.extent);
    ASH_CHECK(!pos->second.needs_delete);

    if (pos->second.needs_upload || pos->second.is_real_time)
    {
      copy_pixels(view, pos->second.staging_buffer.value().span());
    }
    else
    {
      CommandQueue const &queue          = *this->queue.value();
      vk::Buffer          staging_buffer = create_host_visible_buffer(queue.device->dev, queue.device->phy_dev->memory_properties, view.extent.area() * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
      copy_pixels(view, staging_buffer.span());
      pos->second.staging_buffer = stx::Some(std::move(staging_buffer));
    }
    pos->second.needs_upload = true;
  }

  void remove(gfx::image image)
  {
    auto pos = images.find(image);
    ASH_CHECK(pos != images.end());
    pos->second.needs_delete = true;
    ASH_LOG_INFO(Vulkan_RenderResourceManager, "Marked image: {} as ready for deletion");
  }

  void submit_uploads()
  {
    bool has_pending_upload = false;

    for (auto const &entry : images)
    {
      if (entry.second.needs_upload)
      {
        has_pending_upload = true;
        break;
      }
    }

    if (!has_pending_upload)
    {
      return;
    }

    CommandQueue const &queue = *this->queue.value();
    VkDevice            dev   = queue.device->dev;

    VkCommandBufferBeginInfo cmd_buffer_begin_info{.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                                   .pNext            = nullptr,
                                                   .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                                   .pInheritanceInfo = nullptr};

    ASH_VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin_info));

    for (auto const &entry : images)
    {
      if (entry.second.needs_upload)
      {
        VkImageMemoryBarrier pre_upload_barrier{
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_NONE_KHR,
            .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout           = entry.second.layout,
            .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = entry.second.image.image,
            .subresourceRange    = VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};

        vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, nullptr, 1, &pre_upload_barrier);

        VkBufferImageCopy copy{
            .bufferOffset      = 0,
            .bufferRowLength   = 0,
            .bufferImageHeight = 0,
            .imageSubresource  = VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
            .imageOffset       = VkOffset3D{.x = 0, .y = 0, .z = 0},
            .imageExtent       = VkExtent3D{.width = entry.second.extent.width, .height = entry.second.extent.height, .depth = 1}};

        vkCmdCopyBufferToImage(cmd_buffer, entry.second.staging_buffer.value().buffer, entry.second.image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

        VkImageMemoryBarrier post_upload_barrier{
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout           = entry.second.dst_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = entry.second.image.image,
            .subresourceRange    = VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};

        vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &post_upload_barrier);
      }
    }

    ASH_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkSubmitInfo submit_info{.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext                = nullptr,
                             .waitSemaphoreCount   = 0,
                             .pWaitSemaphores      = nullptr,
                             .pWaitDstStageMask    = nullptr,
                             .commandBufferCount   = 1,
                             .pCommandBuffers      = &cmd_buffer,
                             .signalSemaphoreCount = 0,
                             .pSignalSemaphores    = nullptr};

    ASH_VK_CHECK(vkResetFences(dev, 1, &fence));

    ASH_VK_CHECK(vkQueueSubmit(queue.info.queue, 1, &submit_info, fence));

    ASH_VK_CHECK(vkWaitForFences(dev, 1, &fence, VK_TRUE, VULKAN_TIMEOUT));

    ASH_VK_CHECK(vkResetCommandBuffer(cmd_buffer, 0));

    for (auto &entry : images)
    {
      if (entry.second.needs_upload)
      {
        entry.second.needs_upload = false;
        if (!entry.second.is_real_time)
        {
          entry.second.staging_buffer.value().destroy();
          entry.second.staging_buffer = stx::None;
        }
        entry.second.layout = entry.second.dst_layout;
      }
    }

    ASH_LOG_INFO(Vulkan_RenderResourceManager, "Uploaded pending images");
  }

  void flush_deletes()
  {
    bool has_pending_delete = false;

    for (auto const &entry : images)
    {
      if (entry.second.needs_delete)
      {
        has_pending_delete = true;
        break;
      }
    }

    if (!has_pending_delete)
    {
      return;
    }

    ASH_VK_CHECK(vkQueueWaitIdle(queue.value()->info.queue));

    for (auto it = images.begin(); it != images.end(); it++)
    {
      if (it->second.needs_delete)
      {
        it->second.image.destroy();
        if (it->second.staging_buffer.is_some())
        {
          it->second.staging_buffer.value().destroy();
          it->second.staging_buffer = stx::None;
        }
      }
      images.erase(it);
    }

    ASH_LOG_INFO(Vulkan_RenderResourceManager, "Deleted pending images");
  }

  gfx::FontAtlas cache_font(Font const &font, u32 font_height)
  {
    VkImageFormatProperties image_format_properties;

    ASH_VK_CHECK(vkGetPhysicalDeviceImageFormatProperties(queue.value()->device->phy_dev->phy_device, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, &image_format_properties));

    auto [atlas, image_buffer] = gfx::render_atlas(font, font_height, extent{image_format_properties.maxExtent.width, image_format_properties.maxExtent.height});

    atlas.texture = add_image(image_buffer, false);

    return std::move(atlas);
  }
};

struct RecordingContext
{
  VkCommandPool                cmd_pool = VK_NULL_HANDLE;
  stx::Vec<VkCommandBuffer>    cmd_buffers;
  VkShaderModule               vertex_shader   = VK_NULL_HANDLE;
  VkShaderModule               fragment_shader = VK_NULL_HANDLE;
  Pipeline                     pipeline;
  stx::Vec<VkDescriptorPool>   descriptor_pools;        // one descriptor pool per frame in flight
  stx::Vec<DescriptorPoolInfo> descriptor_pool_infos;
  stx::Vec<DescriptorSetSpec>  descriptor_set_specs;                 // specifications describing binding types/layouts
                                                                     // for the descriptor sets used. we will have
                                                                     // multiple of each
  stx::Vec<VkDescriptorSetLayout>     descriptor_set_layouts;        // the created layouts for each of the descriptor sets
  stx::Vec<stx::Vec<VkDescriptorSet>> descriptor_sets;               // the allocated descriptor sets, the first vec is for
                                                                     // each frame in flight and the second vec contains the
                                                                     // descriptor sets repeated for each of the draw calls.
                                                                     // i.e. num_draw_calls x num_descriptor_sets_per_frame
  stx::Vec<VkVertexInputAttributeDescription> vertex_input_attr;
  u32                                         vertex_input_size     = 0;
  u32                                         push_constant_size    = 0;
  u32                                         max_nframes_in_flight = 0;
  u32                                         queue_family          = 0;
  VkDevice                                    dev                   = VK_NULL_HANDLE;

  void init(VkDevice adev, u32 aqueue_family, stx::Span<u32 const> vertex_shader_code,
            stx::Span<u32 const> fragment_shader_code, stx::Span<VkVertexInputAttributeDescription const> avertex_input_attr,
            u32 avertex_input_size, u32 apush_constant_size, u32 amax_nframes_in_flight,
            stx::Span<DescriptorSetSpec> adescriptor_sets_specs, stx::Span<VkDescriptorPoolSize const> adescriptor_pool_sizes,
            u32 max_descriptor_sets)
  {
    dev                   = adev;
    max_nframes_in_flight = amax_nframes_in_flight;
    vertex_input_size     = avertex_input_size;
    push_constant_size    = apush_constant_size;
    queue_family          = aqueue_family;

    auto create_shader = [this](stx::Span<u32 const> code) {
      VkShaderModuleCreateInfo create_info{.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                           .pNext    = nullptr,
                                           .flags    = 0,
                                           .codeSize = code.size_bytes(),
                                           .pCode    = code.data()};

      VkShaderModule shader;

      ASH_VK_CHECK(vkCreateShaderModule(dev, &create_info, nullptr, &shader));

      return shader;
    };

    vertex_shader = create_shader(vertex_shader_code);

    fragment_shader = create_shader(fragment_shader_code);

    VkCommandPoolCreateInfo cmd_pool_create_info{.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                                 .pNext            = nullptr,
                                                 .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                 .queueFamilyIndex = queue_family};

    ASH_VK_CHECK(vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    vertex_input_attr.extend(avertex_input_attr).unwrap();

    descriptor_set_specs.extend_move(adescriptor_sets_specs).unwrap();

    for (DescriptorSetSpec const &spec : descriptor_set_specs)
    {
      stx::Vec<VkDescriptorSetLayoutBinding> bindings;

      u32 ibinding = 0;

      for (VkDescriptorType type : spec.bindings)
      {
        VkDescriptorSetLayoutBinding binding{.binding         = ibinding,
                                             .descriptorType  = type,
                                             .descriptorCount = 1,
                                             .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT};

        bindings.push_inplace(binding).unwrap();
        ibinding++;
      }

      VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                                        .pNext        = nullptr,
                                                                        .flags        = 0,
                                                                        .bindingCount = AS(u32, bindings.size()),
                                                                        .pBindings    = bindings.data()};

      VkDescriptorSetLayout descriptor_set_layout;

      ASH_VK_CHECK(vkCreateDescriptorSetLayout(dev, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout));

      descriptor_set_layouts.push_inplace(descriptor_set_layout).unwrap();
    }

    cmd_buffers.resize(max_nframes_in_flight).unwrap();

    VkCommandBufferAllocateInfo cmd_buffers_allocate_info{.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                          .pNext              = nullptr,
                                                          .commandPool        = cmd_pool,
                                                          .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                          .commandBufferCount = AS(u32, max_nframes_in_flight)};

    ASH_VK_CHECK(vkAllocateCommandBuffers(dev, &cmd_buffers_allocate_info, cmd_buffers.data()));

    for (usize i = 0; i < max_nframes_in_flight; i++)
    {
      stx::Vec<VkDescriptorPoolSize> pool_sizes;
      pool_sizes.extend(adescriptor_pool_sizes).unwrap();

      VkDescriptorPoolCreateInfo descriptor_pool_create_info{.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                             .pNext         = nullptr,
                                                             .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                                             .maxSets       = max_descriptor_sets,
                                                             .poolSizeCount = AS(u32, adescriptor_pool_sizes.size()),
                                                             .pPoolSizes    = adescriptor_pool_sizes.data()};

      VkDescriptorPool descriptor_pool;

      ASH_VK_CHECK(vkCreateDescriptorPool(dev, &descriptor_pool_create_info, nullptr, &descriptor_pool));

      descriptor_pools.push_inplace(descriptor_pool).unwrap();

      descriptor_pool_infos
          .push(DescriptorPoolInfo{.sizes = std::move(pool_sizes), .max_sets = descriptor_pool_create_info.maxSets})
          .unwrap();
    }

    for (usize i = 0; i < max_nframes_in_flight; i++)
    {
      descriptor_sets.push(stx::Vec<VkDescriptorSet>{}).unwrap();
    }
  }

  void rebuild(VkRenderPass target_render_pass, VkSampleCountFlagBits msaa_sample_count)
  {
    if (pipeline.layout != nullptr)
    {
      vkDestroyPipelineLayout(dev, pipeline.layout, nullptr);
    }

    if (pipeline.pipeline != nullptr)
    {
      vkDestroyPipeline(dev, pipeline.pipeline, nullptr);
    }

    pipeline.build(dev, vertex_shader, fragment_shader, target_render_pass, msaa_sample_count, descriptor_set_layouts,
                   vertex_input_attr, vertex_input_size, push_constant_size);
  }

  void destroy()
  {
    ASH_VK_CHECK(vkDeviceWaitIdle(dev));

    vkDestroyShaderModule(dev, vertex_shader, nullptr);

    vkDestroyShaderModule(dev, fragment_shader, nullptr);

    vkFreeCommandBuffers(dev, cmd_pool, AS(u32, max_nframes_in_flight), cmd_buffers.data());

    vkDestroyCommandPool(dev, cmd_pool, nullptr);

    for (VkDescriptorSetLayout layout : descriptor_set_layouts)
    {
      vkDestroyDescriptorSetLayout(dev, layout, nullptr);
    }

    u32 frame_index = 0;
    for (stx::Vec<VkDescriptorSet> const &set : descriptor_sets)
    {
      vkFreeDescriptorSets(dev, descriptor_pools[frame_index], AS(u32, set.size()), set.data());
      frame_index++;
    }

    for (VkDescriptorPool descriptor_pool : descriptor_pools)
    {
      vkDestroyDescriptorPool(dev, descriptor_pool, nullptr);
    }

    pipeline.destroy();
  }
};

}        // namespace vk
}        // namespace ash
