/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct SdfPipelineParams
{
  struct State
  {
    Option<PipelineStencil> stencil;
    RectU                   scissor;
    gpu::Viewport           viewport;
  };

  Framebuffer        framebuffer;
  gpu::DescriptorSet samplers;
  gpu::DescriptorSet textures;
  GpuBufferSpan      world_to_ndc;
  GpuBufferSpan      items;
  Span<State const>  states;
  Span<u32 const>    state_runs;
};

struct SdfPipeline final : IPipeline
{
  static constexpr PipelineVariantId GRADIENT      = PipelineVariantId::Base;
  static constexpr PipelineVariantId NOISE         = PipelineVariantId{1};
  static constexpr PipelineVariantId MESH_GRADIENT = PipelineVariantId{2};

  SparseVec<Tuple<Str, gpu::GraphicsPipeline>> variants_;

  SdfPipeline(Allocator);

  SdfPipeline(SdfPipeline const &)             = delete;
  SdfPipeline(SdfPipeline &&)                  = delete;
  SdfPipeline & operator=(SdfPipeline const &) = delete;
  SdfPipeline & operator=(SdfPipeline &&)      = delete;

  virtual ~SdfPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  PipelineVariantId add_variant(GpuFramePlan plan, Str label,
                                gpu::Shader shader);

  void remove_variant(GpuFramePlan plan, PipelineVariantId id);

  PipelineVariantId get_variant_id(Str label);

  void encode(gpu::CommandEncoder encoder, SdfPipelineParams const & params,
              PipelineVariantId variant);
};

}    // namespace ash
