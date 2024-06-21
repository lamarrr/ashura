#pragma once
#include "ashura/engine/render_context.h"

namespace ash
{

enum class FillRule : u8
{
  EvenOdd = 0,
  NonZero = 1
};

/// @param transform needs to transform from [-1, +1] to clip space
struct PolyParam
{
  Mat4 transform    = {};
  Vec4 tint[4]      = {};
  Vec2 uv[2]        = {};
  f32  tiling       = 1;
  u32  sampler      = 0;
  u32  albedo       = 0;
  u32  first_index  = 0;
  u32  first_vertex = 0;
};

struct PolyPassParams
{
  gfx::RenderingInfo   rendering_info = {};
  gfx::Rect            scissor        = {};
  gfx::Viewport        viewport       = {};
  gfx::DescriptorSet   vertices_ssbo  = nullptr;
  gfx::DescriptorSet   indices_ssbo   = nullptr;
  gfx::DescriptorSet   params_ssbo    = nullptr;
  gfx::DescriptorSet   textures       = nullptr;
  Span<u32 const>      index_counts   = {};
  Span<FillRule const> fill_rules     = {};
};

struct PolyPass
{
  gfx::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, PolyPassParams const &params);
};

}        // namespace ash
