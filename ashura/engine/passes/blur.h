/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BlurPassParams
{
  Framebuffer         framebuffer = {};
  Option<PassStencil> stencil     = none;
  RectU               scissor     = {};
  gpu::Viewport       viewport    = {};
  gpu::DescriptorSet  samplers    = nullptr;
  gpu::DescriptorSet  textures    = nullptr;
  StructBufferSpan    blurs       = {};
  Slice32             instances   = {};
  bool                upsample    = false;
};

struct BlurPass final : Pass
{
  gpu::GraphicsPipeline downsample_pipeline_ = nullptr;

  gpu::GraphicsPipeline upsample_pipeline_ = nullptr;

  BlurPass(AllocatorRef);

  BlurPass(BlurPass const &)             = delete;
  BlurPass(BlurPass &&)                  = default;
  BlurPass & operator=(BlurPass const &) = delete;
  BlurPass & operator=(BlurPass &&)      = default;

  virtual ~BlurPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, BlurPassParams const & params);
};

}    // namespace ash
