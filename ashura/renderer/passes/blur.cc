#include "ashura/renderer/passes/blur.h"
#include "ashura/renderer/passes/gaussian_weights.h"

namespace ash
{

void BlurPass::init(Renderer &renderer)
{
  /*
    BlurPass              *self   = (BlurPass *) self_;
gfx::DeviceImpl const &device = server->device;

// https://www.khronos.org/opengl/wiki/Compute_Shader
//
https://web.engr.oregonstate.edu/~mjb/vulkan/Handouts/OpenglComputeShaders.1pp.pdf
//
https://github.com/lisyarus/compute/blob/master/blur/source/compute_separable_lds.cpp
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
*/
}

void BlurPass::uninit(Renderer &renderer)
{
}

void BlurPass::add_pass(Renderer &renderer, BlurParams const &params)
{
}

}        // namespace ash