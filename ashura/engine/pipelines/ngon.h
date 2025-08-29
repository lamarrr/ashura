/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct NgonPipelineParams
{
  Framebuffer             framebuffer    = {};
  Option<PipelineStencil> stencil        = none;
  RectU                   scissor        = {};
  gpu::Viewport           viewport       = {};
  gpu::DescriptorSet      samplers       = nullptr;
  gpu::DescriptorSet      textures       = nullptr;
  GpuBufferSpan           world_to_ndc   = {};
  GpuBufferSpan           transforms     = {};
  GpuBufferSpan           vertices       = {};
  GpuBufferSpan           indices        = {};
  GpuBufferSpan           materials      = {};
  u32                     first_instance = 0;
  Span<u32 const>         index_counts   = {};
};

struct NgonPipeline final : IPipeline
{
  SparseVec<Tuple<Str, gpu::GraphicsPipeline>> pipelines_;

  NgonPipeline(Allocator);

  NgonPipeline(NgonPipeline const &)             = delete;
  NgonPipeline(NgonPipeline &&)                  = delete;
  NgonPipeline & operator=(NgonPipeline const &) = delete;
  NgonPipeline & operator=(NgonPipeline &&)      = delete;

  virtual ~NgonPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  PipelineVariantId add_variant(GpuFramePlan plan, Str label,
                                gpu::Shader shader);

  void remove_variant(GpuFramePlan plan, PipelineVariantId id);

  PipelineVariantId get_variant_id(Str label);

  void encode(gpu::CommandEncoder encoder, NgonPipelineParams const & params,
              PipelineVariantId variant);
};

}    // namespace ash
