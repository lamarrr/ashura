/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct QuadPipelineParams
{
    struct State
    {
        Option<PipelineStencil> stencil;
        RectU                   scissor;
        gpu::Viewport           viewport;
    };

    Framebuffer              framebuffer;
    gpu::DescriptorSet       samplers;
    gpu::DescriptorSet       textures;
    shader::QuadShaderParams params;
    Span<State const>        states;
    Span<u32 const>          state_runs;
    PipelineVariantId        variant;
};

struct QuadPipeline final : IPipeline
{
    SparseVec<PipelineVariantId, Tuple<Str, gpu::GraphicsPipeline>> variants_;

    QuadPipeline(Allocator);

    QuadPipeline(QuadPipeline const &)             = delete;
    QuadPipeline(QuadPipeline &&)                  = delete;
    QuadPipeline & operator=(QuadPipeline const &) = delete;
    QuadPipeline & operator=(QuadPipeline &&)      = delete;

    virtual ~QuadPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    PipelineVariantId add_variant(GpuFramePlan plan, Str label, gpu::Shader shader,
                                  Allocator allocator);

    void remove_variant(GpuFramePlan plan, PipelineVariantId id);

    PipelineVariantId get_variant_id(Str label);

    void encode(gpu::CommandEncoder encoder, QuadPipelineParams const & params);
};

}    // namespace ash
