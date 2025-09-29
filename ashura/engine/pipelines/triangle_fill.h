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
  Framebuffer             framebuffer  = {};
  Option<PipelineStencil> stencil      = none;
  RectU                   scissor      = {};
  gpu::Viewport           viewport     = {};
  gpu::CullMode           cull_mode    = gpu::CullMode::None;
  gpu::DescriptorSet      samplers     = nullptr;
  gpu::DescriptorSet      textures     = nullptr;
  GpuBufferSpan           world_to_ndc = {};
  GpuBufferSpan           sets         = {};
  GpuBufferSpan           vertices     = {};
  GpuBufferSpan           indices      = {};
  Span<u32 const>         index_counts = {};
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
