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

  gpu::ColorBlendState color_blend_state{
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
    .depth_format           = {},
    .stencil_format         = {&sys->gpu.depth_stencil_format_, 1},
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
  pipeline_ = create_pipeline(
    "Base"_str, sys->shader.get("RRect.Base"_str).unwrap().shader);
}

void SdfPass::add_variant(Str label, gpu::Shader shader)
{
  auto pipeline = create_pipeline(label, shader);
  bool exists;
  variants_.push(label, pipeline, &exists, false).unwrap();
  CHECK(!exists, "");
}

void SdfPass::remove_variant(Str label)
{
  auto pipeline = variants_.try_get(label).unwrap();
  variants_.erase(label);
  sys->gpu.release(pipeline);
}

void SdfPass::encode(gpu::CommandEncoder & e, SdfPassParams const & params,
                     Str variant)
{
  InplaceVec<gpu::RenderingAttachment, 1> color;
  InplaceVec<gpu::RenderingAttachment, 1> stencil;

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

  params.stencil.match([&](PassStencil const & s) {
    stencil
      .push(gpu::RenderingAttachment{.view         = s.texture.stencil_view,
                                     .resolve      = nullptr,
                                     .resolve_mode = gpu::ResolveModes::None,
                                     .load_op      = gpu::LoadOp::Load,
                                     .store_op     = gpu::StoreOp::None,
                                     .clear        = {}})
      .unwrap();
  });

  gpu::RenderingInfo info{
    .render_area{.extent = params.framebuffer.extent().xy()},
    .num_layers         = 1,
    .color_attachments  = color,
    .depth_attachment   = {},
    .stencil_attachment = stencil};

  auto variant_pipeline =
    variant.is_empty() ? pipeline_ : variants_.try_get(variant).unwrap();

  e.begin_rendering(info);
  e.bind_graphics_pipeline(variant_pipeline);
  e.set_graphics_state(gpu::GraphicsState{
    .scissor             = params.scissor,
    .viewport            = params.viewport,
    .stencil_test_enable = params.stencil.is_some(),
    .front_face_stencil =
      params.stencil.map([](auto s) { return s.front; }).unwrap_or(),
    .back_face_stencil =
      params.stencil.map([](auto s) { return s.back; }).unwrap_or()});
  e.bind_descriptor_sets(span({
                           params.samplers,                           //
                           params.textures,                           //
                           params.world_to_ndc.buffer.descriptor_,    //
                           params.shapes.buffer.descriptor_,          //
                           params.transforms.buffer.descriptor_,      //
                           params.materials.buffer.descriptor_        //
                         }),
                         span({
                           params.world_to_ndc.slice.offset,    //
                           params.shapes.slice.offset,          //
                           params.transforms.slice.offset,      //
                           params.materials.slice.offset        //
                         }));
  e.draw(4, params.num_instances, 0, params.first_instance);
  e.end_rendering();
}

void SdfPass::release()
{
  sys->gpu.device_->uninit(pipeline_);
  for (auto [_, pipeline] : variants_)
  {
    sys->gpu.device_->uninit(pipeline);
  }
}

}    // namespace ash
