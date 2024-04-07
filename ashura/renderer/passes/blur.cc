#include "ashura/renderer/passes/blur.h"
#include "ashura/gfx/vulkan.h"

namespace ash
{

void BlurPass::init(RenderContext &ctx)
{
  // https://www.khronos.org/opengl/wiki/Compute_Shader
  //
  // https://web.engr.oregonstate.edu/~mjb/vulkan/Handouts/OpenglComputeShaders.1pp.pdf
  //
  // https://github.com/lisyarus/compute/blob/master/blur/source/compute_separable_lds.cpp
  // https://lisyarus.github.io/blog/graphics/2022/04/21/compute-blur.html
  // https://www.youtube.com/watch?v=ml-5OGZC7vE
  parameter_heap_.init(ctx.device, 8);

  render_pass_ =
      ctx.device
          ->create_render_pass(
              ctx.device.self,
              gfx::RenderPassDesc{
                  .label             = "KawaseBlur RenderPass",
                  .color_attachments = to_span<gfx::RenderPassAttachment>(
                      {{.format           = ctx.color_format,
                        .load_op          = gfx::LoadOp::Load,
                        .store_op         = gfx::StoreOp::Store,
                        .stencil_load_op  = gfx::LoadOp::DontCare,
                        .stencil_store_op = gfx::StoreOp::DontCare}}),
                  .input_attachments = {},
                  .depth_stencil_attachment =
                      {.format           = gfx::Format::Undefined,
                       .load_op          = gfx::LoadOp::DontCare,
                       .store_op         = gfx::StoreOp::DontCare,
                       .stencil_load_op  = gfx::LoadOp::DontCare,
                       .stencil_store_op = gfx::StoreOp::DontCare},
              })
          .unwrap();

  gfx::Shader vertex_shader =
      ctx.get_shader("KawaseBlur_DownSample:VS"_span).unwrap();
  gfx::Shader fragment_shader =
      ctx.get_shader("KawaseBlur_DownSample:FS"_span).unwrap();

  gfx::PipelineRasterizationState raster_state{
      .depth_clamp_enable         = false,
      .polygon_mode               = gfx::PolygonMode::Fill,
      .cull_mode                  = gfx::CullMode::None,
      .front_face                 = gfx::FrontFace::CounterClockWise,
      .depth_bias_enable          = false,
      .depth_bias_constant_factor = 0,
      .depth_bias_clamp           = 0,
      .depth_bias_slope_factor    = 0};

  gfx::PipelineDepthStencilState depth_stencil_state{
      .depth_test_enable        = false,
      .depth_write_enable       = false,
      .depth_compare_op         = gfx::CompareOp::Greater,
      .depth_bounds_test_enable = false,
      .stencil_test_enable      = false,
      .front_stencil            = gfx::StencilOpState{},
      .back_stencil             = gfx::StencilOpState{},
      .min_depth_bounds         = 0,
      .max_depth_bounds         = 0};

  gfx::PipelineColorBlendAttachmentState attachment_states[] = {
      {.blend_enable           = false,
       .src_color_blend_factor = gfx::BlendFactor::SrcAlpha,
       .dst_color_blend_factor = gfx::BlendFactor::OneMinusSrcAlpha,
       .color_blend_op         = gfx::BlendOp::Add,
       .src_alpha_blend_factor = gfx::BlendFactor::One,
       .dst_alpha_blend_factor = gfx::BlendFactor::Zero,
       .alpha_blend_op         = gfx::BlendOp::Add,
       .color_write_mask       = gfx::ColorComponents::All}};

  gfx::PipelineColorBlendState color_blend_state{
      .attachments    = to_span(attachment_states),
      .blend_constant = {1, 1, 1, 1}};

  gfx::VertexAttribute vtx_attrs[] = {{.binding  = 0,
                                       .location = 0,
                                       .format   = gfx::Format::R32G32_SFLOAT,
                                       .offset   = 0}};

  gfx::VertexInputBinding vtx_bindings[] = {
      {.binding    = 0,
       .stride     = sizeof(Vec2),
       .input_rate = gfx::InputRate::Vertex}};

  default_logger->info(
      "refcount: ",
      ((vk::DescriptorSetLayout *) parameter_heap_.layout_)->refcount);

  gfx::DescriptorSetLayout set_layouts[] = {ctx.uniform_layout,
                                            parameter_heap_.layout_};

  gfx::GraphicsPipelineDesc pipeline_desc{
      .label = "KawaseBlur Graphics Pipeline",
      .vertex_shader =
          gfx::ShaderStageDesc{.shader                        = vertex_shader,
                               .entry_point                   = "main",
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .fragment_shader =
          gfx::ShaderStageDesc{.shader                        = fragment_shader,
                               .entry_point                   = "main",
                               .specialization_constants      = {},
                               .specialization_constants_data = {}},
      .render_pass            = render_pass_,
      .vertex_input_bindings  = to_span(vtx_bindings),
      .vertex_attributes      = to_span(vtx_attrs),
      .push_constant_size     = 0,
      .descriptor_set_layouts = to_span(set_layouts),
      .primitive_topology     = gfx::PrimitiveTopology::TriangleList,
      .rasterization_state    = raster_state,
      .depth_stencil_state    = depth_stencil_state,
      .color_blend_state      = color_blend_state,
      .cache                  = ctx.pipeline_cache};

  downsample_pipeline_ =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_desc)
          .unwrap();

  pipeline_desc.vertex_shader.shader =
      ctx.get_shader("KawaseBlur_UpSample:VS"_span).unwrap();
  pipeline_desc.fragment_shader.shader =
      ctx.get_shader("KawaseBlur_UpSample:FS"_span).unwrap();

  upsample_pipeline_ =
      ctx.device->create_graphics_pipeline(ctx.device.self, pipeline_desc)
          .unwrap();
}

void BlurPass::uninit(RenderContext &ctx)
{
}

void BlurPass::add_pass(RenderContext &ctx, BlurPassParams const &params)
{
  CHECK(params.extent.x <= ctx.scatch_framebuffer.color_image_desc.extent.x);
  CHECK(params.extent.y <= ctx.scatch_framebuffer.color_image_desc.extent.y);
  parameter_heap_.heap_->collect(parameter_heap_.heap_.self,
                                 ctx.frame_info.current);
  // TODO(lamarrr): we need to downsample multiple times, hence halfing the
  // extent every time we only need to sample to half the extent
  //
  // radius should have been scaled to src and target ratio
  Vec2 radius{1, 1};

  radius = radius * 2;

  Uniform uniform = ctx.push_uniform(BlurPassShaderUniform{
      .src_offset = Vec2{(f32) params.offset.x, (f32) params.offset.y},
      .src_extent = Vec2{(f32) params.extent.x, (f32) params.extent.y},
      .src_tex_extent =
          Vec2{(f32) params.view_extent.x, (f32) params.view_extent.y},
      .radius = Vec2{(f32) radius.x, (f32) radius.y}});

  /*gfx::DescriptorSet descriptor =
      parameter_heap_.create(BlurPassShaderParameter{
          .src = {{.image_view = params.view}},
          .dst = {{.image_view = ctx.scatch.color_image_view}}});

  gfx::CommandEncoderImpl encoder = ctx.encoder();

  encoder->bind_compute_pipeline(encoder.self, pipeline_);
  encoder->bind_descriptor_sets(encoder.self,
                                to_span({descriptor, uniform.set}),
                                to_span({uniform.buffer_offset}));
  encoder->dispatch(encoder.self, 0x00, 0x00, 1);

  parameter_heap_.release(descriptor);
  */
}

}        // namespace ash