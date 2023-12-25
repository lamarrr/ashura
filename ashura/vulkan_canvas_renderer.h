#pragma once

#include <utility>

#include "ashura/canvas.h"
#include "ashura/primitives.h"
#include "ashura/shaders.h"
#include "ashura/stats.h"
#include "ashura/utils.h"
#include "ashura/vulkan.h"
#include "ashura/vulkan_context.h"

namespace ash
{
namespace vk
{

struct CanvasRenderer
{
  static constexpr VkQueryPipelineStatisticFlags PIPELINE_STATISTIC_QUERIES =
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
      VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
      VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
  static constexpr u32             NPIPELINE_STATISTIC_QUERIES = 7;
  static constexpr u32             NPIPELINE_TIMESTAMP_QUERIES = 2;
  u32                              max_nframes_in_flight       = 0;
  stx::Vec<VecBuffer>              vertex_buffers;
  stx::Vec<VecBuffer>              index_buffers;
  VkCommandPool                    cmd_pool = VK_NULL_HANDLE;
  stx::Vec<VkCommandBuffer>        cmd_buffers;
  stx::Vec<VkQueryPool>            pipeline_statistics_query_pools;
  stx::Vec<VkQueryPool>            pipeline_timestamp_query_pools;
  VkPhysicalDeviceMemoryProperties memory_properties;
  f32                              timestamp_period   = 1;
  u32                              queue_family_index = 0;
  VkQueue                          queue              = VK_NULL_HANDLE;
  VkDevice                         dev                = VK_NULL_HANDLE;

  void init(VkDevice adev, VkQueue aqueue, u32 aqueue_family_index,
            f32                                     atimestamp_period,
            VkPhysicalDeviceMemoryProperties const &amemory_properties,
            u32                                     amax_nframes_in_flight)
  {
    max_nframes_in_flight = amax_nframes_in_flight;
    memory_properties     = amemory_properties;
    queue_family_index    = aqueue_family_index;
    queue                 = aqueue;
    dev                   = adev;
    timestamp_period      = atimestamp_period;

    for (u32 i = 0; i < max_nframes_in_flight; i++)
    {
      VecBuffer vertex_buffer;

      vertex_buffer.init(dev, memory_properties,
                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

      vertex_buffers.push_inplace(vertex_buffer).unwrap();

      VecBuffer index_buffer;

      index_buffer.init(dev, memory_properties,
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

      index_buffers.push_inplace(index_buffer).unwrap();
    }

    VkCommandPoolCreateInfo cmd_pool_create_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_index};

    ASH_VK_CHECK(
        vkCreateCommandPool(dev, &cmd_pool_create_info, nullptr, &cmd_pool));

    cmd_buffers.unsafe_resize_uninitialized(max_nframes_in_flight).unwrap();

    VkCommandBufferAllocateInfo cmd_buffers_allocate_info{
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext              = nullptr,
        .commandPool        = cmd_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<u32>(max_nframes_in_flight)};

    ASH_VK_CHECK(vkAllocateCommandBuffers(dev, &cmd_buffers_allocate_info,
                                          cmd_buffers.data()));

    VkQueryPoolCreateInfo pipeline_statistics_query_pool_create_info{
        .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .queryType          = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        .queryCount         = NPIPELINE_STATISTIC_QUERIES,
        .pipelineStatistics = PIPELINE_STATISTIC_QUERIES};

    VkQueryPoolCreateInfo timestamp_query_pool_create_info{
        .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .queryType          = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount         = NPIPELINE_TIMESTAMP_QUERIES,
        .pipelineStatistics = 0};

    for (u32 i = 0; i < max_nframes_in_flight; i++)
    {
      VkQueryPool pipeline_statistics_query_pool;
      ASH_VK_CHECK(
          vkCreateQueryPool(dev, &pipeline_statistics_query_pool_create_info,
                            nullptr, &pipeline_statistics_query_pool));
      pipeline_statistics_query_pools
          .push_inplace(pipeline_statistics_query_pool)
          .unwrap();

      VkQueryPool timestamp_query_pool;
      ASH_VK_CHECK(vkCreateQueryPool(dev, &timestamp_query_pool_create_info,
                                     nullptr, &timestamp_query_pool));
      pipeline_timestamp_query_pools.push_inplace(timestamp_query_pool)
          .unwrap();
    }
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

    for (VkQueryPool query_pool : pipeline_statistics_query_pools)
    {
      vkDestroyQueryPool(dev, query_pool, nullptr);
    }

    for (VkQueryPool query_pool : pipeline_timestamp_query_pools)
    {
      vkDestroyQueryPool(dev, query_pool, nullptr);
    }

    vkFreeCommandBuffers(dev, cmd_pool, static_cast<u32>(max_nframes_in_flight),
                         cmd_buffers.data());

    vkDestroyCommandPool(dev, cmd_pool, nullptr);
  }

  void submit(VkExtent2D viewport_extent, VkExtent2D image_extent, u32 frame,
              VkFence render_fence, VkSemaphore image_acquisition_semaphore,
              VkSemaphore render_semaphore, VkRenderPass render_pass,
              VkFramebuffer framebuffer, stx::Span<gfx::DrawCommand const> cmds,
              stx::Span<gfx::Vertex2d const> vertices,
              stx::Span<u32 const>           indices,
              CanvasPipelineManager const   &pipeline_manager,
              RenderResourceManager const   &image_manager,
              FrameStats                    &frame_stats)
  {
    ASH_CHECK(frame < max_nframes_in_flight);

    VkCommandBuffer cmd_buffer = cmd_buffers[frame];

    vertex_buffers[frame].write(memory_properties, vertices.as_u8());

    index_buffers[frame].write(memory_properties, indices.as_u8());

    Timepoint gpu_sync_begin = Clock::now();

    ASH_VK_CHECK(
        vkWaitForFences(dev, 1, &render_fence, VK_TRUE, VULKAN_TIMEOUT));

    Timepoint gpu_sync_end = Clock::now();

    frame_stats.gpu_sync_time = gpu_sync_end - gpu_sync_begin;

    ASH_VK_CHECK(vkResetFences(dev, 1, &render_fence));

    // u64 pipeline_statistics_query_query_results[NPIPELINE_STATISTIC_QUERIES];
    // u64 pipeline_timestamp_query_results[NPIPELINE_TIMESTAMP_QUERIES];

    // if (vkGetQueryPoolResults(dev, pipeline_statistics_query_pools[frame], 0, 1,
    //                           sizeof(pipeline_statistics_query_query_results),
    //                           pipeline_statistics_query_query_results,
    //                           sizeof(u64),
    //                           VK_QUERY_RESULT_64_BIT) == VK_SUCCESS)
    // {
    //   frame_stats.input_assembly_vertices =
    //       pipeline_statistics_query_query_results[0];
    //   frame_stats.input_assembly_primitives =
    //       pipeline_statistics_query_query_results[1];
    //   frame_stats.vertex_shader_invocations =
    //       pipeline_statistics_query_query_results[2];
    //   frame_stats.fragment_shader_invocations =
    //       pipeline_statistics_query_query_results[3];
    //   frame_stats.compute_shader_invocations =
    //       pipeline_statistics_query_query_results[4];
    //   frame_stats.task_shader_invocations =
    //       pipeline_statistics_query_query_results[5];
    //   frame_stats.mesh_shader_invocations =
    //       pipeline_statistics_query_query_results[6];
    // }

    // if (vkGetQueryPoolResults(dev, pipeline_timestamp_query_pools[frame], 0,
    //                           NPIPELINE_TIMESTAMP_QUERIES,
    //                           sizeof(pipeline_timestamp_query_results),
    //                           pipeline_timestamp_query_results, sizeof(u64),
    //                           VK_QUERY_RESULT_64_BIT) == VK_SUCCESS)
    // {
    //   frame_stats.gpu_time = Nanoseconds{
    //       (Nanoseconds::rep)(((f64) timestamp_period) *
    //                          (f64) (pipeline_timestamp_query_results[1] -
    //                                 pipeline_timestamp_query_results[0]))};
    // }

    ASH_VK_CHECK(vkResetCommandBuffer(cmd_buffer, 0));

    VkCommandBufferBeginInfo command_buffer_begin_info{
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = nullptr,
        .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    ASH_VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &command_buffer_begin_info));

    // vkCmdResetQueryPool(cmd_buffer, pipeline_statistics_query_pools[frame], 0,
    //                     NPIPELINE_STATISTIC_QUERIES);
    // vkCmdResetQueryPool(cmd_buffer, pipeline_timestamp_query_pools[frame], 0,
    //                     NPIPELINE_TIMESTAMP_QUERIES);

    // vkCmdBeginQuery(cmd_buffer, pipeline_statistics_query_pools[frame], 0, 0);
    // vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        // pipeline_timestamp_query_pools[frame], 0);

    VkClearValue clear_values[] = {
        {.color = VkClearColorValue{{0, 0, 0, 0}}},
        {.depthStencil = VkClearDepthStencilValue{.depth = 1, .stencil = 0}}};

    VkRenderPassBeginInfo render_pass_begin_info{
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext       = nullptr,
        .renderPass  = render_pass,
        .framebuffer = framebuffer,
        .renderArea =
            VkRect2D{.offset = VkOffset2D{0, 0}, .extent = image_extent},
        .clearValueCount = static_cast<u32>(std::size(clear_values)),
        .pClearValues    = clear_values};

    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkPipeline      previous_pipeline = VK_NULL_HANDLE;
    VkDescriptorSet previous_descriptor_sets[NIMAGES_PER_DRAWCALL] = {};
    VkViewport      previouse_viewport{};
    VkRect2D        previouse_scissor{};

    VkDeviceSize const vertices_offsets[] = {0};

    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertex_buffers[frame].buffer,
                           vertices_offsets);
    vkCmdBindIndexBuffer(cmd_buffer, index_buffers[frame].buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    u32 first_index   = 0;
    u32 vertex_offset = 0;

    for (usize icmd = 0; icmd < cmds.size(); icmd++)
    {
      gfx::DrawCommand const &cmd = cmds[icmd];

      if (cmd.scissor_offset.x < 0 || cmd.scissor_offset.y < 0)
      {
        continue;
      }

      auto pipeline_it = pipeline_manager.pipelines.find(cmd.pipeline);
      ASH_CHECK(pipeline_it != pipeline_manager.pipelines.end());
      ASH_CHECK(pipeline_it->second.pipeline.is_some());

      Pipeline const &pipeline = pipeline_it->second.pipeline.value();

      if (pipeline.pipeline != previous_pipeline)
      {
        vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline.pipeline);
      }

      VkViewport viewport{.x        = 0,
                          .y        = 0,
                          .width    = static_cast<f32>(viewport_extent.width),
                          .height   = static_cast<f32>(viewport_extent.height),
                          .minDepth = 0,
                          .maxDepth = 1};

      if (memcmp(&viewport, &previouse_viewport, sizeof(VkViewport)) != 0)
      {
        vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
      }

      VkRect2D scissor{
          .offset = VkOffset2D{static_cast<i32>(cmd.scissor_offset.x),
                               static_cast<i32>(cmd.scissor_offset.y)},
          .extent = VkExtent2D{static_cast<u32>(cmd.scissor_extent.x),
                               static_cast<u32>(cmd.scissor_extent.y)}};

      if (memcmp(&scissor, &previouse_scissor, sizeof(VkRect2D)) != 0)
      {
        vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
      }

      vkCmdPushConstants(cmd_buffer, pipeline.layout,
                         VK_SHADER_STAGE_VERTEX_BIT |
                             VK_SHADER_STAGE_FRAGMENT_BIT,
                         0, PUSH_CONSTANT_SIZE, cmd.push_constant);

      VkDescriptorSet descriptor_sets[NIMAGES_PER_DRAWCALL];

      for (u32 i = 0; i < NIMAGES_PER_DRAWCALL; i++)
      {
        auto image_pos = image_manager.images.find(cmd.textures[i]);
        ASH_CHECK(image_pos != image_manager.images.end());
        descriptor_sets[i] = image_pos->second.descriptor_set;
      }

      if (!stx::Span{descriptor_sets}.equals(
              stx::Span{previous_descriptor_sets}))
      {
        vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline.layout, 0, NIMAGES_PER_DRAWCALL,
                                descriptor_sets, 0, nullptr);
      }

      vkCmdDrawIndexed(cmd_buffer, cmd.nindices, cmd.ninstances, first_index,
                       (i32) vertex_offset, cmd.first_instance);

      first_index += cmd.nindices;
      vertex_offset += cmd.nvertices;

      previous_pipeline  = pipeline.pipeline;
      previouse_viewport = viewport;
      previouse_scissor  = scissor;
      stx::Span{previous_descriptor_sets}.copy(descriptor_sets);
    }

    vkCmdEndRenderPass(cmd_buffer);

    // vkCmdEndQuery(cmd_buffer, pipeline_statistics_query_pools[frame], 0);
    // vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //                     pipeline_timestamp_query_pools[frame], 1);

    ASH_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext = nullptr,
                             .waitSemaphoreCount = 1,
                             .pWaitSemaphores    = &image_acquisition_semaphore,
                             .pWaitDstStageMask  = &wait_stage,
                             .commandBufferCount = 1,
                             .pCommandBuffers    = &cmd_buffer,
                             .signalSemaphoreCount = 1,
                             .pSignalSemaphores    = &render_semaphore};

    ASH_VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, render_fence));
  }
};

}        // namespace vk
}        // namespace ash
