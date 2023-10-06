#include "ashura/rcg.h"

namespace ash
{
namespace rcg
{

void CommandBufferContext::reset()
{
  graphics_pipeline = gfx::GraphicsPipeline::None;
  compute_pipeline  = gfx::ComputePipeline::None;
  render_pass       = gfx::RenderPass::None;
  framebuffer       = gfx::Framebuffer::None;
}

void CommandBuffer::fill_buffer(gfx::Buffer buffer, u64 offset, u64 size, u32 data)
{
  hook->fill_buffer(buffer, offset, size, data);

  gfx::QueueBufferMemoryBarrier barriers[1] = {{.buffer = graph->buffers[buffer].handle}};
  if (graph->buffers[buffer].state.sync(
          gfx::BufferAccess{.stages = gfx::PipelineStages::Transfer,
                            .access = gfx::Access::TransferWrite},
          barriers[0]))
  {
    graph->driver->cmd_insert_barriers(handle, barriers, {});
  }
  graph->driver->cmd_fill_buffer(handle, buffer, offset, size, data);
}

void CommandBuffer::copy_buffer(gfx::Buffer src, gfx::Buffer dst, stx::Span<gfx::BufferCopy const> copies)
{
  hook->copy_buffer(src, dst, copies);

  gfx::QueueBufferMemoryBarrier barriers[2];
  u8                            num_barriers = 0;
  if (graph->buffers[src].state.sync(
          gfx::BufferAccess{.stages = gfx::PipelineStages::Transfer,
                            .access = gfx::Access::TransferRead},
          barriers[num_barriers]))
  {
    barriers[num_barriers].buffer = graph->buffers[src].handle;
    num_barriers++;
  }

  if (graph->buffers[dst].state.sync(
          gfx::BufferAccess{.stages = gfx::PipelineStages::Transfer,
                            .access = gfx::Access::TransferWrite},
          barriers[num_barriers]))
  {
    barriers[num_barriers].buffer = graph->buffers[dst].handle;
    num_barriers++;
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, stx::Span{barriers, num_barriers}, {});
  }

  graph->driver->cmd_copy_buffer(handle, src, dst, copies);
}

void CommandBuffer::update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst)
{
  hook->update_buffer(src, dst_offset, dst);

  gfx::QueueBufferMemoryBarrier barriers[1];
  if (graph->buffers[dst].state.sync(
          gfx::BufferAccess{.stages = gfx::PipelineStages::Transfer,
                            .access = gfx::Access::TransferWrite},
          barriers[0]))
  {
    barriers[0].buffer = graph->buffers[dst].handle;
    graph->driver->cmd_insert_barriers(handle, barriers, {});
  }

  graph->driver->cmd_update_buffer(handle, src, dst_offset, dst);
}

void CommandBuffer::copy_image(gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageCopy const> copies)
{
  hook->copy_image(src, dst, copies);

  gfx::QueueImageMemoryBarrier barriers[2];
  u8                           num_barriers = 0;
  if (graph->images[src].state.sync(
          gfx::ImageAccess{.stages = gfx::PipelineStages::Transfer,
                           .access = gfx::Access::TransferRead,
                           .layout = gfx::ImageLayout::TransferSrcOptimal},
          barriers[num_barriers]))
  {
    barriers[num_barriers].image   = graph->images[src].handle;
    barriers[num_barriers].aspects = graph->images[src].desc.aspects;
    num_barriers++;
  }

  if (graph->images[dst].state.sync(
          gfx::ImageAccess{.stages = gfx::PipelineStages::Transfer,
                           .access = gfx::Access::TransferWrite,
                           .layout = gfx::ImageLayout::TransferDstOptimal},
          barriers[num_barriers]))
  {
    barriers[num_barriers].image   = graph->images[dst].handle;
    barriers[num_barriers].aspects = graph->images[dst].desc.aspects;
    num_barriers++;
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, {}, stx::Span{barriers, num_barriers});
  }

  graph->driver->cmd_copy_image(handle, src, dst, copies);
}

void CommandBuffer::copy_buffer_to_image(gfx::Buffer src, gfx::Image dst, stx::Span<gfx::BufferImageCopy const> copies)
{
  gfx::QueueBufferMemoryBarrier buffer_memory_barriers[1];
  gfx::QueueImageMemoryBarrier  image_memory_barriers[1];
  u8                            num_buffer_memory_barriers = 0;
  u8                            num_image_memory_barriers  = 0;

  if (graph->buffers[src].state.sync(
          gfx::BufferAccess{.stages = gfx::PipelineStages::Transfer,
                            .access = gfx::Access::TransferRead},
          buffer_memory_barriers[num_buffer_memory_barriers]))
  {
    buffer_memory_barriers[num_buffer_memory_barriers].buffer = graph->buffers[src].handle;
    num_buffer_memory_barriers++;
  }

  if (graph->images[dst].state.sync(
          gfx::ImageAccess{.stages = gfx::PipelineStages::Transfer,
                           .access = gfx::Access::TransferWrite,
                           .layout = gfx::ImageLayout::TransferDstOptimal},
          image_memory_barriers[num_image_memory_barriers]))
  {
    image_memory_barriers[num_image_memory_barriers].image   = graph->images[dst].handle;
    image_memory_barriers[num_image_memory_barriers].aspects = graph->images[dst].desc.aspects;
    num_image_memory_barriers++;
  }

  if (num_buffer_memory_barriers > 0 || num_image_memory_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, stx::Span{buffer_memory_barriers, num_buffer_memory_barriers}, stx::Span{image_memory_barriers, num_image_memory_barriers});
  }

  graph->driver->cmd_copy_buffer_to_image(handle, src, dst, copies);
}

void CommandBuffer::blit_image(gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter)
{
  hook->blit_image(src, dst, blits, filter);

  gfx::QueueImageMemoryBarrier barriers[2];
  u8                           num_barriers = 0;
  if (graph->images[src].state.sync(
          gfx::ImageAccess{.stages = gfx::PipelineStages::Transfer,
                           .access = gfx::Access::TransferRead,
                           .layout = gfx::ImageLayout::TransferSrcOptimal},
          barriers[num_barriers]))
  {
    barriers[num_barriers].image   = graph->images[src].handle;
    barriers[num_barriers].aspects = graph->images[src].desc.aspects;
    num_barriers++;
  }

  if (graph->images[dst].state.sync(
          gfx::ImageAccess{.stages = gfx::PipelineStages::Transfer,
                           .access = gfx::Access::TransferWrite,
                           .layout = gfx::ImageLayout::TransferDstOptimal},
          barriers[num_barriers]))
  {
    barriers[num_barriers].image   = graph->images[dst].handle;
    barriers[num_barriers].aspects = graph->images[dst].desc.aspects;
    num_barriers++;
  }

  if (num_barriers > 0)
  {
    graph->driver->cmd_insert_barriers(handle, {}, stx::Span{barriers, num_barriers});
  }

  graph->driver->cmd_blit_image(handle, src, dst, blits, filter);
}

void CommandBuffer::begin_render_pass(gfx::RenderPassBeginInfo const &info)
{
  hook->begin_render_pass(info);

  gfx::RenderPassDesc const             &renderpass_desc = graph->render_passes[info.render_pass].desc;
  stx::Vec<gfx::QueueImageMemoryBarrier> barriers;
  gfx::QueueImageMemoryBarrier           barrier;

  for (u32 i = 0; i < renderpass_desc.num_color_attachments; i++)
  {
    gfx::Image                image    = graph->image_views[graph->framebuffers[info.framebuffer].desc.color_attachments[i]].desc.image;
    gfx::ImageResource const &resource = graph->images[image];

    if (graph->images[image].state.sync(renderpass_desc.color_attachments[i].get_color_image_access(), barrier))
    {
      barrier.image   = resource.handle;
      barrier.aspects = resource.desc.aspects;
      barriers.push_inplace(barrier).unwrap();
    }
  }

  for (u32 i = 0; i < renderpass_desc.num_depth_stencil_attachments; i++)
  {
    gfx::Image                image    = graph->image_views[graph->framebuffers[info.framebuffer].desc.depth_stencil_attachments[i]].desc.image;
    gfx::ImageResource const &resource = graph->images[image];

    if (graph->images[image].state.sync(renderpass_desc.depth_stencil_attachments[i].get_depth_stencil_image_access(), barrier))
    {
      barrier.image   = resource.handle;
      barrier.aspects = resource.desc.aspects;
      barriers.push_inplace(barrier).unwrap();
    }
  }

  gfx::RenderPassBeginInfo info_copy{info};
  info_copy.framebuffer = graph->framebuffers[info_copy.framebuffer].handle;
  info_copy.render_pass = graph->render_passes[info_copy.render_pass].handle;

  graph->driver->cmd_insert_barriers(handle, {}, barriers);
  graph->driver->cmd_begin_render_pass(handle, info_copy);
}

void CommandBuffer::end_render_pass()
{
  hook->end_render_pass();
  graph->driver->cmd_end_render_pass(handle);
}

void CommandBuffer::bind_pipeline(gfx::ComputePipeline pipeline)
{
  hook->bind_pipeline(pipeline);
  context.compute_pipeline  = pipeline;
  context.graphics_pipeline = gfx::GraphicsPipeline::None;
  graph->driver->cmd_bind_pipeline(handle, graph->compute_pipelines[pipeline].handle);
}

void CommandBuffer::bind_pipeline(gfx::GraphicsPipeline pipeline)
{
  hook->bind_pipeline(pipeline);
  context.graphics_pipeline = pipeline;
  context.compute_pipeline  = gfx::ComputePipeline::None;
  graph->driver->cmd_bind_pipeline(handle, graph->graphics_pipelines[pipeline].handle);
}

void CommandBuffer::bind_vertex_buffers(u32 first_binding, stx::Span<gfx::Buffer const> vertex_buffers, stx::Span<u64 const> offsets)
{
  hook->bind_vertex_buffers(first_binding, vertex_buffers, offsets);
  graph->driver->cmd_bind_vertex_buffers(handle, first_binding, vertex_buffers, offsets);
}

void CommandBuffer::bind_index_buffer(gfx::Buffer index_buffer, u64 offset)
{
  hook->bind_index_buffer(index_buffer, offset);
  context.index_buffer = index_buffer;
  graph->driver->cmd_bind_index_buffer(handle, index_buffer, offset);
}

void CommandBuffer::push_constants(stx::Span<u8 const> constants)
{
  hook->push_constants(constants);
  graph->driver->cmd_push_constants(handle, context.compute_pipeline == gfx::ComputePipeline::None ? gfx::PipelineBindPoint::Graphics : gfx::PipelineBindPoint::Compute, constants);
}

void CommandBuffer::push_descriptor_set(u32 set, gfx::DescriptorSetBindings const &bindings)
{
  // todo(lamarrr): add pipeline bind point
  hook->push_descriptor_set(set, bindings);

  context.descriptor_set_barriers[set].clear();

  gfx::QueueImageMemoryBarrier  image_memory_barrier;
  gfx::QueueBufferMemoryBarrier buffer_memory_barrier;

  for (gfx::CombinedImageSamplerBinding const &binding : bindings.combined_image_samplers)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                                                     .access = gfx::Access::ShaderRead,
                                                                                                     .layout = gfx::ImageLayout::ShaderReadOnlyOptimal},
                                                                                    image_memory_barrier))
    {
      image_memory_barrier.image   = graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects = graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  for (gfx::SampledImageBinding const &binding : bindings.sampled_images)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                                                     .access = gfx::Access::ShaderRead,
                                                                                                     .layout = gfx::ImageLayout::ShaderReadOnlyOptimal},
                                                                                    image_memory_barrier))
    {
      image_memory_barrier.image   = graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects = graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                                                     .access = gfx::Access::ShaderWrite,
                                                                                                     .layout = gfx::ImageLayout::General},
                                                                                    image_memory_barrier))
    {
      image_memory_barrier.image   = graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects = graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  for (gfx::UniformTexelBufferBinding const &binding : bindings.uniform_texel_buffers)
  {
    if (graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].state.sync(gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                                                          .access = gfx::Access::ShaderRead},
                                                                                        buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer = graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    if (graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].state.sync(gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                                                          .access = gfx::Access::ShaderWrite},
                                                                                        buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer = graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::UniformBufferBinding const &binding : bindings.uniform_buffers)
  {
    if (graph->buffers[binding.buffer].state.sync(gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                    .access = gfx::Access::ShaderRead},
                                                  buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer = graph->buffers[binding.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    if (graph->buffers[binding.buffer].state.sync(gfx::BufferAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                    .access = gfx::Access::ShaderWrite},
                                                  buffer_memory_barrier))
    {
      buffer_memory_barrier.buffer = graph->buffers[binding.buffer].handle;
      context.descriptor_set_barriers[set].buffer.push_inplace(buffer_memory_barrier).unwrap();
    }
  }

  for (gfx::InputAttachmentBinding const &binding : bindings.input_attachments)
  {
    if (graph->images[graph->image_views[binding.image_view].desc.image].state.sync(gfx::ImageAccess{.stages = gfx::PipelineStages::AllCommands,
                                                                                                     .access = gfx::Access::ShaderRead,
                                                                                                     .layout = gfx::ImageLayout::ShaderReadOnlyOptimal},
                                                                                    image_memory_barrier))
    {
      image_memory_barrier.image   = graph->images[graph->image_views[binding.image_view].desc.image].handle;
      image_memory_barrier.aspects = graph->images[graph->image_views[binding.image_view].desc.image].desc.aspects;
      context.descriptor_set_barriers[set].image.push_inplace(image_memory_barrier).unwrap();
    }
  }

  graph->driver->cmd_push_descriptor_set(handle, set, bindings);
}

void CommandBuffer::set_scissor(IRect scissor)
{
  hook->set_scissor(scissor);
  graph->driver->cmd_set_scissor(handle, scissor);
}

void CommandBuffer::set_viewport(gfx::Viewport const &viewport)
{
  hook->set_viewport(viewport);
  graph->driver->cmd_set_viewport(handle, viewport);
}

void CommandBuffer::set_blend_constants(f32 r, f32 g, f32 b, f32 a)
{
  hook->set_blend_constants(r, g, b, a);
  graph->driver->cmd_set_blend_constants(handle, r, g, b, a);
}

void CommandBuffer::set_stencil_compare_mask(gfx::StencilFaces faces, u32 compare_mask)
{
  hook->set_stencil_compare_mask(faces, compare_mask);
  graph->driver->cmd_set_stencil_compare_mask(handle, faces, compare_mask);
}

void CommandBuffer::set_stencil_reference(gfx::StencilFaces faces, u32 reference)
{
  hook->set_stencil_reference(faces, reference);
  graph->driver->cmd_set_stencil_reference(handle, faces, reference);
}

void CommandBuffer::set_stencil_write_mask(gfx::StencilFaces faces, u32 write_mask)
{
  hook->set_stencil_write_mask(faces, write_mask);
  graph->driver->cmd_set_stencil_write_mask(handle, faces, write_mask);
}

void CommandBuffer::dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z)
{
  hook->dispatch(group_count_x, group_count_y, group_count_z);
  graph->driver->cmd_insert_barriers(handle, context.descriptor_set_barriers.buffer_memory_barriers, image_memory_barriers);
  graph->driver->cmd_dispatch(handle, group_count_x, group_count_y, group_count_z);
}

void CommandBuffer::dispatch_indirect(gfx::Buffer buffer, u64 offset)
{
  hook->dispatch_indirect(buffer, offset);
  graph->driver->cmd_insert_barriers(handle, buffer_memory_barriers, image_memory_barriers);
  graph->driver->cmd_dispatch_indirect(handle, buffer, offset);
}

void CommandBuffer::draw(u32 first_vertex, u32 vertex_count, u32 instance_count, u32 first_instance_id)
{
  hook->draw(first_vertex, vertex_count, instance_count, first_instance_id);
  graph->driver->cmd_insert_barriers(handle, buffer_memory_barriers, image_memory_barriers);
  graph->driver->cmd_draw(handle, first_vertex, vertex_count, instance_count, first_instance_id);
}

void CommandBuffer::draw_indexed(u32 first_index, u32 index_count, u32 instance_count, i32 vertex_offset, u32 first_instance_id)
{
  hook->draw_indexed(first_index, index_count, instance_count, vertex_offset, first_instance_id);
  graph->driver->cmd_insert_barriers(handle, buffer_memory_barriers, image_memory_barriers);
  graph->driver->cmd_draw_indexed(handle, first_index, index_count, instance_count, vertex_offset, first_instance_id);
}

void CommandBuffer::draw_indexed_indirect(gfx::Buffer buffer, u64 offset, u32 draw_count, u32 stride)
{
  hook->draw_indexed_indirect(buffer, offset, draw_count, stride);
  graph->driver->cmd_insert_barriers(handle, buffer_memory_barriers, image_memory_barriers);
  graph->driver->cmd_draw_indexed_indirect(handle, buffer, offset, draw_count, stride);
}

void CommandBuffer::on_execution_complete_fn(stx::UniqueFn<void()> &&fn)
{
  hook->on_execution_complete_fn(fn);
  completion_tasks.push(std::move(fn)).unwrap();
}

constexpr bool is_renderpass_compatible(RenderPassDesc const &a, RenderPassDesc const &b)
{
  return true;
}
/*
Buffer Graph::create_buffer(BufferDesc const &desc)
{
  return buffers.push(BufferResource{.state = BufferState{.desc = desc}, .handle = driver->create_buffer(desc)});
}

BufferView Graph::create_buffer_view(BufferViewDesc const &desc)
{
  return buffer_views.push(BufferViewResource{.desc = desc, .handle = driver->create_buffer_view(desc)});
}

Image Graph::create_image(ImageDesc const &desc)
{
  return images.push(ImageResource{.state = ImageState{.desc = desc}, .handle = driver->create_image(desc)});
}

ImageView Graph::create_image_view(ImageViewDesc const &desc)
{
  return image_views.push(ImageViewResource{.desc = desc, .handle = driver->create_image_view(desc)});
}

RenderPass Graph::create_render_pass(RenderPassDesc const &desc)
{
  return render_passes.push(RenderPassResource{.desc = desc, .handle = driver->create_render_pass(desc)});
}

Framebuffer Graph::create_framebuffer(FramebufferDesc const &desc)
{
  return framebuffers.push(FramebufferResource{.desc= desc,.handle = driver->create_framebuffer(desc)});
}

ComputePipeline Graph::create_compute_pipeline(ComputePipelineDesc const &desc)
{
  return compute_pipelines.push(ComputePipelineResource{.desc= desc,.handle = driver->create_compute_pipeline(desc)});
}

GraphicsPipeline Graph::create_graphics_pipeline(GraphicsPipelineDesc const &desc)
{
  return graphics_pipelines.push(GraphicsPipelineResource{.desc= desc,.handle=driver->create_graphics_pipeline(desc)});
}

BufferDesc const &Graph::get_desc(Buffer buffer) const
{
  return buffers[buffer].desc;
}

BufferViewDesc const &Graph::get_desc(BufferView buffer_view) const
{
  return buffer_views[buffer_view];
}

ImageDesc const &Graph::get_desc(Image image) const
{
  return images[image].desc;
}

ImageViewDesc const &Graph::get_desc(ImageView image_view) const
{
  return image_views[image_view];
}

RenderPassDesc const &Graph::get_desc(RenderPass render_pass) const
{
  return render_passes[render_pass];
}

FramebufferDesc const &Graph::get_desc(Framebuffer framebuffer) const
{
  return framebuffers[framebuffer];
}

ComputePipelineDesc const &Graph::get_desc(ComputePipeline compute_pipeline) const
{
  return compute_pipelines[compute_pipeline];
}

GraphicsPipelineDesc const &Graph::get_desc(GraphicsPipeline graphics_pipeline) const
{
  return graphics_pipelines[graphics_pipeline];
}

BufferState &Graph::get_state(Buffer buffer)
{
  return buffers[buffer];
}

ImageState &Graph::get_state(Image image)
{
  return images[image];
}

void Graph::release(Buffer buffer)
{}

void Graph::release(Image image)
{}

void Graph::release(ImageView image_view)
{}

void Graph::release(RenderPass render_pass)
{}

void Graph::release(Framebuffer framebuffer)
{}

void Graph::release(ComputePipeline compute_pipeline)
{}

void Graph::release(GraphicsPipeline graphics_pipeline)
{}

void validate_resources(Graph &graph)
{
  // TODO(lamarrr): we need a logger
  for (BufferState const &buffer : graph.buffers)
  {
    ASH_CHECK(buffer.desc.usages != BufferUsages::None);
    ASH_CHECK(buffer.desc.size > 0);
    ASH_CHECK(buffer.desc.properties != MemoryProperties::None);
    ASH_CHECK(graph.ctx.device_info.memory_heaps.has_memory(buffer.desc.properties));
  }

  for (ImageState const &image : graph.images)
  {
    ASH_CHECK(image.desc.extent.is_visible());
    ASH_CHECK(image.desc.usages != ImageUsages::None);
    ASH_CHECK(image.desc.mips >= 1);
    ASH_CHECK(image.desc.format != Format::Undefined);
    ASH_CHECK(image.desc.aspects != ImageAspects::None);
  }

  for (ImageViewDesc const &image_view : graph.image_views)
  {
    ASH_CHECK(image_view.aspects != ImageAspects::None);
    ASH_CHECK(image_view.view_format != Format::Undefined);
    ASH_CHECK(graph.images.is_valid(image_view.image));
    ASH_CHECK(image_view.num_mip_levels >= 1);
    ImageDesc const resource = graph.get_desc(image_view.image);
    ASH_CHECK((resource.aspects | image_view.aspects) != ImageAspects::None);
    ASH_CHECK(image_view.first_mip_level < resource.mips);
    ASH_CHECK((image_view.first_mip_level + image_view.num_mip_levels) <= resource.mips);
  }

  // for (RenderPassDesc const &render_pass : graph.render_passes)
  // {
  //   for (RenderPassAttachment const &attachment : render_pass.color_attachments)
  //   {
  //     ASH_CHECK(attachment.format != Format::Undefined);
  //   }
  //   for (RenderPassAttachment const &attachment : render_pass.depth_stencil_attachments)
  //   {
  //     ASH_CHECK(attachment.format != Format::Undefined);
  //   }
  // }

  // // check buffer views

  // for (FramebufferDesc const &framebuffer : graph.framebuffers)
  // {
  //   ASH_CHECK(graph.render_passes.is_valid(framebuffer.renderpass));
  //   ASH_CHECK(framebuffer.extent.is_visible());
  //   RenderPassDesc const render_pass_desc = graph.get_desc(framebuffer.renderpass);
  //   ASH_CHECK(framebuffer.color_attachments.size() == render_pass_desc.color_attachments.size());
  //   ASH_CHECK(framebuffer.depth_stencil_attachments.size() == render_pass_desc.depth_stencil_attachments.size());
  //   for (usize i = 0; i < framebuffer.color_attachments.size(); i++)
  //   {
  //     ImageView attachment = framebuffer.color_attachments[i];
  //     ASH_CHECK(graph.image_views.is_valid(attachment));
  //     ImageViewDesc const attachment_desc = graph.get_desc(attachment);
  //     ASH_CHECK((attachment_desc.aspects & ImageAspects::Color) != ImageAspects::None);
  //     ASH_CHECK(attachment_desc.num_mip_levels >= 1);
  //     ASH_CHECK(render_pass_desc.color_attachments[i].format == attachment_desc.view_format);
  //     ImageDesc const image_desc = graph.get_desc(attachment_desc.image);
  //     ASH_CHECK((image_desc.usages & ImageUsages::ColorAttachment) != ImageUsages::None);
  //     ASH_CHECK(image_desc.extent.width >= framebuffer.extent.width);
  //     ASH_CHECK(image_desc.extent.height >= framebuffer.extent.height);
  //   }
  //   for (usize i = 0; i < framebuffer.depth_stencil_attachments.size(); i++)
  //   {
  //     ImageView attachment = framebuffer.depth_stencil_attachments[i];
  //     ASH_CHECK(graph.image_views.is_valid(attachment));
  //     ImageViewDesc const attachment_desc = graph.get_desc(attachment);
  //     ASH_CHECK((attachment_desc.aspects & (ImageAspects::Depth | ImageAspects::Stencil)) != ImageAspects::None);
  //     ASH_CHECK(attachment_desc.num_mip_levels >= 1);
  //     ASH_CHECK(render_pass_desc.depth_stencil_attachments[i].format == attachment_desc.view_format);
  //     ImageDesc const image_desc = graph.get_desc(attachment_desc.image);
  //     ASH_CHECK((image_desc.usages & ImageUsages::DepthStencilAttachment) != ImageUsages::None);
  //     ASH_CHECK(image_desc.extent.width >= framebuffer.extent.width);
  //     ASH_CHECK(image_desc.extent.height >= framebuffer.extent.height);
  //   }
  // }
}

void CmdValidator::copy_buffer(Graph &graph, Buffer src, Buffer dst, stx::Span<BufferCopy const> copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(graph.buffers.is_valid(src));
  ASH_CHECK(graph.buffers.is_valid(dst));
  BufferDesc const src_desc = graph.get_desc(src);
  BufferDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & BufferUsages::TransferSrc) != BufferUsages::None);
  ASH_CHECK((dst_desc.usages & BufferUsages::TransferDst) != BufferUsages::None);
  for (BufferCopy const &copy : copies)
  {
    ASH_CHECK(src_desc.size > copy.src_offset);
    ASH_CHECK(src_desc.size >= (copy.src_offset + copy.size));
    ASH_CHECK(dst_desc.size > copy.dst_offset);
    ASH_CHECK(dst_desc.size >= (copy.dst_offset + copy.size));
  }
}

void CmdValidator::copy_host_buffer(Graph &graph, stx::Span<u8 const> src, Buffer dst, stx::Span<BufferCopy const> copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(!src.is_empty());
  ASH_CHECK(graph.buffers.is_valid(dst));
  BufferDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((dst_desc.usages & BufferUsages::TransferDst) != BufferUsages::None);
  for (BufferCopy const &copy : copies)
  {
    ASH_CHECK(src.size() > copy.src_offset);
    ASH_CHECK(src.size() >= (copy.src_offset + copy.size));
    ASH_CHECK(dst_desc.size > copy.dst_offset);
    ASH_CHECK(dst_desc.size >= (copy.dst_offset + copy.size));
  }
}

void CmdValidator::copy_image(Graph &graph, Image src, Image dst, stx::Span<ImageCopy const> copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(src != Image::None);
  ASH_CHECK(graph.images.is_valid(src));
  ASH_CHECK(graph.images.is_valid(dst));
  ImageDesc const src_desc = graph.get_desc(src);
  ImageDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & ImageUsages::TransferSrc) != ImageUsages::None);
  ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);

  for (ImageCopy const &copy : copies)
  {
    ASH_CHECK(copy.src_aspects != ImageAspects::None);
    ASH_CHECK(copy.dst_aspects != ImageAspects::None);
    ASH_CHECK((copy.src_aspects | src_desc.aspects) != ImageAspects::None);
    ASH_CHECK((copy.dst_aspects | dst_desc.aspects) != ImageAspects::None);
    ASH_CHECK(copy.src_mip_level < src_desc.mips);
    ASH_CHECK(copy.dst_mip_level < dst_desc.mips);
    // ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(copy.src_area)));
    // ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(copy.src_area.with_offset(copy.dst_offset))));
  }
}

void CmdValidator::copy_buffer_to_image(Graph &graph, Buffer src, Image dst, stx::Span<BufferImageCopy const> copies)
{
  ASH_CHECK(!copies.is_empty());
  ASH_CHECK(graph.buffers.is_valid(src));
  ASH_CHECK(graph.images.is_valid(dst));
  BufferDesc const src_desc = graph.get_desc(src);
  ImageDesc const  dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & BufferUsages::TransferSrc) != BufferUsages::None);
  ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);
  for (BufferImageCopy const &copy : copies)
  {
    ASH_CHECK(copy.buffer_image_height > 0);
    ASH_CHECK(copy.buffer_row_length > 0);
    ASH_CHECK(copy.buffer_offset < src_desc.size);
    ASH_CHECK(copy.image_mip_level < dst_desc.mips);
    ASH_CHECK(copy.image_aspects != ImageAspects::None);
    ASH_CHECK((copy.image_aspects | dst_desc.aspects) != ImageAspects::None);
    // ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(copy.image_area)));
  }
}

void CmdValidator::blit_image(Graph &graph, Image src, Image dst, stx::Span<ImageBlit const> blits, Filter filter)
{
  ASH_CHECK(!blits.is_empty());
  ASH_CHECK(graph.images.is_valid(src));
  ASH_CHECK(graph.images.is_valid(dst));
  ImageDesc const src_desc = graph.get_desc(src);
  ImageDesc const dst_desc = graph.get_desc(dst);
  ASH_CHECK((src_desc.usages & ImageUsages::TransferSrc) != ImageUsages::None);
  ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);
  for (ImageBlit const &blit : blits)
  {
    ASH_CHECK(blit.src_aspects != ImageAspects::None);
    ASH_CHECK(blit.dst_aspects != ImageAspects::None);
    ASH_CHECK((blit.src_aspects | src_desc.aspects) != ImageAspects::None);
    ASH_CHECK((blit.dst_aspects | dst_desc.aspects) != ImageAspects::None);
    ASH_CHECK(blit.src_mip_level < src_desc.mips);
    ASH_CHECK(blit.dst_mip_level < dst_desc.mips);
    // ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(blit.src_area)));
    // ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(blit.dst_area)));
  }
}

void CmdValidator::begin_render_pass(Graph &graph, Framebuffer framebuffer, RenderPass render_pass, IRect render_area, stx::Span<Color const> color_attachments_clear_values, stx::Span<DepthStencil const> depth_stencil_attachments_clear_values)
{
  ASH_CHECK(graph.render_passes.is_valid(render_pass));
  ASH_CHECK(graph.framebuffers.is_valid(framebuffer));
  FramebufferDesc const framebuffer_desc = graph.get_desc(framebuffer);
  // ASH_CHECK(framebuffer_desc.color_attachments.size() == color_attachments_clear_values.size());
  // ASH_CHECK(framebuffer_desc.depth_stencil_attachments.size() == depth_stencil_attachments_clear_values.size());
  // TODO(lamarrr): check renderpass compatibility
}

void CmdValidator::end_render_pass(Graph &graph)
{}

void CmdValidator::push_constants(Graph &graph, stx::Span<u8 const> constants)
{
  ASH_CHECK(constants.size_bytes() <= 128);
}

void CmdValidator::bind_compute_pipeline(Graph &graph, ComputePipeline pipeline)
{
}

void CmdValidator::bind_graphics_pipeline(Graph &graph, GraphicsPipeline pipeline)
{
}

void CmdValidator::bind_vertex_buffers(Graph &graph, stx::Span<Buffer const> vertex_buffers, stx::Span<u64 const> vertex_buffer_offsets, Buffer index_buffer, u64 index_buffer_offset)
{}

void CmdValidator::set_scissor(Graph &graph, IRect scissor)
{}

void CmdValidator::set_viewport(Graph &graph, Viewport viewport)
{}

void CmdValidator::compute(Graph &graph, u32 base_group_x, u32 group_count_x, u32 base_group_y, u32 group_count_y, u32 base_group_z, u32 group_count_z)
{}

void CmdValidator::compute_indirect(Graph &graph, Buffer buffer, u64 offset)
{}

void CmdValidator::draw_indexed(Graph &graph, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance)
{
}

void CmdValidator::draw_indexed_indirect(Graph &graph, Buffer buffer, u64 offset, u32 draw_count, u32 stride)
{}

void CmdBarrierGenerator::copy_buffer(Graph &graph, BarrierInserter &inserter, Buffer src, Buffer dst, stx::Span<BufferCopy const> copies)
{
  BufferState &src_state = graph.get_state(src);
  BufferState &dst_state = graph.get_state(dst);

  QueueBufferMemoryBarrier src_barrier{.buffer          = src,
                                       .offset          = 0,
                                       .size            = src_state.desc.size,
                                       .src_stage_mask  = src_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = src_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};
  QueueBufferMemoryBarrier dst_barrier{.buffer          = dst,
                                       .offset          = 0,
                                       .size            = dst_state.desc.size,
                                       .src_stage_mask  = dst_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = dst_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
}

void CmdBarrierGenerator::copy_host_buffer(Graph &graph, BarrierInserter &inserter, stx::Span<u8 const> src, Buffer dst, stx::Span<BufferCopy const> copies)
{
  BufferState &dst_state = graph.get_state(dst);

  QueueBufferMemoryBarrier dst_barrier{.buffer          = dst,
                                       .offset          = 0,
                                       .size            = dst_state.desc.size,
                                       .src_stage_mask  = dst_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = dst_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};

  inserter.insert(dst_barrier);

  dst_state.access_mask = Access::TransferWrite;
  dst_state.stages      = PipelineStages::Transfer;
}

void CmdBarrierGenerator::copy_image(Graph &graph, BarrierInserter &inserter, Image src, Image dst, stx::Span<ImageCopy const> copies)
{
  ImageState &src_state = graph.get_state(src);
  ImageState &dst_state = graph.get_state(dst);

  QueueImageMemoryBarrier src_barrier{.image           = src,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = src_state.desc.aspects,
                                      .old_layout      = src_state.layout,
                                      .new_layout      = ImageLayout::TransferSrcOptimal,
                                      .src_stage_mask  = src_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = src_state.access_mask,
                                      .dst_access_mask = Access::TransferRead};
  QueueImageMemoryBarrier dst_barrier{.image           = dst,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = dst_state.desc.aspects,
                                      .old_layout      = dst_state.layout,
                                      .new_layout      = ImageLayout::TransferDstOptimal,
                                      .src_stage_mask  = dst_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = dst_state.access_mask,
                                      .dst_access_mask = Access::TransferWrite};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
  src_state.layout      = ImageLayout::TransferSrcOptimal;
  dst_state.layout      = ImageLayout::TransferDstOptimal;
}

void CmdBarrierGenerator::copy_buffer_to_image(Graph &graph, BarrierInserter &inserter, Buffer src, Image dst, stx::Span<BufferImageCopy const> copies)
{
  // get latest expected state
  // convert to transfer dst layout and transfer write access with transfer stage
  // leave as-is. the next usage should conver it back to how it is needed if necessary
  BufferState &src_state = graph.get_state(src);
  ImageState  &dst_state = graph.get_state(dst);

  QueueBufferMemoryBarrier src_barrier{.buffer          = src,
                                       .offset          = 0,
                                       .size            = WHOLE_SIZE,
                                       .src_stage_mask  = src_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = src_state.access_mask,
                                       .dst_access_mask = Access::TransferRead};
  QueueImageMemoryBarrier  dst_barrier{.image           = dst,
                                       .first_mip_level = 0,
                                       .num_mip_levels  = REMAINING_MIP_LEVELS,
                                       .aspects         = dst_state.desc.aspects,
                                       .old_layout      = dst_state.layout,
                                       .new_layout      = ImageLayout::TransferDstOptimal,
                                       .src_stage_mask  = dst_state.stages,
                                       .dst_stage_mask  = PipelineStages::Transfer,
                                       .src_access_mask = dst_state.access_mask,
                                       .dst_access_mask = Access::TransferWrite};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
  dst_state.layout      = ImageLayout::TransferDstOptimal;
}

void CmdBarrierGenerator::blit_image(Graph &graph, BarrierInserter &inserter, Image src, Image dst, stx::Span<ImageBlit const> blits, Filter filter)
{
  // check RID
  // check current layout and usage, and all accessors
  // check all memory aliasing
  // convert layout to transfer dst and write access with whatever access type is needed for blitting
  // update barrier tracker
  // on next usage in command buffer, get the last usage and update accordingly
  ImageState &src_state = graph.get_state(src);
  ImageState &dst_state = graph.get_state(dst);

  QueueImageMemoryBarrier src_barrier{.image           = src,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = src_state.desc.aspects,
                                      .old_layout      = src_state.layout,
                                      .new_layout      = ImageLayout::TransferSrcOptimal,
                                      .src_stage_mask  = src_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = src_state.access_mask,
                                      .dst_access_mask = Access::TransferRead};
  QueueImageMemoryBarrier dst_barrier{.image           = dst,
                                      .first_mip_level = 0,
                                      .num_mip_levels  = REMAINING_MIP_LEVELS,
                                      .aspects         = dst_state.desc.aspects,
                                      .old_layout      = dst_state.layout,
                                      .new_layout      = ImageLayout::TransferDstOptimal,
                                      .src_stage_mask  = dst_state.stages,
                                      .dst_stage_mask  = PipelineStages::Transfer,
                                      .src_access_mask = dst_state.access_mask,
                                      .dst_access_mask = Access::TransferWrite};

  inserter.insert(src_barrier);
  inserter.insert(dst_barrier);

  src_state.access_mask = Access::TransferRead;
  dst_state.access_mask = Access::TransferWrite;
  src_state.stages      = PipelineStages::Transfer;
  dst_state.stages      = PipelineStages::Transfer;
  src_state.layout      = ImageLayout::TransferSrcOptimal;
  dst_state.layout      = ImageLayout::TransferDstOptimal;
}

void CmdBarrierGenerator::begin_render_pass(Graph &graph, BarrierInserter &inserter, Framebuffer framebuffer, RenderPass render_pass, IRect render_area, stx::Span<Color const> color_attachments_clear_values, stx::Span<DepthStencil const> depth_stencil_attachments_clear_values)
{
  // TODO(lamarrr): in-between barriers access??? i.e. in shader?
  //
  // TODO(lamarrr): should we make the renderpass not convert the layouts?
  //
  // FramebufferDesc framebuffer_desc = graph.get_desc(framebuffer);
  // for (ImageView attachment : framebuffer_desc.color_attachments)
  // {
  //   ImageState &state = graph.get_state(graph.get_desc(attachment).image);                 // TODO(lamarrr): color attachment may not be written to depending on the renderpass ops
  //   state.access_mask = Access::ColorAttachmentRead | Access::ColorAttachmentWrite;        // LoadOp and StoreOp
  //   state.stages      = PipelineStages::ColorAttachmentOutput | PipelineStages::EarlyFragmentTests;
  //   state.layout      = ImageLayout::ColorAttachmentOptimal;
  // }

  // for (ImageView attachment : framebuffer_desc.depth_stencil_attachments)
  // {
  //   ImageState &state = graph.get_state(graph.get_desc(attachment).image);
  //   state.access_mask = Access::DepthStencilAttachmentRead | Access::DepthStencilAttachmentWrite;
  //   state.stages      = PipelineStages::ColorAttachmentOutput | PipelineStages::EarlyFragmentTests;
  //   state.layout      = ImageLayout::DepthStencilAttachmentOptimal;
  // }
}

void CmdBarrierGenerator::end_render_pass(Graph &graph, BarrierInserter &inserter)
{
  // TODO(lamarrr): post-renderpass state transitions
  // TODO(lamarrr): DontCare generates write access to the attachment in renderpasses
}
void CmdBarrierGenerator::bind_compute_pipeline(Graph &graph, BarrierInserter &inserter, ComputePipeline pipeline)
{}
void CmdBarrierGenerator::bind_graphics_pipeline(Graph &graph, BarrierInserter &inserter, GraphicsPipeline pipeline)
{}
void CmdBarrierGenerator::bind_vertex_buffers(Graph &graph, BarrierInserter &inserter, stx::Span<Buffer const> vertex_buffers, stx::Span<u64 const> vertex_buffer_offsets, Buffer index_buffer, u64 index_buffer_offset)
{}
// TODO(lamarrr): remove unnecessary barriers: i.e. newly created resources with None access and None states. and double-reads
//
// some scenarios reset the state of the barriers i.e. when we wait for fences on a swapchain, all resources in that frame would not be in use
//
// accumulate states until they are no longer needed
// use previous frame's states and barriers states
// detect unused resources
// insert fences, barriers, and whatnot
// smart aliasing image memory barriers with aliasing will enable us to perform better
// store current usage so the next usage will know how to access
//
// render passes perform layout and transitions neccessary

PipelineStages to_pipeline_stage(ShaderStages stages)
{
  PipelineStages out = PipelineStages::None;

  if ((stages & ShaderStages::Vertex) != ShaderStages::None)
  {
    out |= PipelineStages::VertexShader;
  }

  if ((stages & ShaderStages::Geometry) != ShaderStages::None)
  {
    out |= PipelineStages::GeometryShader;
  }

  if ((stages & ShaderStages::Fragment) != ShaderStages::None)
  {
    out |= PipelineStages::FragmentShader;
  }

  if ((stages & ShaderStages::Compute) != ShaderStages::None)
  {
    out |= PipelineStages::ComputeShader;
  }

  if ((stages & ShaderStages::AllGraphics) != ShaderStages::None)
  {
    out |= PipelineStages::AllGraphics;
  }

  if ((stages & ShaderStages::All) != ShaderStages::None)
  {
    out |= PipelineStages::AllCommands;
  }

  return out;
}

void CmdBarrierGenerator::set_bind_group(Graph &graph, BarrierInserter &inserter, BindGroup bind_group)
{
  BindGroupDesc const &desc = graph.get_desc(bind_group);
  for (u32 i = 0; i < desc.num_bindings; i++)
  {
    DescriptorBinding const &binding = desc.bindings[i];
    BindGroupEntry const     entry;        // TODO(lamarrr)   = desc.layout[i];
    PipelineStages           pipeline_stages = to_pipeline_stage(entry.stages);
    switch (binding.type)
    {
      case DescriptorType::CombinedImageSampler:
      {
        Image            image = graph.get_desc(binding.combined_image_sampler.image_view).image;
        ImageDesc const &desc  = graph.get_desc(image);
        ImageState      &state = graph.get_state(image);

        inserter.insert(QueueImageMemoryBarrier{
            .image           = image,
            .first_mip_level = 0,
            .num_mip_levels  = REMAINING_MIP_LEVELS,
            .aspects         = desc.aspects,
            .old_layout      = state.layout,
            .new_layout      = ImageLayout::ShaderReadOnlyOptimal,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderSampledRead});
      }
      break;

      case DescriptorType::InputAttachment:
      {
      }
      break;

      case DescriptorType::SampledImage:
      {
        Image            image = graph.get_desc(binding.sampled_image.image_view).image;
        ImageDesc const &desc  = graph.get_desc(image);
        ImageState      &state = graph.get_state(image);

        inserter.insert(QueueImageMemoryBarrier{
            .image           = image,
            .first_mip_level = 0,
            .num_mip_levels  = REMAINING_MIP_LEVELS,
            .aspects         = desc.aspects,
            .old_layout      = state.layout,
            .new_layout      = ImageLayout::ShaderReadOnlyOptimal,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderSampledRead});
      }
      break;

      case DescriptorType::Sampler:
      {
      }
      break;

      case DescriptorType::StorageBuffer:
      {
        BufferState &state = graph.get_state(binding.storage_buffer.buffer);

        inserter.insert(QueueBufferMemoryBarrier{
            .buffer          = binding.storage_buffer.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderStorageWrite});
      }
      break;

      case DescriptorType::StorageImage:
      {
        Image            image = graph.get_desc(binding.storage_image.image_view).image;
        ImageDesc const &desc  = graph.get_desc(image);
        ImageState      &state = graph.get_state(image);

        inserter.insert(QueueImageMemoryBarrier{
            .image           = image,
            .first_mip_level = 0,
            .num_mip_levels  = REMAINING_MIP_LEVELS,
            .aspects         = desc.aspects,
            .old_layout      = state.layout,
            .new_layout      = ImageLayout::General,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderStorageWrite});
      }
      break;

      case DescriptorType::StorageTexelBuffer:
      {
        BufferViewDesc const &subdesc = graph.get_desc(binding.storage_texel_buffer.buffer_view);
        BufferDesc const     &desc    = graph.get_desc(subdesc.buffer);
        BufferState          &state   = graph.get_state(subdesc.buffer);

        inserter.insert(QueueBufferMemoryBarrier{
            .buffer          = subdesc.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::ShaderStorageWrite});
      }
      break;

      case DescriptorType::UniformBuffer:
      {
        BufferDesc const &desc  = graph.get_desc(binding.uniform_buffer.buffer);
        BufferState      &state = graph.get_state(binding.uniform_buffer.buffer);

        inserter.insert(QueueBufferMemoryBarrier{
            .buffer          = binding.uniform_buffer.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::UniformRead});
      }
      break;

      case DescriptorType::UniformTexelBuffer:
      {
        BufferViewDesc const &subdesc = graph.get_desc(binding.uniform_texel_buffer.buffer_view);
        BufferDesc const     &desc    = graph.get_desc(subdesc.buffer);
        BufferState          &state   = graph.get_state(subdesc.buffer);

        inserter.insert(QueueBufferMemoryBarrier{
            .buffer          = binding.uniform_buffer.buffer,
            .offset          = 0,
            .size            = WHOLE_SIZE,
            .src_stage_mask  = state.stages,
            .dst_stage_mask  = pipeline_stages,
            .src_access_mask = state.access_mask,
            .dst_access_mask = Access::UniformRead});
      }
      break;

      default:
        break;
    }
  }
}
void CmdBarrierGenerator::compute(Graph &graph, BarrierInserter &inserter, u32 base_group_x, u32 group_count_x, u32 base_group_y, u32 group_count_y, u32 base_group_z, u32 group_count_z)
{}

void CmdBarrierGenerator::compute_indirect(Graph &graph, BarrierInserter &inserter, Buffer buffer, u64 offset)
{}

void CmdBarrierGenerator::draw_indexed(Graph &graph, BarrierInserter &inserter, u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset, u32 first_instance)
{}

void CmdBarrierGenerator::draw_indexed_indirect(Graph &graph, BarrierInserter &inserter, Buffer buffer, u64 offset, u32 draw_count, u32 stride)
{}
*/

}        // namespace rcg
}        // namespace ash
