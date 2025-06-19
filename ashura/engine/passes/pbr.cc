/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/pbr.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"

namespace ash
{

void PBRPass::acquire()
{
  gpu::Shader shader = sys->shader.get("PBR"_str).unwrap().shader;

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
    sys->gpu.sb_layout_, sys->gpu.sb_layout_,       sys->gpu.sb_layout_,
    sys->gpu.sb_layout_, sys->gpu.samplers_layout_, sys->gpu.textures_layout_};

  gpu::GraphicsPipelineInfo pipeline_info{
    .label         = "PBR Graphics Pipeline"_str,
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
    .push_constants_size    = sizeof(ShaderConstants),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache_
  };

  pipeline = sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();

  pipeline_info.rasterization_state.polygon_mode = gpu::PolygonMode::Line;

  wireframe_pipeline =
    sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();
}

void PBRPass::encode(gpu::CommandEncoder & e, PBRPassParams const & params)
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
  e.bind_graphics_pipeline(params.wireframe ? this->wireframe_pipeline :
                                              pipeline);

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
  e.bind_descriptor_sets(
    span({params.vertices_ssbo, params.indices_ssbo, params.params_ssbo,
          params.lights_ssbo, sys->gpu.samplers_, params.textures}),
    span({params.vertices_ssbo_offset, params.indices_ssbo_offset,
          params.params_ssbo_offset, params.lights_ssbo_offset}));
  e.push_constants(span({
                          ShaderConstants{.world_to_ndc = params.world_to_ndc,
                                          .uv_transform = params.uv_transform}
  })
                     .as_u8());
  e.draw(params.num_indices, 1, 0, params.instance);
  e.end_rendering();
}

void PBRPass::release()
{
  sys->gpu.device_->uninit(pipeline);
  sys->gpu.device_->uninit(wireframe_pipeline);
}

}    // namespace ash
