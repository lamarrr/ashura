#pragma once
#include "ashura/renderer/passes/bloom.h"
#include "ashura/renderer/passes/blur.h"
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
  BloomPass          bloom;
  BlurPass           blur;
  FXAAPass           fxaa;
  MSAAPass           msaa;
  PBRPass            pbr;
  RRectPass          rrect;
  RenderContext      ctx;
  gfx::Buffer        params_buffer = nullptr;
  gfx::Image         image         = nullptr;
  gfx::ImageView     texture       = nullptr;
  gfx::Sampler       sampler       = nullptr;
  gfx::DescriptorSet params_ssbo   = nullptr;
  gfx::DescriptorSet textures      = nullptr;

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
    rrect.init(ctx);

    params_buffer =
        ctx.device
            ->create_buffer(
                ctx.device.self,
                gfx::BufferDesc{.label = "SSBO Buffer"_span,
                                .size  = sizeof(RRectParam) * 20,
                                .usage = gfx::BufferUsage::StorageBuffer |
                                         gfx::BufferUsage::TransferDst})
            .unwrap();

    image = ctx.device
                ->create_image(
                    ctx.device.self,
                    gfx::ImageDesc{.type   = gfx::ImageType::Type2D,
                                   .format = gfx::Format::B8G8R8A8_UNORM,
                                   .usage  = gfx::ImageUsage::Sampled |
                                            gfx::ImageUsage::TransferDst,
                                   .aspects      = gfx::ImageAspects::Color,
                                   .extent       = {1, 1, 1},
                                   .mip_levels   = 1,
                                   .array_layers = 1,
                                   .sample_count = gfx::SampleCount::Count1})
                .unwrap();

    texture =
        ctx.device
            ->create_image_view(
                ctx.device.self,
                gfx::ImageViewDesc{.image       = image,
                                   .view_type   = gfx::ImageViewType::Type2D,
                                   .view_format = gfx::Format::B8G8R8A8_UNORM,
                                   .aspects     = gfx::ImageAspects::Color,
                                   .first_mip_level   = 0,
                                   .num_mip_levels    = 1,
                                   .first_array_layer = 0,
                                   .num_array_layers  = 1})
            .unwrap();

    textures = ctx.device
                   ->create_descriptor_set(ctx.device.self, ctx.textures_layout,
                                           to_span({16ui32}))
                   .unwrap();

    params_ssbo =
        ctx.device->create_descriptor_set(ctx.device.self, ctx.ssbo_layout, {})
            .unwrap();

    sampler = ctx.device->create_sampler(ctx.device.self, gfx::SamplerDesc{})
                  .unwrap();

    ctx.device->update_descriptor_set(
        ctx.device.self, gfx::DescriptorSetUpdate{
                             .set     = textures,
                             .binding = 0,
                             .element = 0,
                             .images  = to_span({gfx::ImageBinding{
                                  .sampler = sampler, .image_view = texture}})});

    ctx.device->update_descriptor_set(
        ctx.device.self,
        gfx::DescriptorSetUpdate{.set     = params_ssbo,
                                 .binding = 0,
                                 .element = 0,
                                 .buffers = to_span({gfx::BufferBinding{
                                     .buffer = params_buffer,
                                     .offset = 0,
                                     .size   = sizeof(RRectParam) * 20}})});
  }

  void uninit()
  {
    ctx.device->wait_idle(ctx.device.self).unwrap();
    bloom.uninit(ctx);
    blur.uninit(ctx);
    fxaa.uninit(ctx);
    msaa.uninit(ctx);
    pbr.uninit(ctx);
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

  void record_frame()
  {
    auto enc = ctx.encoder();

    enc->clear_color_image(
        enc.self, ctx.framebuffer_attachments.color_image,
        gfx::Color{.float32 = {1, 1, 1, 1}},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                            .first_mip_level   = 0,
                                            .num_mip_levels    = 1,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1}}));

    enc->clear_color_image(
        enc.self, image, gfx::Color{.float32 = {1, 1, 1, 1}},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                            .first_mip_level   = 0,
                                            .num_mip_levels    = 1,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1}}));
    enc->update_buffer(
        enc.self,
        to_span<RRectParam>(
            {{.transform =
                  ViewTransform{.model = affine_scale3d({.8, .75, 1}) *
                                         affine_rotate3d_z(0.5),
                                .view = affine_scale3d({1080.0 / 1920, 1, 1}),
                                .projection = Mat4::identity()},
              .radii = {.2, .2, .2, .2},
              .uv    = {{0, 0}, {1, 1}},
              .tint  = {{1, 0, 1, 1}, {1, 0, 0, 1}, {0, 0, 1, 1}, {1, 1, 1, 1}},
              .aspect_ratio = {1, .75 / .8},
              .albedo       = 0}})
            .as_u8(),
        0, params_buffer);

    rrect.add_pass(
        ctx,
        RRectPassParams{
            .rendering_info =
                gfx::RenderingInfo{
                    .extent            = ctx.framebuffer_attachments.extent,
                    .num_layers        = 1,
                    .color_attachments = to_span({gfx::RenderingAttachment{
                        .view = ctx.framebuffer_attachments.color_image_view,
                    }})},
            .params_ssbo        = params_ssbo,
            .params_ssbo_offset = 0,
            .textures           = textures,
            .first_instance     = 0,
            .num_instances      = 1});
  }
};

}        // namespace ash
