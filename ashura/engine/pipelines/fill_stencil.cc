/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipelines/fill_stencil.h"
#include "ashura/engine/pipelines/fill_rule_stencil.h"
#include "ashura/engine/shader_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"
#include "ashura/std/sformat.h"

namespace ash
{

Str FillStencilPipeline::label()
{
  return "FillStencil"_str;
}

FillStencilPipeline::FillStencilPipeline(Allocator)
{
}

void FillStencilPipeline::acquire(GpuFramePlan plan)
{
  u8                scratch_buffer_[1'024];
  auto &            gpu = *plan->sys();
  FallbackAllocator scratch{scratch_buffer_, gpu.allocator()};

  auto tagged_label =
    sformat(scratch, "Fill Stencil Graphics Pipeline"_str).unwrap();

  auto raster_state =
    gpu::RasterizationState{.depth_clamp_enable = false,
                            .polygon_mode       = gpu::PolygonMode::Fill,
                            .cull_mode          = gpu::CullMode::None,
                            .front_face = gpu::FrontFace::CounterClockWise,
                            .depth_bias_enable          = false,
                            .depth_bias_constant_factor = 0,
                            .depth_bias_clamp           = 0,
                            .depth_bias_slope_factor    = 0,
                            .sample_count               = gpu.sample_count()};

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

  auto color_blend_state =
    gpu::ColorBlendState{.attachments = {}, .blend_constant = {}};

  auto const & layout = gpu.descriptors_layout();

  gpu::DescriptorSetLayout set_layouts[] = {
    layout.read_storage_buffer,    // 0: world_to_ndc
    layout.read_storage_buffer,    // 1: world_transforms
    layout.read_storage_buffer,    // 2: vertices
    layout.read_storage_buffer,    // 3: indices
  };

  auto shader = sys.shader->get("FillStencil"_str).unwrap().shader;

  auto pipeline_info = gpu::GraphicsPipelineInfo{
    .label                  = tagged_label,
    .vertex_shader          = gpu::ShaderStageInfo{.shader      = shader,
                                                   .entry_point = "vert"_str,
                                                   .specialization_constants      = {},
                                                   .specialization_constants_data = {}},
    .fragment_shader        = {},
    .color_formats          = {},
    .depth_format           = {},
    .stencil_format         = gpu.depth_stencil_format(),
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = 0,
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache()
  };

  pipeline_ = gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

void FillStencilPipeline::encode(gpu::CommandEncoder               e,
                                 FillStencilPipelineParams const & params)
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

  e->begin_rendering(info);

  e->bind_graphics_pipeline(pipeline_);
  e->bind_descriptor_sets(
    span({
      params.world_to_ndc.buffer.read_storage_buffer,    // 0: world_to_ndc
      params.world_transforms.buffer
        .read_storage_buffer,                        // 1: world_transforms
      params.vertices.buffer.read_storage_buffer,    // 2: vertices
      params.indices.buffer.read_storage_buffer,     // 3: indices
    }),
    span({
      params.world_to_ndc.slice.as_u32().offset,        // 0: world_to_ndc
      params.world_transforms.slice.as_u32().offset,    // 1: world_transforms
      params.vertices.slice.as_u32().offset,            // 2: vertices
      params.indices.slice.as_u32().offset,             // 3: indices
    }));

  u32 first_index = 0;
  for (auto [i, index_count, write_mask] :
       zip(range(size32(params.index_counts)), params.index_counts,
           params.write_masks))
  {
    auto [front_stencil, back_stencil] =
      fill_rule_stencil(params.fill_rule, params.invert, write_mask);
    e->set_graphics_state(
      gpu::GraphicsState{.scissor             = params.scissor,
                         .viewport            = params.viewport,
                         .stencil_test_enable = false,
                         .front_face_stencil  = front_stencil,
                         .back_face_stencil   = back_stencil});
    e->draw({first_index, index_count}, {  i, 1});
    first_index += index_count;
  }

  e->end_rendering();
}

void FillStencilPipeline::release(GpuFramePlan plan)
{
  plan->add_preframe_task(
    [d = plan->device(), p = pipeline_] { d->uninit(p); });
}

}    // namespace ash
