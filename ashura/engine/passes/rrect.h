#pragma once
#include "ashura/renderer/render_context.h"

namespace ash
{

/// @transform needs to transform from [-1, +1] to clip space
struct RRectParam
{
  Mat4 transform       = {};
  Vec4 radii           = {};
  Vec2 uv[2]           = {};
  Vec4 tint[4]         = {};
  Vec2 aspect_ratio    = {1, 1};
  f32  edge_smoothness = 0.0015F;
  u32  albedo          = 0;
};

struct RRectPassParams
{
  gfx::RenderingInfo rendering_info     = {};
  gfx::DescriptorSet params_ssbo        = nullptr;
  u32                params_ssbo_offset = 0;
  gfx::DescriptorSet textures           = nullptr;
  u32                first_instance     = 0;
  u32                num_instances      = 0;
};

struct RRectPass
{
  gfx::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, RRectPassParams const &params);
};

}        // namespace ash
