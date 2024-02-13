#include "ashura/engine/passes/pbr.h"

namespace ash
{

void PBRPass::init(Pass self_, RenderServer *server, uid32 id)
{
  PBRPass               *self   = (PBRPass *) self_;
  gfx::DeviceImpl const &device = server->device;
  self->descriptor_set_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gfx::DescriptorSetLayoutDesc{
                  .label    = "PBR Descriptor Layout",
                  .bindings = to_span<gfx::DescriptorBindingDesc>(
                      {{.type = gfx::DescriptorType::Sampler, .count = 1},
                       {.type = gfx::DescriptorType::SampledImage, .count = 6},
                       {.type  = gfx::DescriptorType::UniformBuffer,
                        .count = 1}})})
          .unwrap();

  self->descriptor_heap =
      device
          ->create_descriptor_heap(device.self,
                                   {&self->descriptor_set_layout, 1}, 256,
                                   default_allocator)
          .unwrap();

  self->sampler =
      device
          ->create_sampler(
              device.self,
              gfx::SamplerDesc{
                  .label             = "PBR Sampler",
                  .mag_filter        = gfx::Filter::Linear,
                  .min_filter        = gfx::Filter::Linear,
                  .mip_map_mode      = gfx::SamplerMipMapMode::Linear,
                  .address_mode_u    = gfx::SamplerAddressMode::ClampToEdge,
                  .address_mode_v    = gfx::SamplerAddressMode::ClampToEdge,
                  .address_mode_w    = gfx::SamplerAddressMode::ClampToEdge,
                  .mip_lod_bias      = 0,
                  .anisotropy_enable = false,
                  .max_anisotropy    = 0,
                  .compare_enable    = false,
                  .compare_op        = gfx::CompareOp::Never,
                  .min_lod           = 0,
                  .max_lod           = 1,
                  .border_color      = gfx::BorderColor::FloatTransparentBlack,
                  .unnormalized_coordinates = false})
          .unwrap();

  // get shader from server
  // get or create render pass
  // get or create frame buffer for each view on view added
  self->pipeline =
      device
          ->create_graphics_pipeline(
              device.self,
              gfx::GraphicsPipelineDesc{
                  .label = "PBR Graphics Pipeline",
                  .vertex_shader =
                      gfx::ShaderStageDesc{.shader      = nullptr,
                                           .entry_point = "vs_main",
                                           .specialization_constants_data = {},
                                           .specialization_constants      = {}},
                  .fragment_shader =
                      gfx::ShaderStageDesc{.shader      = nullptr,
                                           .entry_point = "fs_main",
                                           .specialization_constants_data = {},
                                           .specialization_constants      = {}},
                  .render_pass           = gfx::RenderPass{},
                  .vertex_input_bindings = to_span<gfx::VertexInputBinding>({}),
                  .vertex_attributes     = to_span<gfx::VertexAttribute>({}),
                  .push_constant_size    = gfx::MAX_PUSH_CONSTANT_SIZE,
                  .descriptor_set_layouts = {&self->descriptor_set_layout, 1},
                  .primitive_topology = gfx::PrimitiveTopology::TriangleList,
                  .rasterization_state =
                      gfx::PipelineRasterizationState{
                          .depth_clamp_enable = false,
                          .polygon_mode       = gfx::PolygonMode::Fill,
                          .cull_mode          = gfx::CullMode::None,
                          .front_face        = gfx::FrontFace::CounterClockWise,
                          .depth_bias_enable = false,
                          .depth_bias_constant_factor = 0,
                          .depth_bias_clamp           = 0,
                          .depth_bias_slope_factor    = 0},
                  .depth_stencil_state =
                      gfx::PipelineDepthStencilState{
                          .depth_test_enable        = true,
                          .depth_write_enable       = true,
                          .depth_compare_op         = gfx::CompareOp::Greater,
                          .depth_bounds_test_enable = false,
                          .stencil_test_enable      = false,
                          .front_stencil            = gfx::StencilOpState{},
                          .back_stencil             = gfx::StencilOpState{},
                          .min_depth_bounds         = 0,
                          .max_depth_bounds         = 0},
                  .color_blend_state =
                      gfx::PipelineColorBlendState{
                          .logic_op_enable = true,
                          .logic_op        = gfx::LogicOp::Set,
                          .attachments =
                              to_span<gfx::PipelineColorBlendAttachmentState>(
                                  {{.blend_enable = false,
                                    .src_color_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .dst_color_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .color_blend_op = gfx::BlendOp::Add,
                                    .src_alpha_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .dst_alpha_blend_factor =
                                        gfx::BlendFactor::Zero,
                                    .alpha_blend_op = gfx::BlendOp::Add,
                                    .color_write_mask =
                                        gfx::ColorComponents::All}}),
                          .blend_constant = {1, 1, 1, 1}},
                  .cache = server->pipeline_cache})
          .unwrap();
}

}        // namespace ash