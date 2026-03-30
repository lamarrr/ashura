/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.hpp"
#include "ashura/engine/pipeline.hpp"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.hpp"

namespace ash
{

struct BlurPipelineParams
{
    Framebuffer              framebuffer;
    Option<PipelineStencil>  stencil;
    RectU                    scissor;
    gpu::Viewport            viewport;
    gpu::DescriptorSet       samplers;
    gpu::DescriptorSet       textures;
    shader::BlurShaderParams params;
    Slice32                  instances;
    bool                     upsample;
};

struct BlurPipeline final : IPipeline
{
    gpu::GraphicsPipeline downsample_pipeline_ = nullptr;

    gpu::GraphicsPipeline upsample_pipeline_ = nullptr;

    BlurPipeline(Allocator);

    BlurPipeline(BlurPipeline const &)             = delete;
    BlurPipeline(BlurPipeline &&)                  = delete;
    BlurPipeline & operator=(BlurPipeline const &) = delete;
    BlurPipeline & operator=(BlurPipeline &&)      = delete;

    virtual ~BlurPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    void encode(gpu::CommandEncoder encoder, BlurPipelineParams const & params);
};

}    // namespace ash
