/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct PBRPassParams
{
  Framebuffer         framebuffer = {};
  RectU               scissor     = {};
  gpu::Viewport       viewport    = {};
  bool                wireframe   = false;
  gpu::DescriptorSet  samplers    = nullptr;
  gpu::DescriptorSet  textures    = nullptr;
  StructBufferSpan    vertices    = {};
  StructBufferSpan    indices     = {};
  StructBufferSpan    world       = {};
  StructBufferSpan    material    = {};
  StructBufferSpan    lights      = {};
  u32                 num_indices = 0;
  Option<PassStencil> stencil     = none;
};

struct PBRPass final : Pass
{
  gpu::GraphicsPipeline pipeline           = nullptr;
  gpu::GraphicsPipeline wireframe_pipeline = nullptr;

  PBRPass() = default;

  virtual ~PBRPass() override = default;

  virtual Str label() override
  {
    return "PBR"_str;
  }

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, PBRPassParams const & params);
};

}    // namespace ash
