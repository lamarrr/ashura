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

struct BlurPassParams
{
  gfx::ImageView     image_view   = nullptr;
  Vec2U              extent       = {};
  gfx::DescriptorSet texture_view = nullptr;
  u32                texture      = 0;
  Vec2U              blur_offset  = {};
  Vec2U              blur_extent  = {};
  u32                blur_radius  = 0;
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
