/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/contour_stencil.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"

namespace ash
{

void ContourStencilPass::acquire()
{
  gpu::Shader shader = sys->shader.get("Stencil"_str).unwrap().shader;

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                         gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0,
                                       .sample_count = sys->gpu.sample_count_};

  gpu::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                               gpu::CompareOp::Never,
                                             .depth_bounds_test_enable = false,
                                             .stencil_test_enable      = false,
                                             .front_stencil            = {},
                                             .back_stencil             = {},
                                             .min_depth_bounds         = 0,
                                             .max_depth_bounds         = 0};

  gpu::DescriptorSetLayout set_layouts[] = {sys->gpu.sb_layout_};

  gpu::GraphicsPipelineInfo pipeline_info{
    .label         = "Stencil Graphics Pipeline"_str,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = shader,
                                          .entry_point = "vert"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                        = shader,
                                          .entry_point                   = "frag"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .color_formats          = {},
    .depth_format           = {},
    .stencil_format         = {&sys->gpu.depth_stencil_format_, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = 0,
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = {},
    .cache                  = sys->gpu.pipeline_cache_
  };

  pipeline = sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();
}

void ContourStencilPass::encode(gpu::CommandEncoder &            e,
                                ContourStencilPassParams const & params)
{
  InplaceVec<gpu::RenderingAttachment, 1> stencil;
  stencil
    .push(gpu::RenderingAttachment{.view         = params.stencil.stencil_view,
                                   .resolve      = nullptr,
                                   .resolve_mode = gpu::ResolveModes::None,
                                   .load_op      = gpu::LoadOp::Load,
                                   .store_op     = gpu::StoreOp::Store,
                                   .clear        = {}})
    .unwrap();

  gpu::RenderingInfo info{.render_area{.extent = params.stencil.extent().xy()},
                          .num_layers         = 1,
                          .color_attachments  = {},
                          .depth_attachment   = {},
                          .stencil_attachment = stencil};

  e.begin_rendering(info);
  e.bind_graphics_pipeline(pipeline);

  auto const even_odd_fail_op =
    params.invert ? gpu::StencilOp::Keep : gpu::StencilOp::Invert;
  auto const even_odd_pass_op =
    params.invert ? gpu::StencilOp::Invert : gpu::StencilOp::Keep;

  auto const non_zero_front_fail_op =
    params.invert ? gpu::StencilOp::Keep : gpu::StencilOp::IncrementAndWrap;
  auto const non_zero_front_pass_op =
    params.invert ? gpu::StencilOp::IncrementAndWrap : gpu::StencilOp::Keep;

  auto const non_zero_back_fail_op =
    params.invert ? gpu::StencilOp::Keep : gpu::StencilOp::DecrementAndWrap;
  auto const non_zero_back_pass_op =
    params.invert ? gpu::StencilOp::DecrementAndWrap : gpu::StencilOp::Keep;

  auto const front_fail_op = (params.fill_rule == FillRule::EvenOdd) ?
                               even_odd_fail_op :
                               non_zero_front_fail_op;

  auto const front_pass_op = (params.fill_rule == FillRule::EvenOdd) ?
                               even_odd_pass_op :
                               non_zero_front_pass_op;

  auto const back_fail_op = (params.fill_rule == FillRule::EvenOdd) ?
                              even_odd_fail_op :
                              non_zero_back_fail_op;

  auto const back_pass_op = (params.fill_rule == FillRule::EvenOdd) ?
                              even_odd_pass_op :
                              non_zero_back_pass_op;

  e.set_graphics_state(gpu::GraphicsState{
    .scissor             = params.scissor,
    .viewport            = params.viewport,
    .stencil_test_enable = false,
    .front_face_stencil =
      gpu::StencilState{.fail_op       = front_fail_op,
                        .pass_op       = front_pass_op,
                        .depth_fail_op = gpu::StencilOp::Keep,
                        .compare_op    = gpu::CompareOp::Never,
                        .compare_mask  = 0,
                        .write_mask    = params.write_mask,
                        .reference     = 0},
    .back_face_stencil =
      gpu::StencilState{.fail_op       = back_fail_op,
                        .pass_op       = back_pass_op,
                        .depth_fail_op = gpu::StencilOp::Keep,
                        .compare_op    = gpu::CompareOp::Never,
                        .compare_mask  = 0,
                        .write_mask    = params.write_mask,
                        .reference     = 0}
  });

  e.bind_descriptor_sets(span({params.params_ssbo}),
                         span({params.params_ssbo_offset}));
  e.draw(3, params.num_instances, 0, params.first_instance);
  e.end_rendering();
}

void ContourStencilPass::release()
{
  sys->gpu.device_->uninit(pipeline);
}

}    // namespace ash
