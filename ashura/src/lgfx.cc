#include "ashura/lgfx.h"

namespace ash
{
namespace lgfx
{

Resource Graph::create()
{
  if (resources.is_empty())
  {
    resources.push(ResourceDesc{}).unwrap();
    resource_states.push_inplace(ResourceState{}).unwrap();
  }

  return (Resource) free_indices.pop().unwrap_or_else([&]() {
    u32 index = resources.size();
    resources.push(ResourceDesc{}).unwrap();
    resource_states.push_inplace(ResourceState{}).unwrap();
    return index;
  });
}

Buffer Graph::create_buffer(BufferDesc const &desc)
{
  Resource resource               = create();
  resources[(u32) resource]       = desc;
  resource_states[(u32) resource] = BufferState{};
  return (Buffer) resource;
}

Image Graph::create_image(ImageDesc const &desc)
{
  Resource resource               = create();
  resources[(u32) resource]       = desc;
  resource_states[(u32) resource] = ImageState{};
  return (Image) resource;
}

ImageView Graph::create_image_view(ImageViewDesc const &desc)
{
  Resource resource         = create();
  resources[(u32) resource] = desc;
  return (ImageView) resource;
}

RenderPass Graph::create_render_pass(RenderPassDesc const &desc)
{
  Resource resource         = create();
  resources[(u32) resource] = desc;
  return (RenderPass) resource;
}

Framebuffer Graph::create_framebuffer(FramebufferDesc const &desc)
{
  Resource resource         = create();
  resources[(u32) resource] = desc;
  return (Framebuffer) resource;
}

BufferDesc Graph::get_desc(Buffer buffer) const
{
  return resources[(u32) buffer].buffer;
}

ImageDesc Graph::get_desc(Image image) const
{
  return resources[(u32) image].image;
}

ImageViewDesc Graph::get_desc(ImageView image_view) const
{
  return resources[(u32) image_view].image_view;
}

RenderPassDesc Graph::get_desc(RenderPass render_pass) const
{
  return resources[(u32) render_pass].render_pass;
}

FramebufferDesc Graph::get_desc(Framebuffer framebuffer) const
{
  return resources[(u32) framebuffer].framebuffer;
}

BufferState &Graph::get_state(Buffer buffer)
{
  return resource_states[(u32) buffer].buffer;
}

ImageState &Graph::get_state(Image image)
{
  return resource_states[(u32) image].image;
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

void Graph::validate_resources()
{
  // TODO(lamarrr): we need a logger
  for (ResourceDesc const &desc : resources)
  {
    switch (desc.type)
    {
      case ResourceType::None:
        break;
      case ResourceType::Buffer:
      {
        ASH_CHECK(desc.buffer.usages != BufferUsages::None);
        ASH_CHECK(desc.buffer.size > 0);
        ASH_CHECK(desc.buffer.properties != MemoryProperties::None);
        ASH_CHECK(ctx.device_info.memory_heaps.has_memory(desc.buffer.properties));
      }
      break;
      case ResourceType::Image:
      {
        ASH_CHECK(desc.image.extent.is_visible());
        ASH_CHECK(desc.image.usages != ImageUsages::None);
        ASH_CHECK(desc.image.mips >= 1);
        ASH_CHECK(desc.image.format != Format::Undefined);
      }
      break;
      case ResourceType::ImageView:
      {
        ASH_CHECK(desc.image_view.aspect != ImageAspect::None);
        ASH_CHECK(desc.image_view.view_format != Format::Undefined);
        ASH_CHECK(desc.image_view.image != Image::None);
        ASH_CHECK(desc.image_view.num_mip_levels >= 1);
        ASH_CHECK(((u32) desc.image_view.image) < resources.size());
        ASH_CHECK(resources[(u32) desc.image_view.image].type == ResourceType::Image);
        ImageDesc const resource = get_desc(desc.image_view.image);
        ASH_CHECK(desc.image_view.first_mip_level < resource.mips);
        ASH_CHECK((desc.image_view.first_mip_level + desc.image_view.num_mip_levels) <= resource.mips);
      }
      break;
      case ResourceType::RenderPass:
      {
        for (RenderPassAttachment const &attachment : desc.render_pass.color_attachments)
        {
          ASH_CHECK(attachment.format != Format::Undefined);
        }
        for (RenderPassAttachment const &attachment : desc.render_pass.depth_stencil_attachments)
        {
          ASH_CHECK(attachment.format != Format::Undefined);
        }
      }
      break;
      case ResourceType::Framebuffer:
      {
        ASH_CHECK(desc.framebuffer.renderpass != RenderPass::None);
        ASH_CHECK(resources[(u32) desc.framebuffer.renderpass].type == ResourceType::RenderPass);
        ASH_CHECK(desc.framebuffer.extent.is_visible());
        RenderPassDesc const render_pass_desc = get_desc(desc.framebuffer.renderpass);
        ASH_CHECK(desc.framebuffer.color_attachments.size() == render_pass_desc.color_attachments.size());
        ASH_CHECK(desc.framebuffer.depth_stencil_attachments.size() == render_pass_desc.depth_stencil_attachments.size());
        for (usize i = 0; i < desc.framebuffer.color_attachments.size(); i++)
        {
          ImageView attachment = desc.framebuffer.color_attachments[i];
          ASH_CHECK(attachment != ImageView::None);
          ASH_CHECK(((u32) attachment) < resources.size());
          ImageViewDesc const attachment_desc = get_desc(attachment);
          ASH_CHECK(attachment_desc.aspect == ImageAspect::Color);
          ASH_CHECK(attachment_desc.num_mip_levels >= 1);
          ASH_CHECK(render_pass_desc.color_attachments[i].format == attachment_desc.view_format);
          ImageDesc const image_desc = get_desc(attachment_desc.image);
          ASH_CHECK((image_desc.usages & ImageUsages::ColorAttachment) != ImageUsages::None);
          ASH_CHECK(image_desc.extent.width >= desc.framebuffer.extent.width);
          ASH_CHECK(image_desc.extent.height >= desc.framebuffer.extent.height);
        }
        for (usize i = 0; i < desc.framebuffer.depth_stencil_attachments.size(); i++)
        {
          ImageView attachment = desc.framebuffer.depth_stencil_attachments[i];
          ASH_CHECK(attachment != ImageView::None);
          ASH_CHECK(((u32) attachment) < resources.size());
          ImageViewDesc const attachment_desc = get_desc(attachment);
          ASH_CHECK((attachment_desc.aspect & (ImageAspect::Depth | ImageAspect::Stencil)) != ImageAspect::None);
          ASH_CHECK(attachment_desc.num_mip_levels >= 1);
          ASH_CHECK(render_pass_desc.depth_stencil_attachments[i].format == attachment_desc.view_format);
          ImageDesc const image_desc = get_desc(attachment_desc.image);
          ASH_CHECK((image_desc.usages & ImageUsages::DepthStencilAttachment) != ImageUsages::None);
          ASH_CHECK(image_desc.extent.width >= desc.framebuffer.extent.width);
          ASH_CHECK(image_desc.extent.height >= desc.framebuffer.extent.height);
        }
      }
      break;
    }
  }
}

void validate_commands(Graph const &graph, stx::Span<Cmd const> cmds)
{
  for (Cmd const &cmd : cmds)
  {
    switch (cmd.type)
    {
      case CmdType::None:
        break;
      case CmdType::CopyBuffer:
      {
        ASH_CHECK(!cmd.copy_buffer.copies.is_empty());
        ASH_CHECK(cmd.copy_buffer.src != Buffer::None);
        ASH_CHECK(((u32) cmd.copy_buffer.src) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.copy_buffer.src].type == ResourceType::Buffer);
        ASH_CHECK(cmd.copy_buffer.dst != Buffer::None);
        ASH_CHECK(((u32) cmd.copy_buffer.dst) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.copy_buffer.dst].type == ResourceType::Buffer);
        BufferDesc const src_desc = graph.get_desc(cmd.copy_buffer.src);
        BufferDesc const dst_desc = graph.get_desc(cmd.copy_buffer.dst);
        ASH_CHECK((src_desc.usages & BufferUsages::TransferSrc) != BufferUsages::None);
        ASH_CHECK((dst_desc.usages & BufferUsages::TransferDst) != BufferUsages::None);
        for (BufferCopy const &copy : cmd.copy_buffer.copies)
        {
          ASH_CHECK(src_desc.size > copy.src_offset);
          ASH_CHECK(src_desc.size >= (copy.src_offset + copy.size));
          ASH_CHECK(dst_desc.size > copy.dst_offset);
          ASH_CHECK(dst_desc.size >= (copy.dst_offset + copy.size));
        }
      }
      break;
      case CmdType::CopyHostBuffer:
      {
        ASH_CHECK(!cmd.copy_host_buffer.copies.is_empty());
        ASH_CHECK(!cmd.copy_host_buffer.src.is_empty());
        ASH_CHECK(cmd.copy_host_buffer.dst != Buffer::None);
        ASH_CHECK(((u32) cmd.copy_host_buffer.dst) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.copy_host_buffer.dst].type == ResourceType::Buffer);
        stx::Span        src      = cmd.copy_host_buffer.src;
        BufferDesc const dst_desc = graph.get_desc(cmd.copy_host_buffer.dst);
        ASH_CHECK((dst_desc.usages & BufferUsages::TransferDst) != BufferUsages::None);
        for (BufferCopy const &copy : cmd.copy_host_buffer.copies)
        {
          ASH_CHECK(src.size() > copy.src_offset);
          ASH_CHECK(src.size() >= (copy.src_offset + copy.size));
          ASH_CHECK(dst_desc.size > copy.dst_offset);
          ASH_CHECK(dst_desc.size >= (copy.dst_offset + copy.size));
        }
      }
      break;
      case CmdType::CopyImage:
      {
        ASH_CHECK(!cmd.copy_image.copies.is_empty());
        ASH_CHECK(cmd.copy_image.src != Image::None);
        ASH_CHECK(((u32) cmd.copy_image.src) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.copy_image.src].type == ResourceType::Image);
        ASH_CHECK(cmd.copy_image.dst != Image::None);
        ASH_CHECK(((u32) cmd.copy_image.dst) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.copy_image.dst].type == ResourceType::Image);
        ImageDesc const src_desc = graph.get_desc(cmd.copy_image.src);
        ImageDesc const dst_desc = graph.get_desc(cmd.copy_image.dst);
        ASH_CHECK((src_desc.usages & ImageUsages::TransferSrc) != ImageUsages::None);
        ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);

        for (ImageCopy const &copy : cmd.copy_image.copies)
        {
          ASH_CHECK(copy.src_aspect != ImageAspect::None);
          ASH_CHECK(copy.dst_aspect != ImageAspect::None);
          ASH_CHECK(copy.src_mip_level < src_desc.mips);
          ASH_CHECK(copy.dst_mip_level < dst_desc.mips);
          ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(copy.src_area)));
          ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(copy.src_area.with_offset(copy.dst_offset))));
        }
      }
      break;
      case CmdType::CopyBufferToImage:
      {
        ASH_CHECK(!cmd.copy_buffer_to_image.copies.is_empty());
        ASH_CHECK(cmd.copy_buffer_to_image.src != Buffer::None);
        ASH_CHECK(((u32) cmd.copy_buffer_to_image.src) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.copy_buffer_to_image.src].type == ResourceType::Buffer);
        ASH_CHECK(cmd.copy_buffer_to_image.dst != Image::None);
        ASH_CHECK(((u32) cmd.copy_buffer_to_image.dst) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.copy_buffer_to_image.dst].type == ResourceType::Image);
        BufferDesc const src_desc = graph.get_desc(cmd.copy_buffer_to_image.src);
        ImageDesc const  dst_desc = graph.get_desc(cmd.copy_buffer_to_image.dst);
        ASH_CHECK((src_desc.usages & BufferUsages::TransferSrc) != BufferUsages::None);
        ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);
        for (BufferImageCopy const &copy : cmd.copy_buffer_to_image.copies)
        {
          ASH_CHECK(copy.buffer_image_height > 0);
          ASH_CHECK(copy.buffer_row_length > 0);
          ASH_CHECK(copy.buffer_offset < src_desc.size);
          ASH_CHECK(copy.image_mip_level < dst_desc.mips);
          ASH_CHECK(copy.image_aspect != ImageAspect::None);
          ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(copy.image_area)));
        }
      }
      break;
      case CmdType::BlitImage:
      {
        ASH_CHECK(!cmd.blit_image.blits.is_empty());
        ASH_CHECK(cmd.blit_image.src != Image::None);
        ASH_CHECK(((u32) cmd.blit_image.src) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.blit_image.src].type == ResourceType::Image);
        ASH_CHECK(cmd.blit_image.dst != Image::None);
        ASH_CHECK(((u32) cmd.blit_image.dst) < graph.resources.size());
        ASH_CHECK(graph.resources[(u32) cmd.blit_image.dst].type == ResourceType::Image);
        ImageDesc const src_desc = graph.get_desc(cmd.blit_image.src);
        ImageDesc const dst_desc = graph.get_desc(cmd.blit_image.dst);
        ASH_CHECK((src_desc.usages & ImageUsages::TransferSrc) != ImageUsages::None);
        ASH_CHECK((dst_desc.usages & ImageUsages::TransferDst) != ImageUsages::None);
        for (ImageBlit const &blit : cmd.blit_image.blits)
        {
          ASH_CHECK(blit.src_aspect != ImageAspect::None);
          ASH_CHECK(blit.dst_aspect != ImageAspect::None);
          ASH_CHECK(blit.src_mip_level < src_desc.mips);
          ASH_CHECK(blit.dst_mip_level < dst_desc.mips);
          ASH_CHECK((URect{.offset = {}, .extent = src_desc.extent}.contains(blit.src_area)));
          ASH_CHECK((URect{.offset = {}, .extent = dst_desc.extent}.contains(blit.dst_area)));
        }
      }
      break;
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
      case CmdType::BeginRenderPass:
      {
        ASH_CHECK(cmd.begin_render_pass.render_pass != RenderPass::None);
        ASH_CHECK(graph.resources[(u32) cmd.begin_render_pass.render_pass].type == ResourceType::RenderPass);
        ASH_CHECK(cmd.begin_render_pass.framebuffer != Framebuffer::None);
        ASH_CHECK(graph.resources[(u32) cmd.begin_render_pass.framebuffer].type == ResourceType::Framebuffer);
        FramebufferDesc const framebuffer_desc = graph.get_desc(cmd.begin_render_pass.framebuffer);
        ASH_CHECK(framebuffer_desc.color_attachments.size() == cmd.begin_render_pass.color_attachments_clear_values.size());
        ASH_CHECK(framebuffer_desc.depth_stencil_attachments.size() == cmd.begin_render_pass.depth_stencil_attachments_clear_values.size());
      }
      break;
      case CmdType::EndRenderPass:
      {
        // TODO(lamarrr)
      }
      break;
      default:
        break;
    }
  }
}

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
