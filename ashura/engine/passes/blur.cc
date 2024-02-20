#include "ashura/engine/passes/blur.h"

namespace ash
{

void BlurPass::init(Pass self_, RenderServer *server, uid32 id)
{
  BlurPass              *self   = (BlurPass *) self_;
  gfx::DeviceImpl const &device = server->device;

  self->descriptor_set_layout =
      device
          ->create_descriptor_set_layout(
              device.self,
              gfx::DescriptorSetLayoutDesc{
                  .label    = "Gaussian Blur Descriptor Layout",
                  .bindings = to_span<gfx::DescriptorBindingDesc>(
                      {{.type = gfx::DescriptorType::StorageImage, .count = 1},
                       {.type = gfx::DescriptorType::StorageImage, .count = 1},
                       {.type  = gfx::DescriptorType::UniformBuffer,
                        .count = 1}})})
          .unwrap();

  self->pipeline =
      device
          ->create_compute_pipeline(
              device.self,
              gfx::ComputePipelineDesc{
                  .label = "Gaussian Blur",
                  .compute_shader =
                      gfx::ShaderStageDesc{
                          .entry_point = "cs_main",
                          .shader =
                              server->get_shader("GAUSSIAN_BLUR_SHADER"_span).unwrap(),
                          .specialization_constants      = {},
                          .specialization_constants_data = {}},
                  .push_constant_size     = gfx::MAX_PUSH_CONSTANT_SIZE,
                  .descriptor_set_layouts = to_span({self->descriptor_set_layout}),
                  .cache                  = server->pipeline_cache})
          .unwrap();
}

void BlurPass::deinit(Pass self, RenderServer *server)
{
}

void BlurPass::acquire_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void BlurPass::release_scene(Pass self, RenderServer *server, uid32 scene)
{
}

void BlurPass::acquire_view(Pass self, RenderServer *server, uid32 view)
{
}

void BlurPass::release_view(Pass self, RenderServer *server, uid32 view)
{
}

void BlurPass::release_object(Pass self, RenderServer *server, uid32 scene,
                              uid32 object)
{
}

void BlurPass::begin(Pass self, RenderServer *server, uid32 view,
                     gfx::CommandEncoderImpl const *encoder)
{
}

void BlurPass::encode(Pass self, RenderServer *server, uid32 view,
                      PassEncodeInfo const *info)
{
  gfx::CommandEncoderImpl enc = info->command_encoder;
  // enc->copy_image(enc.self, );
}

void BlurPass::end(Pass self, RenderServer *server, uid32 view,
                   gfx::CommandEncoderImpl const *encoder)
{
}

}        // namespace ash