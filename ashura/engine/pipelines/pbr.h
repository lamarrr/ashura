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
  GpuBufferSpan           vertices;
  GpuBufferSpan           indices;
  GpuBufferSpan           items;
  GpuBufferSpan           lights;
  u32                     num_indices;
  gpu::CullMode           cull_mode;
  gpu::FrontFace          front_face;
};

struct PBRPipeline final : IPipeline
{
  struct Pipeline
  {
    gpu::GraphicsPipeline fill  = nullptr;
    gpu::GraphicsPipeline line  = nullptr;
    gpu::GraphicsPipeline point = nullptr;
  };

  SparseVec<Tuple<Str, Pipeline>> variants_;

  PBRPipeline(Allocator);

  PBRPipeline(PBRPipeline const &)             = delete;
  PBRPipeline(PBRPipeline &&)                  = delete;
  PBRPipeline & operator=(PBRPipeline const &) = delete;
  PBRPipeline & operator=(PBRPipeline &&)      = delete;

  virtual ~PBRPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  PipelineVariantId add_variant(GpuFramePlan plan, Str label,
                                gpu::Shader shader);

  void remove_variant(GpuFramePlan plan, PipelineVariantId id);

  PipelineVariantId get_variant_id(GpuFramePlan plan, Str label);

  void encode(gpu::CommandEncoder encoder, PBRPipelineParams const & params,
              PipelineVariantId variant);
};

}    // namespace ash
