/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct VectorPathCoveragePipelineParams
{
  struct State
  {
    gpu::FrontFace front_face : 1;
    RectU          scissor;
    gpu::Viewport  viewport;
  };

  DepthStencilImage     stencil;
  shader::VectorPathCfg cfg;
  gpu::DescriptorSet    write_alpha_masks;
  gpu::DescriptorSet    write_fill_ids;
  GpuBufferSpan         world_to_ndc;
  GpuBufferSpan         vertices;
  GpuBufferSpan         indices;
  GpuBufferSpan         coverage_items;
  Span<u32 const>       index_runs;
  Span<State const>     states;
  Span<u32 const>       state_runs;
};

struct VectorPathFillPipelineParams
{
  using State = VectorPathCoveragePipelineParams::State;

  Framebuffer           framebuffer;
  shader::VectorPathCfg cfg;
  gpu::DescriptorSet    samplers;
  gpu::DescriptorSet    textures;
  gpu::DescriptorSet    read_alpha_masks;
  gpu::DescriptorSet    read_fill_ids;
  GpuBufferSpan         world_to_ndc;
  GpuBufferSpan         fill_items;
  Span<State const>     states;
  Span<u32 const>       state_runs;
  PipelineVariantId     variant;
};

struct VectorPathPipeline final : IPipeline
{
  gpu::GraphicsPipeline                        coverage_pipeline_;
  SparseVec<Tuple<Str, gpu::GraphicsPipeline>> fill_pipelines_;

  VectorPathPipeline(Allocator);

  VectorPathPipeline(VectorPathPipeline const &)             = delete;
  VectorPathPipeline(VectorPathPipeline &&)                  = delete;
  VectorPathPipeline & operator=(VectorPathPipeline const &) = delete;
  VectorPathPipeline & operator=(VectorPathPipeline &&)      = delete;

  virtual ~VectorPathPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  PipelineVariantId add_fill_variant(GpuFramePlan plan, Str label,
                                     gpu::Shader shader);

  void remove_fill_variant(GpuFramePlan plan, PipelineVariantId id);

  PipelineVariantId get_fill_variant_id(Str label);

  void encode(gpu::CommandEncoder                      encoder,
              VectorPathCoveragePipelineParams const & params);

  void encode(gpu::CommandEncoder                  encoder,
              VectorPathFillPipelineParams const & params);
};

}    // namespace ash
