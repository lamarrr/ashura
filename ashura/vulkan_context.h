#pragma once

#include <algorithm>
#include <map>
#include <utility>

#include "ashura/font.h"
#include "ashura/image.h"
#include "ashura/loggers.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/vulkan.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/span.h"
#include "stx/vec.h"

namespace ash
{

namespace vk
{

struct RenderImage
{
  Image       image;
  ImageFormat format =
      ImageFormat::Rgba8888;        // requested format from the frontend
  VkFormat gpu_format =
      VK_FORMAT_R8G8B8A8_UNORM;        // format used to store texture on GPU
  VkImageLayout       layout     = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageLayout       dst_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  ash::Extent         extent;
  stx::Option<Buffer> staging_buffer;
  VkDescriptorSet     descriptor_set = VK_NULL_HANDLE;
  bool                needs_upload   = false;
  bool                needs_delete   = false;
  bool                is_real_time   = false;

  void destroy(VkDescriptorPool descriptor_pool)
  {
    image.destroy();
    staging_buffer.match(&Buffer::destroy, []() {});
    ASH_VK_CHECK(
        vkFreeDescriptorSets(image.dev, descriptor_pool, 1, &descriptor_set));
  }
};

struct RenderResourceManager
{
  static constexpr u32 NMAX_IMAGE_DESCRIPTOR_SETS = 1024;

  VkCommandPool         cmd_pool              = VK_NULL_HANDLE;
  VkCommandBuffer       cmd_buffer            = VK_NULL_HANDLE;
  VkFence               fence                 = VK_NULL_HANDLE;
  VkDescriptorPool      descriptor_pool       = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
  VkSampler             sampler =
      VK_NULL_HANDLE;        // ideally list of samplers of different types
  stx::Option<stx::Rc<CommandQueue *>> queue;
  std::map<gfx::image, RenderImage>    images;
  u64                                  next_image_id = 0;

  void init(stx::Rc<CommandQueue *> aqueue)
  {
    queue        = stx::Some(std::move(aqueue));
    VkDevice dev = queue.value()->device->dev;

    VkCommandPoolCreateInfo cmd_pool_create_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue.value()->info.family.index};

    ASH_VK_CHECK(
        vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = cmd_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    ASH_VK_CHECK(
        vkAllocateCommandBuffers(dev, &cmd_buffer_allocate_info, &cmd_buffer));

    VkFenceCreateInfo fence_create_info{.sType =
                                            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                        .pNext = nullptr,
                                        .flags = 0};

    ASH_VK_CHECK(vkCreateFence(dev, &fence_create_info, nullptr, &fence));

    VkDescriptorSetLayoutBinding descriptor_set_bindings[] = {
        {.binding         = 0,
         .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags =
             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         .pImmutableSamplers = nullptr}};

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = static_cast<u32>(std::size(descriptor_set_bindings)),
        .pBindings    = descriptor_set_bindings,
    };

    ASH_VK_CHECK(vkCreateDescriptorSetLayout(dev,
                                             &descriptor_set_layout_create_info,
                                             nullptr, &descriptor_set_layout));

    VkDescriptorPoolSize descriptor_pool_sizes[] = {
        {.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = NMAX_IMAGE_DESCRIPTOR_SETS}};

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets       = NMAX_IMAGE_DESCRIPTOR_SETS,
        .poolSizeCount = static_cast<u32>(std::size(descriptor_pool_sizes)),
        .pPoolSizes    = descriptor_pool_sizes};

    ASH_VK_CHECK(vkCreateDescriptorPool(dev, &descriptor_pool_create_info,
                                        nullptr, &descriptor_pool));

    VkSamplerCreateInfo sampler_create_info{
        .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = 0,
        .magFilter        = VK_FILTER_LINEAR,
        .minFilter        = VK_FILTER_LINEAR,
        .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias       = 0,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy =
            queue.value()
                ->device->phy_dev->properties.limits.maxSamplerAnisotropy,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0,
        .maxLod                  = 0,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE};

    ASH_VK_CHECK(vkCreateSampler(dev, &sampler_create_info, nullptr, &sampler));
  }

  void destroy()
  {
    VkDevice dev = queue.value()->device->dev;

    ASH_VK_CHECK(vkDeviceWaitIdle(dev));

    for (auto &entry : images)
    {
      entry.second.needs_delete = true;
    }

    execute_deletes();
    images.clear();

    vkFreeCommandBuffers(dev, cmd_pool, 1, &cmd_buffer);

    vkDestroyCommandPool(dev, cmd_pool, nullptr);

    vkDestroyFence(dev, fence, nullptr);

    vkDestroyDescriptorSetLayout(dev, descriptor_set_layout, nullptr);

    vkDestroyDescriptorPool(dev, descriptor_pool, nullptr);

    vkDestroySampler(dev, sampler, nullptr);
  }

  static ImageFormat to_rep_format(ImageFormat fmt)
  {
    switch (fmt)
    {
      case ImageFormat::Rgba8888:
      case ImageFormat::Bgra8888:
      case ImageFormat::R8:
      {
        return fmt;
      }

      case ImageFormat::Rgb888:
      {
        return ImageFormat::Rgba8888;
      }

      default:
      {
        ASH_PANIC("Unsupported Texture Format Passed To Vulkan Backend");
      }
    }
  }

  static constexpr VkFormat to_vk(ImageFormat fmt)
  {
    switch (fmt)
    {
      case ImageFormat::Rgba8888:
        return VK_FORMAT_R8G8B8A8_UNORM;

      case ImageFormat::Rgb888:
        return VK_FORMAT_R8G8B8_UNORM;

      case ImageFormat::Bgra8888:
        return VK_FORMAT_B8G8R8A8_UNORM;

      case ImageFormat::R8:
        return VK_FORMAT_R8_UNORM;

      default:
      {
        ASH_UNREACHABLE();
      }
    }
  }

  /// Converts textures to their GPU representations
  /// Note that RGB is converted to RGBA as neither OpenGL nor Vulkan require
  /// implementation support for RGB it.
  static void copy_image_to_GPU_Buffer(ImageView<u8 const> src,
                                       ImageView<u8>       rep_dst)
  {
    ASH_CHECK(src.extent.x <= rep_dst.extent.x);
    ASH_CHECK(src.extent.y <= rep_dst.extent.y);

    switch (src.format)
    {
      case ImageFormat::Rgba8888:
      case ImageFormat::Bgra8888:
      case ImageFormat::R8:
      {
        rep_dst.copy(src);
      }
      break;

      case ImageFormat::Rgb888:
      {
        ASH_CHECK(rep_dst.format == ImageFormat::Rgba8888);
        u8 const   *in            = src.span.data();
        u8         *out           = rep_dst.span.data();
        usize const src_row_bytes = src.row_bytes();

        for (usize irow = 0; irow < src.extent.y;
             irow++, in += src.pitch, out += rep_dst.pitch)
        {
          u8 const *in_row  = in;
          u8       *out_row = out;
          for (; in_row < in + src_row_bytes; in_row += 3, out_row += 4)
          {
            out_row[0] = in_row[0];
            out_row[1] = in_row[1];
            out_row[2] = in_row[2];
            out_row[3] = 0xFF;
          }
        }
      }
      break;

      default:
      {
        ASH_UNREACHABLE();
      }
      break;
    }
  }

  /// image will uploaded and be available for use before next frame
  gfx::image add_image(ImageView<u8 const> image_view, bool is_real_time)
  {
    gfx::image id = next_image_id;
    next_image_id++;

    CommandQueue const                     &queue = *this->queue.value();
    VkDevice                                dev   = queue.device->dev;
    VkPhysicalDeviceMemoryProperties const &memory_properties =
        queue.device->phy_dev->memory_properties;

    ASH_CHECK(image_view.extent.x != 0 && image_view.extent.y != 0,
              "Encounted unsupported zero extent image");

    ImageFormat rep_format    = to_rep_format(image_view.format);
    VkFormat    rep_format_vk = to_vk(rep_format);

    VkImageCreateInfo create_info{
        .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext       = nullptr,
        .flags       = 0,
        .imageType   = VK_IMAGE_TYPE_2D,
        .format      = rep_format_vk,
        .extent      = VkExtent3D{.width  = image_view.extent.x,
                                  .height = image_view.extent.y,
                                  .depth  = 1},
        .mipLevels   = 1,
        .arrayLayers = 1,
        .samples     = VK_SAMPLE_COUNT_1_BIT,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = nullptr,
        .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED};

    VkImage image;

    ASH_VK_CHECK(vkCreateImage(dev, &create_info, nullptr, &image));

    VkMemoryRequirements memory_requirements;

    vkGetImageMemoryRequirements(dev, image, &memory_requirements);

    u32 memory_type_index =
        find_suitable_memory_type(memory_properties, memory_requirements,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .unwrap();

    VkMemoryAllocateInfo alloc_info{.sType =
                                        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                                    .pNext           = nullptr,
                                    .allocationSize  = memory_requirements.size,
                                    .memoryTypeIndex = memory_type_index};

    VkDeviceMemory memory;

    ASH_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

    ASH_VK_CHECK(vkBindImageMemory(dev, image, memory, 0));

    VkImageViewCreateInfo view_create_info{
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext      = nullptr,
        .flags      = 0,
        .image      = image,
        .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        .format     = rep_format_vk,
        .components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                         .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                         .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                         .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange =
            VkImageSubresourceRange{.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel   = 0,
                                    .levelCount     = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount     = 1}};

    VkImageView view;

    ASH_VK_CHECK(vkCreateImageView(dev, &view_create_info, nullptr, &view));

    Buffer staging_buffer = create_host_visible_buffer(
        dev, memory_properties, fitted_byte_size(image_view.extent, rep_format),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    auto begin = std::chrono::steady_clock::now();
    copy_image_to_GPU_Buffer(image_view,
                             ImageView<u8>{.span   = staging_buffer.span(),
                                           .extent = image_view.extent,
                                           .pitch  = image_view.extent.x *
                                                    pixel_byte_size(rep_format),
                                           .format = rep_format});
    ASH_LOG_INFO(Vulkan_RenderResourceManager,
                 "Copied Image #{} to Host Visible Staging Buffer in {} ms", id,
                 (std::chrono::steady_clock::now() - begin).count() /
                     1'000'000.0f);

    VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext              = nullptr,
        .descriptorPool     = descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &descriptor_set_layout};

    VkDescriptorSet descriptor_set;

    ASH_VK_CHECK(vkAllocateDescriptorSets(dev, &descriptor_set_allocate_info,
                                          &descriptor_set));

    {
      VkDescriptorImageInfo image_info{
          .sampler     = sampler,
          .imageView   = view,
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

      VkWriteDescriptorSet write{
          .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .pNext            = nullptr,
          .dstSet           = descriptor_set,
          .dstBinding       = 0,
          .dstArrayElement  = 0,
          .descriptorCount  = 1,
          .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo       = &image_info,
          .pBufferInfo      = nullptr,
          .pTexelBufferView = nullptr};

      vkUpdateDescriptorSets(dev, 1, &write, 0, nullptr);
    }

    images.emplace(
        id, RenderImage{.image      = Image{.image  = image,
                                            .view   = view,
                                            .memory = memory,
                                            .dev    = dev},
                        .format     = image_view.format,
                        .gpu_format = rep_format_vk,
                        .layout     = VK_IMAGE_LAYOUT_UNDEFINED,
                        .dst_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        .extent     = image_view.extent,
                        .staging_buffer = stx::Some(std::move(staging_buffer)),
                        .descriptor_set = descriptor_set,
                        .needs_upload   = true,
                        .needs_delete   = false,
                        .is_real_time   = is_real_time});

    ASH_LOG_INFO(
        Vulkan_RenderResourceManager,
        "Created {}{} {}x{} Image #{} with format={} and size={} bytes",
        is_real_time ? "" : "non-", "real-time", image_view.extent.x,
        image_view.extent.y, id, string_VkFormat(rep_format_vk),
        memory_requirements.size);

    return id;
  }

  void update(gfx::image image, ImageView<u8 const> view)
  {
    auto pos = images.find(image);
    ASH_CHECK(pos != images.end());
    auto &rimage = pos->second;
    ASH_CHECK(rimage.format == view.format);
    ASH_CHECK(rimage.extent == view.extent);
    ASH_CHECK(!rimage.needs_delete);

    ImageFormat rep_format = to_rep_format(rimage.format);

    if (rimage.needs_upload || rimage.is_real_time)
    {
      copy_image_to_GPU_Buffer(
          view, ImageView<u8>{.span   = rimage.staging_buffer.value().span(),
                              .extent = rimage.extent,
                              .pitch  = rimage.extent.x *
                                       pixel_byte_size(rep_format),
                              .format = rep_format});
    }
    else
    {
      CommandQueue const &queue          = *this->queue.value();
      Buffer              staging_buffer = create_host_visible_buffer(
          queue.device->dev, queue.device->phy_dev->memory_properties,
          fitted_byte_size(rimage.extent, rep_format),
          VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
      copy_image_to_GPU_Buffer(
          view, ImageView<u8>{.span   = staging_buffer.span(),
                              .extent = rimage.extent,
                              .pitch  = rimage.extent.x *
                                       pixel_byte_size(rep_format),
                              .format = rep_format});
      rimage.staging_buffer = stx::Some(std::move(staging_buffer));
    }
    rimage.needs_upload = true;
  }

  void remove(gfx::image image)
  {
    auto pos = images.find(image);
    ASH_CHECK(pos != images.end());
    pos->second.needs_delete = true;
    ASH_LOG_INFO(Vulkan_RenderResourceManager,
                 "Marked image: {} as ready for deletion");
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

    VkCommandBufferBeginInfo cmd_buffer_begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
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
            .subresourceRange =
                VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel   = 0,
                                        .levelCount     = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount     = 1}};

        vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, nullptr, 1,
                             &pre_upload_barrier);

        VkBufferImageCopy copy{
            .bufferOffset      = 0,
            .bufferRowLength   = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                VkImageSubresourceLayers{.aspectMask =
                                             VK_IMAGE_ASPECT_COLOR_BIT,
                                         .mipLevel       = 0,
                                         .baseArrayLayer = 0,
                                         .layerCount     = 1},
            .imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
            .imageExtent = VkExtent3D{.width  = entry.second.extent.x,
                                      .height = entry.second.extent.y,
                                      .depth  = 1}};

        vkCmdCopyBufferToImage(cmd_buffer,
                               entry.second.staging_buffer.value().buffer,
                               entry.second.image.image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

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
            .subresourceRange =
                VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel   = 0,
                                        .levelCount     = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount     = 1}};

        vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0,
                             nullptr, 1, &post_upload_barrier);
      }
    }

    ASH_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext = nullptr,
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

  void execute_deletes()
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

    // stable iterator
    for (auto it = images.begin(); it != images.end(); it++)
    {
      if (it->second.needs_delete)
      {
        it->second.destroy(descriptor_pool);
      }
      images.erase(it);
    }

    ASH_LOG_INFO(Vulkan_RenderResourceManager, "Deleted pending images");
  }

  gfx::image upload_font_atlas(Font &font, ImageView<u8 const> atlas)
  {
    // VkImageFormatProperties image_format_properties;
    // ASH_VK_CHECK(vkGetPhysicalDeviceImageFormatProperties(queue.value()->device->phy_dev->phy_device,
    // VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
    // VK_IMAGE_USAGE_SAMPLED_BIT, 0, &image_format_properties)); auto [atlas,
    // image_buffer] = render_font_atlas(font, font_height,
    // extent{image_format_properties.maxExtent.width,
    // image_format_properties.maxExtent.height});
    ASH_LOG_INFO(Vulkan_RenderResourceManager,
                 "Uploading Atlas for Font: {} to GPU",
                 font.postscript_name.c_str());
    gfx::image image = add_image(atlas, false);
    ASH_LOG_INFO(Vulkan_RenderResourceManager,
                 "Uploaded Atlas For Font: {} to GPU",
                 font.postscript_name.c_str());
    return image;
  }
};

/// @brief NOTE: each pipeline is binded with 8 descriptor sets, each containing
/// an image sampler. buffers are passed via push constants with a limit of 128
/// bytes.
///
/// Rendering pipelines are rebuilt when the render target changes (i.e.
/// swapchain) Pipelines also have to be declared at program startup.
///
struct CanvasPipeline
{
  VkShaderModule        vertex_shader   = VK_NULL_HANDLE;
  VkShaderModule        fragment_shader = VK_NULL_HANDLE;
  stx::Option<Pipeline> pipeline;

  void destroy(VkDevice dev)
  {
    vkDestroyShaderModule(dev, vertex_shader, nullptr);
    vkDestroyShaderModule(dev, fragment_shader, nullptr);
    pipeline.match(&Pipeline::destroy, []() {});
  }
};

/// pipelines are static and can not be removed once added
struct CanvasPipelineManager
{
  std::map<std::string, CanvasPipeline, std::less<>> pipelines;
  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
  VkDevice              dev                   = VK_NULL_HANDLE;

  void init(VkDevice adev)
  {
    dev = adev;

    VkDescriptorSetLayoutBinding descriptor_set_bindings[] = {
        {.binding         = 0,
         .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .stageFlags =
             VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         .pImmutableSamplers = nullptr}};

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
        .flags        = 0,
        .bindingCount = std::size(descriptor_set_bindings),
        .pBindings    = descriptor_set_bindings};

    ASH_VK_CHECK(vkCreateDescriptorSetLayout(dev,
                                             &descriptor_set_layout_create_info,
                                             nullptr, &descriptor_set_layout));
  }

  void add_pipeline(CanvasPipelineSpec const &spec)
  {
    VkShaderModule           vertex_shader;
    VkShaderModuleCreateInfo vertex_shader_module_create_info{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .codeSize = spec.vertex_shader.size_bytes(),
        .pCode    = spec.vertex_shader.data()};
    ASH_VK_CHECK(vkCreateShaderModule(dev, &vertex_shader_module_create_info,
                                      nullptr, &vertex_shader));

    VkShaderModule           fragment_shader;
    VkShaderModuleCreateInfo fragment_shader_module_create_info{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext    = nullptr,
        .flags    = 0,
        .codeSize = spec.fragment_shader.size_bytes(),
        .pCode    = spec.fragment_shader.data()};
    ASH_VK_CHECK(vkCreateShaderModule(dev, &fragment_shader_module_create_info,
                                      nullptr, &fragment_shader));

    pipelines.emplace(spec.name,
                      CanvasPipeline{.vertex_shader   = vertex_shader,
                                     .fragment_shader = fragment_shader,
                                     .pipeline        = stx::None});
  }

  /// re-build pipelines to meet renderpass specification
  void rebuild_for_renderpass(VkRenderPass          target_render_pass,
                              VkSampleCountFlagBits msaa_sample_count)
  {
    static constexpr VkVertexInputAttributeDescription
        vertex_input_attributes[] = {
            {.location = 0,
             .binding  = 0,
             .format   = VK_FORMAT_R32G32_SFLOAT,
             .offset   = offsetof(gfx::Vertex2d, position)},
            {.location = 1,
             .binding  = 0,
             .format   = VK_FORMAT_R32G32_SFLOAT,
             .offset   = offsetof(gfx::Vertex2d, uv)},
            {.location = 2,
             .binding  = 0,
             .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
             .offset   = offsetof(gfx::Vertex2d, color)}};

    VkDescriptorSetLayout descriptor_sets_layout[NIMAGES_PER_DRAWCALL];
    stx::Span{descriptor_sets_layout}.fill(descriptor_set_layout);

    ASH_VK_CHECK(vkDeviceWaitIdle(dev));

    for (auto &p : pipelines)
    {
      ASH_LOG_INFO(Vulkan_CanvasPipelineManager, "Rebuilding Pipeline #{}",
                   p.first);
      p.second.pipeline.match(&Pipeline::destroy, []() {});
      p.second.pipeline = stx::None;

      Pipeline pipeline;

      pipeline.build(dev, p.second.vertex_shader, p.second.fragment_shader,
                     target_render_pass, msaa_sample_count,
                     descriptor_sets_layout, vertex_input_attributes,
                     sizeof(gfx::Vertex2d), PUSH_CONSTANT_SIZE);

      p.second.pipeline = stx::Some(std::move(pipeline));

      ASH_LOG_INFO(Vulkan_CanvasPipelineManager, "Rebuilt Pipeline #{}",
                   p.first);
    }
  }

  void destroy()
  {
    for (auto &p : pipelines)
    {
      p.second.destroy(dev);
    }
  }
};

}        // namespace vk
}        // namespace ash
