/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/sdf.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/sformat.h"

namespace ash
{

SdfPass::SdfPass(AllocatorRef allocator) : variants_{allocator}
{
}

Str SdfPass::label()
{
  return "SDF"_str;
}

gpu::GraphicsPipeline create_pipeline(Str label, gpu::Shader shader)
{
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

  gpu::DescriptorSetLayout set_layouts[] = {
    sys->gpu.samplers_layout_,    // 0: samplers
    sys->gpu.textures_layout_,    // 1: textures
    sys->gpu.sb_layout_,          // 2: world_to_ndc
    sys->gpu.sb_layout_,          // 3: shapes
    sys->gpu.sb_layout_,          // 4: transforms
    sys->gpu.sb_layout_           // 5: materials
  };

  auto tagged_label =
    snformat<gpu::MAX_LABEL_SIZE>("SDF Graphics Pipeline: {}"_str, label)
      .unwrap();

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
    .color_formats          = {&sys->gpu.color_format_, 1},
    .depth_format           = {},
    .stencil_format         = sys->gpu.depth_stencil_format_,
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = 0,
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache_
  };

  return sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();
}

void SdfPass::acquire()
{
  auto id =
    add_variant("Base"_str, sys->shader.get("SDF.Base"_str).unwrap().shader);
  CHECK(id == ShaderVariantId::Base, "");
}

ShaderVariantId SdfPass::add_variant(Str label, gpu::Shader shader)
{
  auto pipeline = create_pipeline(label, shader);
  auto id       = variants_.push(Tuple{label, pipeline}).unwrap();
  return (ShaderVariantId) id;
}

void SdfPass::remove_variant(ShaderVariantId id)
{
  auto pipeline = variants_[(usize) id].v0;
  variants_.erase((usize) id);
  sys->gpu.release(pipeline.v1);
}

void SdfPass::encode(gpu::CommandEncoder & e, SdfPassParams const & params,
                     ShaderVariantId variant)
{
  InplaceVec<gpu::RenderingAttachment, 1> color;

  params.framebuffer.color_msaa.match(
    [&](ColorMsaaTexture const & tex) {
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

  auto stencil = params.stencil.map([&](PassStencil const &) {
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
                       .depth_attachment   = {},
                       .stencil_attachment = stencil};

  auto pipeline = variants_[(usize) variant].v0.v1;

  e.begin_rendering(info);
  e.bind_graphics_pipeline(pipeline);
  e.set_graphics_state(gpu::GraphicsState{
    .scissor             = params.scissor,
    .viewport            = params.viewport,
    .stencil_test_enable = params.stencil.is_some(),
    .front_face_stencil =
      params.stencil.map([](auto s) { return s.front; }).unwrap_or(),
    .back_face_stencil =
      params.stencil.map([](auto s) { return s.back; }).unwrap_or()});
  e.bind_descriptor_sets(
    span({
      params.samplers,                           // 0: samplers
      params.textures,                           // 1: textures
      params.world_to_ndc.buffer.descriptor_,    // 2: world_to_ndc
      params.shapes.buffer.descriptor_,          // 3: shapes
      params.transforms.buffer.descriptor_,      // 4: transforms
      params.materials.buffer.descriptor_        // 5: materials
    }),
    span({
      params.world_to_ndc.slice.offset,    // 2: world_to_ndc
      params.shapes.slice.offset,          // 3: shapes
      params.transforms.slice.offset,      // 4: transforms
      params.materials.slice.offset        // 5: materials
    }));
  e.draw(4, params.instances.span, 0, params.instances.offset);
  e.end_rendering();
}

void SdfPass::release()
{
  for (auto [v] : variants_)
  {
    sys->gpu.device_->uninit(v.v1);
  }
}

}    // namespace ash
