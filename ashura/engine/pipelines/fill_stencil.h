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
  GpuBufferSpan            world_transforms;
  GpuBufferSpan            vertices;
  GpuBufferSpan            indices;
  Span<u32 const>          index_runs;
  Span<State const>        states;
  Span<u32 const>          state_runs;
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
