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
  struct State
  {
    FillRule       fill_rule  : 1;
    bool           invert     : 1;
    gpu::FrontFace front_face : 1;
    u32            write_mask;
    RectU          scissor;
    gpu::Viewport  viewport;
  };

  gpu::RenderingAttachment stencil_attachment;
  RectU                    render_area;
  GpuBufferSpan            world_to_ndc;
  GpuBufferSpan            items;
  GpuBufferSpan            vertices;
  GpuBufferSpan            indices;
  Span<u32 const>          index_runs;
  Span<State const>        states;
  Span<u32 const>          state_runs;
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
