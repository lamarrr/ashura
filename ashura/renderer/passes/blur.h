#pragma once
#include "ashura/renderer/render_context.h"

namespace ash
{

struct BlurParam
{
  Vec2 src_offset;
  Vec2 src_extent;
  Vec2 src_tex_extent;
  Vec2 radius;
};

struct BlurPassParams
{
  Vec2U          offset      = {};
  Vec2U          extent      = {};
  Vec2U          radius      = {0, 0};
  u32            num_levels  = 0;
  Vec2U          view_extent = {0, 0};
  gfx::ImageView view        = nullptr;
};

struct BlurPass
{
  gfx::GraphicsPipeline downsample_pipeline = nullptr;
  gfx::GraphicsPipeline upsample_pipeline   = nullptr;
  gfx::Sampler          sampler             = nullptr;
  gfx::RenderPass       render_pass         = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BlurPassParams const &params);
};

}        // namespace ash
