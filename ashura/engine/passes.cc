/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes.h"
#include "ashura/std/math.h"

namespace ash
{

void BloomPass::acquire(GpuSystem &, AssetMap &)
{
}

void BloomPass::release(GpuSystem &, AssetMap &)
{
}

void BloomPass::encode(GpuSystem &, gpu::CommandEncoder &,
                       BloomPassParams const &)
{
  /// E' = Blur(E)
  /// D' = Blur(D) + E'
  /// C' = Blur(C) + D'
  /// B' = Blur(B) + C'
  /// A' = Blur(A) + B'
}

void BlurPass::acquire(GpuSystem & gpu, AssetMap & assets)
{
  // https://www.youtube.com/watch?v=ml-5OGZC7vE
  //
  // An investigation of fast real-time GPU-based image blur algorithms -
  // https://www.intel.cn/content/www/cn/zh/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html

  //
  // Algorithm described here:
  // https://community.arm.com/cfs-file/__key/communityserver-blogs-components-weblogfiles/00-00-00-20-66/siggraph2015_2D00_mmg_2D00_marius_2D00_slides.pdf
  //
  gpu::Shader vertex_shader   = assets.shaders["Blur/DownSample:VS"_str];
  gpu::Shader fragment_shader = assets.shaders["Blur/DownSample:FS"_str];

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

  gpu::DescriptorSetLayout set_layouts[] = {gpu.samplers_layout,
                                            gpu.textures_layout};

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
    .color_formats          = {&gpu.color_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(BlurParam),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache
  };

  downsample_pipeline =
    gpu.device->create_graphics_pipeline(pipeline_info).unwrap();

  pipeline_info.vertex_shader.shader   = assets.shaders["Blur/UpSample:VS"_str];
  pipeline_info.fragment_shader.shader = assets.shaders["Blur/UpSample:FS"_str];

  upsample_pipeline =
    gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void BlurPass::release(GpuSystem & gpu, AssetMap &)
{
  gpu.device->uninit(downsample_pipeline);
  gpu.device->uninit(upsample_pipeline);
}

void sample(BlurPass & b, GpuSystem & c, gpu::CommandEncoder & e, Vec2 radius,
            gpu::DescriptorSet src_texture, TextureId src_id,
            gpu::Extent src_extent, gpu::Rect src_area, gpu::ImageView dst,
            Vec2U dst_offset, bool upsample)
{
  radius /= as_vec2(src_extent);
  Vec2 uv0 = as_vec2(src_area.offset) / as_vec2(src_extent);
  Vec2 uv1 = as_vec2(src_area.end()) / as_vec2(src_extent);

  e.begin_rendering(gpu::RenderingInfo{
    .render_area = {.offset = dst_offset, .extent = src_area.extent},
    .num_layers  = 1,
    .color_attachments =
      span({gpu::RenderingAttachment{.view         = dst,
                                     .resolve      = nullptr,
                                     .resolve_mode = gpu::ResolveModes::None,
                                     .load_op      = gpu::LoadOp::Load,
                                     .store_op     = gpu::StoreOp::Store}}
      ),
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
  e.bind_descriptor_sets(span({c.samplers, src_texture}), {});
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

void BlurPass::encode(GpuSystem & gpu, gpu::CommandEncoder & e,
                      BlurPassParams const & params)
{
  if (params.passes == 0)
  {
    return;
  }

  Framebuffer fbs[2]   = {params.framebuffer, gpu.scratch_fb};
  gpu::Rect   areas[2] = {
    params.area, {.offset = {0, 0}, .extent = gpu.scratch_fb.extent()}
  };

  u32 src = 0;
  u32 dst = 1;

  Vec2 const radius = params.radius / (f32) params.passes;

  // [ ] verify

  // downsample pass
  for (u32 i = 0; i < params.passes; i++)
  {
    sample(*this, gpu, e, radius * (f32) (i + 1), fbs[src].color.texture,
           fbs[src].color.texture_id, areas[src].extent, areas[dst],
           fbs[dst].color.view, areas[dst].offset, false);

    src = (src + 1) & 1;
    dst = (dst + 1) & 1;
  }

  // upsample pass
  for (u32 i = 0; i < params.passes; i++)
  {
    radius -= 1;

    sample(*this, gpu, e, (f32) radius, fbs[src].color.texture,
           fbs[src].color.texture_id, areas[src].extent, areas[dst],
           fbs[dst].color.view, areas[dst].offset, true);

    src = (src + 1) & 1;
    dst = (dst + 1) & 1;
  }
}

void NgonPass::acquire(GpuSystem & gpu, AssetMap & assets)
{
  gpu::Shader vertex_shader   = assets.shaders["Ngon:VS"_str];
  gpu::Shader fragment_shader = assets.shaders["Ngon:FS"_str];

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                         gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0,
                                       .sample_count = gpu.sample_count};

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
    gpu.ssbo_layout, gpu.ssbo_layout, gpu.ssbo_layout, gpu.samplers_layout,
    gpu.textures_layout};

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
    .color_formats          = {&gpu.color_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(Mat4),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache
  };

  pipeline = gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void NgonPass::encode(GpuSystem & gpu, gpu::CommandEncoder & e,
                      NgonPassParams const & params)
{
  gpu::RenderingAttachment color[1];

  if (params.framebuffer.color_msaa.is_some())
  {
    color[0] = gpu::RenderingAttachment{
      .view         = params.framebuffer.color_msaa.value().view,
      .resolve      = params.framebuffer.color.view,
      .resolve_mode = gpu::ResolveModes::Average,
      .load_op      = gpu::LoadOp::DontCare,
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
  e.bind_descriptor_sets(
    span({params.vertices_ssbo, params.indices_ssbo, params.params_ssbo,
          gpu.samplers, params.textures}),
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

void NgonPass::release(GpuSystem & gpu, AssetMap &)
{
  gpu.device->uninit(pipeline);
}

void PBRPass::acquire(GpuSystem & gpu, AssetMap & assets)
{
  gpu::Shader vertex_shader   = assets.shaders["PBR:VS"_str];
  gpu::Shader fragment_shader = assets.shaders["PBR:FS"_str];

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                         gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0,
                                       .sample_count = gpu.sample_count};

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
    gpu.ssbo_layout, gpu.ssbo_layout,     gpu.ssbo_layout,
    gpu.ssbo_layout, gpu.samplers_layout, gpu.textures_layout};

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
    .color_formats          = {&gpu.color_format, 1},
    .depth_format           = {&gpu.depth_stencil_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(Mat4),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleList,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache
  };

  pipeline = gpu.device->create_graphics_pipeline(pipeline_info).unwrap();

  pipeline_info.rasterization_state.polygon_mode = gpu::PolygonMode::Line;

  wireframe_pipeline =
    gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void PBRPass::encode(GpuSystem & gpu, gpu::CommandEncoder & e,
                     PBRPassParams const & params)
{
  gpu::RenderingAttachment color[1];

  if (params.framebuffer.color_msaa.is_some())
  {
    color[0] = gpu::RenderingAttachment{
      .view         = params.framebuffer.color_msaa.value().view,
      .resolve      = params.framebuffer.color.view,
      .resolve_mode = gpu::ResolveModes::Average,
      .load_op      = gpu::LoadOp::DontCare,
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
          params.lights_ssbo, gpu.samplers, params.textures}),
    span<u32>({0, 0, 0, 0}));
  e.push_constants(span({params.world_to_view}).as_u8());
  e.draw(params.num_indices, 1, 0, params.instance);
  e.end_rendering();
}

void PBRPass::release(GpuSystem & gpu, AssetMap &)
{
  gpu.device->uninit(pipeline);
  gpu.device->uninit(wireframe_pipeline);
}

void RRectPass::acquire(GpuSystem & gpu, AssetMap & assets)
{
  gpu::Shader vertex_shader   = assets.shaders["RRect:VS"_str];
  gpu::Shader fragment_shader = assets.shaders["RRect:FS"_str];

  gpu::RasterizationState raster_state{.depth_clamp_enable = false,
                                       .polygon_mode = gpu::PolygonMode::Fill,
                                       .cull_mode    = gpu::CullMode::None,
                                       .front_face =
                                         gpu::FrontFace::CounterClockWise,
                                       .depth_bias_enable          = false,
                                       .depth_bias_constant_factor = 0,
                                       .depth_bias_clamp           = 0,
                                       .depth_bias_slope_factor    = 0,
                                       .sample_count = gpu.sample_count};

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
    gpu.ssbo_layout, gpu.samplers_layout, gpu.textures_layout};

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
    .color_formats          = {&gpu.color_format, 1},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(Mat4),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = gpu.pipeline_cache
  };

  pipeline = gpu.device->create_graphics_pipeline(pipeline_info).unwrap();
}

void RRectPass::encode(GpuSystem & gpu, gpu::CommandEncoder & e,
                       RRectPassParams const & params)
{
  gpu::RenderingAttachment color[1];

  if (params.framebuffer.color_msaa.is_some())
  {
    color[0] = gpu::RenderingAttachment{
      .view         = params.framebuffer.color_msaa.value().view,
      .resolve      = params.framebuffer.color.view,
      .resolve_mode = gpu::ResolveModes::Average,
      .load_op      = gpu::LoadOp::DontCare,
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
    span({params.params_ssbo, gpu.samplers, params.textures}), span<u32>({0}));
  e.push_constants(span({params.world_to_view}).as_u8());
  e.draw(4, params.num_instances, 0, params.first_instance);
  e.end_rendering();
}

void RRectPass::release(GpuSystem & gpu, AssetMap &)
{
  gpu.device->uninit(pipeline);
}

}    // namespace ash
