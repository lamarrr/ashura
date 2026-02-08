/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct PBRPipelineParams
{
    Framebuffer             framebuffer;
    Option<PipelineStencil> stencil;
    RectU                   scissor;
    gpu::Viewport           viewport;
    gpu::PolygonMode        polygon_mode;
    gpu::DescriptorSet      samplers;
    gpu::DescriptorSet      textures;
    shader::PbrShaderParams params;
    u32                     num_indices;
    gpu::CullMode           cull_mode;
    gpu::FrontFace          front_face;
    PipelineVariantId       variant;
};

struct PBRPipeline final : IPipeline
{
    struct Pipeline
    {
        gpu::GraphicsPipeline fill  = nullptr;
        gpu::GraphicsPipeline line  = nullptr;
        gpu::GraphicsPipeline point = nullptr;
    };

    SparseVec<PipelineVariantId, Tuple<Str, Pipeline>> variants_;

    PBRPipeline(Allocator);

    PBRPipeline(PBRPipeline const &)             = delete;
    PBRPipeline(PBRPipeline &&)                  = delete;
    PBRPipeline & operator=(PBRPipeline const &) = delete;
    PBRPipeline & operator=(PBRPipeline &&)      = delete;

    virtual ~PBRPipeline() override = default;

    virtual Str label() override;

    virtual void acquire(GpuFramePlan plan, Allocator allocator) override;

    virtual void release(GpuFramePlan plan, Allocator allocator) override;

    PipelineVariantId add_variant(GpuFramePlan plan, Str label, gpu::Shader shader,
                                  Allocator allocator);

    void remove_variant(GpuFramePlan plan, PipelineVariantId id);

    PipelineVariantId get_variant_id(GpuFramePlan plan, Str label);

    void encode(gpu::CommandEncoder encoder, PBRPipelineParams const & params);
};

}    // namespace ash
