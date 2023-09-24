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

struct ScreenPass
{
  struct Arguments
  {
    Extent extent;
    Format format;
    bool   suboptimal  = false;        // updated by vulkan
    u32    num_buffers = 1;
  } arguments;

  struct Resources
  {
    Image       color_images[16];        // screen has implicit pass to present the screen_color_image
    Image       depth_stencil_images[16];
    RenderPass  render_passes[16];
    Framebuffer framebuffers[16];
  } resources;

  struct State
  {
  } state;

  struct Bindings
  {
    u32 image_index = 0;
  } bindings;

  // to check if to recreate resources
  bool diff(Graph const &graph, Arguments const &new_args)
  {
    return false;
  }

  void init(Graph &graph, CmdBuffer &cmd_buffer)
  {
    // RENDER
    // transition color attachment layout from presentation optimal to color attachment optimal
    //
    //
    // perform intermediate rendering operations
    //
    //
    // transition color attachment layout from color_attachment optimal to presentation optimal
    // THIS IS POINTLESSSS, it is on-screen
    // TODO(lamarrr): graph check?

    // ASH_CHECK( ctx.screen_pass.ctx.num_buffers <= 16);

    // for (u32 i = 0; i < ctx.screen_pass.ctx.num_buffers; i++)
    // {
    //   rid                   color_image = graph.create_image(ImageDesc{.format = ctx.screen_pass.ctx.format,
    //                                                                    .usages = ImageUsages::ColorAttachment,
    //                                                                    .size   = ctx.screen_pass.ctx.extent,
    //                                                                    .mips   = 1});
    //   FramebufferAttachment color_attachment{.image    = color_image,
    //                                          .load_op  = LoadOp::Clear,
    //                                          .store_op = StoreOp::Store};

    //   rid depth_stencil_image = graph.create_image(ImageDesc{.format = Format::D16_Unorm,
    //                                                          .usages = ImageUsages::DepthStencilAttachment,
    //                                                          .size   = ctx.screen_pass.ctx.extent,
    //                                                          .mips   = 1});

    //   FramebufferAttachment depth_stencil_attachment{.image    = depth_stencil_image,
    //                                                  .load_op  = LoadOp::Clear,
    //                                                  .store_op = StoreOp::Store};

    //   rid framebuffer                                   = graph.create_framebuffer(RenderPassDesc{.render_pass   = render_pass,
    //                                                                                               .color         = color_image,
    //                                                                                               .depth_stencil = depth_stencil_image});
    //   ctx.screen_pass.resources.color_images[i]         = color_image;
    //   ctx.screen_pass.resources.depth_stencil_images[i] = depth_stencil_image;
    //   ctx.screen_pass.resources.render_passes[i]        = render_pass;
    //   ctx.screen_pass.resources.framebuffers[i]         = framebuffer;
    // }
    // record render ops
  }

  void execute(Graph &graph, CmdBuffer &cmd_buffer)
  {
  }
};


inline void clipped_draw_pass()
{
}


inline void outline3d_pass()
{
  // SETUP
  // create depth attachment
  // RENDER
  // clear depth attachment
  // disable depth test and depth buffer
  // draw commands using colors only
  // enable depth test and depth buffer
  // draw object
}

inline void chromatic_aberration_pass()
{
  // https://www.shadertoy.com/view/Mds3zn
  // SETUP
  // RENDER
}

inline void effect_pass()
{
  // SETUP
  // RENDER
}

}        // namespace lgfx
}        // namespace ash
