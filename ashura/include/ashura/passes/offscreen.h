#pragma once
#include <string_view>

#include "ashura/lgfx.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "stx/fn.h"
#include "stx/vec.h"

namespace ash
{
namespace lgfx
{

struct OffscreenPass
{
  struct Arguments
  {
    ImageDesc color_attachment_desc;
    LoadOp    color_load_op  = LoadOp::DontCare;
    StoreOp   color_store_op = StoreOp::DontCare;
    ImageDesc depth_stencil_attachment_desc;
    LoadOp    depth_stencil_load_op  = LoadOp::DontCare;
    StoreOp   depth_stencil_store_op = StoreOp::DontCare;
  } arguments;

  struct Resources
  {
    Image       color_images[1]              = {Image::None};
    ImageView   color_image_views[1]         = {ImageView::None};
    Image       depth_stencil_images[1]      = {Image::None};
    ImageView   depth_stencil_image_views[1] = {ImageView::None};
    RenderPass  render_pass                  = RenderPass::None;
    Framebuffer framebuffer                  = Framebuffer::None;
  } resources;

  struct State
  {
    RenderPassAttachment color_attachments[1];
    RenderPassAttachment depth_stencil_attachments[1];
    Color                clear_colors[1]         = {Color{.uint32 = {0, 0, 0, 0}}};
    DepthStencil         clear_depth_stencils[1] = {DepthStencil{.depth = 0, .stencil = 0}};
  } state;

  // bindings don't require changes to the resources, and can change for every task execution
  // these are input or output bindings
  struct Bindings
  {
  } bindings;

  // to check if to recreate resources
  bool diff(Graph const &graph, Arguments const &new_args)
  {
    return false;
  }

  void init(Graph &graph, CmdBuffer &cmd_buffer)
  {
    // SETUP
    //
    // get the number maximum number of offscreen draw passes in the scene = N
    //
    // create N color output render targets with undefined layout
    // optionally create N depth stencil output render targets with undefined layout
    // left to the pipeline to determine the inputs???
    //

    bool has_color         = arguments.color_attachment_desc.format != Format::Undefined;
    bool has_depth_stencil = arguments.depth_stencil_attachment_desc.format != Format::Undefined;

    arguments.color_attachment_desc.usages |= ImageUsages::ColorAttachment;
    arguments.depth_stencil_attachment_desc.usages |= ImageUsages::DepthStencilAttachment;

    if (has_color)
    {
      resources.color_images[0]      = graph.create_image(arguments.color_attachment_desc);
      resources.color_image_views[0] = graph.create_image_view(ImageViewDesc{.image           = resources.color_images[0],
                                                                             .view_format     = arguments.color_attachment_desc.format,
                                                                             .mapping         = ComponentMapping{},
                                                                             .first_mip_level = 0,
                                                                             .num_mip_levels  = 1,
                                                                             .aspect          = ImageAspect::Color});
    }

    if (has_depth_stencil)
    {
      resources.depth_stencil_images[0]      = graph.create_image(arguments.depth_stencil_attachment_desc);
      resources.depth_stencil_image_views[0] = graph.create_image_view(ImageViewDesc{.image           = resources.depth_stencil_images[0],
                                                                                     .view_format     = arguments.depth_stencil_attachment_desc.format,
                                                                                     .mapping         = ComponentMapping{},
                                                                                     .first_mip_level = 0,
                                                                                     .num_mip_levels  = 1,
                                                                                     .aspect          = ImageAspect::Depth | ImageAspect::Stencil});
    }

    // TODO(lamarrr): what if we need to change the renderpass?
    // or re-use the resources

    state.color_attachments[0]         = RenderPassAttachment{.format   = arguments.color_attachment_desc.format,
                                                              .load_op  = arguments.color_load_op,
                                                              .store_op = arguments.color_store_op};
    state.depth_stencil_attachments[0] = RenderPassAttachment{.format   = arguments.depth_stencil_attachment_desc.format,
                                                              .load_op  = arguments.depth_stencil_load_op,
                                                              .store_op = arguments.depth_stencil_store_op};
    resources.render_pass              = graph.create_render_pass(RenderPassDesc{.color_attachments         = has_color ? stx::Span{state.color_attachments} : stx::Span<RenderPassAttachment>{},
                                                                                 .depth_stencil_attachments = has_depth_stencil ? stx::Span{state.depth_stencil_attachments} : stx::Span<RenderPassAttachment>{}});
    resources.framebuffer              = graph.create_framebuffer(FramebufferDesc{.renderpass                = resources.render_pass,
                                                                                  .color_attachments         = has_color ? stx::Span{resources.color_image_views} : stx::Span<ImageView>{},
                                                                                  .depth_stencil_attachments = has_depth_stencil ? stx::Span{resources.depth_stencil_image_views} : stx::Span<ImageView>{}});
  }

  void execute(Graph &graph, CmdBuffer &cmd_buffer)
  {
    //
    // for all N outputs insert barrier to convert from used or newly created layout to color attachment output layout
    //
    // for each N batch:
    //
    // for each z-sorted offscreen render pass:
    //
    //
    // RENDER
    // perform all intermediate rendering operations
    //
    // transition layout of color render target to shader read or transfer src or dst
    //
    // render to target
    // insert barrier to convert layout back to color attachment output
    //
    // we might want to leave the final image layout or state until completion of the pipeline as we don't know how exactly the will be used
    //

    bool has_color         = arguments.color_attachment_desc.format != Format::Undefined;
    bool has_depth_stencil = arguments.depth_stencil_attachment_desc.format != Format::Undefined;

    cmd_buffer.add(cmd::BeginRenderPass{.framebuffer                            = resources.framebuffer,
                                        .render_pass                            = resources.render_pass,
                                        .render_area                            = IRect{.offset = {0, 0}, .extent = arguments.color_attachment_desc.extent},
                                        .color_attachments_clear_values         = has_color ? stx::Span{state.clear_colors} : stx::Span<Color>{},
                                        .depth_stencil_attachments_clear_values = has_depth_stencil ? stx::Span{state.clear_depth_stencils} : stx::Span<DepthStencil>{}});
    cmd_buffer.add(cmd::DispatchTask{.index       = 0,
                                     .type        = PipelineType::Graphics,
                                     .bindings    = {},
                                     .framebuffer = resources.framebuffer});        // TODO(lamarrr): what are we using framebuffer for here?>
    cmd_buffer.add(cmd::EndRenderPass{});
  }
};
}        // namespace lgfx
}        // namespace ash
