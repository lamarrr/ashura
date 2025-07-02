/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/pbr.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/sformat.h"

namespace ash
{

Str PBRPass::label()
{
  return "PBR"_str;
}

gpu::GraphicsPipeline create_pipeline(Str label, gpu::Shader shader,
                                      gpu::PolygonMode polygon_mode)
{
  auto tagged_label =
    snformat<gpu::MAX_LABEL_SIZE>("PBR Graphics Pipeline: {}"_str, label)
      .unwrap();

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode       = polygon_mode,
                                       .cull_mode = gpu::CullMode::None,
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
                                               gpu::CompareOp::Greater,
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

  gpu::ColorBlendState color_blend_state{
    .attachments = attachment_states, .blend_constant = {1, 1, 1, 1}
  };

  gpu::DescriptorSetLayout const set_layouts[] = {
    sys->gpu.samplers_layout_,    // 0: samplers
    sys->gpu.textures_layout_,    // 1: textures
    sys->gpu.sb_layout_,          // 2: vertices
    sys->gpu.sb_layout_,          // 3: indices
    sys->gpu.sb_layout_,          // 4: world constantts
    sys->gpu.sb_layout_,          // 5: materials
    sys->gpu.sb_layout_           // 6: lights
  };

  gpu::GraphicsPipelineInfo pipeline_info{
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
    .depth_format           = sys->gpu.depth_stencil_format_,
    .stencil_format         = sys->gpu.depth_stencil_format_,
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = 0,
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache_
  };

  return sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();
}

PBRPass::Pipeline create_pipeline(Str label, gpu::Shader shader)
{
  return {
    .fill  = create_pipeline(label, shader, gpu::PolygonMode::Fill),
    .line  = create_pipeline(label, shader, gpu::PolygonMode::Line),
    .point = create_pipeline(label, shader, gpu::PolygonMode::Point),
  };
}

PBRPass::PBRPass(AllocatorRef allocator) : variants_{allocator}
{
}

void PBRPass::acquire()
{
  auto id =
    add_variant("Base"_str, sys->shader.get("PBR.Base"_str).unwrap().shader);
  CHECK(id == ShaderVariantId::Base, "");
}

ShaderVariantId PBRPass::add_variant(Str label, gpu::Shader shader)
{
  auto pipeline = create_pipeline(label, shader);
  auto id = (ShaderVariantId) variants_.push(Tuple{label, pipeline}).unwrap();
  CHECK(id == ShaderVariantId::Base, "");
  return id;
}

void PBRPass::remove_variant(ShaderVariantId id)
{
  auto pipeline = variants_[(usize) id].v0.v1;
  sys->gpu.release(pipeline.fill);
  sys->gpu.release(pipeline.line);
  sys->gpu.release(pipeline.point);
  variants_.erase((usize) id);
}

void PBRPass::encode(gpu::CommandEncoder & e, PBRPassParams const & params,
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

  auto depth =
    gpu::RenderingAttachment{.view    = params.framebuffer.depth_stencil.view,
                             .resolve = nullptr,
                             .resolve_mode = gpu::ResolveModes::None,
                             .load_op      = gpu::LoadOp::Load,
                             .store_op     = gpu::StoreOp::Store,
                             .clear        = {}};

  auto stencil = params.stencil.map([&](PassStencil const &) {
    return gpu::RenderingAttachment{
      .view         = params.framebuffer.depth_stencil.stencil_view,
      .resolve      = nullptr,
      .resolve_mode = gpu::ResolveModes::None,
      .load_op      = gpu::LoadOp::Load,
      .store_op     = gpu::StoreOp::None,
      .clear        = {}};
  });

  gpu::RenderingInfo info{
    .render_area{.extent = params.framebuffer.extent().xy()},
    .num_layers         = 1,
    .color_attachments  = color,
    .depth_attachment   = depth,
    .stencil_attachment = stencil};

  e.begin_rendering(info);

  auto pipelines = variants_[(usize) variant].v0.v1;

  gpu::GraphicsPipeline pipeline = pipelines.fill;

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

  e.bind_graphics_pipeline(pipeline);

  e.set_graphics_state(gpu::GraphicsState{
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
    .depth_test_enable  = true,
    .depth_compare_op   = gpu::CompareOp::Less,
    .depth_write_enable = true
  });
  e.bind_descriptor_sets(span({
                           params.samplers,                       //
                           params.textures,                       //
                           params.vertices.buffer.descriptor_,    //
                           params.indices.buffer.descriptor_,     //
                           params.world.buffer.descriptor_,       //
                           params.material.buffer.descriptor_,    //
                           params.lights.buffer.descriptor_,      //
                         }),
                         span({
                           params.vertices.slice.offset,    //
                           params.indices.slice.offset,     //
                           params.world.slice.offset,       //
                           params.material.slice.offset,    //
                           params.lights.slice.offset       //
                         }));
  e.draw(params.num_indices, 1, 0, 0);
  e.end_rendering();
}

void PBRPass::release()
{
  for (auto [v] : variants_)
  {
    sys->gpu.device_->uninit(v.v1.fill);
    sys->gpu.device_->uninit(v.v1.line);
    sys->gpu.device_->uninit(v.v1.point);
  }
}

}    // namespace ash
