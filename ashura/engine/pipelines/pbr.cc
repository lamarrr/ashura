/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipelines/pbr.h"
#include "ashura/engine/shader_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/sformat.h"

namespace ash
{

Str PBRPipeline::label()
{
  return "PBR"_str;
}

gpu::GraphicsPipeline create_pipeline(GpuFramePlan plan, Str label,
                                      gpu::Shader      shader,
                                      gpu::PolygonMode polygon_mode)
{
  u8                scratch_buffer_[1'024];
  auto &            gpu = *plan->sys();
  FallbackAllocator scratch{scratch_buffer_, gpu.allocator()};

  auto tagged_label =
    sformat(scratch, "PBR Graphics Pipeline: {}"_str, label).unwrap();

  auto raster_state =
    gpu::RasterizationState{.depth_clamp_enable = false,
                            .polygon_mode       = polygon_mode,
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
                           .depth_compare_op         = gpu::CompareOp::Greater,
                           .depth_bounds_test_enable = false,
                           .stencil_test_enable      = false,
                           .front_stencil            = {},
                           .back_stencil             = {},
                           .min_depth_bounds         = 0,
                           .max_depth_bounds         = 1};

  gpu::ColorBlendAttachmentState attachment_states[] = {
    {.blend_enable           = false,
     .src_color_blend_factor = gpu::BlendFactor::Zero,
     .dst_color_blend_factor = gpu::BlendFactor::Zero,
     .color_blend_op         = gpu::BlendOp::Add,
     .src_alpha_blend_factor = gpu::BlendFactor::Zero,
     .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
     .alpha_blend_op         = gpu::BlendOp::Add,
     .color_write_mask       = gpu::ColorComponents::All}
  };

  auto color_blend_state = gpu::ColorBlendState{
    .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
  };

  auto const & layout = gpu.descriptors_layout();

  gpu::DescriptorSetLayout const set_layouts[] = {
    layout.samplers,               // 0: samplers
    layout.sampled_textures,       // 1: textures
    layout.read_storage_buffer,    // 2: vertices
    layout.read_storage_buffer,    // 3: indices
    layout.read_storage_buffer,    // 4: items
    layout.read_storage_buffer     // 5: lights
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
    .depth_format           = gpu.depth_stencil_format(),
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

  return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

PBRPipeline::Pipeline create_pipeline(GpuFramePlan plan, Str label,
                                      gpu::Shader shader)
{
  return {
    .fill  = create_pipeline(plan, label, shader, gpu::PolygonMode::Fill),
    .line  = create_pipeline(plan, label, shader, gpu::PolygonMode::Line),
    .point = create_pipeline(plan, label, shader, gpu::PolygonMode::Point),
  };
}

PBRPipeline::PBRPipeline(Allocator allocator) : variants_{allocator}
{
}

void PBRPipeline::acquire(GpuFramePlan plan)
{
  auto id = add_variant(plan, "Base"_str,
                        sys.shader->get("PBR.Base"_str).unwrap().shader);
  CHECK(id == PipelineVariantId::Base, "");
}

PipelineVariantId PBRPipeline::add_variant(GpuFramePlan plan, Str label,
                                           gpu::Shader shader)
{
  auto pipeline = create_pipeline(plan, label, shader);
  auto id = (PipelineVariantId) variants_.push(Tuple{label, pipeline}).unwrap();
  CHECK(id == PipelineVariantId::Base, "");
  return id;
}

void PBRPipeline::remove_variant(GpuFramePlan plan, PipelineVariantId id)
{
  auto pipeline = variants_[(usize) id].v0.v1;

  variants_.erase((usize) id);

  plan->add_preframe_task([p = pipeline, d = plan->device()] {
    d->uninit(p.fill);
    d->uninit(p.line);
    d->uninit(p.point);
  });
}

void PBRPipeline::encode(gpu::CommandEncoder       e,
                         PBRPipelineParams const & params,
                         PipelineVariantId         variant)
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

  auto depth = gpu::RenderingAttachment{
    .view         = params.framebuffer.depth_stencil.depth_view,
    .resolve      = nullptr,
    .resolve_mode = gpu::ResolveModes::None,
    .load_op      = gpu::LoadOp::Load,
    .store_op     = gpu::StoreOp::Store,
    .clear        = {}};

  auto stencil = params.stencil.map([&](PipelineStencil const &) {
    return gpu::RenderingAttachment{
      .view         = params.framebuffer.depth_stencil.stencil_view,
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
                       .depth_attachment   = depth,
                       .stencil_attachment = stencil};

  e->begin_rendering(info);

  auto pipelines = variants_[(usize) variant].v0.v1;

  auto pipeline = pipelines.fill;

  switch (params.polygon_mode)
  {
    case gpu::PolygonMode::Fill:
      pipeline = pipelines.fill;
      break;
    case gpu::PolygonMode::Line:
      pipeline = pipelines.line;
      break;
    case gpu::PolygonMode::Point:
      pipeline = pipelines.point;
      break;
  }

  e->bind_graphics_pipeline(pipeline);

  e->set_graphics_state(gpu::GraphicsState{
    .scissor             = params.scissor,
    .viewport            = params.viewport,
    .blend_constant      = {1, 1, 1, 1},
    .stencil_test_enable = params.stencil.is_some(),
    .front_face_stencil =
      params.stencil.map([](auto s) { return s.front; }
           ).unwrap_or(),
    .back_face_stencil =
      params.stencil.map([](auto s) { return s.back; }
           ).unwrap_or(),
    .cull_mode                = params.cull_mode,
    .front_face               = gpu::FrontFace::CounterClockWise,
    .depth_test_enable        = true,
    .depth_compare_op         = gpu::CompareOp::Less,
    .depth_write_enable       = true,
    .depth_bounds_test_enable = false
  });
  e->bind_descriptor_sets(
    span({
      params.samplers,                               // 0: samplers
      params.textures,                               // 1: textures
      params.vertices.buffer.read_storage_buffer,    // 2: vertices
      params.indices.buffer.read_storage_buffer,     // 3: indices
      params.items.buffer.read_storage_buffer,    // 4: items
      params.lights.buffer.read_storage_buffer,      // 5: lights
    }),
    span({
      params.vertices.slice.as_u32().offset,    // 2: vertices
      params.indices.slice.as_u32().offset,     // 3: indices
      params.items.slice.as_u32().offset,    // 4: items
      params.lights.slice.as_u32().offset       // 5: lights
    }));
  e->draw({0, params.num_indices}, {0, 1});
  e->end_rendering();
}

void PBRPipeline::release(GpuFramePlan plan)
{
  for (auto [v] : variants_)
  {
    plan->add_preframe_task([p = v.v1, d = plan->device()] {
      d->uninit(p.fill);
      d->uninit(p.line);
      d->uninit(p.point);
    });
  }
}

}    // namespace ash
