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
  Framebuffer             framebuffer  = {};
  Option<PipelineStencil> stencil      = none;
  RectU                   scissor      = {};
  gpu::Viewport           viewport     = {};
  gpu::DescriptorSet      samplers     = nullptr;
  gpu::DescriptorSet      textures     = nullptr;
  GpuBufferSpan           world_to_ndc = {};
  GpuBufferSpan           shapes       = {};
  Slice32                 instances    = {};
};

struct SdfPipeline final : IPipeline
{
  static constexpr PipelineVariantId FLAT          = PipelineVariantId::Base;
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
