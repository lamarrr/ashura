/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct TriangleFillPipelineParams
{
  struct State
  {
    gpu::CullMode           cull_mode  : 2;
    gpu::FrontFace          front_face : 1;
    RectU                   scissor;
    gpu::Viewport           viewport;
    Option<PipelineStencil> stencil;
  };

  Framebuffer        framebuffer;
  gpu::DescriptorSet samplers;
  gpu::DescriptorSet textures;
  GpuBufferSpan      world_to_ndc;
  GpuBufferSpan      sets;
  GpuBufferSpan      vertices;
  GpuBufferSpan      indices;
  Span<u32 const>    index_runs;
  Span<State const>  states;
  Span<u32 const>    state_runs;
};

struct TriangleFillPipeline final : IPipeline
{
  SparseVec<Tuple<Str, gpu::GraphicsPipeline>> pipelines_;

  TriangleFillPipeline(Allocator);

  TriangleFillPipeline(TriangleFillPipeline const &)             = delete;
  TriangleFillPipeline(TriangleFillPipeline &&)                  = delete;
  TriangleFillPipeline & operator=(TriangleFillPipeline const &) = delete;
  TriangleFillPipeline & operator=(TriangleFillPipeline &&)      = delete;

  virtual ~TriangleFillPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  PipelineVariantId add_variant(GpuFramePlan plan, Str label,
                                gpu::Shader shader);

  void remove_variant(GpuFramePlan plan, PipelineVariantId id);

  PipelineVariantId get_variant_id(Str label);

  void encode(gpu::CommandEncoder                encoder,
              TriangleFillPipelineParams const & params,
              PipelineVariantId                  variant);
};

}    // namespace ash
