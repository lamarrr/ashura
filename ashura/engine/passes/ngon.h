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
  Option<PassStencil> stencil        = none;
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
};

struct NgonPass final : Pass
{
  SparseVec<Tuple<Str, gpu::GraphicsPipeline>> pipelines_;

  NgonPass(Allocator);

  NgonPass(NgonPass const &)             = delete;
  NgonPass(NgonPass &&)                  = default;
  NgonPass & operator=(NgonPass const &) = delete;
  NgonPass & operator=(NgonPass &&)      = default;

  virtual ~NgonPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  ShaderVariantId add_variant(Str label, gpu::Shader shader);

  void remove_variant(ShaderVariantId id);

  ShaderVariantId get_variant_id(Str label);

  void encode(gpu::CommandEncoder & encoder, NgonPassParams const & params,
              ShaderVariantId variant);
};

}    // namespace ash
