#pragma once
#include "ashura/engine/render_context.h"

namespace ash
{

struct BlurParam
{
  Vec2 uv[2]   = {};
  Vec2 radius  = {};
  u32  texture = 0;
};

// TODO(lamarrr): how to avoid using as render target and descriptor set element
// at the same time? difficult to know which is actually used and which isn't
struct BlurPassParams
{
  gfx::RenderingInfo rendering_info = {};
  // BlurParam          param          = {};
  // gfx::DescriptorSet textures   = nullptr;
  // u32                num_passes = 0;
};

struct BlurPass
{
  gfx::GraphicsPipeline downsample_pipeline = nullptr;
  gfx::GraphicsPipeline upsample_pipeline   = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BlurPassParams const &params);
};

}        // namespace ash
