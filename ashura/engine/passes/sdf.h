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
  Framebuffer         framebuffer  = {};
  Option<PassStencil> stencil      = none;
  RectU               scissor      = {};
  gpu::Viewport       viewport     = {};
  gpu::DescriptorSet  samplers     = nullptr;
  gpu::DescriptorSet  textures     = nullptr;
  StructBufferSpan    world_to_ndc = {};
  StructBufferSpan    shapes       = {};
  StructBufferSpan    transforms   = {};
  StructBufferSpan    materials    = {};
  Slice32             instances    = {};
};

struct SdfPass final : Pass
{
  SparseVec<Tuple<Str, gpu::GraphicsPipeline>> variants_;

  SdfPass(AllocatorRef);

  SdfPass(SdfPass const &)             = delete;
  SdfPass(SdfPass &&)                  = default;
  SdfPass & operator=(SdfPass const &) = delete;
  SdfPass & operator=(SdfPass &&)      = default;

  virtual ~SdfPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  ShaderVariantId add_variant(Str label, gpu::Shader shader);

  void remove_variant(ShaderVariantId id);

  ShaderVariantId get_variant_id(Str label);

  void encode(gpu::CommandEncoder & encoder, SdfPassParams const & params,
              ShaderVariantId variant);
};

}    // namespace ash
