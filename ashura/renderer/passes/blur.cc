#include "ashura/renderer/passes/blur.h"

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
  pipeline_ =
      ctx.device
          ->create_compute_pipeline(
              ctx.device.self,
              gfx::ComputePipelineDesc{
                  .label = "Kawase Blur Pipeline",
                  .compute_shader =
                      gfx::ShaderStageDesc{
                          .shader =
                              ctx.get_shader("KawaseBlur.CS"_span).unwrap(),
                          .entry_point                   = "cs_main",
                          .specialization_constants      = {},
                          .specialization_constants_data = {}},
                  .push_constant_size = 0,
                  .descriptor_set_layouts =
                      to_span({parameter_heap_.layout_, ctx.uniform_layout}),
                  .cache = ctx.pipeline_cache})
          .unwrap();
}

void BlurPass::uninit(RenderContext &ctx)
{
}

void BlurPass::add_pass(RenderContext &ctx, BlurPassParams const &params)
{
  ENSURE(params.extent.x <= ctx.scatch.color_image_desc.extent.x);
  ENSURE(params.extent.y <= ctx.scatch.color_image_desc.extent.y);
  parameter_heap_.heap_->collect(parameter_heap_.heap_.self,
                                 ctx.frame_info.current);
  // TODO(lamarrr): we need to downsample multiple times, hence halfing the
  // extent every time we only need to sample to half the extent
  Uniform uniform = ctx.push_uniform(BlurPassShaderUniform{
      .src_offset = Vec2I{(i32) params.offset.x, (i32) params.offset.y},
      .dst_offset = Vec2I{0, 0},
      .extent     = Vec2I{(i32) params.extent.x, (i32) params.extent.y},
      .radius     = Vec2I{(i32) params.radius.x, (i32) params.radius.y}});

  gfx::DescriptorSet descriptor =
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
}

}        // namespace ash