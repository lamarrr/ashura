#pragma once
#include "ashura/renderer/passes/bloom.h"
#include "ashura/renderer/passes/blur.h"
#include "ashura/renderer/passes/fxaa.h"
#include "ashura/renderer/passes/msaa.h"
#include "ashura/renderer/passes/pbr.h"
#include "ashura/renderer/passes/rrect.h"
#include "ashura/renderer/render_context.h"
#include "ashura/std/hash_map.h"
#include "ashura/std/object.h"

namespace ash
{

// sky render pass
// render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
// perform bloom, blur, msaa on 3d scene
// render UI pass + custom shaders, blur ???
// copy and composite 3d and 2d scenes
//
// TODO(lamarrr): composite of
// TODO(lamarrr): pass initialization and shutdown procedure?
//
//

typedef struct RenderPass_T *RenderPass;
typedef struct Renderer      Renderer;

struct RenderPassImpl
{
  RenderPass pass                              = nullptr;
  void (*attach)(RenderPass pass, Renderer &r) = nullptr;
  void (*uninit)(RenderPass pass, Renderer &r) = nullptr;
};

struct Renderer
{
  RenderContext              ctx    = {};
  BloomPass                  bloom  = {};
  BlurPass                   blur   = {};
  FXAAPass                   fxaa   = {};
  MSAAPass                   msaa   = {};
  PBRPass                    pbr    = {};
  RRectPass                  rrect  = {};
  StrHashMap<RenderPassImpl> custom = {};

  gfx::Buffer        params_buffer = nullptr;
  gfx::Image         image         = nullptr;
  gfx::ImageView     texture       = nullptr;
  gfx::Sampler       sampler       = nullptr;
  gfx::DescriptorSet params_ssbo   = nullptr;
  gfx::DescriptorSet textures      = nullptr;
  gfx::DescriptorSet textures_2    = nullptr;
  /// TODO(lamarrr): attachment textures group, updated on every frame
  /// recreation. update descriptor sets when either attachments are recreated/

  gfx::Buffer        pbr_vtx_buff      = nullptr;
  gfx::Buffer        pbr_idx_buff      = nullptr;
  gfx::Buffer        pbr_prm_buff      = nullptr;
  gfx::Buffer        pbr_indirect_buff = nullptr;
  gfx::Buffer        pbr_lights_buff   = nullptr;
  gfx::DescriptorSet pbr_vtx_ssbo      = nullptr;
  gfx::DescriptorSet pbr_idx_ssbo      = nullptr;
  gfx::DescriptorSet pbr_prm_ssbo      = nullptr;
  gfx::DescriptorSet pbr_lights_ssbo   = nullptr;

  u64 stage();

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
                                           to_span<u32>({16}))
                   .unwrap();

    textures_2 =
        ctx.device
            ->create_descriptor_set(ctx.device.self, ctx.textures_layout,
                                    to_span<u32>({16}))
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

    pbr_vtx_buff =
        ctx.device
            ->create_buffer(
                ctx.device.self,
                gfx::BufferDesc{.size  = sizeof(PBRVertex) * 8,
                                .usage = gfx::BufferUsage::TransferDst |
                                         gfx::BufferUsage::TransferSrc |
                                         gfx::BufferUsage::StorageBuffer})
            .unwrap();

    pbr_idx_buff =
        ctx.device
            ->create_buffer(
                ctx.device.self,
                gfx::BufferDesc{.size  = sizeof(u32) * 12 * 3,
                                .usage = gfx::BufferUsage::TransferDst |
                                         gfx::BufferUsage::TransferSrc |
                                         gfx::BufferUsage::StorageBuffer})
            .unwrap();

    pbr_prm_buff =
        ctx.device
            ->create_buffer(
                ctx.device.self,
                gfx::BufferDesc{.size  = sizeof(PBRParam) * 1,
                                .usage = gfx::BufferUsage::TransferDst |
                                         gfx::BufferUsage::TransferSrc |
                                         gfx::BufferUsage::StorageBuffer})
            .unwrap();

    pbr_indirect_buff =
        ctx.device
            ->create_buffer(
                ctx.device.self,
                gfx::BufferDesc{.size  = sizeof(gfx::DrawCommand) * 1,
                                .usage = gfx::BufferUsage::TransferDst |
                                         gfx::BufferUsage::TransferSrc |
                                         gfx::BufferUsage::StorageBuffer |
                                         gfx::BufferUsage::IndirectBuffer})
            .unwrap();

    pbr_lights_buff =
        ctx.device
            ->create_buffer(
                ctx.device.self,
                gfx::BufferDesc{.size  = sizeof(PunctualLight) * 1,
                                .usage = gfx::BufferUsage::TransferDst |
                                         gfx::BufferUsage::TransferSrc |
                                         gfx::BufferUsage::StorageBuffer})
            .unwrap();

    pbr_vtx_ssbo =
        ctx.device->create_descriptor_set(ctx.device.self, ctx.ssbo_layout, {})
            .unwrap();
    pbr_idx_ssbo =
        ctx.device->create_descriptor_set(ctx.device.self, ctx.ssbo_layout, {})
            .unwrap();
    pbr_prm_ssbo =
        ctx.device->create_descriptor_set(ctx.device.self, ctx.ssbo_layout, {})
            .unwrap();
    pbr_lights_ssbo =
        ctx.device->create_descriptor_set(ctx.device.self, ctx.ssbo_layout, {})
            .unwrap();

    ctx.device->update_descriptor_set(
        ctx.device.self,
        gfx::DescriptorSetUpdate{
            .set     = pbr_vtx_ssbo,
            .binding = 0,
            .element = 0,
            .buffers = to_span({gfx::BufferBinding{.buffer = pbr_vtx_buff,
                                                   .offset = 0,
                                                   .size = gfx::WHOLE_SIZE}})});
    ctx.device->update_descriptor_set(
        ctx.device.self,
        gfx::DescriptorSetUpdate{
            .set     = pbr_idx_ssbo,
            .binding = 0,
            .element = 0,
            .buffers = to_span({gfx::BufferBinding{.buffer = pbr_idx_buff,
                                                   .offset = 0,
                                                   .size = gfx::WHOLE_SIZE}})});
    ctx.device->update_descriptor_set(
        ctx.device.self,
        gfx::DescriptorSetUpdate{
            .set     = pbr_prm_ssbo,
            .binding = 0,
            .element = 0,
            .buffers = to_span({gfx::BufferBinding{.buffer = pbr_prm_buff,
                                                   .offset = 0,
                                                   .size = gfx::WHOLE_SIZE}})});
    ctx.device->update_descriptor_set(
        ctx.device.self,
        gfx::DescriptorSetUpdate{
            .set     = pbr_lights_ssbo,
            .binding = 0,
            .element = 0,
            .buffers = to_span({gfx::BufferBinding{.buffer = pbr_lights_buff,
                                                   .offset = 0,
                                                   .size = gfx::WHOLE_SIZE}})});
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

  void record_frame(f32 rot_x, f32 rot_z)
  {
    // zeroth texture is always plain white
    auto enc = ctx.encoder();

    /*
        ctx.device->update_descriptor_set(
            ctx.device.self,
            gfx::DescriptorSetUpdate{
                .set     = textures_2,
                .binding = 0,
                .element = 0,
                .images  = to_span({gfx::ImageBinding{
                     .sampler    = sampler,
                     .image_view = ctx.framebuffer.color_image_view}})});*/

    enc->clear_color_image(
        enc.self, ctx.scratch_framebuffer.color_image,
        gfx::Color{.float32 = {1, 1, 1, 1}},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                            .first_mip_level   = 0,
                                            .num_mip_levels    = 1,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1}}));

    enc->clear_color_image(
        enc.self, ctx.framebuffer.color_image,
        gfx::Color{.float32 = {1, 1, 1, 1}},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                            .first_mip_level   = 0,
                                            .num_mip_levels    = 1,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1}}));

    enc->clear_color_image(
        enc.self, ctx.scratch_framebuffer.color_image,
        gfx::Color{.float32 = {1, 1, 1, 1}},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Color,
                                            .first_mip_level   = 0,
                                            .num_mip_levels    = 1,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1}}));

    enc->clear_depth_stencil_image(
        enc.self, ctx.framebuffer.depth_stencil_image,
        gfx::DepthStencil{.depth = 0, .stencil = 0},
        to_span({gfx::ImageSubresourceRange{.aspects = gfx::ImageAspects::Depth,
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

    timespec ts;
    timespec_get(&ts, TIME_UTC);
    enc->update_buffer(
        enc.self,
        to_span<RRectParam>(
            {{.transform =
                  ViewTransform{.model = affine_scale3d({.8F, .75F, 1}) *
                                         affine_rotate3d_z(0.5F),
                                .view = affine_scale3d({1080.0 / 1920, 1, 1}),
                                .projection = Mat4::identity()}
                      .mul(),
              .radii = {.25F, .2F, .1F, .4F},
              .uv    = {{0, 0}, {1, 1}},
              .tint  = {{1, 0, 1, 1}, {1, 0, 0, 1}, {0, 0, 1, 1}, {1, 1, 1, 1}},
              .aspect_ratio = {1, .75 / .8},
              .albedo       = 0},
             {.transform =
                  ViewTransform{.model =
                                    affine_scale3d({.8F, .75F, 1}) *
                                    affine_rotate3d_z(
                                        (ts.tv_nsec / 5'000'000'000.0f) * 1),
                                .view = affine_scale3d({1080.0 / 1920, 1, 1}),
                                .projection = Mat4::identity()}
                      .mul(),
              .radii = {.25F, .2F, .1F, .4F},
              .uv    = {{0, 0}, {1, 1}},
              .tint  = {{1, 0, 1, 1}, {1, 1, 0, 1}, {0, 0, 1, 1}, {1, 1, 1, 1}},
              .aspect_ratio = {1, .75 / .8},
              .albedo       = 0}})
            .as_u8(),
        0, params_buffer);

    rrect.add_pass(
        ctx, RRectPassParams{
                 .rendering_info =
                     gfx::RenderingInfo{
                         .extent            = ctx.framebuffer.extent,
                         .num_layers        = 1,
                         .color_attachments = to_span({gfx::RenderingAttachment{
                             .view = ctx.framebuffer.color_image_view,
                         }})},
                 .params_ssbo        = params_ssbo,
                 .params_ssbo_offset = 0,
                 .textures           = textures,
                 .first_instance     = 0,
                 .num_instances      = 2});

    enc->update_buffer(enc.self,
                       to_span<PBRVertex>({{-1, -1, 0.5F, 0, 0},
                                           {1, -1, 0.5F, 0, 1},
                                           {-1, 1, 0.5F, 0, 1},
                                           {1, 1, 0.5F, 1, 1},
                                           {-1, -1, -0.5F, 0, 0},
                                           {1, -1, -0.5F, 0, 1},
                                           {-1, 1, -0.5F, 0, 1},
                                           {1, 1, -0.5F, 1, 1}})
                           .as_u8(),
                       0, pbr_vtx_buff);

    enc->update_buffer(enc.self,
                       to_span<u32>({// Top
                                     2, 6, 7, 2, 3, 7,
                                     // Bottom
                                     0, 4, 5, 0, 1, 5,
                                     // Left
                                     0, 2, 6, 0, 4, 6,
                                     // Right
                                     1, 3, 7, 1, 5, 7,
                                     // Front
                                     0, 2, 3, 0, 1, 3,
                                     // Back
                                     4, 6, 7, 4, 5, 7})
                           .as_u8(),
                       0, pbr_idx_buff);

    enc->update_buffer(
        enc.self,
        to_span<PBRParam>(
            {{.transform =
                  ViewTransform{.model = affine_scale3d({.5F, .5F, .5F}) *
                                         affine_rotate3d_z(rot_z) *
                                         affine_rotate3d_x(rot_x),
                                .view = affine_scale3d({1080.0f / 1920, 1, 1}),
                                .projection = OrthographicCamera{.x_mag  = 1,
                                                                 .y_mag  = 1,
                                                                 .z_near = 0.1F,
                                                                 .z_far  = 100}
                                                  .to_projection_mat()},
              .albedo     = {1, 0, 1, 1},
              .num_lights = 1}})
            .as_u8(),
        0, pbr_prm_buff);

    enc->update_buffer(enc.self,
                       to_span<PunctualLight>({{.direction   = {0, 0, 0},
                                                .position    = {-1, -1, 0},
                                                .color       = {1, 1, 0, 1},
                                                .inner_angle = 0,
                                                .outer_angle = PI,
                                                .intensity   = 1,
                                                .radius      = 0.5}})
                           .as_u8(),
                       0, pbr_lights_buff);

    enc->update_buffer(enc.self,
                       to_span({gfx::DrawCommand{.vertex_count   = 36,
                                                 .instance_count = 1,
                                                 .first_vertex   = 0,
                                                 .first_instance = 0}})
                           .as_u8(),
                       0, pbr_indirect_buff);

    // pbr.add_pass(
    //     ctx,
    //     PBRPassParams{
    //         .rendering_info =
    //             gfx::RenderingInfo{
    //                 .extent            = ctx.framebuffer.extent,
    //                 .num_layers        = 1,
    //                 .color_attachments = to_span({gfx::RenderingAttachment{
    //                     .view = ctx.framebuffer.color_image_view,
    //                 }}),
    //                 .depth_attachment  = to_span({gfx::RenderingAttachment{
    //                      .view =
    //                      ctx.framebuffer.depth_stencil_image_view}})},
    //         .wireframe         = false,
    //         .vertex_ssbo       = pbr_vtx_ssbo,
    //         .index_ssbo        = pbr_idx_ssbo,
    //         .param_ssbo        = pbr_prm_ssbo,
    //         .param_ssbo_offset = 0,
    //         .light_ssbo        = pbr_lights_ssbo,
    //         .textures          = textures,
    //         .indirect          = {.buffer     = pbr_indirect_buff,
    //                               .offset     = 0,
    //                               .draw_count = 1,
    //                               .stride     = sizeof(gfx::DrawCommand)}});

    // TODO(lamarrr): needs to be two-pass??

    /// downsample 4 times then upsample 4 times, ping-ponging between two
    /// textures. use the mips? or specific areas of the images

    /* blur.add_pass(
         ctx, BlurPassParams{
                  .rendering_info =
                      gfx::RenderingInfo{
                          .extent            = ctx.scratch_framebuffer.extent,
                          .num_layers        = 1,
                          .color_attachments =
     to_span({gfx::RenderingAttachment{ .view =
     ctx.scratch_framebuffer.color_image_view,
                          }})},
                  .param    = BlurParam{.offset  = {0, 0},
                                        .extent  = {1, 1},
                                        .radius  = {6 / 1920.0, 6 / 1080.0},
                                        .texture = 0},
                  .textures = textures_2});

     enc->copy_image(enc.self, ctx.scratch_framebuffer.color_image,
                     ctx.framebuffer.color_image,
                     to_span<gfx::ImageCopy>(
                         {{.src_layers = {.aspects = gfx::ImageAspects::Color,
                                          .num_array_layers = 1},
                           .src_offset = {},
                           .dst_layers = {.aspects = gfx::ImageAspects::Color,
                                          .num_array_layers = 1},
                           .dst_offset = {},
                           .extent =
     ctx.framebuffer.color_image_desc.extent}}));
                           */
  }
};

}        // namespace ash
