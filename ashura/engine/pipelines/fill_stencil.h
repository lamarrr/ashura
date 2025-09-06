/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct FillStencilPipelineParams
{
  DepthStencilImage stencil          = {};
  RectU             scissor          = {};
  gpu::Viewport     viewport         = {};
  FillRule          fill_rule        = FillRule::EvenOdd;
  bool              invert           = false;
  GpuBufferSpan     world_to_ndc     = {};
  GpuBufferSpan     world_transforms = {};
  GpuBufferSpan     vertices         = {};
  GpuBufferSpan     indices          = {};
  Span<u32 const>   index_counts     = {};
  Span<u32 const>   write_masks      = {};
};

struct FillStencilPipeline final : IPipeline
{
  gpu::GraphicsPipeline pipeline_;

  FillStencilPipeline(Allocator);
  FillStencilPipeline(FillStencilPipeline const &)             = delete;
  FillStencilPipeline(FillStencilPipeline &&)                  = delete;
  FillStencilPipeline & operator=(FillStencilPipeline const &) = delete;
  FillStencilPipeline & operator=(FillStencilPipeline &&)      = delete;

  virtual ~FillStencilPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  void encode(gpu::CommandEncoder               encoder,
              FillStencilPipelineParams const & params);
};

}    // namespace ash
