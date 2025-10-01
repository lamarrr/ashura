/// SPDX-License-Identifier: MIT
#include "ashura/engine/pipelines/quad.h"
#include "ashura/engine/shader_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/range.h"
#include "ashura/std/sformat.h"

namespace ash
{

QuadPipeline::QuadPipeline(Allocator allocator) : variants_{allocator}
{
}

Str QuadPipeline::label()
{
  return "Quad"_str;
}

gpu::GraphicsPipeline create_pipeline(GpuFramePlan plan, Str label,
                                      gpu::Shader shader)
{
  u8     scratch_buffer_[1'024];
  auto & gpu = *plan->sys();

  FallbackAllocator scratch_{scratch_buffer_, gpu.allocator()};

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
    layout.read_storage_buffer,    // 3: quads
  };

  auto tagged_label =
    sformat(scratch_, "Quad Graphics Pipeline: {}"_str, label).unwrap();

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
    .push_constants_size    = 0,
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache()
  };

  return gpu.device()->create_graphics_pipeline(pipeline_info).unwrap();
}

void QuadPipeline::acquire(GpuFramePlan plan)
{
  auto id = add_variant(plan, "Base"_str,
                        sys.shader->get("Quad.Base"_str).unwrap().shader);
  CHECK(id == PipelineVariantId::Base, "");
}

PipelineVariantId QuadPipeline::add_variant(GpuFramePlan plan, Str label,
                                            gpu::Shader shader)
{
  auto pipeline = create_pipeline(plan, label, shader);
  auto id       = variants_.push(Tuple{label, pipeline}).unwrap();
  return (PipelineVariantId) id;
}

void QuadPipeline::remove_variant(GpuFramePlan plan, PipelineVariantId id)
{
  auto pipeline = variants_[(usize) id].v0.v1;
  variants_.erase((usize) id);
  plan->add_preframe_task([p = pipeline, d = plan->device()] { d->uninit(p); });
}

void QuadPipeline::encode(gpu::CommandEncoder        e,
                          QuadPipelineParams const & params,
                          PipelineVariantId          variant)
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

  auto pipeline = variants_[(usize) variant].v0.v1;

  e->begin_rendering(info);
  e->bind_graphics_pipeline(pipeline);
  e->bind_descriptor_sets(
    span({
      params.samplers,                                   // 0: samplers
      params.textures,                                   // 1: textures
      params.world_to_ndc.buffer.read_storage_buffer,    // 2: world_to_ndc
      params.quads.buffer.read_storage_buffer,           // 3: quads
    }),
    span({
      params.world_to_ndc.slice.as_u32().offset,    // 2: world_to_ndc
      params.quads.slice.as_u32().offset            // 3: quads
    }));

  CHECK(size32(params.states) > 0, "");
  CHECK(size32(params.state_runs) == (size32(params.states) + 1), "");
  auto num_states = size32(params.states);

  for (auto i : range(num_states))
  {
    auto & state = params.states[i];

    e->set_graphics_state(gpu::GraphicsState{
      .scissor             = state.scissor,
      .viewport            = state.viewport,
      .stencil_test_enable = state.stencil.is_some(),
      .front_face_stencil =
        state.stencil.map([](auto s) { return s.front; }).unwrap_or(),
      .back_face_stencil =
        state.stencil.map([](auto s) { return s.back; }).unwrap_or()});

    e->draw({0, 4},
            Slice32::range(params.state_runs[i], params.state_runs[i + 1]));
  }

  e->end_rendering();
}

void QuadPipeline::release(GpuFramePlan plan)
{
  for (auto [v] : variants_)
  {
    plan->add_preframe_task([d = plan->device(), p = v.v1] { d->uninit(p); });
  }
}

}    // namespace ash
