#include "ashura/engine/passes/blur.h"

namespace ash
{

void BlurPass::init(RenderContext &ctx)
{
  // https://www.youtube.com/watch?v=ml-5OGZC7vE
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
       .src_color_blend_factor = gfx::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gfx::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gfx::BlendOp::Add,
       .src_alpha_blend_factor = gfx::BlendFactor::One,
       .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
       .alpha_blend_op         = gfx::BlendOp::Add,
       .color_write_mask       = gfx::ColorComponents::All}};

  gfx::ColorBlendState color_blend_state{.attachments =
                                             to_span(attachment_states),
                                         .blend_constant = {1, 1, 1, 1}};

  gfx::DescriptorSetLayout set_layouts[] = {ctx.textures_layout};

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
      .descriptor_set_layouts = to_span(set_layouts),
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
}

void BlurPass::add_pass(RenderContext &ctx, BlurPassParams const &params)
{
  gfx::CommandEncoderImpl encoder = ctx.encoder();

  f32 blur_radius = 0;

  //
  //
  //
  // downsampling pass
  encoder->begin_rendering(
      encoder.self,
      gfx::RenderingInfo{.offset            = {0, 0},
                         .extent            = params.blur_extent / 2,
                         .num_layers        = 1,
                         .color_attachments = to_span({gfx::RenderingAttachment{
                             .view = ctx.scratch_framebuffer.color_image_view,
                             .resolve      = nullptr,
                             .resolve_mode = gfx::ResolveModes::None,
                             .load_op      = gfx::LoadOp::Clear,
                             .store_op     = gfx::StoreOp::Store}}),
                         .depth_attachment  = {},
                         .stencil_attachment = {}});
  encoder->bind_graphics_pipeline(encoder.self, downsample_pipeline);
  encoder->set_graphics_state(
      encoder.self,
      gfx::GraphicsState{
          .scissor  = {.offset = {0, 0}, .extent = params.blur_extent / 2},
          .viewport = {.offset = {0, 0},
                       .extent = {params.blur_extent.x / 2.0f,
                                  params.blur_extent.y / 2.0f}}});
  encoder->bind_descriptor_sets(encoder.self, to_span({params.texture_view}),
                                {});
  encoder->push_constants(
      encoder.self, to_span({BlurParam{.texture = params.texture,
      }}).as_u8());
  encoder->draw(encoder.self, 4, 1, 0, 0);
  encoder->end_rendering(encoder.self);

  //
  //
  //
  // upsampling pass
  encoder->begin_rendering(
      encoder.self,
      gfx::RenderingInfo{.offset            = params.blur_offset,
                         .extent            = params.blur_extent,
                         .num_layers        = 1,
                         .color_attachments = to_span({gfx::RenderingAttachment{
                             .view         = params.image_view,
                             .resolve      = nullptr,
                             .resolve_mode = gfx::ResolveModes::None,
                             .load_op      = gfx::LoadOp::Load,
                             .store_op     = gfx::StoreOp::Store}}),
                         .depth_attachment  = {},
                         .stencil_attachment = {}});
  encoder->bind_graphics_pipeline(encoder.self, upsample_pipeline);
  encoder->set_graphics_state(
      encoder.self,
      gfx::GraphicsState{.scissor  = {.offset = params.blur_offset,
                                      .extent = params.blur_extent},
                         .viewport = {.offset = {0, 0},
                                      .extent = {params.extent.x * 1.0f,
                                                 params.extent.y * 1.0f}}});
  encoder->bind_descriptor_sets(encoder.self,
                                to_span({ctx.scratch_color_texture_view}), {});
  encoder->push_constants(encoder.self,
                          to_span({BlurParam{.texture = 0}}).as_u8());
  encoder->draw(encoder.self, 4, 1, 0, 0);
  encoder->end_rendering(encoder.self);
}

}        // namespace ash