/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/engine/shaders/shaders.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct ContourStencilPassParams
{
  DepthStencilTexture stencil        = {};
  u32                 write_mask     = 0;
  RectU               scissor        = {};
  gpu::Viewport       viewport       = {};
  StructBufferSpan    vertices       = {};
  StructBufferSpan    transforms     = {};
  StructBufferSpan    fills          = {};
  u32                 first_instance = 0;
  u32                 num_instances  = 0;
  shader::FillRule    fill_rule      = shader::FillRule::EvenOdd;
  bool                invert         = false;
};

struct ContourStencilPass final : Pass
{
  gpu::GraphicsPipeline pipeline_ = nullptr;

  ContourStencilPass(AllocatorRef);

  virtual ~ContourStencilPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder &            encoder,
              ContourStencilPassParams const & params);
};

}    // namespace ash
