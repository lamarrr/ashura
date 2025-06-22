/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/engine/shaders.gen.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct ContourStencilPassParams
{
  DepthStencilTexture stencil          = {};
  u32                 write_mask       = 0;
  RectU               scissor          = {};
  gpu::Viewport       viewport         = {};
  shader::FillRule    fill_rule        = shader::FillRule::EvenOdd;
  bool                invert           = false;
  StructBufferSpan    world_to_ndc     = {};
  StructBufferSpan    triangle_offsets = {};
  StructBufferSpan    transforms       = {};
  StructBufferSpan    vertices         = {};
  StructBufferSpan    regions          = {};
  Span<u32 const>     triangle_counts  = {};
};

struct ContourStencilPass final : Pass
{
  gpu::GraphicsPipeline pipeline_ = nullptr;

  ContourStencilPass(AllocatorRef);

  ContourStencilPass(ContourStencilPass const &)             = delete;
  ContourStencilPass(ContourStencilPass &&)                  = default;
  ContourStencilPass & operator=(ContourStencilPass const &) = delete;
  ContourStencilPass & operator=(ContourStencilPass &&)      = default;

  virtual ~ContourStencilPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder &            encoder,
              ContourStencilPassParams const & params);
};

}    // namespace ash
