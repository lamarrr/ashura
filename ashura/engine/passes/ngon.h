/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct NgonPassParams
{
  Framebuffer         framebuffer    = {};
  RectU               scissor        = {};
  gpu::Viewport       viewport       = {};
  gpu::DescriptorSet  samplers       = nullptr;
  gpu::DescriptorSet  textures       = nullptr;
  StructBufferSpan    world_to_ndc   = {};
  StructBufferSpan    transforms     = {};
  StructBufferSpan    vertices       = {};
  StructBufferSpan    indices        = {};
  StructBufferSpan    materials      = {};
  u32                 first_instance = 0;
  Span<u32 const>     index_counts   = {};
  Option<PassStencil> stencil        = none;
};

struct NgonPass : Pass
{
  gpu::GraphicsPipeline pipeline = nullptr;

  NgonPass() = default;

  virtual ~NgonPass() override = default;

  virtual Str label() override
  {
    return "Ngon"_str;
  }

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, NgonPassParams const & params);
};

}    // namespace ash
