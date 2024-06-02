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
  u32  albedo       = 0;
  u32  first_vertex = 0;
};

struct NgonPassParams
{
  gfx::RenderingInfo rendering_info     = {};
  gfx::DescriptorSet params_ssbo        = nullptr;
  u32                params_ssbo_offset = 0;
  gfx::DescriptorSet textures           = nullptr;
  u32                first_instance     = 0;
  u32                num_instances      = 0;
};

struct NgonPass
{
  gfx::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, NgonPassParams const &params);
};

}        // namespace ash
