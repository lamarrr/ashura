/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct SdfPassParams
{
  Framebuffer         framebuffer    = {};
  RectU               scissor        = {};
  gpu::Viewport       viewport       = {};
  gpu::DescriptorSet  samplers       = nullptr;
  gpu::DescriptorSet  textures       = nullptr;
  StructBufferSpan    world_to_ndc   = {};
  StructBufferSpan    shapes         = {};
  StructBufferSpan    transforms     = {};
  StructBufferSpan    materials      = {};
  u32                 first_instance = 0;
  u32                 num_instances  = 0;
  Option<PassStencil> stencil        = none;
};

struct SdfPass final : Pass
{
  gpu::GraphicsPipeline pipeline = nullptr;
  StrDict<gpu::GraphicsPipeline> variants;

  virtual Str label() override
  {
    return "SDF"_str;
  }

  SdfPass() = default;

  virtual ~SdfPass() override = default;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, SdfPassParams const & params);
};

}    // namespace ash
