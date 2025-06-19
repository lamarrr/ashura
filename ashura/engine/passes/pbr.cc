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

gpu::GraphicsPipeline create_pipeline(Str label, gpu::Shader shader)
{
  auto tagged_label =
    snformat<gpu::MAX_LABEL_SIZE>("PBR Graphics Pipeline: {}"_str, label)
      .unwrap();

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
    .depth_format           = {&sys->gpu.depth_stencil_format_, 1},
    .stencil_format         = {&sys->gpu.depth_stencil_format_, 1},
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

PBRPass::PBRPass(AllocatorRef allocator) : variants_{allocator}
{
}

void PBRPass::acquire()
{
  gpu::Shader shader = sys->shader.get("PBR"_str).unwrap().shader;
  pipeline_info.rasterization_state.polygon_mode = gpu::PolygonMode::Line;

  wireframe_pipeline_ =
    sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();
}

void PBRPass::add_variant(Str label, gpu::Shader shader)
{
  auto pipeline = create_pipeline(label, shader);
  bool exists;
  variants_.push(label, pipeline, &exists, false).unwrap();
  CHECK(!exists, "");
}

void PBRPass::remove_variant(Str label)
{
  auto pipeline = variants_.try_get(label).unwrap();
  variants_.erase(label);
  sys->gpu.release(pipeline);
}

void PBRPass::encode(gpu::CommandEncoder & e, PBRPassParams const & params,
                     Str variant)
{
  InplaceVec<gpu::RenderingAttachment, 1> color;
  InplaceVec<gpu::RenderingAttachment, 1> depth;
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

  depth
    .push(
      gpu::RenderingAttachment{.view    = params.framebuffer.depth_stencil.view,
                               .resolve = nullptr,
                               .resolve_mode = gpu::ResolveModes::None,
                               .load_op      = gpu::LoadOp::Load,
                               .store_op     = gpu::StoreOp::Store,
                               .clear        = {}})
    .unwrap();

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
    .depth_attachment   = depth,
    .stencil_attachment = stencil};

  e.begin_rendering(info);

  auto variant_pipeline =
    variant.is_empty() ? pipeline_ : variants_.try_get(variant).unwrap();

  e.bind_graphics_pipeline(params.wireframe ? wireframe_pipeline_ :
                                              variant_pipeline);

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
  sys->gpu.device_->uninit(pipeline_);
  sys->gpu.device_->uninit(wireframe_pipeline_);
  for (auto [_, pipeline] : variants_)
  {
    sys->gpu.device_->uninit(pipeline);
  }
}

}    // namespace ash
