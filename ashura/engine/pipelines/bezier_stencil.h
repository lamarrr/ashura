/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BezierStencilPipelineParams
{
  DepthStencilImage stencil             = {};
  u32               write_mask          = 0;
  RectU             scissor             = {};
  gpu::Viewport     viewport            = {};
  FillRule          fill_rule           = FillRule::EvenOdd;
  bool              invert              = false;
  GpuBufferSpan     world_to_ndc        = {};
  GpuBufferSpan     transforms          = {};
  GpuBufferSpan     vertices            = {};
  GpuBufferSpan     indices             = {};
  GpuBufferSpan     regions             = {};
  Span<u32 const>   region_index_counts = {};
};

struct BezierStencilPipeline final : IPipeline
{
  gpu::GraphicsPipeline pipeline_ = nullptr;

  BezierStencilPipeline(Allocator);

  BezierStencilPipeline(BezierStencilPipeline const &)             = delete;
  BezierStencilPipeline(BezierStencilPipeline &&)                  = delete;
  BezierStencilPipeline & operator=(BezierStencilPipeline const &) = delete;
  BezierStencilPipeline & operator=(BezierStencilPipeline &&)      = delete;

  virtual ~BezierStencilPipeline() override = default;

  virtual Str label() override;

  virtual void acquire(GpuFramePlan plan) override;

  virtual void release(GpuFramePlan plan) override;

  void encode(gpu::CommandEncoder                 encoder,
              BezierStencilPipelineParams const & params);
};

}    // namespace ash
