#include "ashura/renderer/passes/pbr.h"

namespace ash
{

void PBRPass::init(Renderer &renderer)
{
  constexpr Array bindings_desc = PBRShaderParameter::GET_BINDINGS_DESC();
  descriptor_set_layout =
      renderer.device
          ->create_descriptor_set_layout(
              renderer.device.self,
              gfx::DescriptorSetLayoutDesc{.label    = "PBR Parameters",
                                           .bindings = to_span(bindings_desc)})
          .unwrap();

  render_pass =
      renderer.device
          ->create_render_pass(
              renderer.device.self,
              gfx::RenderPassDesc{
                  .label             = "PBR RenderPass",
                  .color_attachments = to_span<gfx::RenderPassAttachment>(
                      {{.format           = renderer.color_format,
                        .load_op          = gfx::LoadOp::Load,
                        .store_op         = gfx::StoreOp::Store,
                        .stencil_load_op  = gfx::LoadOp::DontCare,
                        .stencil_store_op = gfx::StoreOp::DontCare}}),
                  .input_attachments = {},
                  .depth_stencil_attachment =
                      {.format           = renderer.depth_stencil_format,
                       .load_op          = gfx::LoadOp::Clear,
                       .store_op         = gfx::StoreOp::Store,
                       .stencil_load_op  = gfx::LoadOp::DontCare,
                       .stencil_store_op = gfx::StoreOp::DontCare},
              })
          .unwrap();

  gfx::Shader vertex_shader   = renderer.get_shader("PBR.VS"_span).unwrap();
  gfx::Shader fragment_shader = renderer.get_shader("PBR.FS"_span).unwrap();

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

  gfx::PipelineColorBlendState color_blend_state{
      .logic_op_enable = true,
      .logic_op        = gfx::LogicOp::Set,
      .attachments     = to_span<gfx::PipelineColorBlendAttachmentState>(
          {{.blend_enable           = false,
                .src_color_blend_factor = gfx::BlendFactor::Zero,
                .dst_color_blend_factor = gfx::BlendFactor::Zero,
                .color_blend_op         = gfx::BlendOp::Add,
                .src_alpha_blend_factor = gfx::BlendFactor::Zero,
                .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
                .alpha_blend_op         = gfx::BlendOp::Add,
                .color_write_mask       = gfx::ColorComponents::All}}),
      .blend_constant = {1, 1, 1, 1}};

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label = "PBR Graphics Pipeline",
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
          to_span({renderer.uniform_layout, renderer.uniform_layout,
                   descriptor_set_layout}),
      .primitive_topology  = gfx::PrimitiveTopology::TriangleList,
      .rasterization_state = raster_state,
      .depth_stencil_state = depth_stencil_state,
      .color_blend_state   = color_blend_state,
      .cache               = renderer.pipeline_cache};

  pipeline = renderer.device
                 ->create_graphics_pipeline(renderer.device.self, pipeline_desc)
                 .unwrap();

  pipeline_desc.rasterization_state.polygon_mode = gfx::PolygonMode::Line;

  wireframe_pipeline =
      renderer.device
          ->create_graphics_pipeline(renderer.device.self, pipeline_desc)
          .unwrap();
}

void PBRPass::add_pass(Renderer &renderer, PBRParams const &params)
{
  ENSURE(params.render_target.color_images.size() != 0);
  ENSURE(has_bits(params.render_target.depth_stencil_aspects,
                  gfx::ImageAspects::Depth));

  gfx::Framebuffer framebuffer =
      renderer.device
          ->create_framebuffer(
              renderer.device.self,
              gfx::FramebufferDesc{.label       = "PBR Framebuffer",
                                   .render_pass = render_pass,
                                   .extent      = params.render_target.extent,
                                   .color_attachments = to_span(
                                       params.render_target.color_images),
                                   .depth_stencil_attachment =
                                       params.render_target.depth_stencil_image,
                                   .layers = 1})
          .unwrap();

  renderer.encoder->begin_render_pass(
      renderer.encoder.self, framebuffer, render_pass,
      params.render_target.render_offset, params.render_target.render_extent,
      {}, {});

  Uniform lights_uniform =
      renderer.frame_uniform_heaps[renderer.ring_index()].push(params.lights);

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
      renderer.encoder->bind_graphics_pipeline(renderer.encoder.self,
                                               object_pipeline);
      renderer.encoder->set_scissor(renderer.encoder.self,
                                    params.render_target.render_offset,
                                    params.render_target.render_extent);
      renderer.encoder->set_viewport(
          renderer.encoder.self,
          gfx::Viewport{
              .offset = {}, .extent = {}, .min_depth = 0, .max_depth = 1});
      prev_pipeline = object_pipeline;
    }

    Uniform object_uniform =
        renderer.frame_uniform_heaps[renderer.ring_index()].push(
            object.uniform);
    if (prev_vtx_buff != object.mesh.vertex_buffer ||
        prev_vtx_buff_offset != object.mesh.vertex_buffer_offset)
    {
      renderer.encoder->bind_vertex_buffers(
          renderer.encoder.self, to_span({object.mesh.vertex_buffer}),
          to_span({object.mesh.vertex_buffer_offset}));
      prev_vtx_buff        = object.mesh.vertex_buffer;
      prev_vtx_buff_offset = object.mesh.vertex_buffer_offset;
    }
    if (prev_idx_buff != object.mesh.index_buffer ||
        prev_idx_buff_offset != object.mesh.index_buffer_offset)
    {
      renderer.encoder->bind_index_buffer(
          renderer.encoder.self, object.mesh.index_buffer,
          object.mesh.index_buffer_offset, object.mesh.index_type);
      prev_idx_buff        = object.mesh.index_buffer;
      prev_idx_buff_offset = object.mesh.index_buffer_offset;
    }

    gfx::DescriptorSet sets[]{lights_uniform.set, object_uniform.set,
                              object.descriptor};
    u32 offsets[]{lights_uniform.buffer_offset, object_uniform.buffer_offset};

    renderer.encoder->bind_descriptor_sets(renderer.encoder.self, to_span(sets),
                                           to_span(offsets));

    renderer.encoder->draw(renderer.encoder.self, object.mesh.first_index,
                           object.mesh.num_indices, object.mesh.vertex_offset,
                           0, 1);
  }

  renderer.encoder->end_render_pass(renderer.encoder.self);

  renderer.release(framebuffer);
}

void PBRPass::uninit(Renderer &renderer)
{
  renderer.device->unref_descriptor_set_layout(renderer.device.self,
                                               descriptor_set_layout);
  renderer.device->unref_render_pass(renderer.device.self, render_pass);
  renderer.device->unref_graphics_pipeline(renderer.device.self, pipeline);
}

}        // namespace ash