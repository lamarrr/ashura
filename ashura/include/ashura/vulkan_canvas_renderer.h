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

// Command dispaTCHER???
struct CanvasRenderer
{
  u32                              max_nframes_in_flight = 0;
  stx::Vec<VecBuffer>              vertex_buffers;
  stx::Vec<VecBuffer>              index_buffers;
  VkCommandPool                    cmd_pool = VK_NULL_HANDLE;
  stx::Vec<VkCommandBuffer>        cmd_buffers;
  VkPhysicalDeviceMemoryProperties memory_properties;
  u32                              queue_family_index = 0;
  VkQueue                          queue              = VK_NULL_HANDLE;
  VkDevice                         dev                = VK_NULL_HANDLE;

  void init(VkDevice adev, VkQueue aqueue, u32 aqueue_family_index, VkPhysicalDeviceMemoryProperties const &amemory_properties, u32 amax_nframes_in_flight)
  {
    max_nframes_in_flight = amax_nframes_in_flight;
    memory_properties     = amemory_properties;
    queue_family_index    = aqueue_family_index;
    queue                 = aqueue;
    dev                   = adev;

    for (u32 i = 0; i < max_nframes_in_flight; i++)
    {
      VecBuffer vertex_buffer;

      vertex_buffer.init(dev, memory_properties, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

      vertex_buffers.push_inplace(vertex_buffer).unwrap();

      VecBuffer index_buffer;

      index_buffer.init(dev, memory_properties, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

      index_buffers.push_inplace(index_buffer).unwrap();
    }

    VkCommandPoolCreateInfo cmd_pool_create_info{.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                                 .pNext            = nullptr,
                                                 .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                 .queueFamilyIndex = queue_family_index};

    ASH_VK_CHECK(vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    cmd_buffers.resize(max_nframes_in_flight).unwrap();

    VkCommandBufferAllocateInfo cmd_buffers_allocate_info{.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                          .pNext              = nullptr,
                                                          .commandPool        = cmd_pool,
                                                          .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                          .commandBufferCount = AS(u32, max_nframes_in_flight)};

    ASH_VK_CHECK(vkAllocateCommandBuffers(dev, &cmd_buffers_allocate_info, cmd_buffers.data()));
  }

  void destroy()
  {
    ASH_VK_CHECK(vkDeviceWaitIdle(dev));
    for (VecBuffer &buff : vertex_buffers)
    {
      buff.destroy();
    }

    for (VecBuffer &buff : index_buffers)
    {
      buff.destroy();
    }

    vkFreeCommandBuffers(dev, cmd_pool, AS(u32, max_nframes_in_flight), cmd_buffers.data());

    vkDestroyCommandPool(dev, cmd_pool, nullptr);
  }

  void submit(VkExtent2D                        viewport_extent,
              VkExtent2D                        image_extent,
              u32                               frame,
              VkFence                           render_fence,
              VkSemaphore                       image_acquisition_semaphore,
              VkSemaphore                       render_semaphore,
              VkRenderPass                      render_pass,
              VkFramebuffer                     framebuffer,
              stx::Span<gfx::DrawCommand const> cmds,
              stx::Span<vertex const>           vertices,
              stx::Span<u32 const>              indices,
              CanvasPipelineManager const      &pipeline_manager,
              RenderResourceManager const      &image_manager)
  {
    ASH_CHECK(frame < max_nframes_in_flight);

    VkCommandBuffer cmd_buffer = cmd_buffers[frame];

    vertex_buffers[frame].write(memory_properties, vertices.as_u8());

    index_buffers[frame].write(memory_properties, indices.as_u8());

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

    VkPipeline      previous_pipeline                              = VK_NULL_HANDLE;
    VkDescriptorSet previous_descriptor_sets[NIMAGES_PER_DRAWCALL] = {};

    VkDeviceSize const vertices_offsets[] = {0};

    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertex_buffers[frame].buffer, vertices_offsets);
    vkCmdBindIndexBuffer(cmd_buffer, index_buffers[frame].buffer, 0, VK_INDEX_TYPE_UINT32);

    u32 first_index   = 0;
    u32 vertex_offset = 0;

    for (usize icmd = 0; icmd < cmds.size(); icmd++)
    {
      gfx::DrawCommand const &cmd = cmds[icmd];

      if (cmd.scissor.offset.x < 0 || cmd.scissor.offset.y < 0)
      {
        continue;
      }

      auto itPipeline = pipeline_manager.pipelines.find(cmd.pipeline);
      ASH_CHECK(itPipeline != pipeline_manager.pipelines.end());
      ASH_CHECK(itPipeline->second.pipeline.is_some());

      Pipeline pipeline = itPipeline->second.pipeline.value();

      if (pipeline.pipeline != previous_pipeline)
      {
        vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
      }

      VkViewport viewport{.x        = 0,
                          .y        = 0,
                          .width    = AS(f32, viewport_extent.width),
                          .height   = AS(f32, viewport_extent.height),
                          .minDepth = 0,
                          .maxDepth = 1};

      vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

      VkRect2D scissor{.offset = VkOffset2D{AS(i32, cmd.scissor.offset.x), AS(i32, cmd.scissor.offset.y)},
                       .extent = VkExtent2D{AS(u32, cmd.scissor.extent.x), AS(u32, cmd.scissor.extent.y)}};

      vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

      vkCmdPushConstants(cmd_buffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, PUSH_CONSTANT_SIZE, cmd.push_constant);

      VkDescriptorSet descriptor_sets[NIMAGES_PER_DRAWCALL];

      for (u32 i = 0; i < NIMAGES_PER_DRAWCALL; i++)
      {
        auto image_pos = image_manager.images.find(cmd.textures[i]);
        ASH_CHECK(image_pos != image_manager.images.end());
        descriptor_sets[i] = image_pos->second.descriptor_set;
      }

      if (!stx::Span{descriptor_sets}.equals(stx::Span{previous_descriptor_sets}))
      {
        vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, NIMAGES_PER_DRAWCALL, descriptor_sets, 0, nullptr);
      }

      vkCmdDrawIndexed(cmd_buffer, cmd.nindices, cmd.ninstances, first_index, 0, cmd.first_instance);

      first_index += cmd.nindices;
      vertex_offset += cmd.nindices;

      previous_pipeline = pipeline.pipeline;
      stx::Span{previous_descriptor_sets}.copy(descriptor_sets);
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
