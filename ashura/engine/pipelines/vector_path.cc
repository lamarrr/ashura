/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipelines/vector_path.h"
#include "ashura/engine/pipelines/fill_rule_stencil.h"
#include "ashura/engine/shader_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"
#include "ashura/std/sformat.h"

namespace ash
{

Str VectorPathPipeline::label()
{
  return "VectorPath"_str;
}

gpu::GraphicsPipeline create_coverage_pipeline(GpuFramePlan plan, Str label,
                                               gpu::Shader shader)
{
  u8                scratch_buffer_[1'024];
  auto &            gpu = *plan->sys();
  FallbackAllocator scratch{scratch_buffer_, gpu.allocator()};

  auto tagged_label =
    sformat(scratch, "VectorPath Coverage Graphics Pipeline: {}"_str, label)
      .unwrap();

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

  auto color_blend_state = gpu::ColorBlendState{
    .attachments = {},
      .blend_constant = {1, 1, 1, 1}
  };

  auto const & layout = gpu.descriptors_layout();

  gpu::DescriptorSetLayout set_layouts[] = {
    layout.read_storage_buffer,      // 0: world_to_ndc
    layout.read_storage_buffer,      // 1: items
    layout.read_storage_buffer,      // 2: vertices
    layout.read_storage_buffer,      // 3: indices
    layout.storage_texel_buffers,    // 4: alpha_masks
    layout.storage_texel_buffers     // 5: fill_ids
  };

  auto pipeline_info = gpu::GraphicsPipelineInfo{
    .label         = tagged_label,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = shader,
                                          .entry_point = "vert"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                        = shader,
                                          .entry_point                   = "frag"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .color_formats          = span({gpu.color_format()}
      ),
    .depth_format           = {},
    .stencil_format         = gpu.depth_stencil_format(),
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(shader::VectorPathCfg),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache()
  };

  return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

gpu::GraphicsPipeline create_fill_pipeline(GpuFramePlan plan, Str label,
                                           gpu::Shader shader)
{
  u8                scratch_buffer_[1'024];
  auto &            gpu = *plan->sys();
  FallbackAllocator scratch{scratch_buffer_, gpu.allocator()};

  auto tagged_label =
    sformat(scratch, "VectorPath Fill Graphics Pipeline: {}"_str, label)
      .unwrap();

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

  gpu::ColorBlendAttachmentState attachment_states[] = {
    {.blend_enable           = true,
     .src_color_blend_factor = gpu::BlendFactor::SrcAlpha,
     .dst_color_blend_factor = gpu::BlendFactor::OneMinusSrcAlpha,
     .color_blend_op         = gpu::BlendOp::Add,
     .src_alpha_blend_factor = gpu::BlendFactor::One,
     .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
     .alpha_blend_op         = gpu::BlendOp::Add,
     .color_write_mask       = gpu::ColorComponents::All}
  };

  auto color_blend_state = gpu::ColorBlendState{
    .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
  };

  auto const & layout = gpu.descriptors_layout();

  gpu::DescriptorSetLayout set_layouts[] = {
    layout.samplers,               // 0: samplers
    layout.sampled_textures,       // 1: textures
    layout.read_storage_buffer,    // 2: world_to_ndc
    layout.read_storage_buffer,    // 3: sets
    layout.read_storage_buffer,    // 4: vertices
    layout.read_storage_buffer     // 5: indices
  };

  auto pipeline_info = gpu::GraphicsPipelineInfo{
    .label         = tagged_label,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = shader,
                                          .entry_point = "vert"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                        = shader,
                                          .entry_point                   = "frag"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .color_formats          = span({gpu.color_format()}
      ),
    .depth_format           = {},
    .stencil_format         = gpu.depth_stencil_format(),
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(shader::VectorPathCfg),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache()
  };

  return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

VectorPathPipeline::VectorPathPipeline(Allocator allocator) :
  coverage_pipeline_{nullptr},
  fill_pipelines_{allocator}
{
}

void VectorPathPipeline::acquire(GpuFramePlan plan)
{
  auto id = add_fill_variant(
    plan, "Base"_str, sys.shader->get("VectorPath.Base"_str).unwrap().shader);
  CHECK(id == PipelineVariantId::Base, "");
}

PipelineVariantId VectorPathPipeline::add_fill_variant(GpuFramePlan plan,
                                                       Str          label,
                                                       gpu::Shader  shader)
{
  auto pipeline = create_fill_pipeline(plan, label, shader);
  auto id       = fill_pipelines_.push(Tuple{label, pipeline}).unwrap();
  return (PipelineVariantId) id;
}

void VectorPathPipeline::remove_fill_variant(GpuFramePlan      plan,
                                             PipelineVariantId id)
{
  auto pipeline = fill_pipelines_[(usize) id];
  fill_pipelines_.erase((usize) id);
  plan->add_preframe_task(
    [p = pipeline.v0, d = plan->device()] { d->uninit(p.v1); });
}

void VectorPathPipeline::encode(gpu::CommandEncoder                      e,
                                VectorPathCoveragePipelineParams const & params)
{
  auto stencil =
    gpu::RenderingAttachment{.view         = params.stencil.stencil_view,
                             .resolve      = nullptr,
                             .resolve_mode = gpu::ResolveModes::None,
                             .load_op      = gpu::LoadOp::Load,
                             .store_op     = gpu::StoreOp::None,
                             .clear        = {}};

  auto info =
    gpu::RenderingInfo{.render_area{.extent = params.stencil.extent().xy()},
                       .num_layers         = 1,
                       .color_attachments  = {},
                       .depth_attachment   = {},
                       .stencil_attachment = stencil};

  e->begin_rendering(info);

  e->bind_graphics_pipeline(coverage_pipeline_);
  e->push_constants(as_u8_span(params.cfg));
  e->bind_descriptor_sets(
    span({
      params.world_to_ndc.buffer.read_storage_buffer,      //
      params.coverage_items.buffer.read_storage_buffer,    //
      params.vertices.buffer.read_storage_buffer,          //
      params.indices.buffer.read_storage_buffer,           //
      params.write_alpha_masks,                            //
      params.write_fill_ids                                //
    }),
    span({
      params.world_to_ndc.slice.as_u32().offset,      //
      params.coverage_items.slice.as_u32().offset,    //
      params.vertices.slice.as_u32().offset,          //
      params.indices.slice.as_u32().offset            //
    }));

  CHECK(size32(params.states) > 0, "");
  CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
  CHECK(size32(params.index_runs) > 1, "");
  auto num_states = size32(params.states);

  for (u32 i = 0; i < num_states; i++)
  {
    auto & state       = params.states[i];
    auto [front, back] = fill_rule_stencil(FillRule::NonZero, false, U32_MAX);

    e->set_graphics_state(gpu::GraphicsState{.scissor  = state.scissor,
                                             .viewport = state.viewport,
                                             .stencil_test_enable = false,
                                             .front_face_stencil  = front,
                                             .back_face_stencil   = back,
                                             .cull_mode = gpu::CullMode::None});

    for (auto i :
         range(Slice32::range(params.state_runs[i], params.state_runs[i + 1])))
    {
      e->draw(Slice32::range(params.index_runs[i], params.index_runs[i + 1]),
              {i, 1});
    }
  }

  e->end_rendering();
}

void VectorPathPipeline::encode(gpu::CommandEncoder                  e,
                                VectorPathFillPipelineParams const & params)
{
  InplaceVec<gpu::RenderingAttachment, 1> color;

  params.framebuffer.color_msaa.match(
    [&](ColorMsaaImage const & tex) {
      color
        .push(
          gpu::RenderingAttachment{.view    = tex.view,
                                   .resolve = params.framebuffer.color.view,
                                   .resolve_mode = gpu::ResolveModes::Average,
                                   .load_op      = gpu::LoadOp::Load,
                                   .store_op     = gpu::StoreOp::Store,
                                   .clear        = {}})
        .unwrap();
    },
    [&]() {
      color
        .push(gpu::RenderingAttachment{.view    = params.framebuffer.color.view,
                                       .resolve = nullptr,
                                       .resolve_mode = gpu::ResolveModes::None,
                                       .load_op      = gpu::LoadOp::Load,
                                       .store_op     = gpu::StoreOp::Store,
                                       .clear        = {}})
        .unwrap();
    });

  auto stencil = params.framebuffer.depth_stencil.map([&](auto const & s) {
    return gpu::RenderingAttachment{.view         = s.stencil_view,
                                    .resolve      = nullptr,
                                    .resolve_mode = gpu::ResolveModes::None,
                                    .load_op      = gpu::LoadOp::Load,
                                    .store_op     = gpu::StoreOp::None,
                                    .clear        = {}};
  });

  auto info =
    gpu::RenderingInfo{.render_area{.extent = params.framebuffer.extent().xy()},
                       .num_layers         = 1,
                       .color_attachments  = color,
                       .depth_attachment   = {},
                       .stencil_attachment = stencil};

  auto pipeline = fill_pipelines_[(usize) params.variant].v0.v1;

  e->begin_rendering(info);
  e->bind_graphics_pipeline(pipeline);
  e->push_constants(as_u8_span(params.cfg));
  e->bind_descriptor_sets(
    span({
      params.samplers,                                   // 0: samplers
      params.textures,                                   // 1: textures
      params.world_to_ndc.buffer.read_storage_buffer,    // 2: world_to_ndc
      params.fill_items.buffer.read_storage_buffer,      // 3: fill_items
      params.read_alpha_masks,                           // 4: read_alpha_masks
      params.read_fill_ids                               // 5: read_fill_ids
    }),
    span({
      params.world_to_ndc.slice.as_u32().offset,    // 2: world_to_ndc
      params.fill_items.slice.as_u32().offset       // 3: fill_items
    }));

  CHECK(size32(params.states) > 0, "");
  CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
  auto num_states = size32(params.states);

  for (auto i : range(num_states))
  {
    auto & state = params.states[i];
    auto   non_zero_stencil =
      gpu::StencilState{.fail_op       = gpu::StencilOp::Keep,
                        .pass_op       = gpu::StencilOp::Keep,
                        .depth_fail_op = gpu::StencilOp::Keep,
                        .compare_op    = gpu::CompareOp::Greater,
                        .compare_mask  = 0xFF,
                        .write_mask    = 0x00,
                        .reference     = 0x00};

    e->set_graphics_state(
      gpu::GraphicsState{.scissor             = state.scissor,
                         .viewport            = state.viewport,
                         .stencil_test_enable = true,
                         .front_face_stencil  = non_zero_stencil,
                         .back_face_stencil   = non_zero_stencil});

    e->draw({0, 4},
            Slice32::range(params.state_runs[i], params.state_runs[i + 1]));
  }
  e->end_rendering();
}

void VectorPathPipeline::release(GpuFramePlan plan)
{
  plan->add_preframe_task(
    [p = coverage_pipeline_, d = plan->device()] { d->uninit(p); });
  for (auto [v] : fill_pipelines_)
  {
    plan->add_preframe_task([p = v.v1, d = plan->device()] { d->uninit(p); });
  }
}

}    // namespace ash
