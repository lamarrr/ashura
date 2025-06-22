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
  Framebuffer         framebuffer  = {};
  Option<PassStencil> stencil      = none;
  RectU               scissor      = {};
  gpu::Viewport       viewport     = {};
  gpu::PolygonMode    polygon_mode = gpu::PolygonMode::Fill;
  gpu::DescriptorSet  samplers     = nullptr;
  gpu::DescriptorSet  textures     = nullptr;
  StructBufferSpan    vertices     = {};
  StructBufferSpan    indices      = {};
  StructBufferSpan    world        = {};
  StructBufferSpan    material     = {};
  StructBufferSpan    lights       = {};
  u32                 num_indices  = 0;
};

struct PBRPass final : Pass
{
  struct Pipeline
  {
    gpu::GraphicsPipeline fill  = nullptr;
    gpu::GraphicsPipeline line  = nullptr;
    gpu::GraphicsPipeline point = nullptr;
  };

  Pipeline          pipeline_;
  StrDict<Pipeline> variants_;

  PBRPass(AllocatorRef);

  PBRPass(PBRPass const &)             = delete;
  PBRPass(PBRPass &&)                  = default;
  PBRPass & operator=(PBRPass const &) = delete;
  PBRPass & operator=(PBRPass &&)      = default;

  virtual ~PBRPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  void add_variant(Str label, gpu::Shader shader);

  void remove_variant(Str label);

  void encode(gpu::CommandEncoder & encoder, PBRPassParams const & params,
              Str variant = {});
};

}    // namespace ash
