#include "ashura/engine/passes/blur.h"
#include "ashura/engine/passes/gaussian_weights.h"

namespace ash
{

void BlurPass::init(Pass self_, RenderServer *server, uid32 id)
{
  BlurPass              *self   = (BlurPass *) self_;
  gfx::DeviceImpl const &device = server->device;

  // https://www.khronos.org/opengl/wiki/Compute_Shader
  // https://web.engr.oregonstate.edu/~mjb/vulkan/Handouts/OpenglComputeShaders.1pp.pdf
  // https://github.com/lisyarus/compute/blob/master/blur/source/compute_separable_lds.cpp
  // https://lisyarus.github.io/blog/graphics/2022/04/21/compute-blur.html
  // https://www.youtube.com/watch?v=ml-5OGZC7vE
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
                          .shader =
                              server->get_shader("GAUSSIAN_BLUR_SHADER"_span)
                                  .unwrap(),
                          .entry_point                   = "cs_main",
                          .specialization_constants      = {},
                          .specialization_constants_data = {}},
                  .push_constant_size     = gfx::MAX_PUSH_CONSTANT_SIZE,
                  .descriptor_set_layouts = {&self->descriptor_set_layout, 1},
                  .cache                  = server->pipeline_cache})
          .unwrap();

  // kernels stored in single buffer
  // radius 2, 4, 8, 16
  // TODO(lamarrr): global kernel buffer
  //
  //
  //
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

void BlurPass::begin(Pass self, RenderServer *server, PassBeginInfo const *info)
{
}

void BlurPass::encode(Pass self, RenderServer *server,
                      PassEncodeInfo const *info)
{
}

void BlurPass::end(Pass self, RenderServer *server, PassEndInfo const *info)
{
}

}        // namespace ash