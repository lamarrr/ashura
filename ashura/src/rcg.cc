#include "ashura/rcg.h"

namespace ash
{
namespace rcg
{



void CommandBuffer::fill_buffer(gfx::Buffer dst, u64 offset, u64 size, u32 data)
{
  hook->fill_buffer(dst, offset, size, data);

  BufferScope scope = transfer_buffer_scope(graph->buffers[dst].desc.scope);

  tmp_buffer_barriers.clear();
  acquire_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});

  driver->cmd_fill_buffer(rhi, graph->to_rhi(dst), offset, size, data);

  tmp_buffer_barriers.clear();
  release_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
}

void CommandBuffer::copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                                stx::Span<gfx::BufferCopy const> copies)
{
  hook->copy_buffer(src, dst, copies);

  BufferScope src_scope = transfer_buffer_scope(graph->buffers[src].desc.scope);
  BufferScope dst_scope = transfer_buffer_scope(graph->buffers[dst].desc.scope);

  tmp_buffer_barriers.clear();

  acquire_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  acquire_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, dst_scope.stages,
                 dst_scope.access, tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});

  driver->cmd_copy_buffer(rhi, graph->to_rhi(src), graph->to_rhi(dst), copies);

  tmp_buffer_barriers.clear();

  release_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  release_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, dst_scope.stages,
                 dst_scope.access, tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
}

void CommandBuffer::update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst)
{
  hook->update_buffer(src, dst_offset, dst);

  BufferScope scope = transfer_buffer_scope(graph->buffers[dst].desc.scope);

  tmp_buffer_barriers.clear();

  acquire_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});

  driver->cmd_update_buffer(rhi, src, dst_offset, graph->to_rhi(dst));

  tmp_buffer_barriers.clear();
  release_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
}

void CommandBuffer::clear_color_image(gfx::Image dst, stx::Span<gfx::Color const> clear_colors,
                                      stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  hook->clear_color_image(dst, clear_colors, ranges);

  ImageScope scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_clear_color_image(rhi, graph->to_rhi(dst), clear_colors, ranges);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::clear_depth_stencil_image(
    gfx::Image dst, stx::Span<gfx::DepthStencil const> clear_depth_stencils,
    stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  hook->clear_depth_stencil_image(dst, clear_depth_stencils, ranges);

  ImageScope scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_clear_depth_stencil_image(rhi, graph->to_rhi(dst), clear_depth_stencils, ranges);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::copy_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageCopy const> copies)
{
  hook->copy_image(src, dst, copies);

  ImageScope src_scope = transfer_image_scope(graph->images[src].desc.scope);
  ImageScope dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_copy_image(rhi, src, dst, copies);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                         stx::Span<gfx::BufferImageCopy const> copies)
{
  hook->copy_buffer_to_image(src, dst, copies);

  BufferScope src_scope = transfer_buffer_scope(graph->buffers[src].desc.scope);
  ImageScope  dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_buffer_barriers.clear();
  tmp_image_barriers.clear();

  acquire_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, tmp_image_barriers);

  driver->cmd_copy_buffer_to_image(rhi, src, dst, copies);

  tmp_buffer_barriers.clear();
  tmp_image_barriers.clear();

  release_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, tmp_image_barriers);
}

void CommandBuffer::blit_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter)
{
  hook->blit_image(src, dst, blits, filter);

  ImageScope src_scope = transfer_image_scope(graph->images[src].desc.scope);
  ImageScope dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_blit_image(rhi, src, dst, blits, filter);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::begin_render_pass(
    gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
    stx::Span<gfx::Color const>        color_attachments_clear_values,
    stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values)
{
  hook->begin_render_pass(framebuffer, render_pass, render_area, color_attachments_clear_values,
                          depth_stencil_attachments_clear_values);
  this->framebuffer = framebuffer;
  this->render_pass = render_pass;

  for (gfx::ImageView view : graph->framebuffers[framebuffer].desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }
    tmp_image_barriers.clear();
    gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
    ImageScope                scope = color_attachment_image_scope(image_resource.desc.scope);
    acquire_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                  scope.layout, image_resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  {
    gfx::ImageView view = graph->framebuffers[framebuffer].desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      tmp_image_barriers.clear();
      gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
      ImageScope scope = depth_stencil_attachment_image_scope(image_resource.desc.scope);
      acquire_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                    scope.layout, image_resource.desc.aspects, tmp_image_barriers);
      driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
    }
  }

  driver->cmd_begin_render_pass(
      rhi, graph->framebuffers[framebuffer].handle, graph->render_passes[render_pass].handle,
      render_area, color_attachments_clear_values, depth_stencil_attachments_clear_values);
}

void CommandBuffer::end_render_pass()
{
  hook->end_render_pass();
  driver->cmd_end_render_pass(rhi);

  for (gfx::ImageView view : graph->framebuffers[framebuffer].desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }
    tmp_image_barriers.clear();
    gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
    ImageScope                scope = color_attachment_image_scope(image_resource.desc.scope);
    release_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                  scope.layout, image_resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  {
    gfx::ImageView view = graph->framebuffers[framebuffer].desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      tmp_image_barriers.clear();
      gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
      ImageScope scope = depth_stencil_attachment_image_scope(image_resource.desc.scope);
      release_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                    scope.layout, image_resource.desc.aspects, tmp_image_barriers);
      driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
    }
  }
}

void CommandBuffer::bind_pipeline(gfx::ComputePipeline pipeline)
{
  hook->bind_pipeline(pipeline);
  this->compute_pipeline  = pipeline;
  this->graphics_pipeline = nullptr;

  driver->bind_pipeline(pipeline);
}

void CommandBuffer::bind_pipeline(gfx::GraphicsPipeline pipeline);

void CommandBuffer::dispatch(gfx::ComputePipeline pipeline, u32 group_count_x, u32 group_count_y,
                             u32 group_count_z, gfx::DescriptorSetBindings const &bindings,
                             stx::Span<u8 const> push_constants_data)
{
  hook->dispatch(pipeline, group_count_x, group_count_y, group_count_z, bindings,
                 push_constants_data);
  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    acquire_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  driver->cmd_dispatch(rhi, pipeline, group_count_x, group_count_y, group_count_z, bindings,
                       push_constants_data);

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    release_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }
}

void CommandBuffer::dispatch_indirect(gfx::ComputePipeline pipeline, gfx::Buffer buffer, u64 offset,
                                      gfx::DescriptorSetBindings const &bindings,
                                      stx::Span<u8 const>               push_constants_data)
{
  hook->dispatch_indirect(pipeline, buffer, offset, bindings, push_constants_data);
  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    acquire_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  driver->cmd_dispatch_indirect(rhi, graph->to_rhi(pipeline), graph->to_rhi(buffer), offset,
                                bindings, push_constants_data);

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    release_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }
}

void CommandBuffer::draw(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                         stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer,
                         u32 first_index, u32 num_indices, u32 vertex_offset, u32 first_instance,
                         u32 num_instances, gfx::DescriptorSetBindings const &bindings,
                         stx::Span<u8 const> push_constants_data)
{
  hook->draw(pipeline, state, vertex_buffers, index_buffer, first_index, num_indices, vertex_offset,
             first_instance, num_instances, bindings, push_constants_data);
  driver->cmd_draw(rhi, pipeline, state, vertex_buffers, index_buffer, first_index, num_indices,
                   vertex_offset, first_instance, num_instances, bindings, push_constants_data);
}

void CommandBuffer::draw_indirect(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                                  stx::Span<gfx::Buffer const> vertex_buffers,
                                  gfx::Buffer index_buffer, gfx::Buffer buffer, u64 offset,
                                  u32 draw_count, u32 stride,
                                  gfx::DescriptorSetBindings const &bindings,
                                  stx::Span<u8 const>               push_constants_data)
{
  hook->draw_indirect(pipeline, state, vertex_buffers, index_buffer, buffer, offset, draw_count,
                      stride, bindings, push_constants_data);
  driver->cmd_draw_indirect(rhi, pipeline, state, vertex_buffers, index_buffer, buffer, offset,
                            draw_count, stride, bindings, push_constants_data);
}

void CommandBuffer::on_execution_complete_fn(stx::UniqueFn<void()> &&fn)
{
  hook->on_execution_complete_fn(fn);
  completion_tasks.push(std::move(fn)).unwrap();
}

}        // namespace rcg
}        // namespace ash
