/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/blur.h"

namespace ash
{

void BlurPass::init(RenderContext &ctx)
{
  // https://www.youtube.com/watch?v=ml-5OGZC7vE
  //
  // An investigation of fast real-time GPU-based image blur algorithms -
  // https://www.intel.cn/content/www/cn/zh/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html

  //
  // Algorithm described here:
  // https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_slides.pdf
  //
  gfx::Shader vertex_shader =
      ctx.get_shader("Blur_DownSample:VS"_span).unwrap();
  gfx::Shader fragment_shader =
      ctx.get_shader("Blur_DownSample:FS"_span).unwrap();

  gfx::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gfx::PolygonMode::Fill,
                                       .cull_mode    = gfx::CullMode::None,
                                       .front_face =
                                           gfx::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0};

  gfx::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                                 gfx::CompareOp::Greater,
                                             .depth_bounds_test_enable = false,
                                             .stencil_test_enable      = false,
                                             .front_stencil            = {},
                                             .back_stencil             = {},
                                             .min_depth_bounds         = 0,
                                             .max_depth_bounds         = 0};

  gfx::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gfx::BlendFactor::Zero,
       .dst_color_blend_factor = gfx::BlendFactor::Zero,
       .color_blend_op         = gfx::BlendOp::Add,
       .src_alpha_blend_factor = gfx::BlendFactor::Zero,
       .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
       .alpha_blend_op         = gfx::BlendOp::Add,
       .color_write_mask       = gfx::ColorComponents::All}};

  gfx::ColorBlendState color_blend_state{.attachments = span(attachment_states),
                                         .blend_constant = {1, 1, 1, 1}};

  gfx::DescriptorSetLayout set_layouts[] = {ctx.samplers_layout,
                                            ctx.textures_layout};

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label = "Blur Graphics Pipeline"_span,
      .vertex_shader =
          gfx::ShaderStageDesc{.shader                        = vertex_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gfx::ShaderStageDesc{.shader                        = fragment_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .color_formats          = {&ctx.color_format, 1},
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(BlurParam),
      .descriptor_set_layouts = span(set_layouts),
      .primitive_topology     = gfx::PrimitiveTopology::TriangleFan,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = ctx.pipeline_cache};

  downsample_pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_desc)
          .unwrap();

  pipeline_desc.vertex_shader.shader =
      ctx.get_shader("Blur_UpSample:VS"_span).unwrap();
  pipeline_desc.fragment_shader.shader =
      ctx.get_shader("Blur_UpSample:FS"_span).unwrap();

  upsample_pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_desc)
          .unwrap();
}

void BlurPass::uninit(RenderContext &ctx)
{
  ctx.device->destroy_graphics_pipeline(ctx.device.self, downsample_pipeline);
  ctx.device->destroy_graphics_pipeline(ctx.device.self, upsample_pipeline);
}

void sample(BlurPass &b, RenderContext &c, gfx::CommandEncoderImpl const &e,
            f32 rad, gfx::DescriptorSet src_texture, u32 src_index,
            gfx::Extent src_extent, gfx::Rect src_area, gfx::ImageView dst,
            Vec2U dst_offset, bool upsample)
{
  Vec2 radius = rad / as_vec2(src_extent);
  Vec2 uv0    = as_vec2(src_area.offset) / as_vec2(src_extent);
  Vec2 uv1    = as_vec2(src_area.end()) / as_vec2(src_extent);

  e->begin_rendering(
      e.self,
      gfx::RenderingInfo{
          .render_area = {.offset = dst_offset, .extent = src_area.extent},
          .num_layers  = 1,
          .color_attachments = span(
              {gfx::RenderingAttachment{.view         = dst,
                                        .resolve      = nullptr,
                                        .resolve_mode = gfx::ResolveModes::None,
                                        .load_op      = gfx::LoadOp::Clear,
                                        .store_op     = gfx::StoreOp::Store}}),
          .depth_attachment   = {},
          .stencil_attachment = {}});

  e->bind_graphics_pipeline(e.self, upsample ? b.upsample_pipeline :
                                               b.downsample_pipeline);
  e->set_graphics_state(
      e.self, gfx::GraphicsState{
                  .scissor  = {.offset = dst_offset, .extent = src_area.extent},
                  .viewport = {.offset = as_vec2(dst_offset),
                               .extent = as_vec2(src_area.extent)}});
  e->bind_descriptor_sets(e.self, span({c.samplers, src_texture}), {});
  e->push_constants(e.self, span({BlurParam{.uv      = {uv0, uv1},
                                            .radius  = radius,
                                            .sampler = SAMPLER_LINEAR_CLAMPED,
                                            .texture = src_index}})
                                .as_u8());
  e->draw(e.self, 4, 1, 0, 0);
  e->end_rendering(e.self);
}

void BlurPass::add_pass(RenderContext &ctx, BlurPassParams const &params)
{
  CHECK(params.passes > 0);
  gfx::CommandEncoderImpl e      = ctx.encoder();
  Vec2U                   extent = params.area.extent;
  extent.x                       = min(extent.x, ctx.scratch_fbs[0].extent.x);
  extent.y                       = min(extent.y, ctx.scratch_fbs[0].extent.y);
  u32 radius                     = 1;

  // downsample pass
  sample(*this, ctx, e, (f32) radius, params.texture_view, params.texture,
         params.extent, params.area, ctx.scratch_fbs[0].color.view, Vec2U{0, 0},
         false);

  u32 src = 0;
  u32 dst = 1;

  for (u32 i = 0; i < params.passes - 1; i++)
  {
    radius += 1;
    sample(*this, ctx, e, (f32) radius, ctx.scratch_fbs[src].color_texture, 0,
           ctx.scratch_fbs[src].extent,
           gfx::Rect{Vec2U{0, 0}, params.area.extent},
           ctx.scratch_fbs[dst].color.view, Vec2U{0, 0}, false);
    src = (src + 1) & 1;
    dst = (dst + 1) & 1;
  }

  // upsample pass
  for (u32 i = 0; i < params.passes - 1; i++)
  {
    sample(*this, ctx, e, (f32) radius, ctx.scratch_fbs[src].color_texture, 0,
           ctx.scratch_fbs[src].extent,
           gfx::Rect{Vec2U{0, 0}, params.area.extent},
           ctx.scratch_fbs[dst].color.view, Vec2U{0, 0}, true);
    src = (src + 1) & 1;
    dst = (dst + 1) & 1;
    radius -= 1;
  }

  sample(*this, ctx, e, (f32) radius, ctx.scratch_fbs[src].color_texture, 0,
         ctx.scratch_fbs[src].extent,
         gfx::Rect{Vec2U{0, 0}, params.area.extent}, params.image_view,
         params.area.offset, true);
}

}        // namespace ash