#pragma once

#include <utility>

#include "ashura/canvas.h"
#include "ashura/primitives.h"
#include "ashura/shaders.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_context.h"

namespace ash
{
namespace vk
{

struct CanvasPushConstants
{
  mat4 transform;
};

struct CanvasRenderer
{
  u32                                  max_nframes_in_flight = 0;
  stx::Vec<VecBuffer>                  vertex_buffers;
  stx::Vec<VecBuffer>                  index_buffers;
  vk::Sampler                          texture_sampler;
  RecordingContext                     ctx;
  stx::Option<stx::Rc<CommandQueue *>> queue;

  void init(stx::Rc<CommandQueue *> aqueue, u32 amax_nframes_in_flight)
  {
    queue                 = stx::Some(std::move(aqueue));
    max_nframes_in_flight = amax_nframes_in_flight;

    VkVertexInputAttributeDescription vertex_input_attributes[] = {
        {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(vertex, position)},
        {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(vertex, uv)},
        {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(vertex, color)}};

    DescriptorSetSpec descriptor_set_specs[] = {DescriptorSetSpec{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}};

    // initial size of the descriptor pool, will grow as needed
    VkDescriptorPoolSize descriptor_pool_sizes[] = {{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1}};

    VkDevice dev = queue.value()->device->dev;

    ctx.init(dev, queue.value()->info.family.index, gfx::vertex_shader_code, gfx::fragment_shader_code, vertex_input_attributes,
             sizeof(vertex), sizeof(CanvasPushConstants), amax_nframes_in_flight, descriptor_set_specs,
             descriptor_pool_sizes, 1);

    VkPhysicalDeviceMemoryProperties const &memory_properties = queue.value()->device->phy_dev->memory_properties;

    for (u32 i = 0; i < amax_nframes_in_flight; i++)
    {
      VecBuffer vertex_buffer;

      vertex_buffer.init(dev, memory_properties, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

      vertex_buffers.push_inplace(vertex_buffer).unwrap();

      VecBuffer index_buffer;

      index_buffer.init(dev, memory_properties, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

      index_buffers.push_inplace(index_buffer).unwrap();
    }

    texture_sampler = vk::create_sampler(queue.value()->device, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_TRUE);
  }

  void destroy()
  {
    for (VecBuffer &buff : vertex_buffers)
    {
      buff.destroy();
    }

    for (VecBuffer &buff : index_buffers)
    {
      buff.destroy();
    }

    ctx.destroy();
  }

  void submit(VkExtent2D viewport_extent, VkExtent2D image_extent, u32 frame, VkFence render_fence,
              VkSemaphore image_acquisition_semaphore, VkSemaphore render_semaphore, VkRenderPass render_pass,
              VkFramebuffer framebuffer, stx::Span<gfx::DrawCommand const> cmds, stx::Span<vertex const> vertices,
              stx::Span<u32 const> indices, RenderResourceManager const &image_manager)
  {
    ASH_CHECK(frame < max_nframes_in_flight);

    stx::Rc<Device *> const &device = queue.value()->device;

    VkPhysicalDeviceMemoryProperties const &memory_properties = queue.value()->device->phy_dev->memory_properties;

    VkDevice dev = device->dev;

    VkQueue queue = this->queue.value()->info.queue;

    VkCommandBuffer cmd_buffer = ctx.cmd_buffers[frame];

    vertex_buffers[frame].write(memory_properties, vertices.as_u8());

    index_buffers[frame].write(memory_properties, indices.as_u8());

    u32 nallocated_descriptor_sets = AS(u32, ctx.descriptor_sets[frame].size());

    u32 ndraw_calls = AS(u32, cmds.size());

    u32 ndescriptor_sets_per_draw_call = AS(u32, ctx.descriptor_set_layouts.size());

    u32 nrequired_descriptor_sets = ndescriptor_sets_per_draw_call * ndraw_calls;

    u32 max_ndescriptor_sets = ctx.descriptor_pool_infos[frame].max_sets;

    if (ndescriptor_sets_per_draw_call > 0)
    {
      if (nrequired_descriptor_sets > nallocated_descriptor_sets)
      {
        u32 nallocatable_combined_image_samplers = 0;

        for (VkDescriptorPoolSize size : ctx.descriptor_pool_infos[frame].sizes)
        {
          if (size.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
          {
            nallocatable_combined_image_samplers = size.descriptorCount;
            break;
          }
        }

        if (nrequired_descriptor_sets > max_ndescriptor_sets || nrequired_descriptor_sets > nallocatable_combined_image_samplers)
        {
          ASH_VK_CHECK(vkDeviceWaitIdle(dev));
          if (!ctx.descriptor_sets[frame].is_empty())
          {
            ASH_VK_CHECK(vkFreeDescriptorSets(dev, ctx.descriptor_pools[frame], AS(u32, ctx.descriptor_sets[frame].size()), ctx.descriptor_sets[frame].data()));
          }

          vkDestroyDescriptorPool(dev, ctx.descriptor_pools[frame], nullptr);

          stx::Vec<VkDescriptorPoolSize> sizes;

          sizes.push({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = nrequired_descriptor_sets}).unwrap();

          VkDescriptorPoolCreateInfo descriptor_pool_create_info{.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                                 .pNext         = nullptr,
                                                                 .flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                                                                 .maxSets       = nrequired_descriptor_sets,
                                                                 .poolSizeCount = AS(u32, sizes.size()),
                                                                 .pPoolSizes    = sizes.data()};

          ASH_VK_CHECK(vkCreateDescriptorPool(dev, &descriptor_pool_create_info, nullptr, &ctx.descriptor_pools[frame]));

          ctx.descriptor_pool_infos[frame] = DescriptorPoolInfo{.sizes = std::move(sizes), .max_sets = nrequired_descriptor_sets};

          ctx.descriptor_sets[frame].resize(nrequired_descriptor_sets).unwrap();

          for (u32 i = 0; i < ndraw_calls; i++)
          {
            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                                     .pNext              = nullptr,
                                                                     .descriptorPool     = ctx.descriptor_pools[frame],
                                                                     .descriptorSetCount = AS(u32, ctx.descriptor_set_layouts.size()),
                                                                     .pSetLayouts        = ctx.descriptor_set_layouts.data()};

            ASH_VK_CHECK(vkAllocateDescriptorSets(dev, &descriptor_set_allocate_info, ctx.descriptor_sets[frame].data() + i * ndescriptor_sets_per_draw_call));
          }
        }
        else
        {
          ctx.descriptor_sets[frame].resize(nrequired_descriptor_sets).unwrap();

          VkDescriptorSetAllocateInfo descriptor_set_allocate_info{.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                                                   .pNext              = nullptr,
                                                                   .descriptorPool     = ctx.descriptor_pools[frame],
                                                                   .descriptorSetCount = AS(u32, ctx.descriptor_set_layouts.size()),
                                                                   .pSetLayouts        = ctx.descriptor_set_layouts.data()};

          for (u32 i = nallocated_descriptor_sets / ndescriptor_sets_per_draw_call; i < nrequired_descriptor_sets / ndescriptor_sets_per_draw_call; i++)
          {
            ASH_VK_CHECK(vkAllocateDescriptorSets(dev, &descriptor_set_allocate_info, ctx.descriptor_sets[frame].data() + i * ndescriptor_sets_per_draw_call));
          }
        }
      }
    }

    ASH_VK_CHECK(vkWaitForFences(dev, 1, &render_fence, VK_TRUE, VULKAN_TIMEOUT));

    ASH_VK_CHECK(vkResetFences(dev, 1, &render_fence));

    ASH_VK_CHECK(vkResetCommandBuffer(cmd_buffer, 0));

    VkCommandBufferBeginInfo command_buffer_begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    ASH_VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &command_buffer_begin_info));

    VkClearValue clear_values[] = {{.color = VkClearColorValue{{0, 0, 0, 0}}}, {.depthStencil = VkClearDepthStencilValue{.depth = 1, .stencil = 0}}};

    VkRenderPassBeginInfo render_pass_begin_info{.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                                 .pNext           = nullptr,
                                                 .renderPass      = render_pass,
                                                 .framebuffer     = framebuffer,
                                                 .renderArea      = VkRect2D{.offset = VkOffset2D{0, 0}, .extent = image_extent},
                                                 .clearValueCount = AS(u32, std::size(clear_values)),
                                                 .pClearValues    = clear_values};

    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    for (usize icmd = 0; icmd < cmds.size(); icmd++)
    {
      auto pos = image_manager.images.find(cmds[icmd].texture);
      ASH_CHECK(pos != image_manager.images.end());

      VkDescriptorImageInfo image_info{.sampler = texture_sampler.sampler, .imageView = pos->second.image.view, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

      VkWriteDescriptorSet write{.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                 .pNext            = nullptr,
                                 .dstSet           = ctx.descriptor_sets[frame][icmd],
                                 .dstBinding       = 0,
                                 .dstArrayElement  = 0,
                                 .descriptorCount  = 1,
                                 .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                 .pImageInfo       = &image_info,
                                 .pBufferInfo      = nullptr,
                                 .pTexelBufferView = nullptr};

      vkUpdateDescriptorSets(dev, 1, &write, 0, nullptr);
    }

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline.pipeline);

    VkDeviceSize vertices_offset = 0;
    VkDeviceSize indices_offset  = 0;

    for (usize icmd = 0; icmd < cmds.size(); icmd++)
    {
      gfx::DrawCommand const &cmd = cmds[icmd];

      vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertex_buffers[frame].buffer, &vertices_offset);

      vkCmdBindIndexBuffer(cmd_buffer, index_buffers[frame].buffer, indices_offset, VK_INDEX_TYPE_UINT32);

      VkViewport viewport{.x        = 0,
                          .y        = 0,
                          .width    = AS(f32, viewport_extent.width),
                          .height   = AS(f32, viewport_extent.height),
                          .minDepth = 0,
                          .maxDepth = 1};

      vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

      VkRect2D scissor{.offset = VkOffset2D{AS(i32, cmd.clip_rect.offset.x), AS(i32, cmd.clip_rect.offset.y)},
                       .extent = VkExtent2D{AS(u32, cmd.clip_rect.extent.x), AS(u32, cmd.clip_rect.extent.y)}};

      vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

      CanvasPushConstants push_constants{.transform = cmd.transform.transpose()};

      vkCmdPushConstants(cmd_buffer, ctx.pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(CanvasPushConstants), &push_constants);

      vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline.layout, 0, ndescriptor_sets_per_draw_call,
                              &ctx.descriptor_sets[frame][icmd * ndescriptor_sets_per_draw_call], 0, nullptr);

      vkCmdDrawIndexed(cmd_buffer, cmd.nindices, 1, 0, 0, 0);

      vertices_offset += cmd.nvertices * sizeof(vertex);
      indices_offset += cmd.nindices * sizeof(u32);
    }

    vkCmdEndRenderPass(cmd_buffer);

    ASH_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext                = nullptr,
                             .waitSemaphoreCount   = 1,
                             .pWaitSemaphores      = &image_acquisition_semaphore,
                             .pWaitDstStageMask    = &wait_stage,
                             .commandBufferCount   = 1,
                             .pCommandBuffers      = &cmd_buffer,
                             .signalSemaphoreCount = 1,
                             .pSignalSemaphores    = &render_semaphore};

    ASH_VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, render_fence));
  }
};

}        // namespace vk
}        // namespace ash
