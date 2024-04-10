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


    // vkCmdResetQueryPool(cmd_buffer, pipeline_statistics_query_pools[frame], 0,
    //                     NPIPELINE_STATISTIC_QUERIES);
    // vkCmdResetQueryPool(cmd_buffer, pipeline_timestamp_query_pools[frame], 0,
    //                     NPIPELINE_TIMESTAMP_QUERIES);

    // vkCmdBeginQuery(cmd_buffer, pipeline_statistics_query_pools[frame], 0, 0);
    // vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        // pipeline_timestamp_query_pools[frame], 0);

    u32 first_index   = 0;
    u32 vertex_offset = 0;

    // vkCmdEndQuery(cmd_buffer, pipeline_statistics_query_pools[frame], 0);
    // vkCmdWriteTimestamp(cmd_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //                     pipeline_timestamp_query_pools[frame], 1);

  }
};

}        // namespace vk
}        // namespace ash
