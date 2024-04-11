#include "ashura/renderer/passes/rrect.h"
#include "ashura/std/math.h"

namespace ash
{

void RRectPass::init(RenderContext &ctx)
{
  constexpr Array bindings_desc = RRectShaderParameter::GET_BINDINGS_DESC();
  descriptor_set_layout =
      ctx.device
          ->create_descriptor_set_layout(
              ctx.device.self,
              gfx::DescriptorSetLayoutDesc{.label    = "RRect Parameters"_span,
                                           .bindings = to_span(bindings_desc)})
          .unwrap();

  render_pass =
      ctx.device
          ->create_render_pass(
              ctx.device.self,
              gfx::RenderPassDesc{
                  .label             = "RRect RenderPass"_span,
                  .color_attachments = to_span<gfx::RenderPassAttachment>(
                      {{.format           = ctx.color_format,
                        .load_op          = gfx::LoadOp::Load,
                        .store_op         = gfx::StoreOp::Store,
                        .stencil_load_op  = gfx::LoadOp::DontCare,
                        .stencil_store_op = gfx::StoreOp::DontCare}}),
                  .input_attachments = {},
                  .depth_stencil_attachment =
                      {.format           = gfx::Format::Undefined,
                       .load_op          = gfx::LoadOp::DontCare,
                       .store_op         = gfx::StoreOp::DontCare,
                       .stencil_load_op  = gfx::LoadOp::DontCare,
                       .stencil_store_op = gfx::StoreOp::DontCare},
              })
          .unwrap();

  gfx::Shader vertex_shader   = ctx.get_shader("RRect:VS"_span).unwrap();
  gfx::Shader fragment_shader = ctx.get_shader("RRect:FS"_span).unwrap();

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
      .depth_test_enable        = false,
      .depth_write_enable       = false,
      .depth_compare_op         = gfx::CompareOp::Greater,
      .depth_bounds_test_enable = false,
      .stencil_test_enable      = false,
      .front_stencil            = gfx::StencilOpState{},
      .back_stencil             = gfx::StencilOpState{},
      .min_depth_bounds         = 0,
      .max_depth_bounds         = 0};

  gfx::PipelineColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gfx::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gfx::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gfx::BlendOp::Add,
       .src_alpha_blend_factor = gfx::BlendFactor::One,
       .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
       .alpha_blend_op         = gfx::BlendOp::Add,
       .color_write_mask       = gfx::ColorComponents::All}};

  gfx::PipelineColorBlendState color_blend_state{
      .attachments    = to_span(attachment_states),
      .blend_constant = {1, 1, 1, 1}};

  gfx::VertexAttribute vtx_attrs[] = {{.binding  = 0,
                                       .location = 0,
                                       .format   = gfx::Format::R32G32_SFLOAT,
                                       .offset   = 0}};

  gfx::VertexInputBinding vtx_bindings[] = {
      {.binding    = 0,
       .stride     = sizeof(Vec2),
       .input_rate = gfx::InputRate::Vertex}};

  gfx::DescriptorSetLayout set_layouts[] = {ctx.uniform_layout,
                                            descriptor_set_layout};

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label = "RRect Graphics Pipeline"_span,
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

  vertex_buffer =
      ctx.device
          ->create_buffer(
              ctx.device.self,
              gfx::BufferDesc{.label       = "RRect Vertex Buffer"_span,
                              .size        = sizeof(Vec2) * 4,
                              .host_mapped = true,
                              .usage       = gfx::BufferUsage::VertexBuffer |
                                       gfx::BufferUsage::TransferDst})
          .unwrap();
  index_buffer =
      ctx.device
          ->create_buffer(
              ctx.device.self,
              gfx::BufferDesc{.label       = "RRect Index Buffer"_span,
                              .size        = sizeof(u16) * 6,
                              .host_mapped = true,
                              .usage       = gfx::BufferUsage::IndexBuffer |
                                       gfx::BufferUsage::TransferDst})
          .unwrap();

  f32 *vtx_map =
      (f32 *) ctx.device->get_buffer_memory_map(ctx.device.self, vertex_buffer)
          .unwrap();
  u16 *idx_map =
      (u16 *) ctx.device->get_buffer_memory_map(ctx.device.self, index_buffer)
          .unwrap();

  constexpr f32 vtxs[] = {0, 0, 1, 0, 1, 1, 0, 1};
  constexpr u16 idxs[] = {0, 1, 2, 0, 2, 3};

  mem::copy(to_span(vtxs), vtx_map);
  mem::copy(to_span(idxs), idx_map);

  ctx.device
      ->flush_buffer_memory_map(ctx.device.self, vertex_buffer,
                                gfx::MemoryRange{0, gfx::WHOLE_SIZE})
      .unwrap();
  ctx.device
      ->flush_buffer_memory_map(ctx.device.self, index_buffer,
                                gfx::MemoryRange{0, gfx::WHOLE_SIZE})
      .unwrap();
}

void RRectPass::add_pass(RenderContext &ctx, RRectPassParams const &params)
{
  gfx::CommandEncoderImpl encoder = ctx.encoder();
  gfx::Framebuffer        framebuffer =
      ctx.device
          ->create_framebuffer(
              ctx.device.self,
              gfx::FramebufferDesc{.label       = "RRect Framebuffer"_span,
                                   .render_pass = render_pass,
                                   .extent      = params.render_target.extent,
                                   .color_attachments = to_span(
                                       params.render_target.color_images),
                                   .depth_stencil_attachment = nullptr,
                                   .layers                   = 1})
          .unwrap();

  encoder->begin_render_pass(encoder.self, framebuffer, render_pass,
                             params.render_target.render_offset,
                             params.render_target.render_extent, {}, {});

  encoder->bind_graphics_pipeline(encoder.self, pipeline);
  encoder->bind_vertex_buffers(encoder.self, to_span({vertex_buffer}),
                               to_span<u64>({0}));
  encoder->bind_index_buffer(encoder.self, index_buffer, 0,
                             gfx::IndexType::Uint16);
  encoder->set_scissor(encoder.self, params.render_target.render_offset,
                       params.render_target.render_extent);
  encoder->set_viewport(
      encoder.self,
      gfx::Viewport{.offset = Vec2{(f32) params.render_target.render_offset.x,
                                   (f32) params.render_target.render_offset.y},
                    .extent = Vec2{(f32) params.render_target.render_extent.x,
                                   (f32) params.render_target.render_extent.y},
                    .min_depth = 0,
                    .max_depth = 1});

  for (RRectObject const &object : params.objects)
  {
    encoder->bind_descriptor_sets(
        encoder.self, to_span({object.descriptor, object.uniform.set}),
        to_span({object.uniform.buffer_offset}));
    encoder->draw(encoder.self, 0, 6, 0, 0, 1);
  }

  encoder->end_render_pass(encoder.self);

  ctx.release(framebuffer);
}

void RRectPass::uninit(RenderContext &ctx)
{
}
}        // namespace ash
