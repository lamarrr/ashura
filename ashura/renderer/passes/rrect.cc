#include "ashura/renderer/passes/rrect.h"

namespace ash
{

void RRectPass::init(Renderer &renderer)
{
  constexpr auto bindings_desc = RRectShaderParameter::GET_BINDINGS_DESC();
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
                        .load_op          = gfx::LoadOp::Clear,
                        .store_op         = gfx::StoreOp::Store,
                        .stencil_load_op  = gfx::LoadOp::DontCare,
                        .stencil_store_op = gfx::StoreOp::DontCare}}),
                  .input_attachments = {},
                  .depth_stencil_attachment =
                      {.format           = gfx::Format::Undefined,
                       .load_op          = gfx::LoadOp::DontCare,
                       .store_op         = gfx::StoreOp::DontCare,
                       .stencil_load_op  = gfx::LoadOp::DontCare,
                       .stencil_store_op = gfx::StoreOp::Store},
              })
          .unwrap();

  gfx::Shader vertex_shader   = renderer.get_shader("VS::RRect"_span).unwrap();
  gfx::Shader fragment_shader = renderer.get_shader("FS::RRect"_span).unwrap();

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
      .vertex_input_bindings = {},
      .vertex_attributes     = {},
      .push_constant_size    = 0,
      .descriptor_set_layouts =
          to_span({renderer.uniform_layout,
                   descriptor_set_layout}),        // TODO(lamarrr): MVP, SHADER
                                                   // PARAMS, global lights
      .primitive_topology  = gfx::PrimitiveTopology::TriangleList,
      .rasterization_state = raster_state,
      .depth_stencil_state = depth_stencil_state,
      .color_blend_state   = color_blend_state,
      .cache               = renderer.pipeline_cache};

  pipeline = renderer.device
                 ->create_graphics_pipeline(renderer.device.self, pipeline_desc)
                 .unwrap();
}

}        // namespace ash
