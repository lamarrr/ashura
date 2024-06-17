#pragma once
#include "ashura/engine/render_context.h"

namespace ash
{

/// @param transform needs to transform from [-1, +1] to clip space
struct NgonParam
{
  Mat4 transform    = {};
  Vec4 tint[4]      = {};
  Vec2 uv[2]        = {};
  f32  tiling       = 1;
  u32  albedo       = 0;
  u32  first_index  = 0;
  u32  first_vertex = 0;
};

struct NgonPassParams
{
  gfx::RenderingInfo rendering_info = {};
  gfx::Rect          scissor        = {};
  gfx::Viewport      viewport       = {};
  gfx::DescriptorSet vertices_ssbo  = nullptr;
  gfx::DescriptorSet indices_ssbo   = nullptr;
  gfx::DescriptorSet params_ssbo    = nullptr;
  gfx::SamplerDesc   sampler        = {};
  gfx::DescriptorSet textures       = nullptr;
  Span<u32 const>    index_counts   = {};
};

struct NgonPass
{
  gfx::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, NgonPassParams const &params);
};

}        // namespace ash
