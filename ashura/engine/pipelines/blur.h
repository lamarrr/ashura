/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BlurPipelineParams
{
  Framebuffer             framebuffer = {};
  Option<PipelineStencil> stencil     = none;
  RectU                   scissor     = {};
  gpu::Viewport           viewport    = {};
  gpu::DescriptorSet      samplers    = nullptr;
  gpu::DescriptorSet      textures    = nullptr;
  GpuBufferSpan           blurs       = {};
  Slice32                 instances   = {};
  bool                    upsample    = false;
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

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  void encode(gpu::CommandEncoder encoder, BlurPipelineParams const & params);
};

}    // namespace ash
