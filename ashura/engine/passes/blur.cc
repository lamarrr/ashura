/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"
#include "ashura/std/sformat.h"

namespace ash
{

// https://www.youtube.com/watch?v=ml-5OGZC7vE
//
// An investigation of fast real-time GPU-based image blur algorithms -
// https://www.intel.cn/content/www/cn/zh/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html

//
// Algorithm described here:
// https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_slides.pdf
//

Str BlurPass::label()
{
  return "Blur"_str;
}

gpu::GraphicsPipeline create_pipeline(Str label, gpu::Shader shader)
{
  auto tagged_label =
    snformat<gpu::MAX_LABEL_SIZE>("Blur Graphics Pipeline: {}"_str, label)
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
                                       .sample_count = gpu::SampleCount::C1};

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
    {.blend_enable           = false,
     .src_color_blend_factor = gpu::BlendFactor::Zero,
     .dst_color_blend_factor = gpu::BlendFactor::Zero,
     .color_blend_op         = gpu::BlendOp::Add,
     .src_alpha_blend_factor = gpu::BlendFactor::Zero,
     .dst_alpha_blend_factor = gpu::BlendFactor::Zero,
     .alpha_blend_op         = gpu::BlendOp::Add,
     .color_write_mask       = gpu::ColorComponents::All}
  };

  gpu::ColorBlendState color_blend_state{.attachments    = attachment_states,
                                         .blend_constant = {}};

  gpu::DescriptorSetLayout set_layouts[] = {
    sys->gpu.samplers_layout_,    // 0: samplers
    sys->gpu.textures_layout_,    // 1: textures
    sys->gpu.sb_layout_           // 2: blur
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

void BlurPass::acquire()
{
  downsample_pipeline_ = create_pipeline(
    "Downsample"_str, sys->shader.get("Blur.Downsample"_str).unwrap().shader);
  upsample_pipeline_ = create_pipeline(
    "Upsample"_str, sys->shader.get("Blur.Upsample"_str).unwrap().shader);
}

void BlurPass::release()
{
  sys->gpu.device_->uninit(downsample_pipeline_);
  sys->gpu.device_->uninit(upsample_pipeline_);
}

void BlurPass::encode(gpu::CommandEncoder & e, BlurPassParams const & params)
{
  InplaceVec<gpu::RenderingAttachment, 1> color;

  color
    .push(gpu::RenderingAttachment{.view    = params.framebuffer.color.view,
                                   .resolve = nullptr,
                                   .resolve_mode = gpu::ResolveModes::None,
                                   .load_op      = gpu::LoadOp::Load,
                                   .store_op     = gpu::StoreOp::Store,
                                   .clear        = {}})
    .unwrap();

  auto stencil = params.stencil.map([&](PassStencil const &) {
    return gpu::RenderingAttachment{
      .view         = params.framebuffer.depth_stencil.stencil_view,
      .resolve      = nullptr,
      .resolve_mode = gpu::ResolveModes::None,
      .load_op      = gpu::LoadOp::Load,
      .store_op     = gpu::StoreOp::None,
      .clear        = {}};
  });

  e.begin_rendering(gpu::RenderingInfo{
    .render_area{.offset = {}, .extent = params.framebuffer.extent().xy()},
    .num_layers         = 1,
    .color_attachments  = color,
    .depth_attachment   = {},
    .stencil_attachment = stencil
  });

  e.bind_graphics_pipeline(params.upsample ? upsample_pipeline_ :
                                             downsample_pipeline_);
  e.set_graphics_state(gpu::GraphicsState{
    .scissor             = params.scissor,
    .viewport            = params.viewport,
    .stencil_test_enable = params.stencil.is_some(),
    .front_face_stencil =
      params.stencil.map([](auto s) { return s.front; }).unwrap_or(),
    .back_face_stencil =
      params.stencil.map([](auto s) { return s.back; }).unwrap_or()});
  e.bind_descriptor_sets(span({
                           params.samplers,                   //
                           params.textures,                   //
                           params.blurs.buffer.descriptor_    //
                         }),
                         span({
                           params.blurs.slice.offset    //
                         }));
  e.draw(4, params.instances.span, 0, params.instances.offset);
  e.end_rendering();
}

}    // namespace ash
