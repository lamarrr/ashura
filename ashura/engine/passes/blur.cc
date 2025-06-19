/// SPDX-License-Identifier: MIT
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/systems.h"
#include "ashura/std/math.h"

namespace ash
{

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
  gpu::Shader shader = sys->shader.get("Blur"_str).unwrap().shader;

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

  gpu::DescriptorSetLayout set_layouts[] = {sys->gpu.samplers_layout_,
                                            sys->gpu.textures_layout_};

  gpu::GraphicsPipelineInfo pipeline_info{
    .label         = "Blur Graphics Pipeline"_str,
    .vertex_shader = gpu::ShaderStageInfo{.shader      = shader,
                                          .entry_point = "vert"_str,
                                          .specialization_constants      = {},
                                          .specialization_constants_data = {}},
    .fragment_shader =
      gpu::ShaderStageInfo{.shader                   = shader,
                                          .entry_point              = "frag_downsample"_str,
                                          .specialization_constants = {},
                                          .specialization_constants_data = {}},
    .color_formats          = {&sys->gpu.color_format_, 1},
    .depth_format           = {},
    .stencil_format         = {},
    .vertex_input_bindings  = {},
    .vertex_attributes      = {},
    .push_constants_size    = sizeof(BlurShaderParam),
    .descriptor_set_layouts = set_layouts,
    .primitive_topology     = gpu::PrimitiveTopology::TriangleFan,
    .rasterization_state    = raster_state,
    .depth_stencil_state    = depth_stencil_state,
    .color_blend_state      = color_blend_state,
    .cache                  = sys->gpu.pipeline_cache_
  };

  downsample_pipeline =
    sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();

  pipeline_info.fragment_shader.entry_point = "frag_upsample"_str;

  upsample_pipeline =
    sys->gpu.device_->create_graphics_pipeline(pipeline_info).unwrap();
}

void BlurPass::release()
{
  sys->gpu.device_->uninit(downsample_pipeline);
  sys->gpu.device_->uninit(upsample_pipeline);
}

void sample(BlurPass & b, gpu::CommandEncoder & e, Vec2U spread_radius,
            gpu::DescriptorSet src_texture, TextureId src_id, Vec2U src_extent,
            RectU const & src_area, gpu::ImageView dst, RectU const & dst_area,
            bool upsample)
{
  auto const scale            = 1 / as_vec2(src_extent);
  auto const uv_spread_radius = as_vec2(spread_radius) * scale;
  auto const uv0              = as_vec2(src_area.begin()) * scale;
  auto const uv1              = as_vec2(src_area.end()) * scale;

  InplaceVec<gpu::RenderingAttachment, 1> color;

  color
    .push(gpu::RenderingAttachment{.view         = dst,
                                   .resolve      = nullptr,
                                   .resolve_mode = gpu::ResolveModes::None,
                                   .load_op      = gpu::LoadOp::Load,
                                   .store_op     = gpu::StoreOp::Store,
                                   .clear        = {}})
    .unwrap();

  e.begin_rendering(gpu::RenderingInfo{.render_area        = dst_area,
                                       .num_layers         = 1,
                                       .color_attachments  = color,
                                       .depth_attachment   = {},
                                       .stencil_attachment = {}});

  e.bind_graphics_pipeline(upsample ? b.upsample_pipeline :
                                      b.downsample_pipeline);
  e.set_graphics_state(gpu::GraphicsState{
    .scissor = dst_area,
    .viewport{.offset = as_vec2(dst_area.offset),
              .extent = as_vec2(dst_area.extent)}
  });
  e.bind_descriptor_sets(span({sys->gpu.samplers_, src_texture}), {});
  e.push_constants(span({
                          BlurShaderParam{.uv{uv0, uv1},
                                          .radius  = uv_spread_radius,
                                          .sampler = SamplerId::LinearClamped,
                                          .texture = src_id}
  })
                     .as_u8());
  e.draw(4, 1, 0, 0);
  e.end_rendering();
}

BlurPass::Config BlurPass::config(BlurPassParams const & params)
{
  auto const spread_radius = clamp_vec(params.spread_radius, Vec2U::splat(1U),
                                       Vec2U::splat(MAX_SPREAD_RADIUS));

  auto const major_spread_radius = max(spread_radius.x, spread_radius.y);

  auto const padding = Vec2U::splat(max(major_spread_radius + 8, 16U));

  auto const padded_begin = sat_sub(params.area.begin(), padding);
  auto const padded_end   = sat_add(params.area.end(), padding);

  auto const padded_area = RectU::range(padded_begin, padded_end)
                             .clamp_to_extent(params.framebuffer.extent().xy());

  auto const num_passes = clamp(major_spread_radius, 1U, MAX_PASSES);

  return {.spread_radius       = spread_radius,
          .major_spread_radius = major_spread_radius,
          .padding             = padding,
          .padded_area         = padded_area,
          .num_passes          = num_passes};
}

Option<ColorTextureResult> BlurPass::encode(gpu::CommandEncoder &  e,
                                            BlurPassParams const & params)
{
  if (!(params.area.is_visible() && params.spread_radius.is_visible()))
  {
    return none;
  }

  auto cfg = config(params);

  if (!cfg.padded_area.is_visible() || cfg.num_passes == 0)
  {
    return none;
  }

  e.blit_image(params.framebuffer.color.image, sys->gpu.scratch_color_[0].image,
               span({
                 gpu::ImageBlit{.src_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .src_area = as_boxu(cfg.padded_area),
                                .dst_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .dst_area = as_boxu(cfg.padded_area)}
  }),
               gpu::Filter::Linear);

  // NOTE: we can avoid a second copy operation if the padded area is same as the blur area,
  // which will happen if we are blurring the entire texture.
  e.blit_image(params.framebuffer.color.image, sys->gpu.scratch_color_[1].image,
               span({
                 gpu::ImageBlit{.src_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .src_area = as_boxu(cfg.padded_area),
                                .dst_layers{.aspects = gpu::ImageAspects::Color,
                                            .mip_level         = 0,
                                            .first_array_layer = 0,
                                            .num_array_layers  = 1},
                                .dst_area = as_boxu(cfg.padded_area)}
  }),
               gpu::Filter::Linear);

  ColorTexture const * const fbs[2] = {&sys->gpu.scratch_color_[0],
                                       &sys->gpu.scratch_color_[1]};

  u32 src = 0;
  u32 dst = 1;

  // downsample pass
  for (u32 i = 1; i <= cfg.num_passes; i++)
  {
    src = (src + 1) & 1;
    dst = (src + 1) & 1;
    auto const spread_radius =
      clamp_vec(Vec2U::splat(i), Vec2U::splat(1U), cfg.spread_radius);
    sample(*this, e, spread_radius, fbs[src]->texture, fbs[src]->texture_id,
           fbs[src]->extent().xy(), params.area, fbs[dst]->view, params.area,
           false);
  }

  // upsample pass
  for (u32 i = cfg.num_passes; i != 0; i--)
  {
    src = (src + 1) & 1;
    dst = (src + 1) & 1;
    auto const spread_radius =
      clamp_vec(Vec2U::splat(i), Vec2U::splat(1U), cfg.spread_radius);
    sample(*this, e, spread_radius, fbs[src]->texture, fbs[src]->texture_id,
           fbs[src]->extent().xy(), params.area, fbs[dst]->view, params.area,
           true);
  }

  // the last output was to scratch 1
  CHECK(dst == 1, "");

  return ColorTextureResult{.color = *fbs[dst], .rect = params.area};
}

}    // namespace ash
