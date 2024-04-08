#include "ashura/renderer/passes/pbr.h"

namespace ash
{

void PBRPass::init(RenderContext &ctx)
{
  constexpr Array bindings_desc = PBRShaderParameter::GET_BINDINGS_DESC();
  descriptor_set_layout =
      ctx.device
          ->create_descriptor_set_layout(
              ctx.device.self,
              gfx::DescriptorSetLayoutDesc{.label    = "PBR Parameters",
                                           .bindings = to_span(bindings_desc)})
          .unwrap();

  render_pass =
      ctx.device
          ->create_render_pass(
              ctx.device.self,
              gfx::RenderPassDesc{
                  .label             = "PBR RenderPass",
                  .color_attachments = to_span<gfx::RenderPassAttachment>(
                      {{.format           = ctx.color_format,
                        .load_op          = gfx::LoadOp::Load,
                        .store_op         = gfx::StoreOp::Store,
                        .stencil_load_op  = gfx::LoadOp::DontCare,
                        .stencil_store_op = gfx::StoreOp::DontCare}}),
                  .input_attachments = {},
                  .depth_stencil_attachment =
                      {.format           = ctx.depth_stencil_format,
                       .load_op          = gfx::LoadOp::Load,
                       .store_op         = gfx::StoreOp::Store,
                       .stencil_load_op  = gfx::LoadOp::Load,
                       .stencil_store_op = gfx::StoreOp::Store},
              })
          .unwrap();

  gfx::Shader vertex_shader   = ctx.get_shader("PBR:VS"_span).unwrap();
  gfx::Shader fragment_shader = ctx.get_shader("PBR:FS"_span).unwrap();

  gfx::VertexAttribute vtx_attrs[] = {{.binding  = 0,
                                       .location = 0,
                                       .format = gfx::Format::R32G32B32_SFLOAT,
                                       .offset = offsetof(PBRVertex, x)},
                                      {.binding  = 0,
                                       .location = 1,
                                       .format   = gfx::Format::R32G32_SFLOAT,
                                       .offset   = offsetof(PBRVertex, u)}};

  gfx::VertexInputBinding vtx_bindings[] = {
      {.binding    = 0,
       .stride     = sizeof(PBRVertex),
       .input_rate = gfx::InputRate::Vertex}};

  gfx::PipelineRasterizationState raster_state{
      .depth_clamp_enable         = false,
      .polygon_mode               = gfx::PolygonMode::Fill,
      .cull_mode                  = gfx::CullMode::None,
      .front_face                 = gfx::FrontFace::CounterClockWise,
      .depth_bias_enable          = false,
      .depth_bias_constant_factor = 0,
      .depth_bias_clamp           = 0,
      .depth_bias_slope_factor    = 0};

  gfx::PipelineDepthStencilState depth_stencil_state{
      .depth_test_enable        = true,
      .depth_write_enable       = true,
      .depth_compare_op         = gfx::CompareOp::Greater,
      .depth_bounds_test_enable = false,
      .stencil_test_enable      = false,
      .front_stencil            = gfx::StencilOpState{},
      .back_stencil             = gfx::StencilOpState{},
      .min_depth_bounds         = 0,
      .max_depth_bounds         = 0};

  gfx::PipelineColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gfx::BlendFactor::Zero,
       .dst_color_blend_factor = gfx::BlendFactor::Zero,
       .color_blend_op         = gfx::BlendOp::Add,
       .src_alpha_blend_factor = gfx::BlendFactor::Zero,
       .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
       .alpha_blend_op         = gfx::BlendOp::Add,
       .color_write_mask       = gfx::ColorComponents::All}};

  gfx::PipelineColorBlendState color_blend_state{
      .attachments    = to_span(attachment_states),
      .blend_constant = {1, 1, 1, 1}};

  gfx::DescriptorSetLayout set_layouts[] = {ctx.uniform_layout,
                                            descriptor_set_layout};

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label = "PBR Graphics Pipeline",
      .vertex_shader =
          gfx::ShaderStageDesc{.shader                        = vertex_shader,
                               .entry_point                   = "main",
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gfx::ShaderStageDesc{.shader                        = fragment_shader,
                               .entry_point                   = "main",
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .render_pass            = render_pass,
      .vertex_input_bindings  = to_span(vtx_bindings),
      .vertex_attributes      = to_span(vtx_attrs),
      .push_constant_size     = 0,
      .descriptor_set_layouts = to_span(set_layouts),
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
  CHECK(params.render_target.color_images.size() != 0);
  CHECK(has_bits(params.render_target.depth_stencil_aspects,
                 gfx::ImageAspects::Depth));
  gfx::CommandEncoderImpl encoder = ctx.encoder();

  gfx::Framebuffer framebuffer =
      ctx.device
          ->create_framebuffer(
              ctx.device.self,
              gfx::FramebufferDesc{.label       = "PBR Framebuffer",
                                   .render_pass = render_pass,
                                   .extent      = params.render_target.extent,
                                   .color_attachments = to_span(
                                       params.render_target.color_images),
                                   .depth_stencil_attachment =
                                       params.render_target.depth_stencil_image,
                                   .layers = 1})
          .unwrap();

  encoder->begin_render_pass(encoder.self, framebuffer, render_pass,
                             params.render_target.render_offset,
                             params.render_target.render_extent, {}, {});

  gfx::Buffer           prev_vtx_buff        = nullptr;
  u64                   prev_vtx_buff_offset = 0;
  gfx::Buffer           prev_idx_buff        = nullptr;
  u64                   prev_idx_buff_offset = 0;
  gfx::GraphicsPipeline prev_pipeline        = nullptr;

  for (PBRObject const &object : params.objects)
  {
    gfx::GraphicsPipeline object_pipeline =
        object.wireframe ? wireframe_pipeline : pipeline;
    if (object_pipeline != prev_pipeline)
    {
      encoder->bind_graphics_pipeline(encoder.self, object_pipeline);
      encoder->set_scissor(encoder.self, params.render_target.render_offset,
                           params.render_target.render_extent);
      encoder->set_viewport(
          encoder.self,
          gfx::Viewport{
              .offset    = Vec2{(f32) params.render_target.render_offset.x,
                             (f32) params.render_target.render_offset.y},
              .extent    = Vec2{(f32) params.render_target.render_extent.x,
                             (f32) params.render_target.render_extent.y},
              .min_depth = 0,
              .max_depth = 1});
      prev_pipeline = object_pipeline;
    }

    if (prev_vtx_buff != object.mesh.vertex_buffer ||
        prev_vtx_buff_offset != object.mesh.vertex_buffer_offset)
    {
      encoder->bind_vertex_buffers(encoder.self,
                                   to_span({object.mesh.vertex_buffer}),
                                   to_span({object.mesh.vertex_buffer_offset}));
      prev_vtx_buff        = object.mesh.vertex_buffer;
      prev_vtx_buff_offset = object.mesh.vertex_buffer_offset;
    }
    if (prev_idx_buff != object.mesh.index_buffer ||
        prev_idx_buff_offset != object.mesh.index_buffer_offset)
    {
      encoder->bind_index_buffer(encoder.self, object.mesh.index_buffer,
                                 object.mesh.index_buffer_offset,
                                 object.mesh.index_type);
      prev_idx_buff        = object.mesh.index_buffer;
      prev_idx_buff_offset = object.mesh.index_buffer_offset;
    }

    // TODO(lamaRRR): fix uniforms
    gfx::DescriptorSet const sets[]{params.lights.set, object.uniform.set,
                                    object.descriptor};
    u32 const                offsets[]{params.lights.buffer_offset,
                                       object.uniform.buffer_offset};

    encoder->bind_descriptor_sets(encoder.self, to_span(sets),
                                  to_span(offsets));

    encoder->draw(encoder.self, object.command.first_index,
                  object.command.index_count, object.command.vertex_offset,
                  object.command.first_instance, object.command.instance_count);
  }

  encoder->end_render_pass(encoder.self);

  ctx.release(framebuffer);
}

void PBRPass::uninit(RenderContext &ctx)
{
  ctx.device->unref_descriptor_set_layout(ctx.device.self,
                                          descriptor_set_layout);
  ctx.device->unref_render_pass(ctx.device.self, render_pass);
  ctx.device->unref_graphics_pipeline(ctx.device.self, pipeline);
  ctx.device->unref_graphics_pipeline(ctx.device.self, wireframe_pipeline);
}

}        // namespace ash