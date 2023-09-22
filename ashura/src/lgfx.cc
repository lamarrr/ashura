#include "ashura/lgfx.h"

namespace ash
{
namespace lgfx
{

Buffer Graph::create_buffer(BufferDesc const &desc)
{
  return buffers.push(BufferState{.desc = desc});
}

BufferView Graph::create_buffer_view(BufferViewDesc const &desc)
{
  return buffer_views.push(desc);
}

Image Graph::create_image(ImageDesc const &desc)
{
  return images.push(ImageState{.desc = desc});
}

ImageView Graph::create_image_view(ImageViewDesc const &desc)
{
  return image_views.push(desc);
}

RenderPass Graph::create_render_pass(RenderPassDesc const &desc)
{
  return render_passes.push(desc);
}

Framebuffer Graph::create_framebuffer(FramebufferDesc const &desc)
{
  return framebuffers.push(desc);
}

ComputePipeline Graph::create_compute_pipeline(ComputePipelineDesc const &desc)
{
  return compute_pipelines.push(desc);
}

GraphicsPipeline Graph::create_graphics_pipeline(GraphicsPipelineDesc const &desc)
{
  return graphics_pipelines.push(desc);
}

BufferDesc Graph::get_desc(Buffer buffer) const
{
  return buffers[buffer].desc;
}

BufferViewDesc Graph::get_desc(BufferView buffer_view) const
{
  return buffer_views[buffer_view];
}

ImageDesc Graph::get_desc(Image image) const
{
  return images[image].desc;
}

ImageViewDesc Graph::get_desc(ImageView image_view) const
{
  return image_views[image_view];
}

RenderPassDesc Graph::get_desc(RenderPass render_pass) const
{
  return render_passes[render_pass];
}

FramebufferDesc Graph::get_desc(Framebuffer framebuffer) const
{
  return framebuffers[framebuffer];
}

ComputePipelineDesc Graph::get_desc(ComputePipeline compute_pipeline) const
{
  return compute_pipelines[compute_pipeline];
}

GraphicsPipelineDesc Graph::get_desc(GraphicsPipeline graphics_pipeline) const
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
  }

  for (ImageViewDesc const &image_view : graph.image_views)
  {
    ASH_CHECK(image_view.aspect != ImageAspect::None);
    ASH_CHECK(image_view.view_format != Format::Undefined);
    ASH_CHECK(graph.images.is_valid(image_view.image));
    ASH_CHECK(image_view.num_mip_levels >= 1);
    ImageDesc const resource = graph.get_desc(image_view.image);
    ASH_CHECK(image_view.first_mip_level < resource.mips);
    ASH_CHECK((image_view.first_mip_level + image_view.num_mip_levels) <= resource.mips);
  }

  for (RenderPassDesc const &render_pass : graph.render_passes)
  {
    for (RenderPassAttachment const &attachment : render_pass.color_attachments)
    {
      ASH_CHECK(attachment.format != Format::Undefined);
    }
    for (RenderPassAttachment const &attachment : render_pass.depth_stencil_attachments)
    {
      ASH_CHECK(attachment.format != Format::Undefined);
    }
  }

  // check buffer views

  for (FramebufferDesc const &framebuffer : graph.framebuffers)
  {
    ASH_CHECK(graph.render_passes.is_valid(framebuffer.renderpass));
    ASH_CHECK(framebuffer.extent.is_visible());
    RenderPassDesc const render_pass_desc = graph.get_desc(framebuffer.renderpass);
    ASH_CHECK(framebuffer.color_attachments.size() == render_pass_desc.color_attachments.size());
    ASH_CHECK(framebuffer.depth_stencil_attachments.size() == render_pass_desc.depth_stencil_attachments.size());
    for (usize i = 0; i < framebuffer.color_attachments.size(); i++)
    {
      ImageView attachment = framebuffer.color_attachments[i];
      ASH_CHECK(graph.image_views.is_valid(attachment));
      ImageViewDesc const attachment_desc = graph.get_desc(attachment);
      ASH_CHECK(attachment_desc.aspect == ImageAspect::Color);
      ASH_CHECK(attachment_desc.num_mip_levels >= 1);
      ASH_CHECK(render_pass_desc.color_attachments[i].format == attachment_desc.view_format);
      ImageDesc const image_desc = graph.get_desc(attachment_desc.image);
      ASH_CHECK((image_desc.usages & ImageUsages::ColorAttachment) != ImageUsages::None);
      ASH_CHECK(image_desc.extent.width >= framebuffer.extent.width);
      ASH_CHECK(image_desc.extent.height >= framebuffer.extent.height);
    }
    for (usize i = 0; i < framebuffer.depth_stencil_attachments.size(); i++)
    {
      ImageView attachment = framebuffer.depth_stencil_attachments[i];
      ASH_CHECK(graph.image_views.is_valid(attachment));
      ImageViewDesc const attachment_desc = graph.get_desc(attachment);
      ASH_CHECK((attachment_desc.aspect & (ImageAspect::Depth | ImageAspect::Stencil)) != ImageAspect::None);
      ASH_CHECK(attachment_desc.num_mip_levels >= 1);
      ASH_CHECK(render_pass_desc.depth_stencil_attachments[i].format == attachment_desc.view_format);
      ImageDesc const image_desc = graph.get_desc(attachment_desc.image);
      ASH_CHECK((image_desc.usages & ImageUsages::DepthStencilAttachment) != ImageUsages::None);
      ASH_CHECK(image_desc.extent.width >= framebuffer.extent.width);
      ASH_CHECK(image_desc.extent.height >= framebuffer.extent.height);
    }
  }
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
  stx::Span        src      = src;
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
    ASH_CHECK(copy.src_aspect != ImageAspect::None);
    ASH_CHECK(copy.dst_aspect != ImageAspect::None);
    ASH_CHECK(copy.src_mip_level < src_desc.mips);
    ASH_CHECK(copy.dst_mip_level < dst_desc.mips);
    ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(copy.src_area)));
    ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(copy.src_area.with_offset(copy.dst_offset))));
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
    ASH_CHECK(copy.image_aspect != ImageAspect::None);
    ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(copy.image_area)));
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
    ASH_CHECK(blit.src_aspect != ImageAspect::None);
    ASH_CHECK(blit.dst_aspect != ImageAspect::None);
    ASH_CHECK(blit.src_mip_level < src_desc.mips);
    ASH_CHECK(blit.dst_mip_level < dst_desc.mips);
    ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(blit.src_area)));
    ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(blit.dst_area)));
  }
}

void CmdValidator::begin_render_pass(Graph &graph, Framebuffer framebuffer, RenderPass render_pass, IRect render_area, stx::Span<Color const> color_attachments_clear_values, stx::Span<DepthStencil const> depth_stencil_attachments_clear_values)
{
  ASH_CHECK(graph.render_passes.is_valid(render_pass));
  ASH_CHECK(graph.framebuffers.is_valid(framebuffer));
  FramebufferDesc const framebuffer_desc = graph.get_desc(framebuffer);
  ASH_CHECK(framebuffer_desc.color_attachments.size() == color_attachments_clear_values.size());
  ASH_CHECK(framebuffer_desc.depth_stencil_attachments.size() == depth_stencil_attachments_clear_values.size());
  // TODO(lamarrr): check renderpass compatibility
}

void CmdValidator::end_render_pass(Graph &graph)
{}

void CmdValidator::push_constants(Graph &graph, stx::Span<u8 const> constants)
{}

void CmdValidator::bind_compute_pipeline(Graph &graph, ComputePipeline pipeline)
{}

void CmdValidator::bind_graphics_pipeline(Graph &graph, GraphicsPipeline pipeline)
{}

void CmdValidator::bind_vertex_buffers(Graph &graph, stx::Span<Buffer const> vertex_buffers, stx::Span<u64 const> vertex_buffer_offsets, Buffer index_buffer, u64 index_buffer_offset)
{}

void CmdValidator::bind_descriptors(Graph &graph, stx::Span<DescriptorBinding const> bindings)
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
  // case CmdType::DispatchTask:
  // {
  //   ASH_CHECK(cmd.dispatch_task.framebuffer != Framebuffer::None);
  //   ASH_CHECK(((u32) cmd.dispatch_task.framebuffer) < graph.resources.size());
  //   ASH_CHECK(graph.resources[(u32) cmd.dispatch_task.framebuffer].type == ResourceType::Framebuffer);
  //   for (ResourceBinding const &binding : cmd.dispatch_task.bindings)
  //   {
  //     switch (binding.type)
  //     {
  //       case ResourceBindingType::BufferBinding:
  //       {
  //         ASH_CHECK(binding.buffer.buffer != Buffer::None);
  //         ASH_CHECK(((u32) binding.buffer.buffer) < graph.resources.size());
  //         ASH_CHECK(graph.resources[(u32) binding.buffer.buffer].type == ResourceType::Buffer);
  //         BufferDesc const buffer_desc       = graph.get_desc(binding.buffer.buffer);
  //         bool             written_in_shader = (binding.buffer.access & (Access::ShaderWrite | Access::ShaderSampledWrite | Access::ShaderStorageWrite)) != Access::None;
  //         bool             read_in_shader    = (binding.buffer.access & (Access::ShaderRead | Access::ShaderSampledRead)) != Access::None;
  //         if (read_in_shader)
  //         {
  //           ASH_CHECK((buffer_desc.usages & (BufferUsages::UniformBuffer | BufferUsages::UniformTexelBuffer)) != BufferUsages::None);
  //         }
  //         if (written_in_shader)
  //         {
  //           ASH_CHECK((buffer_desc.usages & (BufferUsages::StorageBuffer | BufferUsages::UniformStorageTexelBuffer)) != BufferUsages::None);
  //         }
  //       }
  //       break;
  //       case ResourceBindingType::ImageViewBinding:
  //       {
  //         ASH_CHECK(binding.image_view.image_view != ImageView::None);
  //         ASH_CHECK(((u32) binding.image_view.image_view) < graph.resources.size());
  //         ASH_CHECK(graph.resources[(u32) binding.image_view.image_view].type == ResourceType::ImageView);
  //         ImageDesc const image_desc        = graph.get_desc(graph.get_desc(binding.image_view.image_view).image);
  //         bool            written_in_shader = (binding.image_view.access & (Access::ShaderWrite | Access::ShaderSampledWrite | Access::ShaderStorageWrite)) != Access::None;
  //         bool            read_in_shader    = (binding.image_view.access & (Access::ShaderRead | Access::ShaderSampledRead)) != Access::None;
  //         if (read_in_shader)
  //         {
  //           ASH_CHECK((image_desc.usages & ImageUsages::Sampled) != ImageUsages::None);
  //         }
  //         if (written_in_shader)
  //         {
  //           ASH_CHECK((image_desc.usages & ImageUsages::Storage) != ImageUsages::None);
  //         }
  //       }
  //       break;
  //     }
  //   }
  // }
  // break;
}

void CmdValidator::draw_indexed_indirect(Graph &graph, Buffer buffer, u64 offset, u32 draw_count, u32 stride)
{}


// TODO(lamarrr): remove unnecessary barriers: i.e. newly created resources with None access and None states. and double-reads
//
// some scenarios reset the state of the barriers i.e. when we wait for fences on a swapchain, all resources in that frame would not be in use
//
void generate_barriers(Graph &graph, stx::Span<Cmd const> cmds, stx::Vec<QueueBarrier> &queue_barriers, stx::Vec<u32> &cmd_barriers)
{
  // accumulate states until they are no longer needed
  // use previous frame's states and barriers states
  // detect unused resources
  // insert fences, barriers, and whatnot
  // smart aliasing image memory barriers with aliasing will enable us to perform better
  // store current usage so the next usage will know how to access
  //
  // render passes perform layout and transitions neccessary
  //
  //

  for (Cmd const &cmd : cmds)
  {
    switch (cmd.type)
    {
      case CmdType::None:
        break;

      case CmdType::CopyBuffer:
      {
        BufferState &src_state = graph.get_state(cmd.copy_buffer.src);
        BufferState &dst_state = graph.get_state(cmd.copy_buffer.dst);

        for (BufferCopy const &copy : cmd.copy_buffer.copies)
        {
          QueueBufferMemoryBarrier src_barrier{.buffer          = cmd.copy_buffer.src,
                                               .offset          = copy.src_offset,
                                               .size            = copy.size,
                                               .src_stage_mask  = src_state.stage,
                                               .dst_stage_mask  = PipelineStages::Transfer,
                                               .src_access_mask = src_state.access_mask,
                                               .dst_access_mask = Access::TransferRead};
          QueueBufferMemoryBarrier dst_barrier{.buffer          = cmd.copy_buffer.dst,
                                               .offset          = copy.dst_offset,
                                               .size            = copy.size,
                                               .src_stage_mask  = dst_state.stage,
                                               .dst_stage_mask  = PipelineStages::Transfer,
                                               .src_access_mask = dst_state.access_mask,
                                               .dst_access_mask = Access::TransferRead};

          queue_barriers.push_inplace(src_barrier).unwrap();
          queue_barriers.push_inplace(dst_barrier).unwrap();

          src_state.access_mask = Access::TransferRead;
          dst_state.access_mask = Access::TransferWrite;
          src_state.stage       = PipelineStages::Transfer;
          dst_state.stage       = PipelineStages::Transfer;
        }

        cmd_barriers.push(cmd.copy_buffer.copies.size() * 2).unwrap();
      }
      break;

      case CmdType::CopyHostBuffer:
      {
        BufferState &dst_state = graph.get_state(cmd.copy_host_buffer.dst);

        for (BufferCopy const &copy : cmd.copy_host_buffer.copies)
        {
          QueueBufferMemoryBarrier dst_barrier{.buffer          = cmd.copy_host_buffer.dst,
                                               .offset          = copy.dst_offset,
                                               .size            = copy.size,
                                               .src_stage_mask  = dst_state.stage,
                                               .dst_stage_mask  = PipelineStages::Transfer,
                                               .src_access_mask = dst_state.access_mask,
                                               .dst_access_mask = Access::TransferRead};

          queue_barriers.push_inplace(dst_barrier).unwrap();

          dst_state.access_mask = Access::TransferWrite;
          dst_state.stage       = PipelineStages::Transfer;
        }

        cmd_barriers.push(cmd.copy_buffer.copies.size()).unwrap();
      }
      break;

      case CmdType::CopyImage:
      {
        ImageState &src_state = graph.get_state(cmd.copy_image.src);
        ImageState &dst_state = graph.get_state(cmd.copy_image.dst);

        for (ImageCopy const &copy : cmd.copy_image.copies)
        {
          QueueImageMemoryBarrier src_barrier{.image           = cmd.copy_image.src,
                                              .first_mip_level = copy.src_mip_level,
                                              .num_mip_levels  = 1,
                                              .aspect          = copy.src_aspect,
                                              .old_layout      = src_state.layout,
                                              .new_layout      = ImageLayout::TransferSrcOptimal,
                                              .src_stage_mask  = src_state.stage,
                                              .dst_stage_mask  = PipelineStages::Transfer,
                                              .src_access_mask = src_state.access_mask,
                                              .dst_access_mask = Access::TransferRead};
          QueueImageMemoryBarrier dst_barrier{.image           = cmd.copy_image.dst,
                                              .first_mip_level = copy.dst_mip_level,
                                              .num_mip_levels  = 1,
                                              .aspect          = copy.dst_aspect,
                                              .old_layout      = dst_state.layout,
                                              .new_layout      = ImageLayout::TransferDstOptimal,
                                              .src_stage_mask  = dst_state.stage,
                                              .dst_stage_mask  = PipelineStages::Transfer,
                                              .src_access_mask = dst_state.access_mask,
                                              .dst_access_mask = Access::TransferWrite};

          queue_barriers.push_inplace(src_barrier).unwrap();
          queue_barriers.push_inplace(dst_barrier).unwrap();

          src_state.access_mask = Access::TransferRead;
          dst_state.access_mask = Access::TransferWrite;
          src_state.stage       = PipelineStages::Transfer;
          dst_state.stage       = PipelineStages::Transfer;
          src_state.layout      = ImageLayout::TransferSrcOptimal;
          dst_state.layout      = ImageLayout::TransferDstOptimal;
        }

        cmd_barriers.push(cmd.copy_image.copies.size() * 2).unwrap();
      }
      break;

      case CmdType::CopyBufferToImage:
      {
        // get latest expected state
        // convert to transfer dst layout and transfer write access with transfer stage
        // leave as-is. the next usage should conver it back to how it is needed if necessary
        BufferState &src_state = graph.get_state(cmd.copy_buffer_to_image.src);
        ImageState  &dst_state = graph.get_state(cmd.copy_buffer_to_image.dst);

        for (BufferImageCopy const &copy : cmd.copy_buffer_to_image.copies)
        {
          QueueBufferMemoryBarrier src_barrier{.buffer          = cmd.copy_buffer_to_image.src,
                                               .offset          = copy.buffer_offset,
                                               .size            = ((u32) copy.buffer_row_length) * copy.buffer_image_height,
                                               .src_stage_mask  = src_state.stage,
                                               .dst_stage_mask  = PipelineStages::Transfer,
                                               .src_access_mask = src_state.access_mask,
                                               .dst_access_mask = Access::TransferRead};
          QueueImageMemoryBarrier  dst_barrier{.image           = cmd.copy_buffer_to_image.dst,
                                               .first_mip_level = copy.image_mip_level,
                                               .num_mip_levels  = 1,
                                               .aspect          = copy.image_aspect,
                                               .old_layout      = dst_state.layout,
                                               .new_layout      = ImageLayout::TransferDstOptimal,
                                               .src_stage_mask  = dst_state.stage,
                                               .dst_stage_mask  = PipelineStages::Transfer,
                                               .src_access_mask = dst_state.access_mask,
                                               .dst_access_mask = Access::TransferWrite};

          queue_barriers.push_inplace(src_barrier).unwrap();
          queue_barriers.push_inplace(dst_barrier).unwrap();

          src_state.access_mask = Access::TransferRead;
          dst_state.access_mask = Access::TransferWrite;
          src_state.stage       = PipelineStages::Transfer;
          dst_state.stage       = PipelineStages::Transfer;
          dst_state.layout      = ImageLayout::TransferDstOptimal;
        }

        cmd_barriers.push(cmd.copy_buffer_to_image.copies.size() * 2).unwrap();
      }

      break;

      case CmdType::BlitImage:
      {
        // check RID
        // check current layout and usage, and all accessors
        // check all memory aliasing
        // convert layout to transfer dst and write access with whatever access type is needed for blitting
        // update barrier tracker
        // on next usage in command buffer, get the last usage and update accordingly
        ImageState &src_state = graph.get_state(cmd.blit_image.src);
        ImageState &dst_state = graph.get_state(cmd.blit_image.dst);

        for (ImageBlit const &blit : cmd.blit_image.blits)
        {
          QueueImageMemoryBarrier src_barrier{.image           = cmd.blit_image.src,
                                              .first_mip_level = blit.src_mip_level,
                                              .num_mip_levels  = 1,
                                              .aspect          = blit.src_aspect,
                                              .old_layout      = src_state.layout,
                                              .new_layout      = ImageLayout::TransferSrcOptimal,
                                              .src_stage_mask  = src_state.stage,
                                              .dst_stage_mask  = PipelineStages::Transfer,
                                              .src_access_mask = src_state.access_mask,
                                              .dst_access_mask = Access::TransferRead};
          QueueImageMemoryBarrier dst_barrier{.image           = cmd.blit_image.dst,
                                              .first_mip_level = blit.dst_mip_level,
                                              .num_mip_levels  = 1,
                                              .aspect          = blit.dst_aspect,
                                              .old_layout      = dst_state.layout,
                                              .new_layout      = ImageLayout::TransferDstOptimal,
                                              .src_stage_mask  = dst_state.stage,
                                              .dst_stage_mask  = PipelineStages::Transfer,
                                              .src_access_mask = dst_state.access_mask,
                                              .dst_access_mask = Access::TransferWrite};

          queue_barriers.push_inplace(src_barrier).unwrap();
          queue_barriers.push_inplace(dst_barrier).unwrap();

          src_state.access_mask = Access::TransferRead;
          dst_state.access_mask = Access::TransferWrite;
          src_state.stage       = PipelineStages::Transfer;
          dst_state.stage       = PipelineStages::Transfer;
          src_state.layout      = ImageLayout::TransferSrcOptimal;
          dst_state.layout      = ImageLayout::TransferDstOptimal;
        }

        cmd_barriers.push(cmd.blit_image.blits.size() * 2).unwrap();
      }
      break;

        // case CmdType::DispatchTask:
        // {
        //   for (ResourceBinding const &binding : cmd.dispatch_task.bindings)
        //   {
        //     switch (binding.type)
        //     {
        //       case ResourceBindingType::BufferBinding:
        //       {
        //         BufferState             &state = graph.get_state(binding.buffer.buffer);
        //         QueueBufferMemoryBarrier barrier{.buffer          = binding.buffer.buffer,
        //                                          .offset          = 0,
        //                                          .size            = stx::U64_MAX,
        //                                          .src_stage_mask  = state.stage,
        //                                          .dst_stage_mask  = binding.buffer.stages,
        //                                          .src_access_mask = state.access_mask,
        //                                          .dst_access_mask = binding.buffer.access};

        //         queue_barriers.push_inplace(barrier).unwrap();
        //         state.access_mask = binding.buffer.access;
        //         state.stage       = binding.buffer.stages;
        //       }
        //       break;
        //       case ResourceBindingType::ImageViewBinding:
        //       {
        //         ImageViewDesc           sub_desc   = graph.get_desc(binding.image_view.image_view);
        //         ImageState             &state      = graph.get_state(sub_desc.image);
        //         bool                    is_storage = (binding.image_view.access & (Access::ShaderStorageWrite | Access::ShaderWrite | Access::ShaderSampledWrite)) != Access::None;
        //         QueueImageMemoryBarrier barrier{.image           = sub_desc.image,
        //                                         .first_mip_level = sub_desc.first_mip_level,
        //                                         .num_mip_levels  = sub_desc.num_mip_levels,
        //                                         .aspect          = sub_desc.aspect,
        //                                         .old_layout      = state.layout,
        //                                         .new_layout      = is_storage ? ImageLayout::General : ImageLayout::ShaderReadOnlyOptimal,
        //                                         .src_stage_mask  = state.stage,
        //                                         .dst_stage_mask  = binding.image_view.stages,
        //                                         .src_access_mask = state.access_mask,
        //                                         .dst_access_mask = binding.image_view.access};

        //         queue_barriers.push_inplace(barrier).unwrap();
        //         state.access_mask = binding.image_view.access;
        //         state.stage       = binding.image_view.stages;
        //       }
        //       break;

        //       default:
        //         break;
        //     }
        //   }

        //   cmd_barriers.push(cmd.dispatch_task.bindings.size()).unwrap();        // TODO(lamarrr)
        // }
        // break;

        // TODO(lamarrr): in-between barriers access??? i.e. in shader?
      case CmdType::BeginRenderPass:
      {
        FramebufferDesc framebuffer_desc = graph.get_desc(cmd.begin_render_pass.framebuffer);
        for (ImageView attachment : framebuffer_desc.color_attachments)
        {
          ImageState &state = graph.get_state(graph.get_desc(attachment).image);                 // TODO(lamarrr): color attachment may not be written to depending on the renderpass ops
          state.access_mask = Access::ColorAttachmentRead | Access::ColorAttachmentWrite;        // LoadOp and StoreOp
          state.stage       = PipelineStages::ColorAttachmentOutput | PipelineStages::EarlyFragmentTests;
          state.layout      = ImageLayout::ColorAttachmentOptimal;
        }

        for (ImageView attachment : framebuffer_desc.depth_stencil_attachments)
        {
          ImageState &state = graph.get_state(graph.get_desc(attachment).image);
          state.access_mask = Access::DepthStencilAttachmentRead | Access::DepthStencilAttachmentWrite;
          state.stage       = PipelineStages::ColorAttachmentOutput | PipelineStages::EarlyFragmentTests;
          state.layout      = ImageLayout::DepthStencilAttachmentOptimal;
        }
      }
      break;

      case CmdType::EndRenderPass:
      {
        // TODO(lamarrr): post-renderpass state transitions
        // TODO(lamarrr): DontCare generates write access to the attachment in renderpasses
      }
      break;

      default:
        break;
    }
  }
}

}        // namespace lgfx
}        // namespace ash
