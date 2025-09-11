/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pipeline.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BezierStencilPipelineParams
{
  DepthStencilImage stencil      = {};
  RectU             scissor      = {};
  gpu::Viewport     viewport     = {};
  FillRule          fill_rule    = FillRule::EvenOdd;
  bool              invert       = false;
  gpu::FrontFace    front_face   = gpu::FrontFace::CounterClockWise;
  GpuBufferSpan     world_to_ndc = {};
  GpuBufferSpan     items        = {};
  GpuBufferSpan     vertices     = {};
  GpuBufferSpan     indices      = {};
  Span<u32 const>   index_counts = {};
  Span<u32 const>   write_masks  = {};
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
