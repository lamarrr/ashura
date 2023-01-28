#pragma once

#include "ashura/canvas.h"
#include "ashura/primitives.h"
#include "ashura/shaders.h"
#include "ashura/vulkan.h"

namespace asr {
namespace vk {

// TODO(lamarrr): break this down and make it suitable for offscreen rendering
//
//
struct CanvasRenderer {
  stx::Vec<vk::SpanBuffer> vertex_buffers{stx::os_allocator};
  stx::Vec<vk::SpanBuffer> index_buffers{stx::os_allocator};

  vk::RecordingContext ctx;

  stx::Rc<vk::CommandQueue*> queue;

  explicit CanvasRenderer(stx::Rc<vk::CommandQueue*> aqueue)
      : queue{std::move(aqueue)} {
    VkVertexInputAttributeDescription vertex_input_attributes[] = {
        {.location = 0,
         .binding = 0,
         .format = VK_FORMAT_R32G32_SFLOAT,
         .offset = offsetof(vertex, position)},
        {.location = 1,
         .binding = 0,
         .format = VK_FORMAT_R32G32_SFLOAT,
         .offset = offsetof(vertex, st)},
        {.location = 2,
         .binding = 0,
         .format = VK_FORMAT_R32G32B32A32_SFLOAT,
         .offset = offsetof(vertex, color)}};

    vk::DescriptorSetSpec descriptor_set_specs[] = {
        vk::DescriptorSetSpec{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}};

    // initial size of the descriptor pool, will grow as needed
    VkDescriptorPoolSize descriptor_pool_sizes[] = {
        {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1}};

    ctx.init(queue.share(), gfx::vertex_shader_code, gfx::fragment_shader_code,
             vertex_input_attributes, sizeof(vertex),
             sizeof(gfx::CanvasPushConstants), descriptor_set_specs,
             descriptor_pool_sizes, 1);

    for (u32 i = 0; i < vk::SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
      vertex_buffers.push(vk::SpanBuffer{}).unwrap();
      index_buffers.push(vk::SpanBuffer{}).unwrap();
    }
  }

  STX_MAKE_PINNED(CanvasRenderer)

  ~CanvasRenderer() {
    VkDevice dev = queue->device->device;

    for (vk::SpanBuffer& buff : vertex_buffers) buff.destroy(dev);

    for (vk::SpanBuffer& buff : index_buffers) buff.destroy(dev);

    ctx.destroy();
  }

  void __write_vertices(stx::Span<vertex const> vertices,
                        stx::Span<u32 const> indices,
                        u32 next_frame_flight_index) {
    VkDevice dev = queue->device->device;
    VkPhysicalDeviceMemoryProperties const& memory_properties =
        queue->device->phy_device->memory_properties;

    vertex_buffers[next_frame_flight_index].write(
        dev, memory_properties, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices);

    index_buffers[next_frame_flight_index].write(
        dev, memory_properties, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);
  }

  void submit(vk::SwapChain const& swapchain, u32 swapchain_image_index,
              gfx::DrawList const& draw_list,
              AssetBundle<stx::Rc<vk::ImageSampler*>> const& bundle) {
    stx::Rc<vk::Device*> const& device = swapchain.queue->device;

    VkDevice dev = device->device;

    VkQueue queue = swapchain.queue->info.queue;

    u32 frame = swapchain.next_frame_flight_index;

    VkCommandBuffer cmd_buffer = ctx.draw_cmd_buffers[frame];

    __write_vertices(draw_list.vertices, draw_list.indices, frame);

    u32 nallocated_descriptor_sets = AS_U32(ctx.descriptor_sets[frame].size());

    u32 ndraw_calls = AS_U32(draw_list.cmds.size());

    u32 ndescriptor_sets_per_draw_call =
        AS_U32(ctx.descriptor_set_layouts.size());

    u32 nrequired_descriptor_sets =
        ndescriptor_sets_per_draw_call * ndraw_calls;

    u32 max_ndescriptor_sets = ctx.descriptor_pool_infos[frame].max_sets;

    if (ndescriptor_sets_per_draw_call > 0) {
      if (nrequired_descriptor_sets > nallocated_descriptor_sets) {
        u32 nallocatable_combined_image_samplers = 0;

        for (VkDescriptorPoolSize size :
             ctx.descriptor_pool_infos[frame].sizes) {
          if (size.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            nallocatable_combined_image_samplers = size.descriptorCount;
            break;
          }
        }

        if (nrequired_descriptor_sets > max_ndescriptor_sets ||
            nrequired_descriptor_sets > nallocatable_combined_image_samplers) {
          if (!ctx.descriptor_sets[frame].is_empty()) {
            ASR_VK_CHECK(
                vkFreeDescriptorSets(dev, ctx.descriptor_pools[frame],
                                     AS_U32(ctx.descriptor_sets[frame].size()),
                                     ctx.descriptor_sets[frame].data()));
          }

          vkDestroyDescriptorPool(dev, ctx.descriptor_pools[frame], nullptr);

          stx::Vec<VkDescriptorPoolSize> sizes{stx::os_allocator};

          sizes
              .push({.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                     .descriptorCount = nrequired_descriptor_sets})
              .unwrap();

          VkDescriptorPoolCreateInfo descriptor_pool_create_info{
              .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
              .pNext = nullptr,
              .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
              .maxSets = nrequired_descriptor_sets,
              .poolSizeCount = AS_U32(sizes.size()),
              .pPoolSizes = sizes.data()};

          ASR_VK_CHECK(vkCreateDescriptorPool(dev, &descriptor_pool_create_info,
                                              nullptr,
                                              &ctx.descriptor_pools[frame]));

          ctx.descriptor_pool_infos[frame] = vk::DescriptorPoolInfo{
              .sizes = std::move(sizes), .max_sets = nrequired_descriptor_sets};

          ctx.descriptor_sets[frame].resize(nrequired_descriptor_sets).unwrap();

          for (u32 i = 0; i < ndraw_calls; i++) {
            VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = ctx.descriptor_pools[frame],
                .descriptorSetCount = AS_U32(ctx.descriptor_set_layouts.size()),
                .pSetLayouts = ctx.descriptor_set_layouts.data()};

            ASR_VK_CHECK(vkAllocateDescriptorSets(
                dev, &descriptor_set_allocate_info,
                ctx.descriptor_sets[frame].data() +
                    i * ndescriptor_sets_per_draw_call));
          }
        } else {
          ctx.descriptor_sets[frame].resize(nrequired_descriptor_sets).unwrap();

          VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
              .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
              .pNext = nullptr,
              .descriptorPool = ctx.descriptor_pools[frame],
              .descriptorSetCount = AS_U32(ctx.descriptor_set_layouts.size()),
              .pSetLayouts = ctx.descriptor_set_layouts.data()};

          for (u32 i =
                   nallocated_descriptor_sets / ndescriptor_sets_per_draw_call;
               i < nrequired_descriptor_sets / ndescriptor_sets_per_draw_call;
               i++) {
            ASR_VK_CHECK(vkAllocateDescriptorSets(
                dev, &descriptor_set_allocate_info,
                ctx.descriptor_sets[frame].data() +
                    i * ndescriptor_sets_per_draw_call));
          }
        }
      }
    }

    ASR_VK_CHECK(vkWaitForFences(dev, 1, &swapchain.rendering_fences[frame],
                                 VK_TRUE, COMMAND_TIMEOUT));

    ASR_VK_CHECK(vkResetFences(dev, 1, &swapchain.rendering_fences[frame]));

    ASR_VK_CHECK(vkResetCommandBuffer(cmd_buffer, 0));

    VkCommandBufferBeginInfo command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    ASR_VK_CHECK(vkBeginCommandBuffer(cmd_buffer, &command_buffer_begin_info));

    VkClearValue clear_values[] = {
        {.color = VkClearColorValue{{0, 0, 0, 0}}},
        {.depthStencil = VkClearDepthStencilValue{.depth = 1, .stencil = 0}}};

    VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = swapchain.render_pass,
        .framebuffer = swapchain.framebuffers[swapchain_image_index],
        .renderArea =
            VkRect2D{.offset = {0, 0}, .extent = swapchain.image_extent},
        .clearValueCount = AS_U32(std::size(clear_values)),
        .pClearValues = clear_values};

    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    VkDeviceSize offset = 0;

    for (usize icmd = 0; icmd < draw_list.cmds.size(); icmd++) {
      vk::ImageSampler const& sampler =
          *bundle.get(draw_list.cmds[icmd].texture).unwrap()->handle;

      VkDescriptorImageInfo image_info{
          .sampler = sampler.sampler,
          .imageView = sampler.image.handle->view,
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

      VkWriteDescriptorSet write{
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .pNext = nullptr,
          .dstSet = ctx.descriptor_sets[frame][icmd],
          .dstBinding = 0,
          .dstArrayElement = 0,
          .descriptorCount = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .pImageInfo = &image_info,
          .pBufferInfo = nullptr,
          .pTexelBufferView = nullptr};

      vkUpdateDescriptorSets(dev, 1, &write, 0, nullptr);
    }

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      ctx.pipeline.pipeline);

    ASR_CHECK(vertex_buffers[frame].is_valid());
    ASR_CHECK(index_buffers[frame].is_valid());

    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vertex_buffers[frame].buffer,
                           &offset);

    vkCmdBindIndexBuffer(cmd_buffer, index_buffers[frame].buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    for (usize icmd = 0; icmd < draw_list.cmds.size(); icmd++) {
      gfx::DrawCommand const& cmd = draw_list.cmds[icmd];

      VkViewport viewport{.x = 0,
                          .y = 0,
                          .width = AS_F32(swapchain.window_extent.width),
                          .height = AS_F32(swapchain.window_extent.height),
                          .minDepth = 0,
                          .maxDepth = 1};

      vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

      VkRect2D scissor{.offset = {AS_I32(cmd.clip_rect.offset.x),
                                  AS_I32(cmd.clip_rect.offset.y)},
                       .extent = {AS_U32(cmd.clip_rect.extent.x),
                                  AS_U32(cmd.clip_rect.extent.y)}};

      vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

      gfx::CanvasPushConstants push_constants{.transform =
                                                  cmd.transform.transpose()};

      vkCmdPushConstants(
          cmd_buffer, ctx.pipeline.layout,
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
          sizeof(gfx::CanvasPushConstants), &push_constants);

      vkCmdBindDescriptorSets(
          cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline.layout, 0,
          ndescriptor_sets_per_draw_call,
          &ctx.descriptor_sets[frame][icmd * ndescriptor_sets_per_draw_call], 0,
          nullptr);

      vkCmdDrawIndexed(cmd_buffer, cmd.nindices, 1, cmd.indices_offset, 0, 0);
    }

    vkCmdEndRenderPass(cmd_buffer);

    ASR_VK_CHECK(vkEndCommandBuffer(cmd_buffer));

    VkPipelineStageFlags wait_stage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &swapchain.image_acquisition_semaphores[frame],
        .pWaitDstStageMask = &wait_stage,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &swapchain.rendering_semaphores[frame]};

    ASR_VK_CHECK(vkQueueSubmit(queue, 1, &submit_info,
                               swapchain.rendering_fences[frame]));
  }
};

struct SwapChainCanvasRenderer {};

}  // namespace vk
}  // namespace asr