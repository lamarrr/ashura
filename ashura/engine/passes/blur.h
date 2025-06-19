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
  Framebuffer        framebuffer = {};
  RectU              area        = {};
  gpu::DescriptorSet samplers    = nullptr;
  gpu::DescriptorSet textures    = nullptr;
  StructBufferSpan   blur        = {};
};

struct BlurPass final : Pass
{
  struct Config
  {
    Vec2U spread_radius       = {};
    u32   major_spread_radius = {};
    Vec2U padding             = {};
    RectU padded_area         = {};
    u32   num_passes          = 0;
  };

  static constexpr u32 MAX_SPREAD_RADIUS = 16;
  static constexpr u32 MAX_PASSES        = 16;

  gpu::GraphicsPipeline downsample_pipeline_ = nullptr;

  gpu::GraphicsPipeline upsample_pipeline_ = nullptr;

  BlurPass(AllocatorRef);

  virtual ~BlurPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  Config config(BlurPassParams const & params);

  Option<ColorTextureResult> encode(gpu::CommandEncoder &  encoder,
                                    BlurPassParams const & params);
};

}    // namespace ash
