/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/bezier_stencil.h"
#include "ashura/engine/passes/fill_stencil_state.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"

namespace ash
{

Str BezierStencilPass::label()
{
  return "BezierStencil"_str;
}

void BezierStencilPass::acquire()
{
  auto shader = sys->shader.get("BezierStencil"_str).unwrap().shader;

  auto raster_state =
    gpu::RasterizationState{.depth_clamp_enable = false,
                            .polygon_mode       = gpu::PolygonMode::Fill,
                            .cull_mode          = gpu::CullMode::None,
                            .front_face = gpu::FrontFace::CounterClockWise,
                            .depth_bias_enable          = false,
                            .depth_bias_constant_factor = 0,
                            .depth_bias_clamp           = 0,
                            .depth_bias_slope_factor    = 0,
                            .sample_count = sys->gpu.sample_count_};

  auto depth_stencil_state =
    gpu::DepthStencilState{.depth_test_enable        = false,
                           .depth_write_enable       = false,
                           .depth_compare_op         = gpu::CompareOp::Never,
                           .depth_bounds_test_enable = false,
                           .stencil_test_enable      = false,
                           .front_stencil            = {},
                           .back_stencil             = {},
                           .min_depth_bounds         = 0,
                           .max_depth_bounds         = 0};

  gpu::DescriptorSetLayout set_layouts[] = {
    sys->gpu.sb_layout_,    // 0: world_to_ndc
    sys->gpu.sb_layout_,    // 1: transforms
    sys->gpu.sb_layout_,    // 2: vertices
    sys->gpu.sb_layout_,    // 3: indices
    sys->gpu.sb_layout_     // 4: regions
  };

  auto pipeline_info = gpu::GraphicsPipelineInfo{
    .label         = "Bezier Stencil Graphics Pipeline"_str,
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
    .stencil_format         = sys->gpu.depth_stencil_format_,
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = 0,
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = {},
    .cache                  = sys->gpu.pipeline_cache_
  };

  pipeline_ =
    sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();
}

void BezierStencilPass::encode(gpu::CommandEncoder &           e,
                               BezierStencilPassParams const & params)
{
  auto stencil =
    gpu::RenderingAttachment{.view         = params.stencil.stencil_view,
                             .resolve      = nullptr,
                             .resolve_mode = gpu::ResolveModes::None,
                             .load_op      = gpu::LoadOp::Load,
                             .store_op     = gpu::StoreOp::Store,
                             .clear        = {}};

  auto info =
    gpu::RenderingInfo{.render_area{.extent = params.stencil.extent().xy()},
                       .num_layers         = 1,
                       .color_attachments  = {},
                       .depth_attachment   = {},
                       .stencil_attachment = stencil};

  e.begin_rendering(info);
  e.bind_graphics_pipeline(pipeline_);

  auto [front_stencil, back_stencil] =
    fill_stencil_state(params.fill_rule, params.invert, params.write_mask);

  e.set_graphics_state(gpu::GraphicsState{.scissor  = params.scissor,
                                          .viewport = params.viewport,
                                          .stencil_test_enable = false,
                                          .front_face_stencil  = front_stencil,
                                          .back_face_stencil   = back_stencil});

  e.bind_descriptor_sets(
    span({
      params.world_to_ndc.buffer.descriptor_,    // 0: world_to_ndc
      params.transforms.buffer.descriptor_,      // 1: transforms
      params.vertices.buffer.descriptor_,        // 2: vertices
      params.indices.buffer.descriptor_,         // 3: indices
      params.regions.buffer.descriptor_,         // 4: regions
    }),
    span({
      params.world_to_ndc.slice.offset,    // 0: world_to_ndc
      params.transforms.slice.offset,      // 1: transforms
      params.vertices.slice.offset,        // 2: vertices
      params.indices.slice.offset,         // 3: indices
      params.regions.slice.offset,         // 4: regions
    }));

  u32 first_index = 0;
  for (auto [i, index_count] : enumerate<u32>(params.region_index_counts))
  {
    e.draw(index_count, 1, first_index, i);
    first_index += index_count;
  }
  e.end_rendering();
}

void BezierStencilPass::release()
{
  sys->gpu.device_->uninit(pipeline_);
}

}    // namespace ash
