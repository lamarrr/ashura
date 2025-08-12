/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BezierStencilPassParams
{
  DepthStencilTexture stencil             = {};
  u32                 write_mask          = 0;
  RectU               scissor             = {};
  gpu::Viewport       viewport            = {};
  FillRule            fill_rule           = FillRule::EvenOdd;
  bool                invert              = false;
  StructBufferSpan    world_to_ndc        = {};
  StructBufferSpan    transforms          = {};
  StructBufferSpan    vertices            = {};
  StructBufferSpan    indices             = {};
  StructBufferSpan    regions             = {};
  Span<u32 const>     region_index_counts = {};
};

struct BezierStencilPass final : Pass
{
  gpu::GraphicsPipeline pipeline_ = nullptr;

  BezierStencilPass(Allocator);

  BezierStencilPass(BezierStencilPass const &)             = delete;
  BezierStencilPass(BezierStencilPass &&)                  = default;
  BezierStencilPass & operator=(BezierStencilPass const &) = delete;
  BezierStencilPass & operator=(BezierStencilPass &&)      = default;

  virtual ~BezierStencilPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder &           encoder,
              BezierStencilPassParams const & params);
};

}    // namespace ash
