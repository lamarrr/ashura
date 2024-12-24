/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes.h"
#include "ashura/engine/assets.h"
#include "ashura/std/math.h"

namespace ash
{

void BloomPass::acquire()
{
}

void BloomPass::release()
{
}

void BloomPass::encode(gpu::CommandEncoder &, BloomPassParams const &)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

void BlurPass::acquire()
{
  // https://www.youtube.com/watch?v=ml-5OGZC7vE
  //
  // An investigation of fast real-time GPU-based image blur algorithms -
  // https://www.intel.cn/content/www/cn/zh/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html

  //
  // Algorithm described here:
  // https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_slides.pdf
  //
  gpu::Shader vertex_shader = sys->shader.get("Blur/DownSample:FS"_str).shader;
  gpu::Shader fragment_shader =
    sys->shader.get("Blur/DownSample:FS"_str).shader;

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
                                               gpu::CompareOp::Greater,
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

  gpu::DescriptorSetLayout set_layouts[] = {sys->gpu.samplers_layout,
                                            sys->gpu.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
    .label         = "Blur Graphics Pipeline"_str,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = vertex_shader,
                                          .entry_point = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                        = fragment_shader,
                                          .entry_point                   = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .color_formats          = {&sys->gpu.color_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(BlurParam),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache
  };

  downsample_pipeline =
    sys->gpu.device->create_graphics_pipeline(pipeline_info).unwrap();

  pipeline_info.vertex_shader.shader =
    sys->shader.get("Blur/UpSample:VS"_str).shader;
  pipeline_info.fragment_shader.shader =
    sys->shader.get("Blur/UpSample:FS"_str).shader;

  upsample_pipeline =
    sys->gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void BlurPass::release()
{
  sys->gpu.device->uninit(downsample_pipeline);
  sys->gpu.device->uninit(upsample_pipeline);
}

void sample(BlurPass & b, gpu::CommandEncoder & e, Vec2 radius,
            gpu::DescriptorSet src_texture, TextureId src_id, Vec2U src_extent,
            RectU const & src_area, gpu::ImageView dst, Vec2U dst_offset,
            bool upsample)
{
  radius /= as_vec2(src_extent);
  Vec2 const uv0 = as_vec2(src_area.offset) / as_vec2(src_extent);
  Vec2 const uv1 = as_vec2(src_area.end()) / as_vec2(src_extent);

  gpu::RenderingAttachment color[1] = {
    {.view         = dst,
     .resolve      = nullptr,
     .resolve_mode = gpu::ResolveModes::None,
     .load_op      = gpu::LoadOp::Load,
     .store_op     = gpu::StoreOp::Store}
  };

  e.begin_rendering(gpu::RenderingInfo{
    .render_area        = {.offset = dst_offset, .extent = src_area.extent},
    .num_layers         = 1,
    .color_attachments  = color,
    .depth_attachment   = {},
    .stencil_attachment = {}
  });

  e.bind_graphics_pipeline(upsample ? b.upsample_pipeline :
                                      b.downsample_pipeline);
  e.set_graphics_state(gpu::GraphicsState{
    .scissor  = {.offset = dst_offset,          .extent = src_area.extent},
    .viewport = {.offset = as_vec2(dst_offset),
                 .extent = as_vec2(src_area.extent)                      }
  });
  e.bind_descriptor_sets(span({sys->gpu.samplers, src_texture}), {});
  e.push_constants(span({
                          BlurParam{.uv      = {uv0, uv1},
                                    .radius  = radius,
                                    .sampler = SamplerId::LinearClamped,
                                    .texture = src_id}
  })
                     .as_u8());
  e.draw(4, 1, 0, 0);
  e.end_rendering();
}

void BlurPass::encode(gpu::CommandEncoder & e, BlurPassParams const & params)
{
  if (params.passes == 0)
  {
    return;
  }

  Framebuffer fbs[2]   = {params.framebuffer, sys->gpu.scratch_fb};
  RectU       areas[2] = {
    params.area, {.offset = {0, 0}, .extent = sys->gpu.scratch_fb.extent()}
  };

  Vec2 const radius = params.radius / (f32) params.passes;

  u32 src = 1;

  // downsample pass
  for (i64 i = 0; i < params.passes; i++)
  {
    src           = (src + 1) & 1;
    u32 const dst = (src + 1) & 1;
    sample(*this, e, radius * (f32) (i + 1), fbs[src].color.texture,
           fbs[src].color.texture_id, areas[src].extent, areas[dst],
           fbs[dst].color.view, areas[dst].offset, false);
  }

  // upsample pass
  for (i64 i = params.passes; i > 0; i--)
  {
    src           = (src + 1) & 1;
    u32 const dst = (src + 1) & 1;
    sample(*this, e, radius * (f32) (i + 1), fbs[src].color.texture,
           fbs[src].color.texture_id, areas[src].extent, areas[dst],
           fbs[dst].color.view, areas[dst].offset, true);
  }
}

void NgonPass::acquire()
{
  gpu::Shader vertex_shader   = sys->shader.get("Ngon:VS"_str).shader;
  gpu::Shader fragment_shader = sys->shader.get("Ngon:FS"_str).shader;

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                         gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0,
                                       .sample_count = sys->gpu.sample_count};

  gpu::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                               gpu::CompareOp::Greater,
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
    sys->gpu.ssbo_layout, sys->gpu.ssbo_layout, sys->gpu.ssbo_layout,
    sys->gpu.samplers_layout, sys->gpu.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
    .label         = "Ngon Graphics Pipeline"_str,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = vertex_shader,
                                          .entry_point = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                        = fragment_shader,
                                          .entry_point                   = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .color_formats          = {&sys->gpu.color_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(Mat4),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache
  };

  pipeline = sys->gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void NgonPass::encode(gpu::CommandEncoder & e, NgonPassParams const & params)
{
  gpu::RenderingAttachment color[1];

  if (params.framebuffer.color_msaa.is_some())
  {
    color[0] = gpu::RenderingAttachment{
      .view         = params.framebuffer.color_msaa.value().view,
      .resolve      = params.framebuffer.color.view,
      .resolve_mode = gpu::ResolveModes::Average,
      .load_op      = gpu::LoadOp::Load,
      .store_op     = gpu::StoreOp::Store};
  }
  else
  {
    color[0] = gpu::RenderingAttachment{.view = params.framebuffer.color.view,
                                        .resolve      = nullptr,
                                        .resolve_mode = gpu::ResolveModes::None,
                                        .load_op      = gpu::LoadOp::Load,
                                        .store_op     = gpu::StoreOp::Store,
                                        .clear        = {}};
  }

  gpu::RenderingInfo info{.render_area{.extent = params.framebuffer.extent()},
                          .num_layers        = 1,
                          .color_attachments = color};

  e.begin_rendering(info);
  e.bind_graphics_pipeline(pipeline);
  e.bind_descriptor_sets(
    span({params.vertices_ssbo, params.indices_ssbo, params.params_ssbo,
          sys->gpu.samplers, params.textures}),
    span<u32>({0, 0, 0}));
  e.push_constants(span({params.world_to_view}).as_u8());
  e.set_graphics_state(
    gpu::GraphicsState{.scissor = params.scissor, .viewport = params.viewport});

  for (auto [i, vertex_count] : enumerate<u32>(params.index_counts))
  {
    e.draw(vertex_count, 1, 0, params.first_instance + i);
  }
  e.end_rendering();
}

void NgonPass::release()
{
  sys->gpu.device->uninit(pipeline);
}

void PBRPass::acquire()
{
  gpu::Shader vertex_shader   = sys->shader.get("PBR:VS"_str).shader;
  gpu::Shader fragment_shader = sys->shader.get("PBR:FS"_str).shader;

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                         gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0,
                                       .sample_count = sys->gpu.sample_count};

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
    sys->gpu.ssbo_layout, sys->gpu.ssbo_layout,     sys->gpu.ssbo_layout,
    sys->gpu.ssbo_layout, sys->gpu.samplers_layout, sys->gpu.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
    .label         = "PBR Graphics Pipeline"_str,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = vertex_shader,
                                          .entry_point = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                        = fragment_shader,
                                          .entry_point                   = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .color_formats          = {&sys->gpu.color_format, 1},
    .depth_format           = {&sys->gpu.depth_stencil_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(Mat4),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache
  };

  pipeline = sys->gpu.device->create_graphics_pipeline(pipeline_info).unwrap();

  pipeline_info.rasterization_state.polygon_mode = gpu::PolygonMode::Line;

  wireframe_pipeline =
    sys->gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void PBRPass::encode(gpu::CommandEncoder & e, PBRPassParams const & params)
{
  gpu::RenderingAttachment color[1];

  if (params.framebuffer.color_msaa.is_some())
  {
    color[0] = gpu::RenderingAttachment{
      .view         = params.framebuffer.color_msaa.value().view,
      .resolve      = params.framebuffer.color.view,
      .resolve_mode = gpu::ResolveModes::Average,
      .load_op      = gpu::LoadOp::Load,
      .store_op     = gpu::StoreOp::Store};
  }
  else
  {
    color[0] = gpu::RenderingAttachment{.view = params.framebuffer.color.view};
  }

  gpu::RenderingAttachment depth[] = {
    {.view     = params.framebuffer.depth.view,
     .load_op  = gpu::LoadOp::Load,
     .store_op = gpu::StoreOp::Store}
  };

  gpu::RenderingInfo info{.render_area{.extent = params.framebuffer.extent()},
                          .num_layers        = 1,
                          .color_attachments = color,
                          .depth_attachment  = depth};

  e.begin_rendering(info);
  e.bind_graphics_pipeline(params.wireframe ? this->wireframe_pipeline :
                                              this->pipeline);

  e.set_graphics_state(gpu::GraphicsState{
    .scissor            = params.scissor,
    .viewport           = params.viewport,
    .blend_constant     = {1, 1, 1, 1},
    .depth_test_enable  = true,
    .depth_compare_op   = gpu::CompareOp::Less,
    .depth_write_enable = true
  });
  e.bind_descriptor_sets(
    span({params.vertices_ssbo, params.indices_ssbo, params.params_ssbo,
          params.lights_ssbo, sys->gpu.samplers, params.textures}),
    span<u32>({0, 0, 0, 0}));
  e.push_constants(span({params.world_to_view}).as_u8());
  e.draw(params.num_indices, 1, 0, params.instance);
  e.end_rendering();
}

void PBRPass::release()
{
  sys->gpu.device->uninit(pipeline);
  sys->gpu.device->uninit(wireframe_pipeline);
}

void RRectPass::acquire()
{
  gpu::Shader vertex_shader   = sys->shader.get("RRect:VS"_str).shader;
  gpu::Shader fragment_shader = sys->shader.get("RRect:FS"_str).shader;

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                         gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0,
                                       .sample_count = sys->gpu.sample_count};

  gpu::DepthStencilState depth_stencil_state{.depth_test_enable  = false,
                                             .depth_write_enable = false,
                                             .depth_compare_op =
                                               gpu::CompareOp::Greater,
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
    sys->gpu.ssbo_layout, sys->gpu.samplers_layout, sys->gpu.textures_layout};

  gpu::GraphicsPipelineInfo pipeline_info{
    .label         = "RRect Graphics Pipeline"_str,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = vertex_shader,
                                          .entry_point = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                        = fragment_shader,
                                          .entry_point                   = "main"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .color_formats          = {&sys->gpu.color_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(Mat4),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache
  };

  pipeline = sys->gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void RRectPass::encode(gpu::CommandEncoder & e, RRectPassParams const & params)
{
  gpu::RenderingAttachment color[1];

  if (params.framebuffer.color_msaa.is_some())
  {
    color[0] = gpu::RenderingAttachment{
      .view         = params.framebuffer.color_msaa.value().view,
      .resolve      = params.framebuffer.color.view,
      .resolve_mode = gpu::ResolveModes::Average,
      .load_op      = gpu::LoadOp::Load,
      .store_op     = gpu::StoreOp::Store};
  }
  else
  {
    color[0] = gpu::RenderingAttachment{.view = params.framebuffer.color.view};
  }

  gpu::RenderingInfo info{.render_area{.extent = params.framebuffer.extent()},
                          .num_layers        = 1,
                          .color_attachments = color};

  e.begin_rendering(info);
  e.bind_graphics_pipeline(pipeline);
  e.set_graphics_state(
    gpu::GraphicsState{.scissor = params.scissor, .viewport = params.viewport});
  e.bind_descriptor_sets(
    span({params.params_ssbo, sys->gpu.samplers, params.textures}),
    span<u32>({0}));
  e.push_constants(span({params.world_to_view}).as_u8());
  e.draw(4, params.num_instances, 0, params.first_instance);
  e.end_rendering();
}

void RRectPass::release()
{
  sys->gpu.device->uninit(pipeline);
}

}    // namespace ash
