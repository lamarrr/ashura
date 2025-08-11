/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct FillStencilPassParams
{
  DepthStencilTexture stencil        = {};
  u32                 write_mask     = 0;
  RectU               scissor        = {};
  gpu::Viewport       viewport       = {};
  FillRule            fill_rule      = FillRule::EvenOdd;
  bool                invert         = false;
  StructBufferSpan    world_to_ndc   = {};
  StructBufferSpan    transforms     = {};
  StructBufferSpan    vertices       = {};
  StructBufferSpan    indices        = {};
  u32                 first_instance = 0;
  Span<u32 const>     index_counts   = {};
};

struct FillStencilPass final : Pass
{
  gpu::GraphicsPipeline pipeline_;

  FillStencilPass(Allocator);
  FillStencilPass(FillStencilPass const &)             = delete;
  FillStencilPass(FillStencilPass &&)                  = default;
  FillStencilPass & operator=(FillStencilPass const &) = delete;
  FillStencilPass & operator=(FillStencilPass &&)      = default;

  virtual ~FillStencilPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder &         encoder,
              FillStencilPassParams const & params);
};

}    // namespace ash
