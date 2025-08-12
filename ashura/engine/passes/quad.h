/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/pass.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct QuadPassParams
{
  Framebuffer         framebuffer  = {};
  Option<PassStencil> stencil      = none;
  RectU               scissor      = {};
  gpu::Viewport       viewport     = {};
  gpu::DescriptorSet  samplers     = nullptr;
  gpu::DescriptorSet  textures     = nullptr;
  StructBufferSpan    world_to_ndc = {};
  StructBufferSpan    quads        = {};
  StructBufferSpan    transforms   = {};
  StructBufferSpan    materials    = {};
  Slice32             instances    = {};
};

struct QuadPass final : Pass
{
  SparseVec<Tuple<Str, gpu::GraphicsPipeline>> variants_;

  QuadPass(Allocator);

  QuadPass(QuadPass const &)             = delete;
  QuadPass(QuadPass &&)                  = default;
  QuadPass & operator=(QuadPass const &) = delete;
  QuadPass & operator=(QuadPass &&)      = default;

  virtual ~QuadPass() override = default;

  virtual Str label() override;

  virtual void acquire() override;

  virtual void release() override;

  ShaderVariantId add_variant(Str label, gpu::Shader shader);

  void remove_variant(ShaderVariantId id);

  ShaderVariantId get_variant_id(Str label);

  void encode(gpu::CommandEncoder & encoder, QuadPassParams const & params,
              ShaderVariantId variant);
};

}    // namespace ash
