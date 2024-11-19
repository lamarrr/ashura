/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes.h"
#include "ashura/std/math.h"

namespace ash
{

void BloomPass::acquire(GpuContext &, AssetMap &)
{
}

void BloomPass::release(GpuContext &, AssetMap &)
{
}

void BloomPass::encode(GpuContext &, gpu::CommandEncoderImpl const &,
                       BloomPassParams const &)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

void BlurPass::acquire(GpuContext &ctx, AssetMap &assets)
{
  // https://www.youtube.com/watch?v=ml-5OGZC7vE
  //
  // An investigation of fast real-time GPU-based image blur algorithms -
  // https://www.intel.cn/content/www/cn/zh/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html

  //
  // Algorithm described here:
  // https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_slides.pdf
  //
  gpu::Shader vertex_shader   = assets.shaders["Blur_DownSample:VS"_span];
  gpu::Shader fragment_shader = assets.shaders["Blur_DownSample:FS"_span];

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                           gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0};

  gpu::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                                 gpu::CompareOp::Greater,
                                             .depth_bounds_test_enable = false,
                                             .stencil_test_enable      = false,
                                             .front_stencil            = {},
                                             .back_stencil             = {},
                                             .min_depth_bounds         = 0,
                                             .max_depth_bounds         = 0};

  gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gpu::BlendFactor::Zero,
       .dst_color_blend_factor = gpu::BlendFactor::Zero,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::Zero,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}};

  gpu::ColorBlendState color_blend_state{.attachments = span(attachment_states),
                                         .blend_constant = {1, 1, 1, 1}};

  gpu::DescriptorSetLayout set_layouts[] = {ctx.samplers_layout,
                                            ctx.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
      .label = "Blur Graphics Pipeline"_span,
      .vertex_shader =
          gpu::ShaderStageInfo{.shader                        = vertex_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gpu::ShaderStageInfo{.shader                        = fragment_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .color_formats          = {&ctx.color_format, 1},
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = sizeof(BlurParam),
      .descriptor_set_layouts = span(set_layouts),
      .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = ctx.pipeline_cache};

  downsample_pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_info)
          .unwrap();

  pipeline_info.vertex_shader.shader = assets.shaders["Blur_UpSample:VS"_span];
  pipeline_info.fragment_shader.shader =
      assets.shaders["Blur_UpSample:FS"_span];

  upsample_pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_info)
          .unwrap();
}

void BlurPass::release(GpuContext &ctx, AssetMap &)
{
  ctx.device->uninit_graphics_pipeline(ctx.device.self, downsample_pipeline);
  ctx.device->uninit_graphics_pipeline(ctx.device.self, upsample_pipeline);
}

void sample(BlurPass &b, GpuContext &c, gpu::CommandEncoderImpl e, f32 rad,
            gpu::DescriptorSet src_texture, u32 src_index,
            gpu::Extent src_extent, gpu::Rect src_area, gpu::ImageView dst,
            Vec2U dst_offset, bool upsample)
{
  Vec2 radius = rad / as_vec2(src_extent);
  Vec2 uv0    = as_vec2(src_area.offset) / as_vec2(src_extent);
  Vec2 uv1    = as_vec2(src_area.end()) / as_vec2(src_extent);

  e->begin_rendering(
      e.self,
      gpu::RenderingInfo{
          .render_area = {.offset = dst_offset, .extent = src_area.extent},
          .num_layers  = 1,
          .color_attachments = span(
              {gpu::RenderingAttachment{.view         = dst,
                                        .resolve      = nullptr,
                                        .resolve_mode = gpu::ResolveModes::None,
                                        .load_op      = gpu::LoadOp::Clear,
                                        .store_op     = gpu::StoreOp::Store}}),
          .depth_attachment   = {},
          .stencil_attachment = {}});

  e->bind_graphics_pipeline(e.self, upsample ? b.upsample_pipeline :
                                               b.downsample_pipeline);
  e->set_graphics_state(
      e.self, gpu::GraphicsState{
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

void BlurPass::encode(GpuContext &ctx, gpu::CommandEncoderImpl const &e,
                      BlurPassParams const &params)
{
  CHECK(params.passes > 0);
  Vec2U extent = params.area.extent;
  extent.x     = min(extent.x, ctx.scratch_fbs[0].extent.x);
  extent.y     = min(extent.y, ctx.scratch_fbs[0].extent.y);
  u32 radius   = 1;

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
           gpu::Rect{Vec2U{0, 0}, params.area.extent},
           ctx.scratch_fbs[dst].color.view, Vec2U{0, 0}, false);
    src = (src + 1) & 1;
    dst = (dst + 1) & 1;
  }

  // upsample pass
  for (u32 i = 0; i < params.passes - 1; i++)
  {
    sample(*this, ctx, e, (f32) radius, ctx.scratch_fbs[src].color_texture, 0,
           ctx.scratch_fbs[src].extent,
           gpu::Rect{Vec2U{0, 0}, params.area.extent},
           ctx.scratch_fbs[dst].color.view, Vec2U{0, 0}, true);
    src = (src + 1) & 1;
    dst = (dst + 1) & 1;
    radius -= 1;
  }

  sample(*this, ctx, e, (f32) radius, ctx.scratch_fbs[src].color_texture, 0,
         ctx.scratch_fbs[src].extent,
         gpu::Rect{Vec2U{0, 0}, params.area.extent}, params.image_view,
         params.area.offset, true);
}

void NgonPass::acquire(GpuContext &ctx, AssetMap &assets)
{
  gpu::Shader vertex_shader   = assets.shaders["Ngon:VS"_span];
  gpu::Shader fragment_shader = assets.shaders["Ngon:FS"_span];

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                           gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0};

  gpu::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                                 gpu::CompareOp::Greater,
                                             .depth_bounds_test_enable = false,
                                             .stencil_test_enable      = false,
                                             .front_stencil            = {},
                                             .back_stencil             = {},
                                             .min_depth_bounds         = 0,
                                             .max_depth_bounds         = 0};

  gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = true,
       .src_color_blend_factor = gpu::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gpu::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::One,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}};

  gpu::ColorBlendState color_blend_state{.attachments = span(attachment_states),
                                         .blend_constant = {1, 1, 1, 1}};

  gpu::DescriptorSetLayout set_layouts[] = {
      ctx.ssbo_layout, ctx.ssbo_layout, ctx.ssbo_layout, ctx.samplers_layout,
      ctx.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
      .label = "Ngon Graphics Pipeline"_span,
      .vertex_shader =
          gpu::ShaderStageInfo{.shader                        = vertex_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gpu::ShaderStageInfo{.shader                        = fragment_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .color_formats          = {&ctx.color_format, 1},
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = 0,
      .descriptor_set_layouts = span(set_layouts),
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = ctx.pipeline_cache};

  pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_info)
          .unwrap();
}

void NgonPass::encode(GpuContext &ctx, gpu::CommandEncoderImpl const &e,
                      NgonPassParams const &params)
{
  e->begin_rendering(e.self, params.rendering_info);
  e->bind_graphics_pipeline(e.self, pipeline);
  e->bind_descriptor_sets(
      e.self,
      span({params.vertices_ssbo, params.indices_ssbo, params.params_ssbo,
            ctx.samplers, params.textures}),
      span<u32>({0, 0, 0}));
  e->push_constants(e.self, span({params.world_to_view}).as_u8());
  e->set_graphics_state(e.self,
                        gpu::GraphicsState{.scissor  = params.scissor,
                                           .viewport = params.viewport});
  u32 const num_instances = params.index_counts.size32();
  for (u32 i = 0; i < num_instances; i++)
  {
    u32 vertex_count = params.index_counts[i];
    e->draw(e.self, vertex_count, 1, 0, i);
  }
  e->end_rendering(e.self);
}

void NgonPass::release(GpuContext &ctx, AssetMap &)
{
  ctx.device->uninit_graphics_pipeline(ctx.device.self, pipeline);
}

void PBRPass::acquire(GpuContext &ctx, AssetMap &assets)
{
  gpu::Shader vertex_shader   = assets.shaders["PBR:VS"_span];
  gpu::Shader fragment_shader = assets.shaders["PBR:FS"_span];

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                           gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0};

  gpu::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                                 gpu::CompareOp::Greater,
                                             .depth_bounds_test_enable = false,
                                             .stencil_test_enable      = false,
                                             .front_stencil            = {},
                                             .back_stencil             = {},
                                             .min_depth_bounds         = 0,
                                             .max_depth_bounds         = 1};

  gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gpu::BlendFactor::Zero,
       .dst_color_blend_factor = gpu::BlendFactor::Zero,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::Zero,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}};

  gpu::ColorBlendState color_blend_state{.attachments = span(attachment_states),
                                         .blend_constant = {1, 1, 1, 1}};

  gpu::DescriptorSetLayout const set_layouts[] = {
      ctx.ssbo_layout, ctx.ssbo_layout,     ctx.ssbo_layout,
      ctx.ssbo_layout, ctx.samplers_layout, ctx.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
      .label = "PBR Graphics Pipeline"_span,
      .vertex_shader =
          gpu::ShaderStageInfo{.shader                        = vertex_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gpu::ShaderStageInfo{.shader                        = fragment_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .color_formats          = {&ctx.color_format, 1},
      .depth_format           = {&ctx.depth_stencil_format, 1},
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = 0,
      .descriptor_set_layouts = span(set_layouts),
      .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = ctx.pipeline_cache};

  pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_info)
          .unwrap();

  pipeline_info.rasterization_state.polygon_mode = gpu::PolygonMode::Line;

  wireframe_pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_info)
          .unwrap();
}

void PBRPass::encode(GpuContext &ctx, gpu::CommandEncoderImpl const &e,
                     PBRPassParams const &params)
{
  e->begin_rendering(e.self, params.rendering_info);
  e->bind_graphics_pipeline(
      e.self, params.wireframe ? this->wireframe_pipeline : this->pipeline);

  e->set_graphics_state(
      e.self, gpu::GraphicsState{.scissor            = params.scissor,
                                 .viewport           = params.viewport,
                                 .blend_constant     = {1, 1, 1, 1},
                                 .depth_test_enable  = true,
                                 .depth_compare_op   = gpu::CompareOp::Less,
                                 .depth_write_enable = true});
  e->bind_descriptor_sets(
      e.self,
      span({params.vertices_ssbo, params.indices_ssbo, params.params_ssbo,
            params.lights_ssbo, ctx.samplers, params.textures}),
      span<u32>({0, 0, 0, 0}));
  e->push_constants(e.self, span({params.world_to_view}).as_u8());
  e->draw(e.self, params.num_indices, 1, 0, params.instance);
  e->end_rendering(e.self);
}

void PBRPass::release(GpuContext &ctx, AssetMap &)
{
  ctx.device->uninit_graphics_pipeline(ctx.device.self, pipeline);
  ctx.device->uninit_graphics_pipeline(ctx.device.self, wireframe_pipeline);
}

void RRectPass::acquire(GpuContext &ctx, AssetMap &assets)
{
  gpu::Shader vertex_shader   = assets.shaders["RRect:VS"_span];
  gpu::Shader fragment_shader = assets.shaders["RRect:FS"_span];

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                           gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0};

  gpu::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                                 gpu::CompareOp::Greater,
                                             .depth_bounds_test_enable = false,
                                             .stencil_test_enable      = false,
                                             .front_stencil            = {},
                                             .back_stencil             = {},
                                             .min_depth_bounds         = 0,
                                             .max_depth_bounds         = 0};

  gpu::ColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = true,
       .src_color_blend_factor = gpu::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gpu::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gpu::BlendOp::Add,
       .src_alpha_blend_factor = gpu::BlendFactor::One,
       .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
       .alpha_blend_op         = gpu::BlendOp::Add,
       .color_write_mask       = gpu::ColorComponents::All}};

  gpu::ColorBlendState color_blend_state{.attachments = span(attachment_states),
                                         .blend_constant = {1, 1, 1, 1}};

  gpu::DescriptorSetLayout set_layouts[] = {
      ctx.ssbo_layout, ctx.samplers_layout, ctx.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
      .label = "RRect Graphics Pipeline"_span,
      .vertex_shader =
          gpu::ShaderStageInfo{.shader                        = vertex_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gpu::ShaderStageInfo{.shader                        = fragment_shader,
                               .entry_point                   = "main"_span,
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .color_formats          = {&ctx.color_format, 1},
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = 0,
      .descriptor_set_layouts = span(set_layouts),
      .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = ctx.pipeline_cache};

  pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_info)
          .unwrap();
}

void RRectPass::encode(GpuContext &ctx, gpu::CommandEncoderImpl const &e,
                       RRectPassParams const &params)
{
  e->begin_rendering(e.self, params.rendering_info);
  e->bind_graphics_pipeline(e.self, pipeline);
  e->set_graphics_state(e.self,
                        gpu::GraphicsState{.scissor  = params.scissor,
                                           .viewport = params.viewport});
  e->bind_descriptor_sets(
      e.self, span({params.params_ssbo, ctx.samplers, params.textures}),
      span<u32>({0}));
  e->push_constants(e.self, span({params.world_to_view}).as_u8());
  e->draw(e.self, 4, params.num_instances, 0, params.first_instance);
  e->end_rendering(e.self);
}

void RRectPass::release(GpuContext &ctx, AssetMap &)
{
  ctx.device->uninit_graphics_pipeline(ctx.device.self, pipeline);
}

}        // namespace ash