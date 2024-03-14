#include "ashura/renderer/passes/rrect.h"

namespace ash
{

void RRectPass::init(Renderer &renderer)
{
  constexpr Array bindings_desc = RRectShaderParameter::GET_BINDINGS_DESC();
  descriptor_set_layout =
      renderer.device
          ->create_descriptor_set_layout(
              renderer.device.self,
              gfx::DescriptorSetLayoutDesc{.label    = "RRect Parameters",
                                           .bindings = to_span(bindings_desc)})
          .unwrap();

  render_pass =
      renderer.device
          ->create_render_pass(
              renderer.device.self,
              gfx::RenderPassDesc{
                  .label             = "RRect RenderPass",
                  .color_attachments = to_span<gfx::RenderPassAttachment>(
                      {{.format           = renderer.color_format,
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

  gfx::Shader vertex_shader   = renderer.get_shader("RRect.VS"_span).unwrap();
  gfx::Shader fragment_shader = renderer.get_shader("RRect.FS"_span).unwrap();

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

  gfx::PipelineColorBlendState color_blend_state{
      .logic_op_enable = true,
      .logic_op        = gfx::LogicOp::Set,
      .attachments     = to_span<gfx::PipelineColorBlendAttachmentState>(
          {{.blend_enable           = false,
                .src_color_blend_factor = gfx::BlendFactor::SrcAlpha,
                .dst_color_blend_factor = gfx::BlendFactor::OneMinusSrcAlpha,
                .color_blend_op         = gfx::BlendOp::Add,
                .src_alpha_blend_factor = gfx::BlendFactor::One,
                .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
                .alpha_blend_op         = gfx::BlendOp::Add,
                .color_write_mask       = gfx::ColorComponents::All}}),
      .blend_constant = {1, 1, 1, 1}};

  gfx::VertexAttribute vtx_attrs[] = {{.binding  = 0,
                                       .location = 0,
                                       .format   = gfx::Format::R32G32_SFLOAT,
                                       .offset   = 0}};

  gfx::VertexInputBinding vtx_bindings[] = {
      {.binding    = 0,
       .stride     = sizeof(Vec2),
       .input_rate = gfx::InputRate::Vertex}};

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label = "RRect Graphics Pipeline",
      .vertex_shader =
          gfx::ShaderStageDesc{.shader                        = vertex_shader,
                               .entry_point                   = "vs_main",
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gfx::ShaderStageDesc{.shader                        = fragment_shader,
                               .entry_point                   = "fs_main",
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .render_pass           = render_pass,
      .vertex_input_bindings = to_span(vtx_bindings),
      .vertex_attributes     = to_span(vtx_attrs),
      .push_constant_size    = 0,
      .descriptor_set_layouts =
          to_span({renderer.uniform_layout, descriptor_set_layout}),
      .primitive_topology  = gfx::PrimitiveTopology::TriangleList,
      .rasterization_state = raster_state,
      .depth_stencil_state = depth_stencil_state,
      .color_blend_state   = color_blend_state,
      .cache               = renderer.pipeline_cache};

  pipeline = renderer.device
                 ->create_graphics_pipeline(renderer.device.self, pipeline_desc)
                 .unwrap();

  vertex_buffer =
      renderer.device
          ->create_buffer(
              renderer.device.self,
              gfx::BufferDesc{.label       = "RRect Vertex Buffer",
                              .size        = sizeof(Vec2) * 4,
                              .host_mapped = true,
                              .usage       = gfx::BufferUsage::VertexBuffer})
          .unwrap();
  index_buffer =
      renderer.device
          ->create_buffer(
              renderer.device.self,
              gfx::BufferDesc{.label       = "RRect Index Buffer",
                              .size        = sizeof(u16) * 6,
                              .host_mapped = true,
                              .usage       = gfx::BufferUsage::IndexBuffer})
          .unwrap();

  // map and write vtx and idx buffers
}

void RRectPass::add_pass(Renderer &renderer, RRectParams const &params)
{
  gfx::Framebuffer framebuffer =
      renderer.device
          ->create_framebuffer(
              renderer.device.self,
              gfx::FramebufferDesc{.label       = "RRect Framebuffer",
                                   .render_pass = render_pass,
                                   .extent      = params.render_target.extent,
                                   .color_attachments = to_span(
                                       params.render_target.color_images),
                                   .depth_stencil_attachment = nullptr,
                                   .layers                   = 1})
          .unwrap();

  renderer.encoder->begin_render_pass(
      renderer.encoder.self, framebuffer, render_pass,
      params.render_target.scissor_offset, params.render_target.scissor_extent,
      {}, {});

  renderer.encoder->bind_graphics_pipeline(renderer.encoder.self, pipeline);
  u64 const vertex_offsets[] = {0};
  renderer.encoder->bind_vertex_buffers(
      renderer.encoder.self, to_span({vertex_buffer}), to_span(vertex_offsets));
  renderer.encoder->bind_index_buffer(renderer.encoder.self, index_buffer, 0,
                                      gfx::IndexType::Uint16);

  for (RRectObject const &object : params.objects)
  {
    // todo(LAMARR): transform from params
    // viewport transform, global transform, local transform
    Uniform uniform = renderer.frame_uniform_heaps[renderer.ring_index()].push(
        object.uniform);
    renderer.encoder->bind_descriptor_sets(renderer.encoder.self, {}, {});
    renderer.encoder->draw(renderer.encoder.self, 0, 6, 0, 0, 1);
  }

  renderer.encoder->end_render_pass(renderer.encoder.self);

  renderer.release(framebuffer);
}

}        // namespace ash
