#pragma once
#include "ashura/renderer/passes/bloom.h"
#include "ashura/renderer/passes/blur.h"
#include "ashura/renderer/passes/custom.h"
#include "ashura/renderer/passes/fxaa.h"
#include "ashura/renderer/passes/msaa.h"
#include "ashura/renderer/passes/pbr.h"
#include "ashura/renderer/passes/rrect.h"
#include "ashura/renderer/render_context.h"

namespace ash
{

// sky render pass
// render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
// perform bloom, blur, msaa on 3d scene
// render UI pass + custom shaders, blur ???
// copy and composite 3d and 2d scenes

struct Renderer
{
  BloomPass        bloom;
  BlurPass         blur;
  FXAAPass         fxaa;
  MSAAPass         msaa;
  PBRPass          pbr;
  CustomShaderPass custom;
  RRectPass        rrect;
  RenderContext    ctx;

  void init(gfx::DeviceImpl p_device, bool p_use_hdr,
            u32 p_max_frames_in_flight, gfx::Extent p_initial_extent,
            ShaderMap p_shader_map)
  {
    ctx.init(p_device, p_use_hdr, p_max_frames_in_flight, p_initial_extent,
             p_shader_map);
    bloom.init(ctx);
    blur.init(ctx);
    fxaa.init(ctx);
    msaa.init(ctx);
    pbr.init(ctx);
    custom.init(ctx);
    rrect.init(ctx);
  }

  void uninit()
  {
    ctx.device->wait_idle(ctx.device.self).unwrap();
    bloom.uninit(ctx);
    blur.uninit(ctx);
    fxaa.uninit(ctx);
    msaa.uninit(ctx);
    pbr.uninit(ctx);
    custom.uninit(ctx);
    rrect.uninit(ctx);
    ctx.uninit();
  }

  void begin_frame(gfx::Swapchain swapchain)
  {
    ctx.begin_frame(swapchain);
  }

  void end_frame(gfx::Swapchain swapchain)
  {
    ctx.end_frame(swapchain);
  }

  void record_frame(gfx::Image img, gfx::ImageView view, gfx::DescriptorSet set)
  {
    auto enc = ctx.encoder();

    enc->clear_color_image(
        enc.self, ctx.framebuffer.color_image,
        gfx::Color{.float32 = {1, 1, 1, 1}},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                            .first_mip_level   = 0,
                                            .num_mip_levels    = 1,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1}}));

    enc->clear_color_image(
        enc.self, img, gfx::Color{.float32 = {1, 1, 1, 1}},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                            .first_mip_level   = 0,
                                            .num_mip_levels    = 1,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1}}));

    rrect.add_pass(
        ctx,
        RRectPassParams{
            .render_target =
                RenderTarget{.color_images =
                                 to_span({ctx.framebuffer.color_image_view}),
                             .depth_stencil_image =
                                 ctx.framebuffer.depth_stencil_image_view,
                             .depth_stencil_aspects = gfx::ImageAspects::Depth,
                             .extent                = ctx.framebuffer.extent,
                             .render_offset         = {0, 0},
                             .render_extent         = ctx.framebuffer.extent},
            .objects = to_span<RRectObject>(
                {{.descriptor = set,
                  .uniform    = ctx.push_uniform(RRectShaderUniform{
                         .transform =
                          ViewTransform{
                                 .model = affine_scale3d({.8, .75, 1}) *
                                       affine_rotate3d_z(0.5),
                                 .view = affine_scale3d({1080.0 / 1920, 1, 1}),
                                 .projection = Mat4::identity()},
                         .radii        = {.2, .2, .2, .2},
                         .uv           = {{0, 0}, {1, 1}},
                         .tint         = {{1, 0, 1, 1},
                                          {1, 0, 0, 1},
                                          {0, 0, 1, 1},
                                          {1, 1, 1, 1}},
                         .aspect_ratio = {1, .75 / .8}})}})});

    pbr.add_pass(
        ctx,
        PBRPassParams{
            .render_target =
                RenderTarget{.color_images =
                                 to_span({ctx.framebuffer.color_image_view}),
                             .depth_stencil_image =
                                 ctx.framebuffer.depth_stencil_image_view,
                             .depth_stencil_aspects = gfx::ImageAspects::Depth,
                             .extent                = ctx.framebuffer.extent,
                             .render_offset         = {0, 0},
                             .render_extent         = ctx.framebuffer.extent},
            .objects = to_span<PBRObject>({})});
  }
};

}        // namespace ash
