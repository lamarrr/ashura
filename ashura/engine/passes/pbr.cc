/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/pbr.h"

namespace ash
{

void PBRPass::init(RenderContext &ctx)
{
  gfx::Shader vertex_shader   = ctx.get_shader("PBR:VS"_span).unwrap();
  gfx::Shader fragment_shader = ctx.get_shader("PBR:FS"_span).unwrap();

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
                                             .max_depth_bounds         = 1};

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

  gfx::DescriptorSetLayout const set_layouts[] = {
      ctx.ssbo_layout, ctx.ssbo_layout,     ctx.ssbo_layout,
      ctx.ssbo_layout, ctx.samplers_layout, ctx.textures_layout};

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label = "PBR Graphics Pipeline"_span,
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
      .depth_format           = {&ctx.depth_stencil_format, 1},
      .vertex_input_bindings  = {},
      .vertex_attributes      = {},
      .push_constants_size    = 0,
      .descriptor_set_layouts = span(set_layouts),
      .primitive_topology     = gfx::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = ctx.pipeline_cache};

  pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_desc)
          .unwrap();

  pipeline_desc.rasterization_state.polygon_mode = gfx::PolygonMode::Line;

  wireframe_pipeline =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_desc)
          .unwrap();
}

void PBRPass::add_pass(RenderContext &ctx, PBRPassParams const &params)
{
  gfx::CommandEncoderImpl encoder = ctx.encoder();

  encoder->begin_rendering(encoder.self, params.rendering_info);
  encoder->bind_graphics_pipeline(encoder.self, params.wireframe ?
                                                    this->wireframe_pipeline :
                                                    this->pipeline);

  encoder->set_graphics_state(
      encoder.self, gfx::GraphicsState{.scissor           = params.scissor,
                                       .viewport          = params.viewport,
                                       .blend_constant    = {1, 1, 1, 1},
                                       .depth_test_enable = true,
                                       .depth_compare_op = gfx::CompareOp::Less,
                                       .depth_write_enable = true});
  encoder->bind_descriptor_sets(
      encoder.self,
      span({params.vertices_ssbo, params.indices_ssbo, params.params_ssbo,
            params.lights_ssbo, ctx.samplers, params.textures}),
      span<u32>({0, 0, 0, 0}));
  encoder->draw(encoder.self, params.num_indices, 1, 0, params.instance);
  encoder->end_rendering(encoder.self);
}

void PBRPass::uninit(RenderContext &ctx)
{
  ctx.device->destroy_graphics_pipeline(ctx.device.self, pipeline);
  ctx.device->destroy_graphics_pipeline(ctx.device.self, wireframe_pipeline);
}

}        // namespace ash